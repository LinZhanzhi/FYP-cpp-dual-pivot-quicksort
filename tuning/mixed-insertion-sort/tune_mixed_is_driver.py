import os
import subprocess
import re
import sys

# Paths
WORKSPACE = "/home/lzz725/FYP"
CONSTANTS_FILE = os.path.join(WORKSPACE, "include/dpqs/constants.hpp")
RESULTS_FILE = os.path.join(WORKSPACE, "benchmarks/results/tune_mixed_is_results.csv")
BENCH_SRC = os.path.join(WORKSPACE, "benchmarks/tune_mixed_is.cpp")
BENCH_BIN = os.path.join(WORKSPACE, "benchmarks/tune_mixed_is")

# Testing range: Baseline (48) up to higher values
# Since standard IS is 60, we expect mixed (faster) IS to be higher, e.g. 60-100.
VALUES = [48, 55, 60, 65, 70, 80, 96, 112]

def set_constant(val):
    with open(CONSTANTS_FILE, 'r') as f:
        content = f.read()

    # Regex to find: constexpr int MAX_MIXED_INSERTION_SORT_SIZE = \d+;
    pattern = r"constexpr int MAX_MIXED_INSERTION_SORT_SIZE = \d+;"
    replacement = f"constexpr int MAX_MIXED_INSERTION_SORT_SIZE = {val};"

    new_content = re.sub(pattern, replacement, content)

    with open(CONSTANTS_FILE, 'w') as f:
        f.write(new_content)

def compile_bench():
    cmd = f"g++ -std=c++20 -O3 -I {os.path.join(WORKSPACE, 'include')} {BENCH_SRC} -o {BENCH_BIN} -pthread"
    subprocess.run(cmd, shell=True, check=True)

def run_bench():
    result = subprocess.run([BENCH_BIN], capture_output=True, text=True)
    return result.stdout.strip()

def main():
    print(f"Starting Mixed IS tuning... Results will be in {RESULTS_FILE}")
    os.makedirs(os.path.dirname(RESULTS_FILE), exist_ok=True)

    with open(RESULTS_FILE, 'w') as f:
        f.write("Threshold,AvgRuntime(ms)\n")

    for val in VALUES:
        print(f"Testing MAX_MIXED_INSERTION_SORT_SIZE = {val}...", flush=True)
        set_constant(val)
        compile_bench()
        output = run_bench()
        print(f"  Result: {output} ms")
        with open(RESULTS_FILE, 'a') as f:
            f.write(f"{val},{output}\n")

    print("\nBenchmark complete.")
    # Reset to default 48 for safety (to be updated later manually)
    set_constant(48)

if __name__ == "__main__":
    main()