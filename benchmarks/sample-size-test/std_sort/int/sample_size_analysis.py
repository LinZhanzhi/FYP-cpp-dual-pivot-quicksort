import subprocess
import os
import statistics
import csv
import time

# Configuration
# We use wsl to run the linux executable
RUNNER_CMD = ["wsl", "/home/lzz725/FYP/benchmarks/build/benchmark_runner"]

SIZES = [
    1000, 1259, 1585, 1995, 2512, 3162, 3981, 5012, 6310, 7943,
    10000, 12589, 15849, 19953, 25119, 31623, 39811, 50119, 63096, 79433,
    100000, 125893, 158489, 199526, 251189, 316228, 398107, 501187, 630957, 794328,
    1000000
]

SAMPLE_SIZES = [300, 200, 100, 80, 60, 40, 30, 20, 10]
ALGO = "std_sort"
TYPE = "int"
PATTERN = "RANDOM"
# Output file path inside WSL
TEMP_OUTPUT_WSL = "/home/lzz725/FYP/benchmarks/sample-size-test/std_sort/int/temp_result.csv"
# Output file path accessible from Windows
TEMP_OUTPUT_WIN = r"\\wsl$\Ubuntu\home\lzz725\FYP\benchmarks\sample-size-test\std_sort\int\temp_result.csv"

def run_benchmark(size, iterations):
    cmd = RUNNER_CMD + [
        "--algorithm", ALGO,
        "--type", TYPE,
        "--pattern", PATTERN,
        "--size", str(size),
        "--output", TEMP_OUTPUT_WSL,
        "--iterations", str(iterations)
    ]
    subprocess.run(cmd, check=True)

    runtimes = []
    try:
        with open(TEMP_OUTPUT_WIN, 'r') as f:
            lines = f.readlines()
            # Skip header
            for line in lines[1:]:
                parts = line.strip().split(',')
                if len(parts) >= 5:
                    runtimes.append(float(parts[-1]))
    except Exception as e:
        print(f"Error reading results: {e}")
        return []

    return runtimes

def analyze_samples(runtimes):
    if not runtimes:
        return 0, 0, 0, 0, 0

    mean = statistics.mean(runtimes)
    stdev = statistics.stdev(runtimes) if len(runtimes) > 1 else 0

    lower_bound = mean - 2 * stdev
    upper_bound = mean + 2 * stdev

    filtered = [x for x in runtimes if lower_bound <= x <= upper_bound]
    outliers = len(runtimes) - len(filtered)

    new_mean = statistics.mean(filtered) if filtered else 0
    new_stdev = statistics.stdev(filtered) if len(filtered) > 1 else 0

    return mean, stdev, outliers, new_mean, new_stdev

def main():
    results = []

    print(f"Starting analysis for {len(SIZES)} array sizes...")

    for size in SIZES:
        print(f"Processing size: {size}")

        # Collect 300 samples in one go
        runtimes = run_benchmark(size, 300)

        if len(runtimes) < 300:
            print(f"Warning: Only collected {len(runtimes)} samples for size {size}")

        # Analyze for each sample size
        for n in SAMPLE_SIZES:
            if n > len(runtimes):
                continue

            subset = runtimes[:n]
            mean, stdev, outliers, new_mean, new_stdev = analyze_samples(subset)

            results.append({
                "ArraySize": size,
                "SampleSize": n,
                "Mean": mean,
                "StdDev": stdev,
                "Outliers": outliers,
                "CleanedMean": new_mean,
                "CleanedStdDev": new_stdev
            })

    # Write results
    output_csv = r"\\wsl$\Ubuntu\home\lzz725\FYP\benchmarks\sample-size-test\std_sort\int\sample_size_analysis.csv"
    with open(output_csv, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["ArraySize", "SampleSize", "Mean", "StdDev", "Outliers", "CleanedMean", "CleanedStdDev"])
        writer.writeheader()
        writer.writerows(results)

    print(f"Analysis complete. Results saved to {output_csv}")

if __name__ == "__main__":
    main()
