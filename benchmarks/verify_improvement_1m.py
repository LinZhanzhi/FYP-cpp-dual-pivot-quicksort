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

def run_benchmark(label, size):
    print(f"Running benchmark for {label} (Size={size})...")
    temp_csv = os.path.join(BENCHMARKS_DIR, "temp_verify.csv")
    cmd = [
        RUNNER,
        "--algorithm", "dual_pivot_sequential",
        "--type", "int",
        "--pattern", "RANDOM",
        "--size", str(size),
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
    SIZE = 1000000
    print(f"--- Performance Verification: Sequential Sort ({SIZE} Integers) ---")
    
    # 1. Test Old Constant (32)
    compile_runner(32)
    time_32 = run_benchmark("Constant=32", SIZE)
    
    # 2. Test Previous 1M Optimal (55)
    compile_runner(55)
    time_55 = run_benchmark("Constant=55", SIZE)

    # 3. Test New 10M Optimal (60)
    compile_runner(60)
    time_60 = run_benchmark("Constant=60", SIZE)

    # 3. Report
    print("\n--- Results (1M Integers) ---")
    print(f"Constant=32 (Baseline): {time_32:.4f} ms")
    print(f"Constant=55 (1M Opt)  : {time_55:.4f} ms")
    print(f"Constant=60 (10M Opt) : {time_60:.4f} ms")
    
    best_time = min(time_32, time_55, time_60)
    if best_time == time_55:
        print(f"Best: 55")
    elif best_time == time_60:
        print(f"Best: 60")
    else:
        print(f"Best: 32")

