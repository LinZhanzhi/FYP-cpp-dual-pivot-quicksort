import os
import subprocess
import re

CONSTANTS_FILE = "../include/dpqs/constants.hpp"
BENCH_SRC = "tune_first_runs.cpp"
BENCH_EXE = "./tune_first_runs"

def update_constant(new_val):
    with open(CONSTANTS_FILE, 'r') as f:
        content = f.read()
    
    # Replace constexpr int MIN_FIRST_RUNS_FACTOR = 7;
    new_content = re.sub(
        r'constexpr int MIN_FIRST_RUNS_FACTOR = \d+;',
        f'constexpr int MIN_FIRST_RUNS_FACTOR = {new_val};',
        content
    )
    
    with open(CONSTANTS_FILE, 'w') as f:
        f.write(new_content)

def compile_bench():
    # Include -pthread because headers might use it, and O3 for optimization
    cmd = ["g++", "-O3", "-std=c++20", "-I../include", "-pthread", BENCH_SRC, "-o", "tune_first_runs"]
    subprocess.check_call(cmd)

def run_bench():
    output = subprocess.check_output([BENCH_EXE]).decode('utf-8')
    results = {}
    for line in output.strip().split('\n'):
        if ',' in line:
            parts = line.split(',')
            try:
                length = int(parts[0])
                time = int(parts[1])
                results[length] = time
            except ValueError:
                pass
    return results

def main():
    # Store original content to revert later
    with open(CONSTANTS_FILE, 'r') as f:
        original_content = f.read()

    try:
        # Factor 1: runs must be > 2 length to pass (effectively "Always Merge" for our tests)
        # Factor 30: runs must be > 2^30 length to pass ("Always QuickSort")
        experiments = {
            "ForceMerge": 1,   
            "ForceQS": 30,     
            "Current": 7
        }
        
        all_results = {}
        
        for name, factor in experiments.items():
            print(f"Running experiment: {name} (Factor={factor})")
            update_constant(factor)
            compile_bench()
            all_results[name] = run_bench()
            print(f"Results for {name}: {all_results[name]}")

        # Analysis
        print("\nAnalysis:")
        print("RunLen | ForceMerge | ForceQS | Current | BestAlgo")
        print("-------|------------|---------|---------|---------")
        
        merge_res = all_results["ForceMerge"]
        qs_res = all_results["ForceQS"]
        curr_res = all_results["Current"]
        
        sorted_lens = sorted(merge_res.keys())
        
        for length in sorted_lens:
            m_time = merge_res.get(length, -1)
            q_time = qs_res.get(length, -1)
            c_time = curr_res.get(length, -1)
            
            best = "Merge" if m_time <= q_time else "QS"
            
            print(f"{length:6d} | {m_time:10d} | {q_time:7d} | {c_time:7d} | {best}")

    finally:
        # Revert
        with open(CONSTANTS_FILE, 'w') as f:
            f.write(original_content)

if __name__ == "__main__":
    main()
