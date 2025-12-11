# Sample Size Analysis Report

## Objective
To determine the minimum sufficient sample size (number of iterations) for benchmarking sorting algorithms, balancing execution time with statistical accuracy.

## Methodology
- **Algorithm**: `std::sort`
- **Data Type**: `int`
- **Pattern**: `RANDOM`
- **Array Sizes**: 31 geometrically spaced sizes from 1,000 to 1,000,000.
- **Baseline**: 300 iterations per array size.
- **Tested Sample Sizes**: 300, 200, 100, 80, 60, 40, 30, 20, 10.
- **Metric**: Relative error of the mean execution time compared to the baseline (300 samples). Outliers (Mean ± 2σ) were removed before calculating the mean.

## Results

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

## Observations
1. **High Precision (>200 samples)**: For errors consistently below 2%, a large sample size is required.
2. **Moderate Precision (30-100 samples)**: The average error stays below 5% for sample sizes down to 30. However, the maximum error increases significantly, likely due to timer resolution limits on small array sizes.
3. **Low Precision (<30 samples)**: Below 30 samples, the error grows rapidly, with maximum errors exceeding 50%.

## Recommendation
- **For Quick Tests**: A sample size of **30** is sufficient to get a general idea of performance (avg error ~4.4%).
- **For Final Benchmarks**: A sample size of **at least 100** is recommended to ensure the average error is below 2.5% and to minimize the impact of system noise.

## Data
The raw analysis data is available in `benchmarks/sample_size_analysis.csv`.

