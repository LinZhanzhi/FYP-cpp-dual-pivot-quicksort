# Performance Analysis: Dual-Pivot Quicksort on Reverse Sorted Arrays

## Observation
Dual-Pivot Quicksort demonstrates exceptional performance on reverse sorted arrays, running approximately **10x faster** than `std::sort`. This document justifies this performance gap based on the specific implementation details found in the codebase.

## Root Cause: Run Detection and Merging Strategy

The primary reason for this performance advantage is the **Run Detection** mechanism implemented in `include/dpqs/run_merger.hpp`. Unlike standard Quicksort implementations (and `std::sort`) which blindly partition the array, this implementation first attempts to identify existing order in the data.

### The Algorithm
The function `tryMergeRuns` (called from `core_sort.hpp`) performs a linear scan of the array to identify "runs" — sequences of ascending or descending elements.

1.  **Detection**: The algorithm scans the array from left to right.
2.  **Handling Descending Runs**: When it encounters a descending sequence (`a[k-1] > a[k]`), it continues scanning until the sequence ends.
3.  **In-Place Reversal**: Crucially, it immediately **reverses** this descending sequence to make it ascending.

### Code Evidence
From `include/dpqs/run_merger.hpp`:

```cpp
        } else if (a[k - 1] > a[k]) {
            // Identify descending sequence
            while (++k < high && a[k - 1] >= a[k]);

            // Reverse into ascending order
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                T temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
```

### Scenario: Fully Reverse Sorted Array
When the input array is fully reverse sorted:
1.  The scan identifies the entire array as a single descending run.
2.  The loop continues from `low` to `high`.
3.  The reversal loop flips the entire array in (N)$ time.
4.  The function detects that the array is now fully sorted (monotonous sequence) and returns `true`.

```cpp
        // Check special cases
        if (run.empty()) {
            if (k == high) {
                // The array is monotonous sequence,
                // and therefore already sorted.
                return true;
            }
```

## Comparison with `std::sort`

*   **Dual-Pivot Quicksort**: Detects the pattern, performs one (N)$ reversal, and terminates. **Complexity: (N)*.
*   **`std::sort` (Introsort)**: Typically picks a pivot (e.g., median-of-3). For a reverse sorted array, it will successfully partition the array, but it still proceeds with the recursive sorting process. Even in the best case, it performs (N \log N)$ comparisons and moves.

## Conclusion
The 10x speedup is not due to the partitioning efficiency of Dual-Pivot Quicksort itself, but rather due to the **adaptive pre-processing step** (`tryMergeRuns`) that converts a reverse-sorted array into a sorted one in linear time. This makes the algorithm extremely efficient for data that is already sorted or reverse-sorted.
