# Memory-Aware Scheduling Implementation Report

**Date:** January 2026
**Author:** LZZ725
**Module:** Parallel Thread Pool Scheduler

## 1. Introduction

Following the implementation of **Adaptive Granularity**, our benchmarks identified a diminishing return in speedup between 8 and 16 threads (4.02x $\to$ 4.41x). This plateau indicates that while we successfully reduced *scheduler overhead* (via coarser grain size), we hit the **Memory Wall**.

In a standard Work-Stealing scheduler, when a thread runs out of work, it steals from a *random* victim. In a recursive divide-and-conquer algorithm like QuickSort, tasks are inherently localized in memory. Random stealing causes threads to jump continuously between disparate memory regions (e.g., from the left partition of the array to the far right), thrashing the L2/L3 caches and saturating the memory bus.

This report documents the implementation of **Memory-Aware Scheduling** via the "Sticky Victim" strategy to mitigate this issue.

## 2. Methodology

### 2.1. The "Sticky Victim" Strategy

The core hypothesis is **Spatial Locality of Tasks**:
> If Thread A has one available task in its queue, it forces a high probability that it has *other* tasks in its queue that operate on adjacent or nearby memory regions (sub-branches of the same partition tree).

Instead of resetting the victim search to a random offset every time, a thief now **remembers** the last thread it successfully stole from. It prioritizes checking this "Sticky Victim" first in subsequent attempts.

### 2.2. Implementation

The change was implemented in `include/dpqs/parallel/threadpool.hpp`.

**Previous Logic (Random/Round-Robin):**
Always started checking from `(i + 1)`.

**New Logic (Sticky):**
1.  Initialize `last_victim` to neighbor `(i + 1)`.
2.  When scanning for work, start the loop offset from `last_victim`.
3.  If a steal is successful `queues[victim]->try_steal()`, update `last_victim = victim`.

#### Code Snippet
\`\`\`cpp
// Worker loop initialization
size_t last_victim = (i + 1) % num_threads;

// ... inside stealing loop ...

// Scan for victims starting from the LAST successful victim (Sticky)
for (size_t k = 0; k < num_threads; ++k) {
    // Calculate via offset from last_victim to maintain cycle
    size_t victim = (last_victim + k) % num_threads;

    if (victim == i) continue; // Don't steal from self

    if (queues[victim]->try_steal(task)) {
        found = true;
        steal_successes++;
        last_victim = victim; // STICK to this victim for next time
        break;
    }
}
\`\`\`

## 3. Theoretical Analysis

### 3.1. Cache Affinity
By sticking to a single victim until it is empty, the thief thread is more likely to process a sequence of tasks that are related in the recursion tree. This increases the probability that the data required for the *next* task is already warm in the L3 cache (shared) or that the prefetcher patterns established by the previous task remain valid.

### 3.2. Reduced Contention
Random stealing spreads contention across all queues. "Sticky" stealing tends to pair thieves with victims for longer durations. This can reduce the coherence traffic on the queue locks themselves, as a thief isn't constantly checking (and invalidating cache lines for) every other thread's mutex.

## 4. Benchmark Verification (10M Integers)

After a full benchmark run on a 10,000,000 integer dataset (Pattern: RANDOM, Type: int32), we observed the following scaling behavior with **Adaptive Granularity + Memory-Aware Scheduling** enabled.

| Threads | Time (ms) | Speedup vs Seq | Efficiency | Comparison (Previous) |
|---------|-----------|----------------|------------|-----------------------|
| 1       | 486.75    | 1.00x          | 100%       | -                     |
| 2       | 261.21    | 1.86x          | 93%        | Slightly faster (was 265.14ms) |
| 4       | 165.22    | 2.95x          | 74%        | Identical (was 165.31ms)       |
| 8       | 119.79    | 4.06x          | 51%        | Marginal gain (was 120.96ms)   |
| 16      | 110.43    | 4.41x          | 28%        | No change (was 110.42ms)       |

### Analysis of Results
The **Sticky Victim** strategy demonstrated:
1.  **Low Thread Count (2-4)**: A slight improvement at 2 threads (~1.5%), suggesting that simplified victim selection helps slightly even with low contention.
2.  **High Thread Count (16)**: **Zero impact**. The execution time at 16 threads remained exactly stable (~110ms).

This indicates that simply "remembering" the victim thread is **insufficient** to overcome the Memory Wall at 16 threads. The bottlenecks at this scale are likely too severe for simple scheduling heuristics to fix. The "Sticky" strategy may be keeping a thief at a victim too long even after that victim has moved to a different part of the memory space, or the underlying hardware bandwidth is simply fully saturated regardless of access pattern.

## 5. Conclusion & Next Steps

While "Memory-Aware Scheduling" via Sticky Victims was theoretically sound and cost-free to implement, it yielded **neutral results** at the bottleneck point (16 threads). It effectively serves as a cleanup/modernization of the scheduler but is not the "silver bullet" for scaling.

**Decision:**
The plateau at ~4.4x speedup (on 16 cores) remains the critical hurdle.
We must proceed to **Hybrid Parallelism (Static Partitioning)**. Switching to a completely different model (Static Partitioning) at the leaves will eliminate the thread pool overhead entirely for the majority of the work, which is the only remaining way to bypass the locking bottlenecks.