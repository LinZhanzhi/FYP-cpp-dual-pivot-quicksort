import subprocess
import os
import itertools
import sys

# Get the directory where the script is located
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Configuration
BUILD_DIR = os.path.join(SCRIPT_DIR, "build")
RUNNER = os.path.join(BUILD_DIR, "benchmark_runner")
RESULTS_DIR = os.path.join(SCRIPT_DIR, "results", "raw")

# WSL Paths (Hardcoded for this environment)
WSL_BASE_DIR = "/home/lzz725/FYP/benchmarks"
WSL_RUNNER = f"{WSL_BASE_DIR}/build/benchmark_runner"
WSL_RESULTS_DIR = f"{WSL_BASE_DIR}/results/raw"

ALGORITHMS = ["std_sort", "std_stable_sort", "qsort", "dual_pivot_parallel", "dual_pivot_sequential"]
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
    1000000
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

    try:
        subprocess.run(cmd, check=True)
        return True, "Success"
    except subprocess.CalledProcessError as e:
        return False, str(e)

def run_benchmark():
    ensure_directories()

    # Cleanup old dual_pivot results to force rerun
    print("Cleaning up old dual_pivot results...")
    if os.path.exists(RESULTS_DIR):
        for f in os.listdir(RESULTS_DIR):
            if f.startswith("res_dual_pivot"):
                try:
                    os.remove(os.path.join(RESULTS_DIR, f))
                except OSError as e:
                    print(f"Error removing {f}: {e}")

    combinations = list(itertools.product(ALGORITHMS, TYPES, PATTERNS, SIZES))
    total = len(combinations)

    print(f"Found {total} benchmark combinations.")

    for i, (algo, type_, pattern, size) in enumerate(combinations):
        output_file = get_output_filename(algo, type_, pattern, size)

        if os.path.exists(output_file):
            print(f"[{i+1}/{total}] Skipping {algo} {type_} {pattern} {size} (Already exists)")
            continue

        print(f"[{i+1}/{total}] Running {algo} {type_} {pattern} {size}...")

        cmd = [
            RUNNER,
            "--algorithm", algo,
            "--type", type_,
            "--pattern", pattern,
            "--size", str(size),
            "--output", output_file,
            "--iterations", "30"
        ]

        try:
            subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as e:
            print(f"Error running benchmark: {e}")
            # Optional: stop or continue? Let's continue.

if __name__ == "__main__":
    run_benchmark()
