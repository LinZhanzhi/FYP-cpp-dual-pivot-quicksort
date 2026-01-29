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

## 4. Initial Verification

A preliminary smoke test was conducted to ensure stability and functionality.

*   **Test**: 1,000,000 Integers (Random), 4 Threads.
*   **Metric**: Execution Time (Representative).
*   **Result**:
    *   *Adaptive Only*: ~14.34ms
    *   *Adaptive + Memory Aware*: ~13.55ms
*   **Improvement**: ~5.5% speedup on small data.

While 5% is modest, it is significant for a simple heuristic change. The primary benefits are expected to scale better on larger datasets (100M+) where memory bandwidth is the primary bottleneck.

## 5. Conclusion & Next Steps

The "Sticky Victim" strategy has been successfully integrated into the logic. It provides a zero-cost optimization that aligns task scheduling with the physical reality of memory hierarchies.

**Next Steps:**
1.  **Full Benchmark**: Re-run the scaling analysis (1M, 10M, 100M) to quantify the impact at 16 threads.
2.  **Hybrid Approach**: If scaling limitation persists, we will move to the final planned optimization: **Hybrid Parallelism** (switching to static partitioning for leaf nodes).