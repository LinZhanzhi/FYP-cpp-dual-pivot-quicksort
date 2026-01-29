# Discussion & Future Improvements

## 8.1. Parallelization Refinement
While the Work-Stealing implementation successfully enables parallel scaling, analysis identifies key areas for future optimization:

- [x] **Grain Size Tuning**: The current static threshold (4096) is effective but rigid. Implemented adaptive granularity, dynamically adjusting the threshold based on current system load (queue depth) to balance overhead vs. load balancing.
- [x] **Memory-Aware Scheduling**: To address the bandwidth bottlenecks identified in Section 8.3.2, the scheduler could be enhanced to favor cache-affine task stealing (stealing tasks that operate on adjacent memory regions) rather than random victim selection.
    *   *Implementation:* Added "Sticky Victim" strategy (workers default to stealing from the last successful victim to preserve locality).
- [ ] **Hybrid Parallelism**: Exploring a hybrid model that switches between "Work Stealing" (for load balancing) and "Static Partitioning" (for strict data locality) during the deeper recursion levels where L2/L3 cache misses become dominant.

## 8.2. Advanced Optimizations
Beyond threading refinements, several low-level optimizations are planned to mitigate the hardware limits associated with the "Memory Wall":

*   **Vectorization & Memory Efficiency (SIMD)**:
    To address the bandwidth saturation observed at 16 threads, future development will explore AVX2/AVX-512 vectorization. Specifically, implementing Non-Temporal Stores (streaming stores) allows writing partitioned data directly to main memory, bypassing the cache hierarchy [11]. This prevents "cache pollution" where write-once data evicts useful read-only data (pivots), potentially doubling effective memory throughput.

*   **Block-Based Partitioning**:
    Adapting the strategy from BlockQuicksort, the partitioning phase can be restructured to process elements in small, cache-resident blocks [12]. This hides memory latency by overlapping computation with prefetching, ensuring the CPU execution units remain saturated.

*   **Explicit Memory Management**:
    The current usage of `std::function` incurs heap allocation overhead for task capture [13]. A proposed optimization is to implement Linear Allocators or Pre-allocated Ring Buffers for task storage. Eliminating dynamic malloc/free calls from the hot path is expected to significantly improve efficiency for fine-grained tasks (subarrays near the 4096 threshold).
