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

# Configuration
BUILD_DIR = os.path.join(SCRIPT_DIR, "build")
RUNNER = os.path.join(BUILD_DIR, "benchmark_runner")
AGGREGATE_DIR = os.path.join(SCRIPT_DIR, "results", "aggregate")
SUMMARY_FULL = os.path.join(AGGREGATE_DIR, "summary_full.csv")
SUMMARY_REP = os.path.join(AGGREGATE_DIR, "summary_representative.csv")

# WSL Paths (Hardcoded for this environment)
WSL_BASE_DIR = "/home/lzz725/FYP/benchmarks"
WSL_RUNNER = f"{WSL_BASE_DIR}/build/benchmark_runner"
WSL_TEMP_RESULT = f"{WSL_BASE_DIR}/temp_runner_output.csv"
TEMP_RESULT = os.path.join(SCRIPT_DIR, "temp_runner_output.csv")

# Generate parallel algorithms based on hardware threads
max_threads = multiprocessing.cpu_count()
parallel_algos = []
t = 2
while t <= max_threads:
    parallel_algos.append(f"dual_pivot_parallel_{t}")
    t *= 2

ALGORITHMS = parallel_algos + ["std_sort", "std_stable_sort", "qsort", "dual_pivot_sequential"]
TYPES = ["int", "double"]
PATTERNS = [
    "RANDOM", "NEARLY_SORTED", "REVERSE_SORTED",
    "MANY_DUPLICATES_10", "MANY_DUPLICATES_50", "MANY_DUPLICATES_90",
    "ORGAN_PIPE", "SAWTOOTH"
]
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
        self.results_cache = defaultdict(list) # Key: (algo, type, pattern, size) -> List of times
        self.rep_cache = {} # Key: (algo, type, pattern, size) -> rep_time
        self.load_history()

    def ensure_dirs(self):
        if not os.path.exists(AGGREGATE_DIR):
            os.makedirs(AGGREGATE_DIR)

    def load_history(self):
        # Load Full Summary
        if os.path.exists(SUMMARY_FULL):
            with open(SUMMARY_FULL, 'r') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    try:
                        key = (row['Algorithm'], row['Type'], row['Pattern'], int(row['Size']))
                        self.results_cache[key].append(float(row['Time(ms)']))
                    except (ValueError, KeyError):
                        pass # Header or bad data
        else:
            # Create header
            with open(SUMMARY_FULL, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(["Algorithm", "Type", "Pattern", "Size", "Iteration", "Time(ms)"])

        # Load Representative
        if os.path.exists(SUMMARY_REP):
             with open(SUMMARY_REP, 'r') as f:
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
        existing = len(self.results_cache[(algo, type_, pattern, size)])
        return max(0, 30 - existing)

    def save_results(self, algo, type_, pattern, size, times):
        if not times:
            return

        # Append to Full Summary
        with open(SUMMARY_FULL, 'a', newline='') as f:
            writer = csv.writer(f)
            # Find next iteration index based on cache
            existing = len(self.results_cache[(algo, type_, pattern, size)])
            for i, t in enumerate(times):
                writer.writerow([algo, type_, pattern, size, existing + i + 1, t])

        # Update Cache
        self.results_cache[(algo, type_, pattern, size)].extend(times)

        # Update Representative
        all_times = self.results_cache[(algo, type_, pattern, size)]
        rep_val = min(all_times)
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

        # Determine output file path
        # output_file_arg = WSL_TEMP_RESULT if sys.platform == "win32" else TEMP_RESULT

        # Prepare command
        cmd = []
        if sys.platform == "win32":
            cmd = ["wsl", WSL_RUNNER]
        else:
            cmd = [RUNNER]

        cmd.extend([
            "--algorithm", algo,
            "--type", type_,
            "--pattern", pattern,
            "--size", str(size),
            "--output", WSL_TEMP_RESULT if sys.platform == "win32" else TEMP_RESULT,
            "--iterations", str(needed)
        ])

        if threads > 0:
            cmd.extend(["--threads", str(threads)])

        try:
            # Run the benchmark
            subprocess.run(cmd, check=True)

            # Read back the results from the temp file
            if os.path.exists(TEMP_RESULT):
                new_times = []
                with open(TEMP_RESULT, 'r') as f:
                    reader = csv.DictReader(f)
                    for row in reader:
                        # Ignore "Representative" line from C++ runner, we calculate our own
                        if row.get('Iteration') == 'Representative':
                            continue
                        try:
                            t_val = float(row['Time(ms)'])
                            new_times.append(t_val)
                        except ValueError:
                            pass

                # Save to aggregate
                manager.save_results(algo, type_, pattern, size, new_times)

                # Clean up temp file
                os.remove(TEMP_RESULT)
            else:
                print(f"Error: Output file not created for {algo} {type_} {pattern} {size}")

        except subprocess.CalledProcessError as e:
            print(f"Error running benchmark: {e}")
            # Continue to next config

    print("Benchmark run complete.")


# ... (previous imports and constants)

# Add new runner constant
RUNNER_COUNT = os.path.join(BUILD_DIR, "count_ops_runner")
OPS_RESULT_FILE = os.path.join(AGGREGATE_DIR, "ops_counts.csv")

# ... (Previous code until run_benchmark)

def run_ops_counting():
    # Only meaningful for sequential algos
    algos = ["dual_pivot_sequential", "std_sort", "std_stable_sort"]

    # All patterns
    patterns = PATTERNS # defined globally

    # All sizes
    sizes = SIZES # defined globally

    print(f"Running Operation Counting for {len(algos)} algos, {len(patterns)} patterns, {len(sizes)} sizes.")

    # Check if header exists in OPS_RESULT_FILE
    if not os.path.exists(OPS_RESULT_FILE):
        os.makedirs(os.path.dirname(OPS_RESULT_FILE), exist_ok=True)
        with open(OPS_RESULT_FILE, 'w') as f:
            f.write("Algorithm,Pattern,Size,Comparisons,Swaps,Assignments\n")

    # Read first to avoid duplicates
    existing_keys = set()
    if os.path.exists(OPS_RESULT_FILE):
        with open(OPS_RESULT_FILE, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                existing_keys.add((row['Algorithm'], row['Pattern'], int(row['Size'])))

    for algo in algos:
        for pattern in patterns:
            for size in sizes:
                if (algo, pattern, size) in existing_keys:
                    # Optional: Uncomment to see skips
                    # print(f"Skipping {algo} {pattern} {size} (Exists)")
                    continue

                print(f"Counting {algo} {pattern} {size}...")

                cmd = []
                if sys.platform == "win32":
                     # Assuming typical WSL mapping if needed, or rely on caller
                     cmd = ["wsl", f"{WSL_BASE_DIR}/build/count_ops_runner"]
                else:
                    cmd = [RUNNER_COUNT]

                cmd.extend([
                    "--size", str(size),
                    "--pattern", pattern,
                    "--algo", algo
                ])

                try:
                    res = subprocess.run(cmd, capture_output=True, text=True, check=True)
                    output = res.stdout.strip()
                    # Output is "comparisons,swaps,assignments"
                    if "," in output:
                        parts = output.split(',')
                        if len(parts) == 3:
                            comps, swaps, assigns = parts
                            with open(OPS_RESULT_FILE, 'a') as f:
                                f.write(f"{algo},{pattern},{size},{comps},{swaps},{assigns}\n")
                        else:
                             # Fallback for old runners? No, we just updated it.
                             print(f"Error parsing output (parts): {output}")
                    print(f"Error running {algo} {pattern} {size}: {e}")

if __name__ == "__main__":
    if "--count-ops" in sys.argv:
        run_ops_counting()
    else:
        run_benchmark()
