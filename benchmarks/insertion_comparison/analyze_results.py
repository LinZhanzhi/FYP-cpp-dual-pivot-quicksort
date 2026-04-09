#!/usr/bin/env python3
"""
Analyze insertion sort variant comparison results (no external dependencies)
"""

import csv

# Read data
data = {}  # size -> {variant: time}
with open('insertion_comparison.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        size = int(row['size'])
        variant = row['variant']
        time = float(row['time_ns'])
        if size not in data:
            data[size] = {}
        data[size][variant] = time

sizes = sorted(data.keys())
variants = ['naive', 'prefetch', 'pin', 'pair', 'mixed']

print("=" * 80)
print("INSERTION SORT VARIANTS COMPARISON - ANALYSIS REPORT")
print("=" * 80)

# Table with speedup vs naive
print("\n## Raw Times (nanoseconds per sort)")
print(f"{'Size':>6}", end="")
for v in variants:
    print(f"{v:>12}", end="")
print()
print("-" * 66)
for size in sizes:
    print(f"{size:>6}", end="")
    for v in variants:
        print(f"{data[size][v]:>12.1f}", end="")
    print()

# Winner per size
print("\n\n## WINNER PER SIZE")
print("-" * 40)
for size in sizes:
    times = [(v, data[size][v]) for v in variants]
    winner = min(times, key=lambda x: x[1])
    runner_up = sorted(times, key=lambda x: x[1])[1]
    margin = (runner_up[1] / winner[1] - 1) * 100
    print(f"Size {size:3d}: {winner[0]:8s} ({winner[1]:6.1f} ns) - {margin:4.1f}% faster than {runner_up[0]}")

# Prefetch vs Mixed detailed comparison
print("\n\n## PREFETCH vs MIXED COMPARISON")
print("(This is the key comparison: simple insertion vs pin+pair)")
print("-" * 50)
print(f"{'Size':>6} {'Prefetch':>12} {'Mixed':>12} {'Speedup':>10} {'Winner':>10}")
print("-" * 50)
for size in sizes:
    pref = data[size]['prefetch']
    mix = data[size]['mixed']
    speedup = pref / mix
    winner = "MIXED" if speedup > 1 else "PREFETCH"
    print(f"{size:>6} {pref:>12.1f} {mix:>12.1f} {speedup:>10.2f}x {winner:>10}")

# Key insights
print("\n\n## KEY INSIGHTS")
print("=" * 60)

# 1. Does prefetch help?
print("\n### 1. Does Prefetch Help vs Naive?")
for size in sizes:
    naive = data[size]['naive']
    pref = data[size]['prefetch']
    ratio = pref / naive
    status = "HELPS" if ratio < 1 else "HURTS" if ratio > 1.05 else "NEUTRAL"
    print(f"  Size {size:3d}: {status:8s} (prefetch is {ratio:.2f}× of naive)")

# 2. When does pair beat prefetch?
print("\n### 2. When Does Pair Beat Prefetch?")
for size in sizes:
    pair = data[size]['pair']
    pref = data[size]['prefetch']
    speedup = pref / pair
    print(f"  Size {size:3d}: pair is {speedup:.2f}× {'faster' if speedup > 1 else 'slower'}")

# 3. Inner boundary analysis
print("\n### 3. Inner Boundary in Mixed Insertion")
print("Mixed insertion uses formula: end = high - 3 * ((size >> 5) << 3)")
print("This transitions from simple to pin+pair at size 32")
print()
for size in sizes:
    pin_pair_portion = 3 * ((size >> 5) << 3)
    simple_portion = size - pin_pair_portion
    print(f"  Size {size:3d}: simple handles first {simple_portion} elements, pin+pair handles last {pin_pair_portion}")

# 4. Average speedup
print("\n### 4. Average Speedup Analysis")
speedups_32plus = []
speedups_under32 = []
for size in sizes:
    speedup = data[size]['prefetch'] / data[size]['mixed']
    if size >= 32:
        speedups_32plus.append(speedup)
    else:
        speedups_under32.append(speedup)

print(f"  Average mixed/prefetch speedup (size >= 32): {sum(speedups_32plus)/len(speedups_32plus):.2f}×")
print(f"  Average mixed/prefetch speedup (size < 32):  {sum(speedups_under32)/len(speedups_under32):.2f}×")

# Recommendations
print("\n\n## RECOMMENDATIONS FOR OUTLINE UPDATE")
print("=" * 60)
print("""
Based on empirical data:

1. **Prefetch doesn't help for small arrays**: For sizes < 40, prefetch 
   actually HURTS performance due to overhead. The outline should NOT 
   claim "prefetch hides latency" for arrays under 40 elements.

2. **Pair insertion is highly effective**: Alone, pair insertion beats 
   all other strategies for many sizes. The "larger first" optimization 
   genuinely reduces shifts.

3. **Mixed (pin + pair) is optimal for 32+**: The inner boundary of 32 
   (inherited from Java) appears reasonable. Mixed insertion achieves 
   ~1.5-1.7× speedup over prefetch insertion for sizes 32-80.

4. **Inner boundary NOT tuned**: We inherited 32 from Java. Experiment 
   to find C++ optimal would be valuable. The formula:
   end = high - 3 * ((size >> 5) << 3)
   rounds down to multiples of 8, creating discontinuities.

5. **Size 40 anomaly**: Mixed shows unexpected regression at size 40.
   This may be a boundary condition in the formula worth investigating.
""")
