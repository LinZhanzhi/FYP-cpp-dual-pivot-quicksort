import csv
import sys
import os

def analyze_benchmark(file_path):
    print(f"Analyzing {file_path}...")

    data = []
    try:
        with open(file_path, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                data.append(row)
    except FileNotFoundError:
        print(f"Error: File {file_path} not found.")
        return

    if not data:
        print("No data found.")
        return

    # 1. Determine Max Size
    sizes = set()
    for row in data:
        try:
            sizes.add(int(row['Size']))
        except ValueError:
            continue

    if not sizes:
        print("No valid sizes found.")
        return

    max_size = max(sizes)
    target_size = str(max_size)
    print(f"Target Size: {target_size}")

    # 2. Determine Data Type (prefer int, then double)
    types_in_file = set(row['Type'] for row in data)
    target_type = 'int'
    if 'int' not in types_in_file:
        if 'double' in types_in_file:
            target_type = 'double'
        else:
            target_type = list(types_in_file)[0]
    print(f"Target Type: {target_type}")

    # 3. Filter Data
    # Structure: filtered_data[pattern][algorithm] = time
    filtered_data = {}

    desired_patterns = [
        "RANDOM", "NEARLY_SORTED", "REVERSE_SORTED",
        "MANY_DUPLICATES_10", "MANY_DUPLICATES_50", "MANY_DUPLICATES_90",
        "ORGAN_PIPE", "SAWTOOTH"
    ]

    all_algorithms = set()

    for row in data:
        if row['Size'] != target_size:
            continue
        if row['Type'] != target_type:
            continue

        pat = row['Pattern']
        algo = row['Algorithm']
        time_val = float(row['Time(ms)'])

        if pat not in filtered_data:
            filtered_data[pat] = {}

        filtered_data[pat][algo] = time_val
        all_algorithms.add(algo)

    # 4. Identify Algorithms for comparison
    seq_algo = 'dual_pivot_sequential'
    par_4_algo = 'dual_pivot_parallel_4'
    par_8_algo = 'dual_pivot_parallel_8'
    par_16_algo = 'dual_pivot_parallel_16'
    par_24_algo = 'dual_pivot_parallel_24'
    std_algo = 'std_sort'

    print("-" * 140)
    print(f"{'Pattern':<20} | {'Seq':<8} | {'Par 4':<10} | {'Par 8':<10} | {'Par 16':<10} | {'Spd 16':<8} | {'Par 24':<10} | {'std::sort':<10}")
    print("-" * 140)

    for pat in desired_patterns:
        if pat not in filtered_data:
            print(f"{pat:<20} | {'NO DATA':<70}")
            continue

        times = filtered_data[pat]

        t_seq = times.get(seq_algo)
        t_p4 = times.get(par_4_algo)
        t_p8 = times.get(par_8_algo)
        t_p16 = times.get(par_16_algo)
        t_p24 = times.get(par_24_algo)
        t_std = times.get(std_algo)

        # Formatting values
        s_seq = f"{t_seq:.2f}" if t_seq is not None else "N/A"

        s_p4 = f"{t_p4:.2f}" if t_p4 is not None else "N/A"
        s_p8 = f"{t_p8:.2f}" if t_p8 is not None else "N/A"
        s_p16 = f"{t_p16:.2f}" if t_p16 is not None else "N/A"

        speedup_16 = f"{t_seq / t_p16:.2f}x" if (t_seq and t_p16) else "-"

        s_p24 = f"{t_p24:.2f}" if t_p24 is not None else "N/A"

        s_std = f"{t_std:.2f}" if t_std is not None else "N/A"

        print(f"{pat:<20} | {s_seq:<8} | {s_p4:<10} | {s_p8:<10} | {s_p16:<10} | {speedup_16:<8} | {s_p24:<10} | {s_std:<10}")

    print("-" * 140)
    print(f"Analyzed {len(desired_patterns)} patterns for Size={target_size}, Type={target_type}")

if __name__ == "__main__":
    default_path = "benchmarks/results/aggregate/summary_representative.csv"
    if len(sys.argv) > 1:
        path = sys.argv[1]
    else:
        path = default_path
        if not os.path.exists(path):
            if os.path.exists("benchmarks/results/aggregate/summary_representative.csv"):
                path = "benchmarks/results/aggregate/summary_representative.csv"

    analyze_benchmark(path)