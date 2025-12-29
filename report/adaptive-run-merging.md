# Feature Report: Adaptive Run Merging (Timsort Optimization)

## 1. Overview
Real-world data is rarely completely random. It often contains subsequences that are already sorted (called "runs"). Standard Quicksort ignores this structure and sorts blindly, potentially doing unnecessary work.

Our library implements an **Adaptive Run Merging** strategy inspired by Timsort. Before attempting to partition an array, it checks if the array is already partially sorted.

## 2. Mechanism
The `try_merge_runs` function (in `include/dpqs/run_merger.hpp`) scans the array to identify existing runs.

### Run Detection
1.  It scans the array linearly.
2.  It identifies sequences that are either:
    - **Ascending**: `a[i] <= a[i+1]`
    - **Descending**: `a[i] > a[i+1]` (these are reversed in-place to become ascending)
3.  It records the boundaries of these runs.

### Heuristic Decision
The algorithm doesn't always merge. It uses heuristics to decide if merging is worth the effort:
- **Run Count**: If the number of runs is small (relative to array size), it implies the data is highly structured.
- **Run Length**: If runs are long, merging them is efficient ($O(N)$).
- **Threshold**: If the number of runs exceeds `MAX_RUN_COUNT`, the algorithm aborts the scan and falls back to Dual-Pivot Quicksort. This ensures we don't waste time scanning random data ($O(N)$ overhead).

### Merging
If the data is deemed "structured":
- It merges the identified runs using a standard merge sort procedure.
- This effectively sorts the array in $O(N)$ time for nearly-sorted data, compared to $O(N \log N)$ for Quicksort.

## 3. Benefits
- **Best-Case Performance**: Achieves $O(N)$ time complexity for sorted or reverse-sorted data.
- **Real-World Efficiency**: Significantly speeds up sorting for data with natural order (e.g., time-series data, appended logs).
- **Low Overhead**: The scan aborts early if the data looks random, minimizing the penalty for the general case.

## 4. Implementation Details
- **File**: `include/dpqs/run_merger.hpp`
- **Function**: `try_merge_runs`
- **Integration**: Called at the beginning of `sort_sequential` in `sequential_sorters.hpp`.
