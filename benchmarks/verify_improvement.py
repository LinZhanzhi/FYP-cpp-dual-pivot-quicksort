import subprocess
import os
import statistics
import time

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BENCHMARKS_DIR = SCRIPT_DIR
BUILD_DIR = os.path.join(BENCHMARKS_DIR, "build")
RUNNER = os.path.join(BUILD_DIR, "benchmark_runner")
INCLUDE_DIR = os.path.join(os.path.dirname(SCRIPT_DIR), "include")

def compile_runner(insertion_threshold):
    print(f"Compiling with MAX_INSERTION_SORT_SIZE={insertion_threshold}...")
    os.makedirs(BUILD_DIR, exist_ok=True)
    cmd = [
        "g++", "-std=c++20", "-O3",
        f"-DMAX_INSERTION_SORT_SIZE={insertion_threshold}",
        "-I", INCLUDE_DIR,
        "-I", os.path.join(BENCHMARKS_DIR, "include"),
        os.path.join(BENCHMARKS_DIR, "src", "benchmark_runner.cpp"),
        "-o", RUNNER,
        "-lpthread"
    ]
    subprocess.run(cmd, check=True)

def run_benchmark(label):
    print(f"Running benchmark for {label}...")
    temp_csv = os.path.join(BENCHMARKS_DIR, "temp_verify.csv")
    cmd = [
        RUNNER,
        "--algorithm", "dual_pivot_sequential",
        "--type", "int",
        "--pattern", "RANDOM",
        "--size", "10000000",
        "--iterations", "10",
        "--output", temp_csv
    ]

    start = time.time()
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

    avg_time = statistics.mean(times) if times else 0
    print(f"Average Time: {avg_time:.4f} ms")
    return avg_time

if __name__ == "__main__":
    print("--- Performance Verification: Sequential Sort (10M Integers) ---")

    # 1. Test Old Constant (32)
    compile_runner(32)
    time_32 = run_benchmark("Constant=32")

    # 2. Test Previous "Optimal" (55)
    compile_runner(55)
    time_55 = run_benchmark("Constant=55")

    # 3. Test New Candidate (60)
    compile_runner(60)
    time_60 = run_benchmark("Constant=60")

    # 3. Report
    print("\n--- Results ---")
    print(f"Constant=32: {time_32:.4f} ms")
    print(f"Constant=55: {time_55:.4f} ms")
    print(f"Constant=60: {time_60:.4f} ms")

    best = min(time_32, time_55, time_60)
    print(f"Best: {best:.4f} ms")

