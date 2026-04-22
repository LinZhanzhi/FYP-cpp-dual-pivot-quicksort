import subprocess
import os
import itertools
import sys
import multiprocessing
import glob
import statistics
import csv
from collections import defaultdict

# Get the directory where the script is located
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Configuration (Windows native paths)
BUILD_DIR = os.path.join(SCRIPT_DIR, "build")
RUNNER = os.path.join(BUILD_DIR, "benchmark_runner.exe")
AGGREGATE_DIR = os.path.join(SCRIPT_DIR, "results", "aggregate")
INDIVIDUAL_DIR = os.path.join(SCRIPT_DIR, "results", "individual")
SUMMARY_FULL = os.path.join(AGGREGATE_DIR, "summary_full.csv")
SUMMARY_REP = os.path.join(AGGREGATE_DIR, "summary_representative.csv")
TEMP_RESULT = os.path.join(SCRIPT_DIR, "temp_runner_output.csv")

# -----------------------------------------------------------------------------
# Benchmark methodology configuration
# -----------------------------------------------------------------------------
# Default (structured / deterministic) patterns: fixed seed, 30 iterations,
# representative = minimum (noise-free lower bound).
DEFAULT_TARGET_ITERATIONS = 30
DEFAULT_SEED = 42

# RANDOM pattern: industry-standard multi-seed methodology.
# N=10 seeds x 10 timed iterations per seed = 100 samples, preceded by
# 3 warmup iterations per seed. Representative = median of per-seed minima.
# Per-seed min removes OS/scheduler timing noise for a fixed permutation;
# outer median across seeds absorbs lucky/unlucky permutation variance.
RANDOM_SEEDS = 10
RANDOM_ITERS_PER_SEED = 10
RANDOM_TARGET_ITERATIONS = RANDOM_SEEDS * RANDOM_ITERS_PER_SEED  # 100

def target_iterations_for(pattern):
    """Return the target sample count for the given pattern."""
    return RANDOM_TARGET_ITERATIONS if pattern == "RANDOM" else DEFAULT_TARGET_ITERATIONS

def representative_for(pattern, samples):
    """Compute the representative runtime from a list of (time_ms, seed) tuples.

    - RANDOM   -> median of per-seed minima (robust to input variance).
    - Others   -> minimum (classical single-seed noise-floor estimator).
    """
    if not samples:
        return None
    if pattern == "RANDOM":
        by_seed = defaultdict(list)
        for t, s in samples:
            by_seed[s].append(t)
        per_seed_mins = sorted(min(ts) for ts in by_seed.values())
        return per_seed_mins[len(per_seed_mins) // 2]
    return min(t for t, _ in samples)

def get_output_filename(algo, type_, pattern, size):
    """Get the filename for individual result storage."""
    return os.path.join(INDIVIDUAL_DIR, f"{algo}_{type_}_{pattern}_{size}.csv")

# Generate parallel algorithms based on hardware threads
max_threads = multiprocessing.cpu_count()
parallel_algos = []
t = 2
while t <= max_threads:
    parallel_algos.append(f"dual_pivot_parallel_{t}")
    t *= 2

ALGORITHMS = parallel_algos + ["std_sort", "dual_pivot_sequential"]
TYPES = ["int", "int8_t", "int16_t", "double"]
PATTERNS = [
    "RANDOM", "NEARLY_SORTED", "REVERSE_SORTED",
    "MANY_DUPLICATES_10", "MANY_DUPLICATES_50", "MANY_DUPLICATES_90",
    "ORGAN_PIPE", "SAWTOOTH"
]
# total number of sizes = 41
SIZES = [
    1000, 1259, 1585, 1995, 2512, 3162, 3981, 5012, 6310, 7943,
    10000, 12589, 15849, 19953, 25119, 31623, 39811, 50119, 63096, 79433,
    100000, 125893, 158489, 199526, 251189, 316228, 398107, 501187, 630957, 794328,
    1000000, 1258925, 1584893, 1995262, 2511886, 3162277, 3981071, 5011872, 6309573, 7943282,
    10000000
]

class BenchmarkManager:
    def __init__(self):
        self.ensure_dirs()
        # Key: (algo, type, pattern, size) -> list of (time_ms, seed) tuples
        self.results_cache = defaultdict(list)
        self.rep_cache = {} # Key: (algo, type, pattern, size) -> rep_time
        self.load_history()

    def ensure_dirs(self):
        if not os.path.exists(AGGREGATE_DIR):
            os.makedirs(AGGREGATE_DIR)

    def load_history(self):
        # Load Full Summary. Seed column is optional (older rows default to DEFAULT_SEED).
        if os.path.exists(SUMMARY_FULL):
            with open(SUMMARY_FULL, 'r', encoding='utf-8-sig') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    try:
                        key = (row['Algorithm'], row['Type'], row['Pattern'], int(row['Size']))
                        time_ms = float(row['Time(ms)'])
                        seed_str = row.get('Seed')
                        try:
                            seed_val = int(seed_str) if seed_str not in (None, '') else DEFAULT_SEED
                        except ValueError:
                            seed_val = DEFAULT_SEED
                        self.results_cache[key].append((time_ms, seed_val))
                    except (ValueError, KeyError):
                        pass # Header or bad data
        else:
            # Create header (with Seed column)
            with open(SUMMARY_FULL, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(["Algorithm", "Type", "Pattern", "Size", "Iteration", "Time(ms)", "Seed"])

        # Load Representative
        if os.path.exists(SUMMARY_REP):
             with open(SUMMARY_REP, 'r', encoding='utf-8-sig') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    try:
                        key = (row['Algorithm'], row['Type'], row['Pattern'], int(row['Size']))
                        self.rep_cache[key] = float(row['Time(ms)'])
                    except (ValueError, KeyError): pass
        else:
             with open(SUMMARY_REP, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(["Algorithm", "Type", "Pattern", "Size", "Time(ms)"])

    def get_needed_iterations(self, algo, type_, pattern, size):
        target = target_iterations_for(pattern)
        existing = len(self.results_cache[(algo, type_, pattern, size)])
        return max(0, target - existing)

    def save_results(self, algo, type_, pattern, size, samples):
        """samples: iterable of (time_ms, seed) tuples."""
        samples = list(samples)
        if not samples:
            return

        # Append to Full Summary
        with open(SUMMARY_FULL, 'a', newline='') as f:
            writer = csv.writer(f)
            existing = len(self.results_cache[(algo, type_, pattern, size)])
            for i, (t, seed_val) in enumerate(samples):
                writer.writerow([algo, type_, pattern, size, existing + i + 1, t, seed_val])

        # Update Cache
        self.results_cache[(algo, type_, pattern, size)].extend(samples)

        # Update Representative (pattern-aware)
        rep_val = representative_for(pattern, self.results_cache[(algo, type_, pattern, size)])
        if rep_val is not None:
            self.rep_cache[(algo, type_, pattern, size)] = rep_val
        self.update_representative_file()

    def update_representative_file(self):
        # Rewrite the representative file
        sorted_keys = sorted(self.rep_cache.keys(), key=lambda k: (k[0], k[1], k[2], k[3]))

        with open(SUMMARY_REP, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(["Algorithm", "Type", "Pattern", "Size", "Time(ms)"])
            for key in sorted_keys:
                algo, type_, pattern, size = key
                writer.writerow([algo, type_, pattern, size, f"{self.rep_cache[key]:.5f}"])

    def delete_results(self, algo, type_, pattern, size):
        """Delete all results for a specific configuration from aggregate files."""
        key = (algo, type_, pattern, size)

        # Check if results exist
        if key not in self.results_cache or len(self.results_cache[key]) == 0:
            return False

        # Remove from cache
        del self.results_cache[key]
        if key in self.rep_cache:
            del self.rep_cache[key]

        # Rewrite summary_full.csv without the deleted entries
        if os.path.exists(SUMMARY_FULL):
            rows_to_keep = []
            with open(SUMMARY_FULL, 'r', newline='', encoding='utf-8-sig') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    if not (row['Algorithm'] == algo and
                            row['Type'] == type_ and
                            row['Pattern'] == pattern and
                            str(row['Size']) == str(size)):
                        rows_to_keep.append(row)

            with open(SUMMARY_FULL, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(["Algorithm", "Type", "Pattern", "Size", "Iteration", "Time(ms)", "Seed"])
                for row in rows_to_keep:
                    writer.writerow([row['Algorithm'], row['Type'], row['Pattern'],
                                   row['Size'], row['Iteration'], row['Time(ms)'],
                                   row.get('Seed') if row.get('Seed') not in (None, '') else DEFAULT_SEED])

        # Update representative file
        self.update_representative_file()
        return True

# Global manager instance (singleton pattern)
_manager_instance = None

def get_manager():
    """Get the global BenchmarkManager instance."""
    global _manager_instance
    if _manager_instance is None:
        _manager_instance = BenchmarkManager()
    return _manager_instance

def delete_results(algo, type_, pattern, size):
    """Delete results for a specific configuration."""
    return get_manager().delete_results(algo, type_, pattern, size)

def run_single_test(algo, type_, pattern, size):
    """Run a single benchmark test configuration."""
    manager = get_manager()

    threads = 0
    if algo.startswith("dual_pivot_parallel_"):
        try:
            threads = int(algo.split("_")[-1])
        except ValueError:
            pass

    # Build command. For RANDOM we always run the full multi-seed batch
    # (10 seeds x 10 iters); incremental top-up is not meaningful when the
    # representative is median-of-minima across a fixed seed set.
    cmd = [RUNNER,
        "--algorithm", algo,
        "--type", type_,
        "--pattern", pattern,
        "--size", str(size),
        "--output", TEMP_RESULT,
    ]

    if pattern == "RANDOM":
        cmd.extend([
            "--iterations", str(RANDOM_ITERS_PER_SEED),
            "--seeds", str(RANDOM_SEEDS),
            "--base-seed", str(DEFAULT_SEED),
        ])
    else:
        cmd.extend(["--iterations", str(DEFAULT_TARGET_ITERATIONS)])

    if threads > 0:
        cmd.extend(["--threads", str(threads)])

    # Run the benchmark
    subprocess.run(cmd, check=True)

    # Read back results
    if os.path.exists(TEMP_RESULT):
        new_samples = []
        with open(TEMP_RESULT, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                if row.get('Iteration') == 'Representative':
                    continue
                try:
                    t_val = float(row['Time(ms)'])
                    seed_str = row.get('Seed')
                    try:
                        seed_val = int(seed_str) if seed_str not in (None, '') else DEFAULT_SEED
                    except ValueError:
                        seed_val = DEFAULT_SEED
                    new_samples.append((t_val, seed_val))
                except ValueError:
                    pass

        # For RANDOM we replace any stale single-seed data so the
        # representative is computed over a clean multi-seed batch.
        if pattern == "RANDOM":
            manager.delete_results(algo, type_, pattern, size)

        # Save to aggregate
        manager.save_results(algo, type_, pattern, size, new_samples)

        # Clean up temp file
        os.remove(TEMP_RESULT)

def run_benchmark():
    manager = BenchmarkManager()

    combinations = list(itertools.product(ALGORITHMS, TYPES, PATTERNS, SIZES))
    total_configs = len(combinations)

    print(f"Plan: {total_configs} configurations locally.")

    for i, (algo, type_, pattern, size) in enumerate(combinations):
        needed = manager.get_needed_iterations(algo, type_, pattern, size)

        if needed == 0:
            # print(f"[{i+1}/{total_configs}] Skipping {algo} {type_} {pattern} {size} (Done)")
            # Only print every 100 or so if skipping, to avoid spam?
            # Or just print skipping.
            if i % 100 == 0:
                print(f"[{i+1}/{total_configs}] Skipping {algo} {type_} {pattern} {size}...")
            continue

        print(f"[{i+1}/{total_configs}] Running {algo} {type_} {pattern} {size} ({needed} iterations)...")

        threads = 0
        if algo.startswith("dual_pivot_parallel_"):
            try:
                threads = int(algo.split("_")[-1])
            except ValueError:
                pass

        # Prepare command for native execution. RANDOM uses the industry-
        # standard multi-seed methodology; other patterns top up to the
        # single-seed target incrementally.
        cmd = [RUNNER,
            "--algorithm", algo,
            "--type", type_,
            "--pattern", pattern,
            "--size", str(size),
            "--output", TEMP_RESULT,
        ]

        if pattern == "RANDOM":
            cmd.extend([
                "--iterations", str(RANDOM_ITERS_PER_SEED),
                "--seeds", str(RANDOM_SEEDS),
                "--base-seed", str(DEFAULT_SEED),
            ])
        else:
            cmd.extend(["--iterations", str(needed)])

        if threads > 0:
            cmd.extend(["--threads", str(threads)])

        try:
            # Run the benchmark
            subprocess.run(cmd, check=True)

            # Read back the results from the temp file
            if os.path.exists(TEMP_RESULT):
                new_samples = []
                with open(TEMP_RESULT, 'r') as f:
                    reader = csv.DictReader(f)
                    for row in reader:
                        # Ignore "Representative" line from C++ runner, we calculate our own
                        if row.get('Iteration') == 'Representative':
                            continue
                        try:
                            t_val = row.get('Time(ms)')
                            if t_val is None:
                                continue
                            time_ms = float(t_val)
                            seed_str = row.get('Seed')
                            try:
                                seed_val = int(seed_str) if seed_str not in (None, '') else DEFAULT_SEED
                            except ValueError:
                                seed_val = DEFAULT_SEED
                            new_samples.append((time_ms, seed_val))
                        except (ValueError, TypeError):
                            pass

                # RANDOM batches are atomic (10 seeds x 10 iters); replace any
                # stale single-seed data before saving the fresh batch.
                if pattern == "RANDOM":
                    manager.delete_results(algo, type_, pattern, size)

                # Save to aggregate
                manager.save_results(algo, type_, pattern, size, new_samples)

                # Clean up temp file
                os.remove(TEMP_RESULT)
            else:
                print(f"Error: Output file not created for {algo} {type_} {pattern} {size}")

        except subprocess.CalledProcessError as e:
            print(f"Error running benchmark: {e}")
            # Continue to next config

    print("Benchmark run complete.")


# Add new runner constant
RUNNER_COUNT = os.path.join(BUILD_DIR, "count_ops_runner.exe")
OPS_RESULT_FILE = os.path.join(AGGREGATE_DIR, "ops_counts.csv")

# ... (Previous code until run_benchmark)

def run_ops_counting():
    # Run for ALL algorithms (parallel included)
    algos = ALGORITHMS

    # All patterns
    patterns = PATTERNS # defined globally

    # All sizes
    sizes = SIZES

    # All types
    types = TYPES

    print(f"Running Operation Counting for {len(algos)} algos, {len(types)} types, {len(patterns)} patterns, {len(sizes)} sizes.")

    # Check if header exists in OPS_RESULT_FILE
    if not os.path.exists(OPS_RESULT_FILE):
        os.makedirs(os.path.dirname(OPS_RESULT_FILE), exist_ok=True)
        with open(OPS_RESULT_FILE, 'w') as f:
            f.write("Algorithm,Type,Pattern,Size,Comparisons,Swaps,Assignments\n")

    # Read first to avoid duplicates
    existing_keys = set()
    if os.path.exists(OPS_RESULT_FILE):
        with open(OPS_RESULT_FILE, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                existing_keys.add((row['Algorithm'], row['Type'], row['Pattern'], int(row['Size'])))

    for algo in algos:
        for type_ in types:
            for pattern in patterns:
                for size in sizes:
                    if (algo, type_, pattern, size) in existing_keys:
                        # Optional: Uncomment to see skips
                        # print(f"Skipping {algo} {type_} {pattern} {size} (Exists)")
                        continue

                    print(f"Counting {algo} {type_} {pattern} {size}...")

                    cmd = [RUNNER_COUNT,
                        "--size", str(size),
                        "--pattern", pattern,
                        "--algo", algo,
                        "--type", type_
                    ]

                    try:
                        res = subprocess.run(cmd, capture_output=True, text=True, check=True)
                        output = res.stdout.strip()
                        # Output is "comparisons,swaps,assignments"
                        if "," in output:
                            parts = output.split(',')
                            if len(parts) == 3:
                                comps, swaps, assigns = parts
                                with open(OPS_RESULT_FILE, 'a') as f:
                                    f.write(f"{algo},{type_},{pattern},{size},{comps},{swaps},{assigns}\n")
                            else:
                                 print(f"Error parsing output (parts): {output}")
                    except subprocess.CalledProcessError as e:
                        print(f"Error running {algo} {type_} {pattern} {size}: {e}")

if __name__ == "__main__":
    if "--count-ops" in sys.argv:
        run_ops_counting()
    else:
        # Default behavior: Run standard benchmarks
        run_benchmark()
