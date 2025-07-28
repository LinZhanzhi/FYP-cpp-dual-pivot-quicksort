#!/usr/bin/env python3
"""
Test script to verify plotting functionality with existing CSV data.
"""

import pandas as pd
import matplotlib.pyplot as plt
import sys
import os
from pathlib import Path

def create_test_csv():
    """Create a small test CSV file with sample data."""
    data = [
        [10, "Random", "std::sort", 0.001, 0.001, 0.0001, 0.001, 0.002],
        [10, "Random", "dual_pivot_quicksort", 0.0012, 0.0011, 0.0001, 0.001, 0.002],
        [100, "Random", "std::sort", 0.01, 0.009, 0.001, 0.008, 0.012],
        [100, "Random", "dual_pivot_quicksort", 0.009, 0.008, 0.001, 0.007, 0.011],
        [1000, "Random", "std::sort", 0.1, 0.095, 0.01, 0.08, 0.12],
        [1000, "Random", "dual_pivot_quicksort", 0.09, 0.085, 0.01, 0.07, 0.11],
        [10, "Nearly Sorted", "std::sort", 0.0008, 0.0008, 0.0001, 0.0007, 0.001],
        [10, "Nearly Sorted", "dual_pivot_quicksort", 0.0009, 0.0009, 0.0001, 0.0008, 0.001],
        [100, "Nearly Sorted", "std::sort", 0.008, 0.007, 0.001, 0.006, 0.01],
        [100, "Nearly Sorted", "dual_pivot_quicksort", 0.007, 0.006, 0.001, 0.005, 0.009],
        [1000, "Nearly Sorted", "std::sort", 0.08, 0.075, 0.01, 0.06, 0.1],
        [1000, "Nearly Sorted", "dual_pivot_quicksort", 0.07, 0.065, 0.01, 0.05, 0.09],
    ]
    
    df = pd.DataFrame(data, columns=["Size", "Pattern", "Algorithm", "Mean_ms", "Median_ms", "StdDev_ms", "Min_ms", "Max_ms"])
    df.to_csv("test_results.csv", index=False)
    return "test_results.csv"

def test_plot_generation():
    """Test the plotting functionality."""
    csv_file = create_test_csv()
    
    # Import the plotting functions from the main script
    sys.path.append(os.path.dirname(os.path.abspath(__file__)))
    
    # Load the data
    df = pd.read_csv(csv_file)
    
    # Create output directory
    output_dir = Path("test_plots")
    output_dir.mkdir(exist_ok=True)
    
    # Test individual pattern plots
    patterns = df['Pattern'].unique()
    for pattern in patterns:
        pattern_data = df[df['Pattern'] == pattern]
        
        plt.figure(figsize=(10, 6))
        
        algorithms = pattern_data['Algorithm'].unique()
        for algorithm in algorithms:
            alg_data = pattern_data[pattern_data['Algorithm'] == algorithm]
            alg_data = alg_data.sort_values('Size')
            
            plt.plot(alg_data['Size'], alg_data['Mean_ms'], 
                    marker='o', label=algorithm, linewidth=2, markersize=6)
        
        plt.xlabel('Array Size')
        plt.ylabel('Time (milliseconds)')
        plt.title(f'Sorting Algorithm Performance - {pattern}')
        plt.legend()
        plt.grid(True, alpha=0.3)
        plt.tight_layout()
        
        safe_pattern_name = pattern.replace(' ', '_').lower()
        output_file = output_dir / f"test_{safe_pattern_name}.png"
        plt.savefig(output_file, dpi=150, bbox_inches='tight')
        plt.close()
        
        print(f"Generated test plot: {output_file}")
    
    print("Test plotting completed successfully!")
    
    # Clean up
    os.remove(csv_file)

if __name__ == "__main__":
    test_plot_generation()