# Space Complexity Analysis & Optimization Report

## 1. Executive Summary
This report analyzes the memory footprint of the current Dual-Pivot Quicksort (DPQS) implementation. The analysis reveals that while the algorithm behaves as a standard in-place sorter for random data, it significantly deviates from this behavior for structured data (nearly sorted, reverse sorted) by utilizing $O(n)$ auxiliary memory to achieve higher performance.

**Verdict:** The current space usage is **acceptable** for a performance-oriented hybrid sorter, provided the $O(n)$ requirement for structured data is clearly documented. It is not a strictly in-place replacement for `std::sort` in all scenarios.

## 2. Default Behavior (Random Data)
For random or unstructured data, the algorithm follows the standard Dual-Pivot Quicksort logic.

*   **Mechanism:** Recursive partitioning.
*   **Space Complexity:** $O(\log n)$ stack space.
*   **Analysis:**
    *   The recursion depth is bounded by `MAX_RECURSION_DEPTH` (default 384) before switching to Heapsort.
    *   Tail-call optimization (looping on the largest partition) is implemented to minimize stack growth.
    *   **Comparison:** Matches `std::sort` (Introsort), which also uses $O(\log n)$ stack space.

## 3. Structured Data Optimization (The $O(n)$ Spike)
The implementation includes a specific optimization for widely structured data (runs detection).

*   **Files Implicated:** `include/dpqs/run_merger.hpp` (function `try_merge_runs`).
*   **Mechanism:**
    *   The algorithm scans for existing runs (ascending/descending).
    *   If a high degree of structure is detected, it switches to a Merge Sort strategy.
    *   **Crucial Detail:** It allocates a full-size auxiliary buffer:
        ```cpp
        // dpqs/run_merger.hpp
        std::vector<T> b(size); // Heap allocation of O(N)
        ```
*   **Space Complexity:** $O(n)$ heap memory.
*   **Risk:**
    *   Sorting a 10GB array requires an additional 10GB of RAM.
    *   Unlike `std::sort` (which strictly uses $O(1)$ auxiliary memory), this can cause Out-Of-Memory (OOM) errors on memory-constrained systems when sorting large structured datasets.
*   **Trade-off:** This provides significant speedups (often 2x-5x faster than Quicksort on nearly sorted data) at the cost of memory.

## 4. Parallel Execution Overhead
The parallel implementation inherently requires more memory for synchronization and buffering.

*   **Files Implicated:** `include/dpqs/parallel/buffer_manager.hpp`.
*   **Mechanism:**
    *   Uses a `BufferManager` to pool auxiliary buffers for parallel merging.
    *   Thread-local storage is used to minimize contention.
*   **Space Complexity:** $O(P \cdot B)$ where $P$ is the number of threads and $B$ is the buffer block size, potentially scaling up to $O(n)$ depending on the merge strategy.

## 5. Comparison with Standard Library

| Feature | `std::sort` (Introsort) | DPQS (Random Input) | DPQS (Structured Input) |
| :--- | :--- | :--- | :--- |
| **Algorithm** | Quicksort + Heapsort | Dual-Pivot Quicksort | Adaptive Merge Sort |
| **Auxiliary Memory** | $O(1)$ | $O(1)$ | **$O(n)$** |
| **Stack Space** | $O(\log n)$ | $O(\log n)$ | $O(1)$ (Iterative) |
| **Worst-Case Space** | $O(\log n)$ | $O(\log n)$ | $O(n)$ |
| **Stability** | Unstable | Unstable | Stable (implementation detail) |

## 6. Recommendations

### Immediate Actions
1.  **Documentation:** Explicitly state in the header comments and README that the algorithm requires $O(n)$ available memory when "Run Merging" is triggered.
2.  **Configuration:** Consider adding a compile-time flag (e.g., `DPQS_LOW_MEMORY`) to disable `try_merge_runs` for environments where strict in-place sorting is required.

### Future Optimizations (Optional)
If strict memory constraints are a priority:
1.  **In-Place Merging:** Replace the buffer-based merge with an in-place merge algorithm (e.g., `std::inplace_merge`), though this will likely degrade performance.
2.  **Chunking:** Limit the buffer size to a fixed constant (e.g., 1MB) and merge in passes, rather than allocating a buffer for the entire array.
