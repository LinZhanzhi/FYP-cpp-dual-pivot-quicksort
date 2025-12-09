# Quicksort vs Insertion Sort Threshold Benchmark

This project is designed to empirically find the optimal threshold for switching from Quicksort to Insertion Sort on your specific machine.

## Project Structure

- `src/`: C++ source code.
  - `sort.hpp`: Implementation of Hybrid Quicksort and Insertion Sort.
  - `main.cpp`: Benchmark harness.
- `scripts/`: Python scripts.
  - `benchmark.py`: Driver script to compile, run benchmarks, and plot results.
- `results/`: Output directory for CSV data and plots.

## Prerequisites

- C++ Compiler (g++ recommended, supporting C++17)
- Python 3
- Python libraries: `matplotlib` (optional, for plotting)

## How to Run

1.  Navigate to the project root:
    ```bash
    cd /home/lzz725/FYP/2025-12-09-Insertion-Quicksort-boundary
    ```

2.  Run the benchmark script:
    ```bash
    python3 scripts/benchmark.py
    ```

    This script will:
    -   Compile the C++ code using `g++`.
    -   Run the benchmark for various array sizes (1k - 256k) and thresholds (0 - 100).
    -   Test different data distributions (Random, Sorted, Reverse, Few Unique).
    -   Save the results to `results/benchmark_results.csv`.
    -   Generate plots in `results/`.

## Interpreting Results

-   Look at the generated plots in `results/`.
-   Find the "valley" in the Time vs Threshold graph.
-   The threshold where the time is minimal is your optimal cutoff.
-   Typically, this is between 10 and 30 for integers.

## Customization

You can modify `scripts/benchmark.py` to change:
-   `SIZES`: Array sizes to test.
-   `THRESHOLDS`: Threshold values to sweep.
-   `RUNS`: Number of repetitions for accuracy.
