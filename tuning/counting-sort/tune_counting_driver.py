import os
import subprocess
import re
import sys

# Paths
WORKSPACE = "/home/lzz725/FYP"
CONSTANTS_FILE = os.path.join(WORKSPACE, "include/dpqs/constants.hpp")
RESULTS_FILE = os.path.join(WORKSPACE, "benchmarks/results/tune_counting_results.csv")
BENCH_SRC = os.path.join(WORKSPACE, "benchmarks/tune_counting.cpp")
BENCH_BIN = os.path.join(WORKSPACE, "benchmarks/tune_counting")

# Testing Values
BYTE_VALUES = [32, 48, 64, 80, 96, 128]
SHORT_VALUES = [500, 1000, 1500, 1750, 2000, 2500, 3000, 4000]

def set_constants(byte_val, short_val):
    with open(CONSTANTS_FILE, 'r') as f:
        content = f.read()

    # 1. Byte
    pat1 = r"constexpr int MIN_BYTE_COUNTING_SORT_SIZE = \d+;"
    rep1 = f"constexpr int MIN_BYTE_COUNTING_SORT_SIZE = {byte_val};"
    content = re.sub(pat1, rep1, content)

    # 2. Short
    pat2 = r"constexpr int MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE = \d+;"
    rep2 = f"constexpr int MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE = {short_val};"
    content = re.sub(pat2, rep2, content)

    with open(CONSTANTS_FILE, 'w') as f:
        f.write(content)

def compile_bench():
    cmd = f"g++ -std=c++20 -O3 -I {os.path.join(WORKSPACE, 'include')} {BENCH_SRC} -o {BENCH_BIN} -pthread"
    subprocess.run(cmd, shell=True, check=True)

def run_bench():
    result = subprocess.run([BENCH_BIN], capture_output=True, text=True)
    return result.stdout.strip()

def main():
    print(f"Starting Counting Sort tuning... Results in {RESULTS_FILE}")
    os.makedirs(os.path.dirname(RESULTS_FILE), exist_ok=True)

    with open(RESULTS_FILE, 'w') as f:
        f.write("Type,Threshold,Time(ms)\n")

    # Phase 2: Tune Short (Hold Byte constant at default 64)
    print("Phase 2: Tuning Short Threshold (Workload: Size 2000)")
    for val in SHORT_VALUES:
        set_constants(64, val) # Keep byte default
        compile_bench()
        out = run_bench().split(',')[1] # Get Short time
        print(f"  Short@{val}: {out} ms")
        with open(RESULTS_FILE, 'a') as f:
            f.write(f"Short,{val},{out}\n")

    print("\nBenchmark complete.")
    # Reset defaults
    set_constants(64, 1750)

if __name__ == "__main__":
    main()