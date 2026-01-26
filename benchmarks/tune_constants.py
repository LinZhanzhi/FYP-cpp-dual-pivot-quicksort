import subprocess
import os
import sys
import json
import statistics
import time

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BENCHMARKS_DIR = SCRIPT_DIR
RESULTS_DIR = os.path.join(BENCHMARKS_DIR, "tuning_results")
BUILD_DIR = os.path.join(BENCHMARKS_DIR, "build")
RUNNER = os.path.join(BUILD_DIR, "benchmark_runner")
INCLUDE_DIR = os.path.join(os.path.dirname(SCRIPT_DIR), "include")

def compile_benchmark(definition):
    print(f"Compiling with {definition}...")
    try:
        os.makedirs(BUILD_DIR, exist_ok=True)
        # Clean build dir? No, just recompile runner
        # Need to compile runner with the definition

        cmd = [
            "g++", "-std=c++20", "-O3",
            f"-D{definition}",
            "-I", INCLUDE_DIR,
            "-I", os.path.join(BENCHMARKS_DIR, "include"),
            os.path.join(BENCHMARKS_DIR, "src", "benchmark_runner.cpp"),
            "-o", RUNNER,
            "-lpthread"
        ]

        subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Compilation failed: {e.stderr.decode()}")
        return False

def run_benchmark_runner(algo, size, iterations):
    cmd = [RUNNER]

    # Create a temp output file
    temp_csv = os.path.join(RESULTS_DIR, "temp_run.csv")

    cmd.extend([
        "--algorithm", algo,
        "--type", "int", # Use int as standard
        "--pattern", "RANDOM",
        "--size", str(size),
        "--iterations", str(iterations),
        "--output", temp_csv
    ])

    try:
        subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL)

        times = []
        if os.path.exists(temp_csv):
            import csv
            with open(temp_csv, "r") as f:
                reader = csv.DictReader(f)
                for row in reader:
                    if row.get("Iteration") != "Representative":
                         try:
                             times.append(float(row["Time(ms)"]))
                         except ValueError:
                             pass
            os.remove(temp_csv)
            return statistics.mean(times) if times else None
        return None
    except subprocess.CalledProcessError as e:
        print(f"Benchmark run failed: {e}")
        return None

def tune_constant(constant_name, start, end, step, algo, size, reps):
    json_path = os.path.join(RESULTS_DIR, f"{constant_name}.json")
    results = {}
    if os.path.exists(json_path):
        try:
            with open(json_path, 'r') as f:
                content = f.read().strip()
                if content:
                    results = json.loads(content)
        except json.JSONDecodeError:
            pass

    print(f"--- Tuning {constant_name} [{start}, {end}] step {step} ---")

    # Handle float/int steps logic manually
    values = []
    curr = start
    while curr <= end:
        values.append(curr)
        if isinstance(step, str) and step == "log":
             curr *= 2
        else:
             curr += step

    best_time = float("inf")
    best_val = -1

    # Check existing best
    for k, v in results.items():
        if v < best_time:
            best_time = v
            best_val = float(k) # or int

    for val in values:
        # If already in results, skip? No, maybe we want to rerun.
        # But to recover from crash, skip if we trust consistent env.
        # Let's simple overwrite for now to be sure, or maybe skip?
        # User wants to find best. Let's skip if present to save time.
        if str(val) in results:
            print(f"Val: {val} -> {results[str(val)]:.4f} ms (Cached)")
            if results[str(val)] < best_time:
                best_time = results[str(val)]
                best_val = val
            continue

        definition = f"{constant_name}={val}"
        if not compile_benchmark(definition):
            continue

        avg_time = run_benchmark_runner(algo, size, reps)

        if avg_time is not None:
            print(f"Val: {val} -> {avg_time:.4f} ms")
            results[str(val)] = avg_time
            if avg_time < best_time:
                best_time = avg_time
                best_val = val

            # Save immediately
            with open(json_path, "w") as f:
                json.dump(results, f, indent=4)
        else:
            print(f"Val: {val} -> Failed")

    # Final Save
    with open(json_path, "w") as f:
        json.dump(results, f, indent=4)

    print(f"Best {constant_name}: {best_val} ({best_time:.4f} ms)")
    return best_val, best_time

if __name__ == "__main__":
    os.makedirs(RESULTS_DIR, exist_ok=True)

    # 1. Base Case Tuning
    tune_constant("MAX_INSERTION_SORT_SIZE", 10, 80, 5, "dual_pivot_sequential", 1000000, 10)

    # 2. Parallel Threshold Tuning - Extended Range based on feedback
    # Previous range stopped at 65536 which was the best. We need to go higher to find the U-curve bottom.
    tune_constant("MIN_PARALLEL_SORT_SIZE", 32768, 1048576, "log", "dual_pivot_parallel_4", 10000000, 5)



