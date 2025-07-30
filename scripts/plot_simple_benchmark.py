#!/usr/bin/env python3
"""
Simple Benchmark Results Plotter
Generates performance comparison plots from CSV benchmark data.
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
import os
from pathlib import Path

def load_benchmark_data(csv_file):
    """Load benchmark data from CSV file."""
    try:
        df = pd.read_csv(csv_file)
        return df
    except FileNotFoundError:
        print(f"Error: CSV file '{csv_file}' not found.")
        sys.exit(1)
    except Exception as e:
        print(f"Error reading CSV file: {e}")
        sys.exit(1)

def create_performance_plot(df, output_dir):
    """Create a performance comparison plot."""
    plt.figure(figsize=(12, 8))
    
    # Define colors for different algorithms
    colors = {
        'std::sort': '#1f77b4',
        'dual_pivot_quicksort': '#d62728'
    }
    
    # Get unique algorithms
    algorithms = df['Algorithm'].unique()
    
    # Plot each algorithm
    for algorithm in algorithms:
        alg_data = df[df['Algorithm'] == algorithm].sort_values('Size')
        
        plt.errorbar(alg_data['Size'], alg_data['Mean_ms'], 
                    yerr=alg_data['StdDev_ms'],
                    marker='o', label=algorithm, linewidth=2, markersize=6,
                    color=colors.get(algorithm, '#7f7f7f'),
                    capsize=5, capthick=2)
    
    # Customize the plot
    plt.xlabel('Array Size', fontsize=12)
    plt.ylabel('Time (milliseconds)', fontsize=12)
    plt.title('Dual-Pivot Quicksort vs std::sort Performance Comparison', fontsize=14, fontweight='bold')
    plt.legend(fontsize=12, loc='upper left')
    plt.grid(True, alpha=0.3)
    
    # Use log scale for better visualization
    plt.xscale('log')
    plt.yscale('log')
    
    # Improve layout
    plt.tight_layout()
    
    # Save the plot
    output_file = output_dir / "performance_comparison.png"
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    plt.close()
    
    print(f"Generated performance plot: {output_file}")

def create_speedup_plot(df, output_dir):
    """Create a speedup analysis plot."""
    plt.figure(figsize=(12, 8))
    
    # Get std::sort baseline data
    std_data = df[df['Algorithm'] == 'std::sort'][['Size', 'Mean_ms']].rename(columns={'Mean_ms': 'std_time'})
    dual_data = df[df['Algorithm'] == 'dual_pivot_quicksort'][['Size', 'Mean_ms']].rename(columns={'Mean_ms': 'dual_time'})
    
    # Merge data on size
    merged = pd.merge(std_data, dual_data, on='Size')
    
    # Calculate speedup
    merged['speedup'] = merged['std_time'] / merged['dual_time']
    
    # Plot speedup
    plt.plot(merged['Size'], merged['speedup'], 
            marker='o', linewidth=3, markersize=8, color='#2ca02c',
            label='Dual-Pivot vs std::sort')
    
    # Add baseline
    plt.axhline(y=1.0, color='black', linestyle='--', alpha=0.7, linewidth=2, label='std::sort baseline')
    
    # Customize the plot
    plt.xlabel('Array Size', fontsize=12)
    plt.ylabel('Speedup Factor', fontsize=12)
    plt.title('Dual-Pivot Quicksort Speedup vs std::sort', fontsize=14, fontweight='bold')
    plt.legend(fontsize=12)
    plt.grid(True, alpha=0.3)
    
    # Use log scale for x-axis
    plt.xscale('log')
    
    # Add speedup values as text annotations
    for _, row in merged.iterrows():
        plt.annotate(f'{row["speedup"]:.2f}x', 
                    (row['Size'], row['speedup']), 
                    textcoords="offset points", 
                    xytext=(0,10), 
                    ha='center', fontsize=10, fontweight='bold')
    
    # Improve layout
    plt.tight_layout()
    
    # Save the plot
    output_file = output_dir / "speedup_analysis.png"
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    plt.close()
    
    print(f"Generated speedup plot: {output_file}")

def create_summary_table(df, output_dir):
    """Create a summary statistics table."""
    print("\n" + "="*60)
    print("BENCHMARK SUMMARY")
    print("="*60)
    
    # Get std::sort baseline data
    std_data = df[df['Algorithm'] == 'std::sort'][['Size', 'Mean_ms']].rename(columns={'Mean_ms': 'std_time'})
    dual_data = df[df['Algorithm'] == 'dual_pivot_quicksort'][['Size', 'Mean_ms']].rename(columns={'Mean_ms': 'dual_time'})
    
    # Merge data on size
    merged = pd.merge(std_data, dual_data, on='Size')
    merged['speedup'] = merged['std_time'] / merged['dual_time']
    merged['improvement'] = (merged['speedup'] - 1) * 100
    
    print(f"{'Size':<10} {'std::sort (ms)':<15} {'dual_pivot (ms)':<15} {'Speedup':<10} {'Improvement':<12}")
    print("-" * 60)
    
    for _, row in merged.iterrows():
        print(f"{row['Size']:<10} {row['std_time']:<15.3f} {row['dual_time']:<15.3f} "
              f"{row['speedup']:<10.2f} {row['improvement']:<12.1f}%")
    
    avg_speedup = merged['speedup'].mean()
    avg_improvement = merged['improvement'].mean()
    
    print("-" * 60)
    print(f"Average speedup: {avg_speedup:.2f}x ({avg_improvement:.1f}% improvement)")
    print("="*60)

def main():
    """Main function to generate all plots."""
    if len(sys.argv) != 2:
        print("Usage: python plot_simple_benchmark.py <csv_file>")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    
    # Load data
    print(f"Loading benchmark data from {csv_file}")
    df = load_benchmark_data(csv_file)
    
    # Create output directory for plots
    output_dir = Path(csv_file).parent / "plots"
    output_dir.mkdir(exist_ok=True)
    
    print(f"Generating plots in {output_dir}")
    
    # Generate plots
    create_performance_plot(df, output_dir)
    create_speedup_plot(df, output_dir)
    
    # Print summary
    create_summary_table(df, output_dir)
    
    print(f"\nAll plots generated successfully in {output_dir}")
    print("Generated plot files:")
    print("- performance_comparison.png")
    print("- speedup_analysis.png")

if __name__ == "__main__":
    main()