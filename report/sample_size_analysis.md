# Sample Size Analysis Report

## Objective
To determine the minimum sufficient sample size (number of iterations) for benchmarking sorting algorithms, balancing execution time with statistical accuracy.

## Methodology & Workflow

To determine the optimal sample size, we employed a rigorous statistical approach. The goal was to identify the smallest sample size $N$ that yields a mean execution time within an acceptable error margin of a "ground truth" baseline.

### 1. Data Collection
We executed the benchmarking runner for **31 geometrically spaced array sizes** ranging from 1,000 to 1,000,000 elements.
For each array size, we performed **300 iterations** (measurements) of the sorting algorithm. This provided a high-resolution dataset to serve as our ground truth.

### 2. Subsampling Simulation
To evaluate the accuracy of smaller sample sizes without re-running benchmarks, we simulated smaller runs by slicing the dataset. For a target sample size $N$ (e.g., 30, 100), we took the **first $N$ iterations** from the original 300. This mimics the real-world scenario where a user would only run the benchmark $N$ times.

Tested sample sizes: 300 (Baseline), 200, 100, 80, 60, 40, 30, 20, 10.

### 3. Statistical Processing
For each sample size $N$ and array size $S$, we applied the following statistical cleaning process to ensure robustness against system noise (context switches, background processes):

1.  **Calculate Initial Statistics**: Compute the raw mean ($\mu$) and standard deviation ($\sigma$) of the $N$ samples.
2.  **Outlier Detection**: Define an acceptance window of $[\mu - 2\sigma, \mu + 2\sigma]$.
3.  **Filtering**: Remove any measurement falling outside this window.
4.  **Refinement**: Calculate the **Cleaned Mean** ($\mu_{clean}$) from the remaining filtered data.

### 4. Error Calculation
We defined the **Baseline Mean** as the Cleaned Mean obtained from the full 300 iterations.
For each smaller sample size $N$, we calculated the **Relative Error** against this baseline:

$$ \text{Error}_N = \left| \frac{\mu_{clean, N} - \mu_{clean, 300}}{\mu_{clean, 300}} \right| \times 100\% $$

Finally, we aggregated these errors across all 31 array sizes to derive:
*   **Avg Relative Error**: The arithmetic mean of errors across all array sizes.
*   **Max Relative Error**: The worst-case error observed for any single array size.

## Case Study: std::sort (int)

### Configuration
- **Algorithm**: `std::sort`
- **Data Type**: `int`
- **Pattern**: `RANDOM`

### Results

The following table summarizes the average and maximum relative error for each sample size across all array sizes:

| Sample Size | Avg Relative Error (%) | Max Relative Error (%) |
|---|---|---|
| 200 | 1.24% | 5.82% |
| 100 | 2.46% | 10.72% |
| 80 | 2.82% | 13.80% |
| 60 | 3.11% | 19.44% |
| 40 | 3.59% | 33.26% |
| 30 | 4.39% | 39.77% |
| 20 | 6.00% | 55.57% |
| 10 | 9.45% | 95.93% |

#### Detailed Analysis

**Table 1: Comparison of Standard Deviations (ms) for Different Sample Sizes**
*Note: Selected array sizes shown for brevity.*

| Array Size | 300 | 200 | 100 | 80 | 60 | 40 | 30 | 20 | 10 |
|---|---|---|---|---|---|---|---|---|---|
| 1,000 | 0.000 | 0.001 | 0.001 | 0.001 | 0.001 | 0.002 | 0.002 | 0.003 | 0.003 |
| 1,995 | 0.003 | 0.003 | 0.002 | 0.003 | 0.003 | 0.003 | 0.003 | 0.004 | 0.006 |
| 3,981 | 0.006 | 0.004 | 0.004 | 0.004 | 0.005 | 0.006 | 0.007 | 0.008 | 0.008 |
| 7,943 | 0.011 | 0.012 | 0.009 | 0.009 | 0.008 | 0.009 | 0.009 | 0.008 | 0.007 |
| 15,849 | 0.025 | 0.023 | 0.020 | 0.022 | 0.024 | 0.024 | 0.021 | 0.021 | 0.025 |
| 31,623 | 0.088 | 0.102 | 0.070 | 0.071 | 0.082 | 0.038 | 0.032 | 0.042 | 0.019 |
| 63,096 | 0.106 | 0.098 | 0.094 | 0.084 | 0.063 | 0.041 | 0.037 | 0.037 | 0.051 |
| 125,893 | 0.250 | 0.238 | 0.231 | 0.255 | 0.277 | 0.274 | 0.266 | 0.201 | 0.145 |
| 251,189 | 0.399 | 0.381 | 0.491 | 0.385 | 0.507 | 0.544 | 0.591 | 0.436 | 0.317 |
| 501,187 | 0.628 | 0.656 | 0.640 | 0.576 | 0.539 | 0.643 | 0.723 | 0.821 | 0.780 |
| 1,000,000 | 1.459 | 1.442 | 1.739 | 1.861 | 1.699 | 1.108 | 1.086 | 1.467 | 2.549 |

**Table 2: Comparison of the Number of Outliers for Different Sample Sizes**
*Note: Outliers are defined as measurements outside $\mu \pm 2\sigma$.*

| Array Size | 300 | 200 | 100 | 80 | 60 | 40 | 30 | 20 | 10 |
|---|---|---|---|---|---|---|---|---|---|
| 1,000 | 7 | 5 | 4 | 4 | 3 | 2 | 2 | 1 | 1 |
| 1,995 | 10 | 3 | 3 | 2 | 2 | 2 | 2 | 1 | 1 |
| 3,981 | 5 | 6 | 5 | 4 | 3 | 2 | 2 | 1 | 1 |
| 7,943 | 8 | 6 | 3 | 3 | 2 | 1 | 1 | 1 | 1 |
| 15,849 | 7 | 4 | 2 | 2 | 2 | 0 | 1 | 1 | 0 |
| 31,623 | 18 | 6 | 4 | 2 | 1 | 3 | 2 | 1 | 0 |
| 63,096 | 10 | 4 | 3 | 2 | 4 | 2 | 1 | 1 | 0 |
| 125,893 | 15 | 7 | 4 | 3 | 3 | 1 | 0 | 0 | 0 |
| 251,189 | 15 | 9 | 3 | 5 | 2 | 2 | 1 | 1 | 1 |
| 501,187 | 18 | 11 | 6 | 3 | 2 | 1 | 1 | 1 | 1 |
| 1,000,000 | 20 | 15 | 6 | 3 | 3 | 3 | 3 | 2 | 0 |

## Observations
1. **High Precision (>200 samples)**: For errors consistently below 2%, a large sample size is required.
2. **Moderate Precision (30-100 samples)**: The average error stays below 5% for sample sizes down to 30. However, the maximum error increases significantly, likely due to timer resolution limits on small array sizes.
3. **Low Precision (<30 samples)**: Below 30 samples, the error grows rapidly, with maximum errors exceeding 50%.

## Recommendation
- **For Quick Tests**: A sample size of **30** is sufficient to get a general idea of performance (avg error ~4.4%).
- **For Final Benchmarks**: A sample size of **at least 100** is recommended to ensure the average error is below 2.5% and to minimize the impact of system noise.

## Data
The raw analysis data is available in `benchmarks/sample_size_analysis.csv`.

