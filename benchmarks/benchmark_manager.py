import subprocess
import os
import itertools
import sys
import multiprocessing
import glob
import statistics
import csv

# Get the directory where the script is located
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Configuration
BUILD_DIR = os.path.join(SCRIPT_DIR, "build")
RUNNER = os.path.join(BUILD_DIR, "benchmark_runner")
RESULTS_DIR = os.path.join(SCRIPT_DIR, "results", "raw")
AGGREGATE_DIR = os.path.join(SCRIPT_DIR, "results", "aggregate")

# WSL Paths (Hardcoded for this environment)
WSL_BASE_DIR = "/home/lzz725/FYP/benchmarks"
WSL_RUNNER = f"{WSL_BASE_DIR}/build/benchmark_runner"
WSL_RESULTS_DIR = f"{WSL_BASE_DIR}/results/raw"

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

# Debug: Run only failing cases
# ALGORITHMS = ["dual_pivot_parallel"]
# TYPES = ["int", "double"]
# PATTERNS = ["RANDOM", "NEARLY_SORTED", "MANY_DUPLICATES_10", "MANY_DUPLICATES_50", "MANY_DUPLICATES_90"]
# SIZES = [7943, 50119, 79433, 125893, 251189, 316228, 398107, 794328, 1000000]

def ensure_directories():
    if not os.path.exists(RESULTS_DIR):
        os.makedirs(RESULTS_DIR)

def get_output_filename(algo, type_, pattern, size):
    return os.path.join(RESULTS_DIR, f"res_{algo}_{type_}_{pattern}_{size}.csv")

def run_single_test(algo, type_, pattern, size):
    ensure_directories()

    threads = 0
    if algo.startswith("dual_pivot_parallel_"):
        try:
            threads = int(algo.split("_")[-1])
        except ValueError:
            pass

    if sys.platform == "win32":
        # Use WSL paths and command
        output_file_wsl = f"{WSL_RESULTS_DIR}/res_{algo}_{type_}_{pattern}_{size}.csv"
        cmd = [
            "wsl",
            WSL_RUNNER,
            "--algorithm", algo,
            "--type", type_,
            "--pattern", pattern,
            "--size", str(size),
            "--output", output_file_wsl,
            "--iterations", "30"
        ]
        if threads > 0:
            cmd.extend(["--threads", str(threads)])
    else:
        # Native Linux/WSL execution
        output_file = get_output_filename(algo, type_, pattern, size)
        cmd = [
            RUNNER,
            "--algorithm", algo,
            "--type", type_,
            "--pattern", pattern,
            "--size", str(size),
            "--output", output_file,
            "--iterations", "30"
        ]
        if threads > 0:
            cmd.extend(["--threads", str(threads)])

    try:
        subprocess.run(cmd, check=True)
        return True, "Success"
    except subprocess.CalledProcessError as e:
        return False, str(e)

def aggregate_results():
    """
    Combines all individual CSV benchmark results into two summary files:
    1. summary_full.csv: Contains all 30 iterations for every test case.
    2. summary_representative.csv: Contains one line per test case using the median time.
    """
    print("Aggregating results...")

    if not os.path.exists(AGGREGATE_DIR):
        os.makedirs(AGGREGATE_DIR)

    full_summary_path = os.path.join(AGGREGATE_DIR, "summary_full.csv")
    rep_summary_path = os.path.join(AGGREGATE_DIR, "summary_representative.csv")

    # CSV Headers
    header_full = ["Algorithm", "Type", "Pattern", "Size", "Iteration", "Time(ms)"]
    # For representative, we drop Iteration and keep Time(ms) as the median
    header_rep = ["Algorithm", "Type", "Pattern", "Size", "Time(ms)"]

    csv_files = glob.glob(os.path.join(RESULTS_DIR, "*.csv"))
    csv_files.sort() # Ensure deterministic order

    print(f"Found {len(csv_files)} result files.")

    with open(full_summary_path, 'w', newline='') as f_full, \
         open(rep_summary_path, 'w', newline='') as f_rep:

        writer_full = csv.writer(f_full)
        writer_rep = csv.writer(f_rep)

        writer_full.writerow(header_full)
        writer_rep.writerow(header_rep)

        count = 0
        for file_path in csv_files:
            try:
                times = []
                algo = ""
                type_ = ""
                pattern = ""
                size = ""

                rep_val = None
                with open(file_path, 'r') as csv_in:
                    reader = csv.reader(csv_in)
                    headers = next(reader, None) # Skip header

                    if not headers:
                        continue

                    for row in reader:
                        if len(row) < 6: continue

                        # Capture metadata if not set
                        if not algo:
                            algo = row[0]
                            type_ = row[1]
                            pattern = row[2]
                            size = row[3]

                        # Check for Representative row
                        if row[4] == "Representative":
                            try:
                                rep_val = float(row[5])
                            except ValueError:
                                pass
                            continue

                        # Copy row to full summary (excluding Representative line)
                        writer_full.writerow(row)

                        try:
                            times.append(float(row[5]))
                        except ValueError:
                            pass

                if rep_val is not None:
                     writer_rep.writerow([algo, type_, pattern, size, f"{rep_val:.5f}"])
                     count += 1
                elif times:
                    min_time = min(times)
                    writer_rep.writerow([algo, type_, pattern, size, f"{min_time:.5f}"])
                    count += 1
            except Exception as e:
                print(f"Error processing {file_path}: {e}")

    print(f"Aggregation complete. Processed {count} files.")
    print(f"Full summary: {full_summary_path}")
    print(f"Representative summary: {rep_summary_path}")

def run_benchmark():
    ensure_directories()

    # Cleanup block removed to allow incremental benchmarking
    # Old behavior was to delete "res_dual_pivot*" files here.

    combinations = list(itertools.product(ALGORITHMS, TYPES, PATTERNS, SIZES))
    total = len(combinations)

    print(f"Found {total} benchmark combinations.")

    for i, (algo, type_, pattern, size) in enumerate(combinations):
        output_file = get_output_filename(algo, type_, pattern, size)

        if os.path.exists(output_file):
            print(f"[{i+1}/{total}] Skipping {algo} {type_} {pattern} {size} (Already exists)")
            continue

        print(f"[{i+1}/{total}] Running {algo} {type_} {pattern} {size}...")

        threads = 0
        if algo.startswith("dual_pivot_parallel_"):
            try:
                threads = int(algo.split("_")[-1])
            except ValueError:
                pass

        cmd = [
            RUNNER,
            "--algorithm", algo,
            "--type", type_,
            "--pattern", pattern,
            "--size", str(size),
            "--output", output_file,
            "--iterations", "30"
        ]
        if threads > 0:
            cmd.extend(["--threads", str(threads)])

        try:
            subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as e:
            print(f"Error running benchmark: {e}")
            # Optional: stop or continue? Let's continue.

    # Aggregate results after the run check
    aggregate_results()

if __name__ == "__main__":
    run_benchmark()
