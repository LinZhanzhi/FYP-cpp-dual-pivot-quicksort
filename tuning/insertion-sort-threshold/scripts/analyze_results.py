import csv
from collections import defaultdict

results = defaultdict(list)

with open('results/benchmark_results.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        size = int(row['Size'])
        dist = row['Distribution']
        threshold = int(row['Threshold'])
        time = float(row['MedianTimeMicroseconds'])
        results[(size, dist)].append((threshold, time))

print(f"{'Size':<10} {'Dist':<10} {'Best Threshold':<15} {'Time':<10} {'Improvement vs T=0':<20}")
print("-" * 70)

for key in sorted(results.keys()):
    size, dist = key
    data = results[key]
    # Sort by time
    best = min(data, key=lambda x: x[1])

    # Find T=0 time
    t0 = next((x[1] for x in data if x[0] == 0), None)

    improvement = ""
    if t0:
        imp_pct = (t0 - best[1]) / t0 * 100
        improvement = f"{imp_pct:.1f}%"

    print(f"{size:<10} {dist:<10} {best[0]:<15} {best[1]:<10} {improvement:<20}")
