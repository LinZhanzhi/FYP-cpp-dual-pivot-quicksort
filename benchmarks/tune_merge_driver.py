import os
import subprocess
import re
import sys

# Paths
WORKSPACE = "/home/lzz725/FYP"
CONSTANTS_FILE = os.path.join(WORKSPACE, "include/dpqs/constants.hpp")
RESULTS_FILE = os.path.join(WORKSPACE, "benchmarks/results/tune_merge_results.csv")
BENCH_SRC = os.path.join(WORKSPACE, "benchmarks/tune_merge.cpp")
BENCH_BIN = os.path.join(WORKSPACE, "benchmarks/tune_merge")

VALUES = [286, 350, 420, 512, 600, 1000]

def set_constant(val):
    with open(CONSTANTS_FILE, 'r') as f:
        content = f.read()

    # Regex to find: constexpr int MIN_TRY_MERGE_SIZE = \d+;
    pattern = r"constexpr int MIN_TRY_MERGE_SIZE = \d+;"
    replacement = f"constexpr int MIN_TRY_MERGE_SIZE = {val};"

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
    print(f"Starting tuning... Results will be in {RESULTS_FILE}")
    os.makedirs(os.path.dirname(RESULTS_FILE), exist_ok=True)

    with open(RESULTS_FILE, 'w') as f:
        f.write("Threshold,LargeRandom,Rnd100,Rnd200,Rnd300,Srt100,Srt200,Srt300,Srt500\n")

    for val in VALUES:
        print(f"Testing value: {val}...", flush=True)
        set_constant(val)
        compile_bench()
        output = run_bench()
        print(f"  Result: {output}")
        with open(RESULTS_FILE, 'a') as f:
            f.write(f"{val},{output}\n")

    print("\nBenchmark complete.")
    # Reset to default of 64 for safety
    set_constant(64)

if __name__ == "__main__":
    main()