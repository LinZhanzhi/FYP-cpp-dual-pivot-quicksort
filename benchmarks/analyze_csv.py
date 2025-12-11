import csv
import statistics

def main():
    data = {}
    with open(r"\\wsl$\Ubuntu\home\lzz725\FYP\benchmarks\sample_size_analysis.csv", "r") as f:
        reader = csv.DictReader(f)
        for row in reader:
            size = int(row["ArraySize"])
            n = int(row["SampleSize"])
            mean = float(row["CleanedMean"])

            if size not in data:
                data[size] = {}
            data[size][n] = mean

    # Calculate relative error compared to n=300
    sample_sizes = [200, 100, 80, 60, 40, 30, 20, 10]
    errors = {n: [] for n in sample_sizes}

    for size, runs in data.items():
        if 300 not in runs:
            continue
        base_mean = runs[300]
        if base_mean == 0:
            continue

        for n in sample_sizes:
            if n in runs:
                error = abs(runs[n] - base_mean) / base_mean * 100
                errors[n].append(error)

    print("Sample Size | Avg Relative Error (%) | Max Relative Error (%)")
    print("---|---|---")
    for n in sample_sizes:
        if errors[n]:
            avg_err = statistics.mean(errors[n])
            max_err = max(errors[n])
            print(f"{n} | {avg_err:.4f}% | {max_err:.4f}%")

if __name__ == "__main__":
    main()
