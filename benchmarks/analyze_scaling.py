import csv

# Target parameters
target_pattern = "RANDOM"
target_size = "10000000"
target_type = "int"

results = {}

with open("benchmarks/results/aggregate/summary_representative.csv", "r") as f:
    reader = csv.DictReader(f)
    for row in reader:
        if row["Pattern"] == target_pattern and row["Size"] == target_size and row["Type"] == target_type:
            results[row["Algorithm"]] = float(row["Time(ms)"])

# Define sequence
algos = ["dual_pivot_sequential", "dual_pivot_parallel_2", "dual_pivot_parallel_4", "dual_pivot_parallel_8", "dual_pivot_parallel_16"]

print(f"| Threads | Time (ms) | Speedup vs Seq | Scaling Efficiency |")
print(f"|---------|-----------|----------------|--------------------|")

if "dual_pivot_sequential" not in results:
    print("Sequential baseline not found!")
    exit()

seq_time = results["dual_pivot_sequential"]

# Manual mapping for thread counts
thread_map = {
    "dual_pivot_sequential": 1,
    "dual_pivot_parallel_2": 2,
    "dual_pivot_parallel_4": 4,
    "dual_pivot_parallel_8": 8,
    "dual_pivot_parallel_16": 16
}

for algo in algos:
    if algo in results:
        t = results[algo]
        threads = thread_map[algo]
        speedup = seq_time / t
        efficiency = speedup / threads
        print(f"| {threads:<7} | {t:<9.2f} | {speedup:<14.2f} | {efficiency:<18.2f} |")
