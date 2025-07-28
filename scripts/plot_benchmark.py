#!/usr/bin/env python3
"""
Benchmark Results Plotter
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

def get_unique_patterns(df):
    """Get all unique data patterns from the benchmark results."""
    return df['Pattern'].unique()

def get_unique_algorithms(df):
    """Get all unique algorithms from the benchmark results."""
    return df['Algorithm'].unique()

def create_time_size_plot(df, pattern, output_dir):
    """Create a time vs size plot for a specific data pattern."""
    # Filter data for the specific pattern
    pattern_data = df[df['Pattern'] == pattern]
    
    # Create figure and axis
    plt.figure(figsize=(12, 8))
    
    # Define colors for different algorithms
    colors = {
        'std::sort': '#1f77b4',
        'std::stable_sort': '#ff7f0e', 
        'classic_quicksort': '#2ca02c',
        'dual_pivot_quicksort': '#d62728',
        'dual_pivot_optimized': '#9467bd'
    }
    
    # Plot each algorithm
    algorithms = get_unique_algorithms(pattern_data)
    for algorithm in algorithms:
        alg_data = pattern_data[pattern_data['Algorithm'] == algorithm]
        if not alg_data.empty:
            # Sort by size for proper line plotting
            alg_data = alg_data.sort_values('Size')
            
            plt.plot(alg_data['Size'], alg_data['Mean_ms'], 
                    marker='o', label=algorithm, linewidth=2, markersize=6,
                    color=colors.get(algorithm, '#7f7f7f'))
    
    # Customize the plot
    plt.xlabel('Array Size', fontsize=12)
    plt.ylabel('Time (milliseconds)', fontsize=12)
    plt.title(f'Sorting Algorithm Performance Comparison\nData Pattern: {pattern}', fontsize=14, fontweight='bold')
    plt.legend(fontsize=10, loc='upper left')
    plt.grid(True, alpha=0.3)
    
    # Use log scale for better visualization if the range is large
    sizes = pattern_data['Size'].unique()
    if len(sizes) > 1 and max(sizes) / min(sizes) > 100:
        plt.xscale('log')
        plt.yscale('log')
    
    # Improve layout
    plt.tight_layout()
    
    # Save the plot
    safe_pattern_name = pattern.replace(' ', '_').replace('/', '_').lower()
    output_file = output_dir / f"performance_{safe_pattern_name}.png"
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    plt.close()
    
    print(f"Generated plot: {output_file}")

def create_comparison_summary(df, output_dir):
    """Create a summary comparison plot showing all patterns."""
    patterns = get_unique_patterns(df)
    algorithms = get_unique_algorithms(df)
    
    # Create subplots for each pattern
    n_patterns = len(patterns)
    cols = 2 if n_patterns > 2 else n_patterns
    rows = (n_patterns + cols - 1) // cols
    
    fig, axes = plt.subplots(rows, cols, figsize=(15, 5 * rows))
    if rows == 1 and cols == 1:
        axes = [axes]
    elif rows == 1:
        axes = axes
    else:
        axes = axes.flatten()
    
    colors = {
        'std::sort': '#1f77b4',
        'std::stable_sort': '#ff7f0e', 
        'classic_quicksort': '#2ca02c',
        'dual_pivot_quicksort': '#d62728',
        'dual_pivot_optimized': '#9467bd'
    }
    
    for i, pattern in enumerate(patterns):
        if i < len(axes):
            ax = axes[i]
            pattern_data = df[df['Pattern'] == pattern]
            
            for algorithm in algorithms:
                alg_data = pattern_data[pattern_data['Algorithm'] == algorithm]
                if not alg_data.empty:
                    alg_data = alg_data.sort_values('Size')
                    ax.plot(alg_data['Size'], alg_data['Mean_ms'], 
                           marker='o', label=algorithm, linewidth=1.5, markersize=4,
                           color=colors.get(algorithm, '#7f7f7f'))
            
            ax.set_xlabel('Array Size')
            ax.set_ylabel('Time (ms)')
            ax.set_title(f'{pattern}', fontweight='bold')
            ax.grid(True, alpha=0.3)
            
            # Use log scale if appropriate
            sizes = pattern_data['Size'].unique()
            if len(sizes) > 1 and max(sizes) / min(sizes) > 100:
                ax.set_xscale('log')
                ax.set_yscale('log')
    
    # Remove unused subplots
    for i in range(len(patterns), len(axes)):
        fig.delaxes(axes[i])
    
    # Add legend outside the subplots
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc='center right', bbox_to_anchor=(0.98, 0.5))
    
    plt.tight_layout()
    plt.subplots_adjust(right=0.85)
    
    # Save the summary plot
    output_file = output_dir / "performance_summary.png"
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    plt.close()
    
    print(f"Generated summary plot: {output_file}")

def create_speedup_analysis(df, output_dir):
    """Create speedup analysis comparing algorithms against std::sort."""
    plt.figure(figsize=(14, 10))
    
    patterns = get_unique_patterns(df)
    algorithms = [alg for alg in get_unique_algorithms(df) if alg != 'std::sort']
    
    # Calculate speedup for each algorithm and pattern
    for i, pattern in enumerate(patterns):
        pattern_data = df[df['Pattern'] == pattern]
        std_sort_data = pattern_data[pattern_data['Algorithm'] == 'std::sort']
        
        plt.subplot(2, 4, i + 1)
        
        for algorithm in algorithms:
            alg_data = pattern_data[pattern_data['Algorithm'] == algorithm]
            
            # Merge on size to calculate speedup
            merged = pd.merge(std_sort_data[['Size', 'Mean_ms']], 
                            alg_data[['Size', 'Mean_ms']], 
                            on='Size', suffixes=('_std', '_alg'))
            
            if not merged.empty:
                speedup = merged['Mean_ms_std'] / merged['Mean_ms_alg']
                plt.plot(merged['Size'], speedup, marker='o', label=algorithm, linewidth=2)
        
        plt.axhline(y=1.0, color='black', linestyle='--', alpha=0.5, label='std::sort baseline')
        plt.xlabel('Array Size')
        plt.ylabel('Speedup vs std::sort')
        plt.title(f'{pattern}')
        plt.grid(True, alpha=0.3)
        
        # Use log scale for x-axis if appropriate
        sizes = pattern_data['Size'].unique()
        if len(sizes) > 1 and max(sizes) / min(sizes) > 100:
            plt.xscale('log')
    
    plt.tight_layout()
    
    # Save the speedup analysis
    output_file = output_dir / "speedup_analysis.png"
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    plt.close()
    
    print(f"Generated speedup analysis: {output_file}")

def main():
    """Main function to generate all plots."""
    if len(sys.argv) != 2:
        print("Usage: python plot_benchmark.py <csv_file>")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    
    # Load data
    print(f"Loading benchmark data from {csv_file}")
    df = load_benchmark_data(csv_file)
    
    # Create output directory for plots
    output_dir = Path(csv_file).parent / "plots"
    output_dir.mkdir(exist_ok=True)
    
    print(f"Generating plots in {output_dir}")
    
    # Generate individual plots for each pattern
    patterns = get_unique_patterns(df)
    for pattern in patterns:
        create_time_size_plot(df, pattern, output_dir)
    
    # Generate summary comparison
    create_comparison_summary(df, output_dir)
    
    # Generate speedup analysis
    create_speedup_analysis(df, output_dir)
    
    print(f"\nAll plots generated successfully in {output_dir}")
    print(f"Generated {len(patterns) + 2} plot files:")
    print("- Individual performance plots for each data pattern")
    print("- Summary comparison plot")
    print("- Speedup analysis plot")

if __name__ == "__main__":
    main()