# Array Size Sampling Strategy

## 1. Design Choice

For the benchmarking of the Dual-Pivot Quicksort algorithm, we have selected a set of **41 array sizes** ranging from **1,000 to 10,000,000**. The sizes are distributed **geometrically (logarithmically)** to ensure uniform coverage across different orders of magnitude.

-   **Minimum Size**: 1,000 ($10^3$)
-   **Maximum Size**: 10,000,000 ($10^7$)
-   **Total Points**: 41
-   **Distribution**: Logarithmic spacing with a step size of 0.1 in base-10 log scale.

## 2. Justification

The decision to use logarithmic spacing with this specific density is based on several key factors:

1.  **Asymptotic Analysis**: Sorting algorithms typically exhibit $O(n \log n)$ behavior. To visualize and fit this curve accurately, it is crucial to cover multiple orders of magnitude. Linear spacing (e.g., 10k, 20k, 30k) would oversample large arrays while neglecting the behavior at smaller sizes where overheads and constant factors are more significant.
2.  **Curve Fitting Requirements**: A common rule of thumb for curve fitting is to have at least 10–15 distinct points per curve. By choosing 31 points, we exceed this minimum, providing a robust dataset (approx. 10 points per order of magnitude) that allows for high-confidence regression analysis and identification of performance anomalies.
3.  **Cache Effects**: Modern processors have multi-level cache hierarchies (L1, L2, L3). Performance characteristics often change drastically when the dataset size exceeds a cache level. A dense, geometrically spaced sampling ensures we capture these transition points effectively.
4.  **Visual Inspection**: 30 points provide a smooth curve when plotted on a log-log or semi-log scale, making it easier to visually compare the performance of Dual-Pivot Quicksort against standard Quicksort and other baselines.

## 3. Calculation Method

The array sizes are calculated using the following formula:

Let $N = 30$ be the number of intervals (for 31 points).
Let $Start = \log_{10}(1000) = 3$.
Let $End = \log_{10}(1000000) = 6$.

The step size $\Delta$ is:
$$ \Delta = \frac{End - Start}{N} = \frac{6 - 3}{30} = 0.1 $$

The size at index $i$ (where $i = 0, 1, \dots, 30$) is given by:
$$ Size_i = \text{round}(10^{3 + 0.1 \times i}) $$

## 4. Selected Array Sizes

The following table lists the exact calculated sizes used in the benchmark:

| Index | Log10 Value | Exact Value | Rounded Size |
|-------|-------------|-------------|--------------|
| 0 | 3.00 | 1000.00 | 1000 |
| 1 | 3.10 | 1258.93 | 1259 |
| 2 | 3.20 | 1584.89 | 1585 |
| 3 | 3.30 | 1995.26 | 1995 |
| 4 | 3.40 | 2511.89 | 2512 |
| 5 | 3.50 | 3162.28 | 3162 |
| 6 | 3.60 | 3981.07 | 3981 |
| 7 | 3.70 | 5011.87 | 5012 |
| 8 | 3.80 | 6309.57 | 6310 |
| 9 | 3.90 | 7943.28 | 7943 |
| 10 | 4.00 | 10000.00 | 10000 |
| 11 | 4.10 | 12589.25 | 12589 |
| 12 | 4.20 | 15848.93 | 15849 |
| 13 | 4.30 | 19952.62 | 19953 |
| 14 | 4.40 | 25118.86 | 25119 |
| 15 | 4.50 | 31622.78 | 31623 |
| 16 | 4.60 | 39810.72 | 39811 |
| 17 | 4.70 | 50118.72 | 50119 |
| 18 | 4.80 | 63095.73 | 63096 |
| 19 | 4.90 | 79432.82 | 79433 |
| 20 | 5.00 | 100000.00 | 100000 |
| 21 | 5.10 | 125892.54 | 125893 |
| 22 | 5.20 | 158489.32 | 158489 |
| 23 | 5.30 | 199526.23 | 199526 |
| 24 | 5.40 | 251188.64 | 251189 |
| 25 | 5.50 | 316227.77 | 316228 |
| 26 | 5.60 | 398107.17 | 398107 |
| 27 | 5.70 | 501187.23 | 501187 |
| 28 | 5.80 | 630957.34 | 630957 |
| 29 | 5.90 | 794328.23 | 794328 |
| 30 | 6.00 | 1000000.00 | 1000000 |

**List format for configuration:**
```python
SIZES = [
    1000, 1259, 1585, 1995, 2512, 3162, 3981, 5012, 6310, 7943,
    10000, 12589, 15849, 19953, 25119, 31623, 39811, 50119, 63096, 79433,
    100000, 125893, 158489, 199526, 251189, 316228, 398107, 501187, 630957, 794328,
    1000000
]
```

## 5. Benefits Summary

-   **Coverage**: Spans 3 orders of magnitude ($10^3$ to $10^6$).
-   **Density**: 10 points per decade ensures high-resolution data.
-   **Efficiency**: Avoids wasteful oversampling of large arrays while providing enough detail for small and medium arrays.
-   **Robustness**: Sufficient data points to smooth out system noise and outliers during analysis.
