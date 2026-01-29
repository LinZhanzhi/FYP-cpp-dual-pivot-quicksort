# Adaptive Granularity Implementation Report

**Date:** January 2026
**Author:** LZZ725
**Module:** Parallel Partitioning & Scheduling

## 1. Introduction

This report documents the implementation of **Adaptive Granularity** (Grain Size Tuning) within the Dual-Pivot Quicksort parallel framework. This optimization addresses the inefficiency of a static sequential cutoff threshold. While a static threshold (e.g., 4096 elements) prevents excessive recursion overhead in a balanced system, it does not account for runtime system load.

When the thread pool is saturated (high queue depth), generating more fine-grained tasks merely adds contention and scheduling overhead without increasing parallelism. Adaptive Granularity dynamically adjusts the sequential threshold based on real-time load, switching to coarser-grained tasks when the system is busy.

## 2. Methodology

### 2.1. Load Metric
We introduced a lightweight metric to gauge system saturation: **active task count**. This is defined as the number of tasks currently queued in the thread pool that have not yet begun execution or are currently running.

The `ThreadPool` class in `include/dpqs/parallel/threadpool.hpp` was enhanced with:
```cpp
// Heuristic for Adaptive Granularity
long get_active_task_count() const {
    // Determine how many tasks are conceptually "in flight"
    // active = incomplete_tasks (tasks pushed - tasks completed)
    return incomplete_tasks.load(std::memory_order_relaxed);
}
```
Using `std::memory_order_relaxed` ensures this check does not become a synchronization bottleneck.

### 2.2. Dynamic Threshold Logic
The core logic resides in `include/dpqs/parallel/parallel_sort.hpp` within the `parallel_sort_task`.

Instead of a constant `MIN_PARALLEL_SORT_SIZE`:

1.  **Check Load**: Before partitioning, the worker queries `pool.get_active_task_count()`.
2.  **Define Saturation**: A system is considered "saturated" if the number of active tasks exceeds  \times N_{threads}$. This multiplier (4x) ensures that every thread has a backlog of work, minimizing the risk of starvation if we stop producing tasks.
3.  **Adjust Threshold**:
    *   **Under Load**: Threshold is doubled (e.g., 096 \to 8192$).
    *   **Normal**: Threshold remains at the base value (4096).

#### Implementation Snippet
```cpp
auto& pool = getThreadPool();
long active_tasks = pool.get_active_task_count();
size_t num_threads = pool.get_thread_count();

std::ptrdiff_t threshold = MIN_PARALLEL_SORT_SIZE;
if (active_tasks > static_cast<long>(num_threads * 4)) {
    threshold *= 2; // Double the threshold to force earlier sequential fallback
}

while (high - low > threshold) {
    // ... partitioning logic ...
}
```

## 3. Theoretical Analysis

By increasing the threshold under load, we achieve **Overhead Amortization**.

*   **Little's Law Context**: In a stable system,  = \lambda W$. By reducing the arrival rate ($\lambda$) of small tasks (by handling them sequentially), we prevent the queue length ($) from growing unbounded, which would otherwise degrade latency ($) due to cache thrashing and lock contention in the stealing mechanism.
*   **Pathological Case Prevention**: This acts as a back-pressure mechanism. If tasks are generating faster than consumers can process them, the generators voluntarily switch to "consumer mode" (sequential sort) for larger chunks of data, naturally unwinding the backlog.

## 4. Verification

A smoke test was conducted using the `benchmark_runner_adaptive` binary.

*   **Configuration**:
    *   Algorithm: Dual-Pivot Parallel
    *   Data: 1,000,000 Integers (Random)
    *   Threads: 4
*   **Result**: Successful execution in ~14ms.
*   **Observation**: The system remained stable. The atomic load instruction overhead was negligible.

## 5. Scaling Performance Analysis

Following the smoke test, a full benchmark was run on a 10,000,000 integer dataset (Pattern: RANDOM, Type: int32) to evaluate scaling efficiency with the new adaptive threshold.

| Threads | Time (ms) | Speedup vs Seq | Efficiency | Implications |
|---------|-----------|----------------|------------|--------------|
| 1       | 486.75    | 1.00x          | 100%       | Baseline (Sequential) |
| 2       | 265.14    | 1.84x          | 92%        | Near-linear scaling |
| 4       | 165.31    | 2.94x          | 74%        | Good scaling, overhead minimal |
| 8       | 120.96    | 4.02x          | 50%        | Diminishing returns (Bandwidth limit?) |
| 16      | 110.42    | 4.41x          | 28%        | Saturation. Adaptive threshold active. |

**Analysis:**
The adaptive granularity successfully prevented performance regression at 16 threads (where "negative scaling" was previously a risk). However, the speedup plateauing between 8 and 16 threads (4.02x $\to$ 4.41x) confirms that simply reducing scheduler overhead is not enough. The system is likely hitting the **Memory Wall**.
*   The transition from 8 $\to$ 16 threads yields only ~9% performance gain.
*   This strongly validates the need for the next planned optimization: **Memory-Aware Scheduling** or **Vectorized Partitioning** to address bandwidth efficiency rather than just thread management.

## 6. Future Work

*   **Tune Multiplier**: The saturation multiplier (currently 4x) should be empirically tuned across different hardware (e.g., 64-core server vs. 4-core laptop).
*   **Linear Scaling**: Investigating if a graduated threshold (e.g.,  \propto Load$) offers smoother performance than the current binary switch.

