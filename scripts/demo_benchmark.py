#!/usr/bin/env python3
"""
Quick benchmark runner with plotting
Runs a small subset of benchmarks and generates plots
"""

import subprocess
import sys
import os
from pathlib import Path

def run_limited_benchmark():
    """Run a limited benchmark to test the plotting functionality."""
    
    # Create a simple test CSV with realistic data
    csv_content = """Size,Pattern,Algorithm,Mean_ms,Median_ms,StdDev_ms,Min_ms,Max_ms
100,Random,std::sort,0.005,0.005,0.0001,0.004,0.006
100,Random,std::stable_sort,0.007,0.007,0.0001,0.006,0.008
100,Random,classic_quicksort,0.012,0.012,0.001,0.011,0.014
100,Random,dual_pivot_quicksort,0.010,0.010,0.001,0.009,0.012
100,Random,dual_pivot_optimized,0.011,0.011,0.001,0.010,0.013
1000,Random,std::sort,0.085,0.077,0.018,0.076,0.185
1000,Random,std::stable_sort,0.099,0.098,0.005,0.094,0.127
1000,Random,classic_quicksort,0.185,0.182,0.020,0.158,0.267
1000,Random,dual_pivot_quicksort,0.147,0.145,0.009,0.136,0.164
1000,Random,dual_pivot_optimized,0.174,0.170,0.014,0.156,0.226
10000,Random,std::sort,1.106,1.079,0.072,1.043,1.359
10000,Random,std::stable_sort,1.370,1.336,0.109,1.218,1.693
10000,Random,classic_quicksort,2.200,2.178,0.173,1.953,2.575
10000,Random,dual_pivot_quicksort,1.842,1.789,0.134,1.708,2.268
10000,Random,dual_pivot_optimized,2.205,2.129,0.163,2.044,2.585
100,Nearly Sorted,std::sort,0.004,0.004,0.0001,0.003,0.005
100,Nearly Sorted,std::stable_sort,0.006,0.005,0.0001,0.005,0.007
100,Nearly Sorted,classic_quicksort,0.039,0.041,0.002,0.036,0.042
100,Nearly Sorted,dual_pivot_quicksort,0.006,0.006,0.0001,0.005,0.007
100,Nearly Sorted,dual_pivot_optimized,0.009,0.009,0.0001,0.008,0.010
1000,Nearly Sorted,std::sort,0.050,0.050,0.004,0.046,0.058
1000,Nearly Sorted,std::stable_sort,0.071,0.069,0.005,0.066,0.081
1000,Nearly Sorted,classic_quicksort,0.472,0.457,0.039,0.436,0.577
1000,Nearly Sorted,dual_pivot_quicksort,0.094,0.094,0.001,0.093,0.095
1000,Nearly Sorted,dual_pivot_optimized,0.149,0.145,0.011,0.142,0.188
10000,Nearly Sorted,std::sort,0.638,0.624,0.031,0.611,0.794
10000,Nearly Sorted,std::stable_sort,0.851,0.818,0.070,0.795,1.066
10000,Nearly Sorted,classic_quicksort,3.607,3.535,0.329,3.170,4.443
10000,Nearly Sorted,dual_pivot_quicksort,1.430,1.403,0.114,1.292,1.801
10000,Nearly Sorted,dual_pivot_optimized,2.396,2.346,0.131,2.242,2.755
100,Reverse Sorted,std::sort,0.004,0.004,0.0001,0.003,0.005
100,Reverse Sorted,std::stable_sort,0.005,0.005,0.0001,0.004,0.006
100,Reverse Sorted,classic_quicksort,0.073,0.075,0.006,0.064,0.082
100,Reverse Sorted,dual_pivot_quicksort,0.007,0.007,0.0001,0.006,0.008
100,Reverse Sorted,dual_pivot_optimized,0.008,0.009,0.0001,0.007,0.009
1000,Reverse Sorted,std::sort,0.038,0.038,0.001,0.036,0.040
1000,Reverse Sorted,std::stable_sort,0.052,0.051,0.004,0.049,0.076
1000,Reverse Sorted,classic_quicksort,5.420,5.343,0.229,5.121,6.316
1000,Reverse Sorted,dual_pivot_quicksort,0.083,0.083,0.008,0.071,0.100
1000,Reverse Sorted,dual_pivot_optimized,0.106,0.106,0.001,0.105,0.108
10000,Reverse Sorted,std::sort,0.480,0.473,0.029,0.455,0.593
10000,Reverse Sorted,std::stable_sort,0.602,0.583,0.042,0.561,0.768
10000,Reverse Sorted,classic_quicksort,54.2,53.4,2.3,51.2,63.1
10000,Reverse Sorted,dual_pivot_quicksort,0.83,0.83,0.08,0.71,1.0
10000,Reverse Sorted,dual_pivot_optimized,1.06,1.06,0.01,1.05,1.08"""

    # Write the test data
    with open("demo_results.csv", "w") as f:
        f.write(csv_content)
    
    print("Generated demo benchmark data...")
    print("Creating performance plots...")
    
    # Run the plotting script
    result = subprocess.run([sys.executable, "../scripts/plot_benchmark.py", "demo_results.csv"], 
                          capture_output=True, text=True)
    
    if result.returncode == 0:
        print("Demo plots generated successfully!")
        print("\nGenerated files:")
        plots_dir = Path("plots")
        if plots_dir.exists():
            for plot_file in plots_dir.glob("*.png"):
                print(f"  - {plot_file}")
    else:
        print("Error generating plots:")
        print(result.stderr)
    
    return result.returncode == 0

if __name__ == "__main__":
    # Change to results directory
    os.chdir("results")
    success = run_limited_benchmark()
    if success:
        print("\nPlotting functionality is working correctly!")
        print("You can now run 'make run_benchmark' to generate plots from real benchmark data.")
    else:
        print("\nThere was an issue with the plotting functionality.")
        sys.exit(1)