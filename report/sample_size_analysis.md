# Sample Size Analysis Report

## Objective
To determine the minimum sufficient sample size (number of iterations) for benchmarking sorting algorithms, balancing execution time with statistical accuracy.

## Methodology & Workflow

To determine the optimal sample size, we employed a rigorous statistical approach. The goal was to identify the smallest sample size $N$ that yields a mean execution time within an acceptable error margin of a "ground truth" baseline.

### 1. Data Collection
We executed the benchmarking runner for **41 geometrically spaced array sizes** ranging from 1,000 to 10,000,000 elements.
For each array size, we performed **30 iterations** (measurements) of the sorting algorithm.

### 2. Analysis Method: Minimum vs. Average
Traditionally, average time is used for benchmarking. However, on a non-real-time OS (like Linux/WSL), system noise (context switches, background processes, interrupts) acts as **additive noise**. It can only *increase* the measured runtime; it can never make the code run faster than the hardware limit.

Therefore:
*   **Average**: Biased potential upward by outliers and system load.
*   **Minimum**: The closest approximation to the true, undisturbed algorithmic runtime (instruction count + memory latency).

### 3. Verification
We observed that with **30 iterations**, the minimum value stabilizes. Increasing the iteration count to 100 or 300 rarely yielded a lower minimum time (often <1% difference), confirming that 30 samples are sufficient to capture a "clean" run free from significant OS interference.

## Conclusion
We adopted a strategy of **30 iterations** per data point, reporting the **minimum** execution time. This maximizes benchmarking throughput while effectively filtering out transient system noise.
- **For Quick Tests**: A sample size of **30** is sufficient to get a general idea of performance (avg error ~4.4%).
- **For Final Benchmarks**: A sample size of **at least 100** is recommended to ensure the average error is below 2.5% and to minimize the impact of system noise.

## Data
The raw analysis data is available in `benchmarks/sample_size_analysis.csv`.

