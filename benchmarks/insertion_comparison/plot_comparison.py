#!/usr/bin/env python3
"""
Visualize insertion sort variant comparison results
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Read data
df = pd.read_csv('insertion_comparison.csv')

# Pivot for plotting
pivot = df.pivot(index='size', columns='variant', values='time_ns')

# Create figure with two subplots
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))

# Plot 1: All variants
colors = {
    'naive': '#888888',
    'prefetch': '#4CAF50',
    'pin': '#2196F3',
    'pair': '#FF9800',
    'mixed': '#E91E63'
}

for variant in ['naive', 'prefetch', 'pin', 'pair', 'mixed']:
    ax1.plot(pivot.index, pivot[variant], 
             marker='o', label=variant.capitalize(), 
             color=colors[variant], linewidth=2, markersize=5)

ax1.set_xlabel('Array Size', fontsize=12)
ax1.set_ylabel('Time per Sort (nanoseconds)', fontsize=12)
ax1.set_title('Insertion Sort Variants: Runtime Comparison', fontsize=14)
ax1.legend(title='Strategy', loc='upper left')
ax1.grid(True, alpha=0.3)
ax1.set_xlim(0, 85)

# Add vertical line at boundary 32
ax1.axvline(x=32, color='red', linestyle='--', alpha=0.5, label='Inner boundary (32)')
ax1.text(33, ax1.get_ylim()[1]*0.9, 'Inner\nboundary\n(32)', fontsize=9, color='red')

# Plot 2: Prefetch vs Mixed with speedup annotation
ax2.plot(pivot.index, pivot['prefetch'], marker='s', label='Simple (prefetch)', 
         color='#4CAF50', linewidth=2, markersize=6)
ax2.plot(pivot.index, pivot['mixed'], marker='o', label='Mixed (pin + pair)', 
         color='#E91E63', linewidth=2, markersize=6)

ax2.set_xlabel('Array Size', fontsize=12)
ax2.set_ylabel('Time per Sort (nanoseconds)', fontsize=12)
ax2.set_title('Prefetch vs Mixed Insertion: Key Comparison', fontsize=14)
ax2.legend(loc='upper left')
ax2.grid(True, alpha=0.3)
ax2.set_xlim(0, 85)

# Fill area where mixed wins
sizes = pivot.index.values
prefetch_times = pivot['prefetch'].values
mixed_times = pivot['mixed'].values
ax2.fill_between(sizes, prefetch_times, mixed_times, 
                 where=(prefetch_times > mixed_times),
                 color='green', alpha=0.1, label='Mixed faster')

# Add speedup annotations for key points
for size in [32, 48, 64]:
    if size in pivot.index:
        pref = pivot.loc[size, 'prefetch']
        mix = pivot.loc[size, 'mixed']
        speedup = pref / mix
        ax2.annotate(f'{speedup:.1f}×', 
                    xy=(size, (pref + mix) / 2),
                    fontsize=9, ha='center', color='darkgreen')

plt.tight_layout()
plt.savefig('insertion_comparison.png', dpi=150, bbox_inches='tight')
print("Saved: insertion_comparison.png")

# Print summary statistics
print("\n=== Summary Statistics ===")
print("\nSpeedup of Mixed over Prefetch:")
speedups = pivot['prefetch'] / pivot['mixed']
for size in pivot.index:
    print(f"  Size {size:2d}: {speedups[size]:.2f}×")

# Find crossover points
print("\n=== Crossover Analysis ===")
prefetch_vs_mixed = pivot['prefetch'].values / pivot['mixed'].values
print(f"Average speedup (size >= 32): {np.mean(prefetch_vs_mixed[pivot.index >= 32]):.2f}×")
print(f"Average speedup (size < 32): {np.mean(prefetch_vs_mixed[pivot.index < 32]):.2f}×")

# Winner per size
print("\n=== Winner per Size ===")
for size in pivot.index:
    row = pivot.loc[size]
    winner = row.idxmin()
    print(f"  Size {size:2d}: {winner:8s} ({row[winner]:.1f} ns)")

plt.show()
