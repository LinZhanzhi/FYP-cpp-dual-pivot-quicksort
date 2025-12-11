import csv
import os

# Paths
BASE_DIR = r"\\wsl$\Ubuntu\home\lzz725\FYP\benchmarks\sample-size-test\std_sort\int"
INPUT_FILE = os.path.join(BASE_DIR, "sample_size_analysis.csv")
STD_DEV_OUTPUT = os.path.join(BASE_DIR, "std_dev_table.csv")
OUTLIERS_OUTPUT = os.path.join(BASE_DIR, "outliers_table.csv")

def main():
    data = {}
    sample_sizes = set()

    # Read data
    try:
        with open(INPUT_FILE, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                size = int(row['ArraySize'])
                n = int(row['SampleSize'])
                std_dev = float(row['CleanedStdDev'])
                outliers = int(row['Outliers'])

                if size not in data:
                    data[size] = {}

                data[size][n] = {'std_dev': std_dev, 'outliers': outliers}
                sample_sizes.add(n)
    except FileNotFoundError:
        print(f"Error: Could not find input file at {INPUT_FILE}")
        return

    sorted_sample_sizes = sorted(list(sample_sizes), reverse=True) # 300, 200, ...
    sorted_array_sizes = sorted(list(data.keys()))

    # Select subset of array sizes for display (every 3rd one)
    display_sizes = sorted_array_sizes[::3]
    if sorted_array_sizes[-1] not in display_sizes:
        display_sizes.append(sorted_array_sizes[-1])

    # Generate Std Dev Table CSV
    with open(STD_DEV_OUTPUT, 'w', newline='') as f:
        writer = csv.writer(f)
        header = ['Array Size'] + [str(s) for s in sorted_sample_sizes]
        writer.writerow(header)

        for size in sorted_array_sizes: # Save ALL sizes to CSV
            row = [size]
            for n in sorted_sample_sizes:
                val = data[size].get(n, {}).get('std_dev', 0)
                row.append(f"{val:.6f}")
            writer.writerow(row)
    print(f"Saved standard deviation table to {STD_DEV_OUTPUT}")

    # Generate Outliers Table CSV
    with open(OUTLIERS_OUTPUT, 'w', newline='') as f:
        writer = csv.writer(f)
        header = ['Array Size'] + [str(s) for s in sorted_sample_sizes]
        writer.writerow(header)

        for size in sorted_array_sizes: # Save ALL sizes to CSV
            row = [size]
            for n in sorted_sample_sizes:
                val = data[size].get(n, {}).get('outliers', 0)
                row.append(str(val))
            writer.writerow(row)
    print(f"Saved outliers table to {OUTLIERS_OUTPUT}")

    # Print Markdown for report (Subset of sizes)
    print("\n### Standard Deviation (ms) vs Sample Size")
    print("| Array Size | " + " | ".join([str(s) for s in sorted_sample_sizes]) + " |")
    print("|---|" + "|".join(["---" for _ in sorted_sample_sizes]) + "|")
    for size in display_sizes:
        row = [f"{size:,}"]
        for n in sorted_sample_sizes:
            val = data[size].get(n, {}).get('std_dev', 0)
            row.append(f"{val:.3f}")
        print("| " + " | ".join(row) + " |")

    print("\n### Number of Outliers vs Sample Size")
    print("| Array Size | " + " | ".join([str(s) for s in sorted_sample_sizes]) + " |")
    print("|---|" + "|".join(["---" for _ in sorted_sample_sizes]) + "|")
    for size in display_sizes:
        row = [f"{size:,}"]
        for n in sorted_sample_sizes:
            val = data[size].get(n, {}).get('outliers', 0)
            row.append(str(val))
        print("| " + " | ".join(row) + " |")

if __name__ == "__main__":
    main()
