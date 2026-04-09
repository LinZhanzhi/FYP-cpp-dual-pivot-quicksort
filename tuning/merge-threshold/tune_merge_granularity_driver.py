import os
import subprocess
import re

CONSTANTS_FILE = "../include/dpqs/constants.hpp"
BENCH_SRC = "tune_merge_granularity.cpp"
BENCH_EXE = "./tune_merge_granularity"

def update_constant(new_val):
    with open(CONSTANTS_FILE, 'r') as f:
        content = f.read()

    new_content = re.sub(
        r'constexpr int MIN_PARALLEL_MERGE_PARTS_SIZE = \d+;',
        f'constexpr int MIN_PARALLEL_MERGE_PARTS_SIZE = {new_val};',
        content
    )

    with open(CONSTANTS_FILE, 'w') as f:
        f.write(new_content)

def compile_bench():
    # Include -pthread, -fopenmp (maybe needed if OMP used, but thread/task based), -ltbb (if tbb used)
    # The project seems to use standard threads or a task system.
    # Looking at makefiles or previous compile commands: "g++ -O3 -std=c++20 -I../include -pthread"
    cmd = ["g++", "-O3", "-std=c++20", "-I../include", "-pthread", BENCH_SRC, "-o", "tune_merge_granularity"]
    subprocess.check_call(cmd)

def run_bench():
    output = subprocess.check_output([BENCH_EXE]).decode('utf-8')
    try:
        return int(output.strip())
    except ValueError:
        return -1

def main():
    with open(CONSTANTS_FILE, 'r') as f:
        original_content = f.read()

    try:
        values = [128, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536]
        results = {}

        print(f"| Value | Time (ms) |")
        print(f"|-------|-----------|")

        for val in values:
            update_constant(val)
            compile_bench()
            time = run_bench()
            results[val] = time
            print(f"| {val:5d} | {time:9d} |")

        # Find best
        best_val = min(results, key=results.get)
        print(f"\nBest Value: {best_val} (Time: {results[best_val]} ms)")

    finally:
        with open(CONSTANTS_FILE, 'w') as f:
            f.write(original_content)

if __name__ == "__main__":
    main()
