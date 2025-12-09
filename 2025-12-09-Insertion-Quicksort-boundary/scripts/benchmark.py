import subprocess
import csv
import os
import statistics
try:
    import matplotlib.pyplot as plt
    PLOT_AVAILABLE = True
except ImportError:
    PLOT_AVAILABLE = False
    print("Warning: matplotlib not found. Plotting will be skipped.")
from collections import defaultdict

# Configuration
SIZES = [1000, 4000, 16000, 64000, 256000]
THRESHOLDS = [0, 4, 8, 12, 16, 24, 32, 48, 64, 80, 100]
DISTRIBUTIONS = ["random", "sorted", "reverse", "few"]
RUNS = 20  # Number of runs per configuration
BUILD_DIR = "build"
BINARY_PATH = os.path.join(BUILD_DIR, "benchmark")
RESULTS_DIR = "results"
CSV_FILE = os.path.join(RESULTS_DIR, "benchmark_results.csv")

def compile_project():
    print("Compiling project...")
    if not os.path.exists(BUILD_DIR):
        os.makedirs(BUILD_DIR)

    # Compile directly with g++ since cmake/make might not be available
    cmd = ["g++", "-O3", "-march=native", "-std=c++17", "-I", "src", "src/main.cpp", "-o", BINARY_PATH]
    print(f"Running: {' '.join(cmd)}")
    subprocess.run(cmd, check=True)
    print("Compilation complete.")

def run_benchmark(size, threshold, dist):
    times = []
    for _ in range(RUNS):
        result = subprocess.run(
            [BINARY_PATH, str(size), str(threshold), dist],
            capture_output=True, text=True, check=True
        )
        times.append(int(result.stdout.strip()))
    return statistics.median(times)

def main():
    compile_project()

    if not os.path.exists(RESULTS_DIR):
        os.makedirs(RESULTS_DIR)

    results = []

    print(f"Starting benchmark with {RUNS} runs per configuration...")
    total_configs = len(SIZES) * len(THRESHOLDS) * len(DISTRIBUTIONS)
    current_config = 0

    with open(CSV_FILE, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["Size", "Threshold", "Distribution", "MedianTimeMicroseconds"])

        for size in SIZES:
            for dist in DISTRIBUTIONS:
                for threshold in THRESHOLDS:
                    current_config += 1
                    print(f"[{current_config}/{total_configs}] Size: {size}, Dist: {dist}, Threshold: {threshold}", end='\r')

                    median_time = run_benchmark(size, threshold, dist)
                    writer.writerow([size, threshold, dist, median_time])
                    results.append({
                        "size": size,
                        "threshold": threshold,
                        "dist": dist,
                        "time": median_time
                    })

    print("\nBenchmark complete. Results saved to", CSV_FILE)
    plot_results(results)

def plot_results(results):
    if not PLOT_AVAILABLE:
        print("Skipping plots because matplotlib is not installed.")
        return

    print("Plotting results...")
    # Organize data
    data = defaultdict(lambda: defaultdict(list))
    for r in results:
        data[(r["size"], r["dist"])]["thresholds"].append(r["threshold"])
        data[(r["size"], r["dist"])]["times"].append(r["time"])

    # Create plots
    for size in SIZES:
        fig, ax = plt.subplots(figsize=(10, 6))
        for dist in DISTRIBUTIONS:
            key = (size, dist)
            if key in data:
                ax.plot(data[key]["thresholds"], data[key]["times"], marker='o', label=dist)

        ax.set_title(f"Quicksort Threshold Performance (Size: {size})")
        ax.set_xlabel("Threshold")
        ax.set_ylabel("Time (microseconds)")
        ax.legend()
        ax.grid(True)

        plot_file = os.path.join(RESULTS_DIR, f"plot_size_{size}.png")
        plt.savefig(plot_file)
        print(f"Saved plot to {plot_file}")

if __name__ == "__main__":
    main()
