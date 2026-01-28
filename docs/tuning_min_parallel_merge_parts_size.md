# Tuning Report: MIN_PARALLEL_MERGE_PARTS_SIZE

**Date:** January 28, 2026
**Component:** Parallel Dual-Pivot Quicksort (Merger)

## 1. Objective
Tune the `MIN_PARALLEL_MERGE_PARTS_SIZE` constant.
This constant determines the threshold for splitting a merge task into smaller parallel sub-tasks.
If `RangeSize >= Threshold`, it is split. Otherwise, it is performed sequentially.

*   **Current Value:** 4096
*   **Goal:** Balance load balancing (favored by small threshold) vs task creation/synchronization overhead (favored by large threshold).

## 2. Methodology
*   **Benchmark:** `benchmarks/tune_merge_granularity.cpp`
*   **Workload:** Sorting 50,000,000 integers.
*   **Data Structure:** Input array composed of sorted runs of length 256. This forces the algorithm into the "Run Merger" path, where `parallel_merge_parts` is utilized.
*   **Implementation**: The underlying thread pool uses a distributed queue with 1 mutex per thread (not lock-free).

## 3. Results (50M Integers)

| Threshold Size | Execution Time (ms) | Notes |
| :--- | :--- | :--- |
| 128 | 245 | High task count |
| 512 | 244 | Best recorded (marginal) |
| 1024 | 250 | |
| 2048 | 248 | |
| **4096** | **249** | **Current Default** |
| 8192 | 249 | |
| 65536 | 251 | Low task count |

## 4. Analysis
*   The performance profile is remarkably flat across the range [128, 65536].
*   Values between 512 and 4096 yield practically identical performance (~244-249ms).
*   The slight dip at 512 (244ms) represents a <2% improvement, which is within the margin of error (noise).
*   Given that our thread pool uses `std::mutex` for queue access, significantly increasing the number of tasks (by lowering the threshold from 4096 to 512) introduces 8x more synchronization operations.

## 5. Conclusion
**Decision:** **Retain 4096.**

While 512 showed a tiny speedup in this specific microbenchmark, 4096 is safer because:
1.  It minimizes mutex contention in the thread pool compared to smaller values.
2.  It matches the Java reference implementation.
3.  It provides ample parallelism for large arrays (50M / 4096 = ~12,000 tasks, sufficient for any reasonable core count).

**Action:** No code change required.
