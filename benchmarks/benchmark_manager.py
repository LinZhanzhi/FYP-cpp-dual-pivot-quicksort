import subprocess
import os
import itertools
import sys

# Configuration
BUILD_DIR = "build"
RUNNER = os.path.join(BUILD_DIR, "benchmark_runner")
RESULTS_DIR = "results/raw"

ALGORITHMS = ["std_sort", "dual_pivot", "std_stable_sort", "qsort"]
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

def ensure_directories():
    if not os.path.exists(RESULTS_DIR):
        os.makedirs(RESULTS_DIR)

def get_output_filename(algo, type_, pattern, size):
    return os.path.join(RESULTS_DIR, f"res_{algo}_{type_}_{pattern}_{size}.csv")

def run_single_test(algo, type_, pattern, size):
    ensure_directories()
    output_file = get_output_filename(algo, type_, pattern, size)

    cmd = [
        RUNNER,
        "--algorithm", algo,
        "--type", type_,
        "--pattern", pattern,
        "--size", str(size),
        "--output", output_file
    ]

    try:
        subprocess.run(cmd, check=True)
        return True, "Success"
    except subprocess.CalledProcessError as e:
        return False, str(e)

def run_benchmark():
    ensure_directories()

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
            "--output", output_file
        ]

        try:
            subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as e:
            print(f"Error running benchmark: {e}")
            # Optional: stop or continue? Let's continue.

if __name__ == "__main__":
    run_benchmark()
