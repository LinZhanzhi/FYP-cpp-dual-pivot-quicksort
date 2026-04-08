# VTune Bottleneck Analysis: Memory vs CPU Bound

**Date**: April 8, 2026
**System**: Intel Raptor Lake (24 logical cores: 8 P-cores + 8 E-cores)
**Dataset**: 10M random integers (38 MB)
**Tool**: Intel VTune Profiler 2025.10

## Executive Summary

This analysis uses Intel VTune to determine whether our dual-pivot quicksort implementation is **memory-bound** or **CPU-bound**, and how the bottleneck profile changes across different thread counts.

---

## Baseline Analysis (16 Threads)

### Pipeline Slot Breakdown

| Category | P-core | E-core | Impact |
|----------|--------|--------|--------|
| **Memory Bound** | 41.1% | 38.2% | 🔥 HIGH |
| Back-End Bound (total) | 44.7% | - | HIGH |
| Bad Speculation | 25.7% | - | ⚠️ MEDIUM |
| Front-End Bound | 19.8% | - | MEDIUM |
| Retiring (Useful Work) | 9.8% | - | LOW |

### Memory Hierarchy Breakdown

| Cache Level | % of Clockticks | Analysis |
|-------------|-----------------|----------|
| **L3 Bound** | 37.9% | 🔥 **PRIMARY BOTTLENECK** |
| L1 Bound | 13.1% | Secondary |
| L2 Bound | 0.3% | Negligible |
| DRAM Bound | 0.2% | Negligible |

### Key Metrics

| Metric | Value | Assessment |
|--------|-------|------------|
| CPI (Cycles Per Instruction) | 1.73 | Poor (ideal < 1.0) |
| Branch Misprediction | 18.6% | ⚠️ High |
| Spin Time (Thread Waiting) | 48.5% | 🔥 Critical |
| DRAM Bandwidth Used | 5.6 / 72 GB/s | 7.8% utilized |
| LLC Miss Count | 1.1M | Low |

### Hotspot Functions

| Function | CPU Time | % of Total |
|----------|----------|------------|
| `sched_yield` (thread sync) | 12.05s | 37.8% |
| `partition_dual_pivot` | 5.51s | 17.3% |
| `partition_dual_pivot` (variant) | 3.04s | 9.5% |
| `mixed_insertion_sort` | 2.71s | 8.5% |
| `pthread_mutex_trylock` | 2.34s | 7.3% |

### Diagnosis

**Primary Bottleneck**: L3 Cache Contention + Thread Synchronization

The algorithm is **L3-cache-bound**, not DRAM-bound or purely CPU-bound:
- 16 threads contending for shared L3 cache
- 38 MB dataset fits in L3 but causes cache-line thrashing
- ~50% of CPU time spent on synchronization overhead

---

## Thread Scaling Analysis

### Test Configuration
- Array size: 10,000,000 integers (38 MB)
- Iterations: 20 per test
- VTune collection: `uarch-exploration`
- System: Intel Raptor Lake (8 P-cores + 8 E-cores, 24 logical)

### Results by Thread Count

| Threads | Avg Time (ms) | Speedup | CPI | Memory Bound % | L3 Bound % | Branch Mispredict % | Back-End Bound % |
|---------|---------------|---------|-----|----------------|------------|---------------------|------------------|
| 1 | 508 | 1.0x | 0.889 | 2.0% | 0.1% | 35.0% | 8.5% |
| 2 | 265 | 1.92x | 0.855 | 2.6% | 0.8% | 36.4% | 11.8% |
| 4 | 154 | 3.30x | 0.919 | 10.0% | 5.0% | 31.9% | 19.7% |
| 8 | 110 | 4.62x | 1.134 | 23.8% | 19.1% | 28.7% | 31.9% |
| 16 | 98 | 5.18x | 1.729 | 41.1% | 37.9% | 18.6% | 44.7% |

### Scaling Efficiency

| Threads | Ideal Speedup | Actual Speedup | Efficiency |
|---------|---------------|----------------|------------|
| 1 | 1.0x | 1.0x | 100% |
| 2 | 2.0x | 1.92x | 96% |
| 4 | 4.0x | 3.30x | 82% |
| 8 | 8.0x | 4.62x | 58% |
| 16 | 16.0x | 5.18x | 32% |

---

## Detailed Results by Thread Count

### 1 Thread
```
Elapsed Time: 10.74s (20 iterations)
Average Sort Time: 508 ms
CPI: 0.889 (excellent)

Pipeline Breakdown (P-core):
├── Retiring:        13.4%
├── Front-End Bound: 31.8%
├── Bad Speculation: 46.4%  ← PRIMARY BOTTLENECK
│   └── Branch Mispredict: 35.0%
└── Back-End Bound:   8.5%
    └── Memory Bound:  2.0%
        ├── L1 Bound: 11.4%
        ├── L3 Bound:  0.1%
        └── DRAM Bound: 0.6%
```

**Analysis**: At single thread, the algorithm is **CPU-bound by branch misprediction**. The dual-pivot partitioning creates highly unpredictable branches when comparing random elements against two pivots.

### 2 Threads
```
Elapsed Time: 5.62s (20 iterations)
Average Sort Time: 265 ms
CPI: 0.855 (excellent)

Pipeline Breakdown (P-core):
├── Retiring:        16.6%
├── Front-End Bound: 36.5%
├── Bad Speculation: 35.1%  ← Still dominant
│   └── Branch Mispredict: 36.4%
└── Back-End Bound:  11.8%
    └── Memory Bound:  2.6%
        ├── L1 Bound: 11.4%
        ├── L3 Bound:  0.8%
        └── DRAM Bound: 0.7%
```

**Analysis**: Near-linear scaling (96% efficiency). Still **branch-misprediction bound**. Memory contention minimal.

### 4 Threads
```
Elapsed Time: 3.28s (20 iterations)
Average Sort Time: 154 ms
CPI: 0.919 (good)

Pipeline Breakdown (P-core):
├── Retiring:        15.2%
├── Front-End Bound: 35.6%
├── Bad Speculation: 29.5%  ← Decreasing
│   └── Branch Mispredict: 31.9%
└── Back-End Bound:  19.7%  ← Growing
    └── Memory Bound: 10.0%
        ├── L1 Bound: 13.0%
        ├── L3 Bound:  5.0%
        └── DRAM Bound: 0.9%
```

**Analysis**: Good scaling (82% efficiency). **Transition point** - memory bound starting to grow. L3 cache contention appearing.

### 8 Threads
```
Elapsed Time: 2.37s (20 iterations)
Average Sort Time: 110 ms
CPI: 1.134 (degrading)

Pipeline Breakdown (P-core):
├── Retiring:        12.7%
├── Front-End Bound: 29.5%
├── Bad Speculation: 25.9%
│   └── Branch Mispredict: 28.7%
└── Back-End Bound:  31.9%  ← Now dominant
    └── Memory Bound: 23.8%  ← Significant
        ├── L1 Bound:  8.4%
        ├── L3 Bound: 19.1%  ← Major factor
        └── DRAM Bound: 0.5%
```

**Analysis**: Scaling efficiency drops to 58%. **Memory-bound** now rivals branch misprediction. L3 cache contention significant at 19.1%.

### 16 Threads (Baseline)
```
Elapsed Time: 2.35s (20 iterations)
Average Sort Time: 98 ms
CPI: 1.729 (poor)

Pipeline Breakdown (P-core):
├── Retiring:         9.8%
├── Front-End Bound: 19.8%
├── Bad Speculation: 25.7%
│   └── Branch Mispredict: 18.6%
└── Back-End Bound:  44.7%  ← PRIMARY BOTTLENECK
    └── Memory Bound: 41.1%  ← CRITICAL
        ├── L1 Bound: 13.1%
        ├── L3 Bound: 37.9%  ← SEVERE
        └── DRAM Bound: 0.2%

Spin Time: 48.5% of CPU time (thread waiting)
```

**Analysis**: Only 32% scaling efficiency. **Severely L3-cache-bound**. Threads spend ~50% of time waiting for locks/synchronization.

---

## Observations

### Key Findings

1. **Bottleneck Transition**: The algorithm transitions from **branch-misprediction bound** (1-2 threads) to **L3-cache bound** (8-16 threads).

2. **L3 Cache Is The Scaling Killer**:
   - 1 thread: 0.1% L3 bound
   - 8 threads: 19.1% L3 bound
   - 16 threads: 37.9% L3 bound (380x increase!)

3. **CPI Degradation**:
   ```
   1 thread:  0.889 CPI (excellent)
   16 threads: 1.729 CPI (poor) - 94% worse!
   ```

4. **Branch Misprediction Paradox**: Misprediction % actually DECREASES with more threads (35% → 18.6%), but it's masked by memory stalls taking over.

5. **DRAM Is NOT The Bottleneck**: DRAM bound stays < 1% at all thread counts. The 38MB dataset fits in L3 cache.

6. **Optimal Thread Count**: Based on efficiency vs. performance:
   - **4 threads**: Best efficiency (82%) with 3.3x speedup
   - **8 threads**: Sweet spot - 4.6x speedup before severe degradation
   - **16 threads**: Diminishing returns - only 5.2x speedup for 16 threads

### Visual: Bottleneck Shift

```
Threads:  1     2     4     8    16
          │     │     │     │     │
Branch    ██████████████████████████ ← Dominates at low threads
Mispredict│     │     │████████████│
          │     │     │     │     │
Memory    ░     ░░    ██████░░░░░░░░ ← Takes over at high threads
Bound     │     │     │████████████████████████████████████████
          │     │     │     │     │
L3 Cache  ·     ·     ░░░░░░████████████████████████████████████
Bound     │     │     │     │████████████████████████████████████
```

---

## Recommendations

Based on VTune analysis, the following optimizations should be considered:

### For Low Thread Counts (1-4 threads) - Address Branch Misprediction

1. **Branchless Partitioning**
   - Use conditional moves (`cmov`) instead of branches for pivot comparisons
   - Example: `dest[mask] = val;` using SIMD gather/scatter

2. **Profile-Guided Optimization (PGO)**
   - Compile with `-fprofile-generate`, run workload, rebuild with `-fprofile-use`
   - Helps CPU branch predictor with likely/unlikely hints

3. **Three-Way Partitioning Variant**
   - Consider BlockQuicksort-style blocking to reduce branch frequency

### For High Thread Counts (8-16 threads) - Address L3 Cache Contention

1. **Reduce Task Granularity**
   - Increase minimum partition size for parallel tasks
   - Avoid spawning tasks for small subarrays that thrash cache

2. **NUMA-Aware Allocation** (if applicable)
   - Pin threads to cores and allocate memory locally
   - Reduce cross-socket memory traffic

3. **Work Stealing Optimization**
   - Use lock-free deque instead of mutex-based task queue
   - Reduce lock contention (currently 37.8% time in `sched_yield`)

4. **Consider Fewer Threads**
   - 8 threads achieves 4.6x speedup with much better efficiency
   - 16 threads only adds 12% more speedup but doubles contention

### General Optimizations

1. **Cache Line Alignment**
   - Ensure pivot storage and per-thread data are cache-line aligned (64 bytes)
   - Avoid false sharing between threads

2. **Prefetching**
   - Add explicit prefetch hints for sequential array access during partitioning
   - `__builtin_prefetch(&arr[i + 64])`

3. **Memory Access Pattern**
   - Consider cache-oblivious merge patterns
   - Block-based processing to improve L2 hit rate

---

## Conclusion

**Primary Bottleneck**: The dual-pivot quicksort implementation exhibits a **bottleneck transition**:

| Thread Count | Primary Bottleneck | Secondary |
|--------------|-------------------|-----------|
| 1-2 | Branch Misprediction (35%) | Front-End Bound |
| 4-8 | Mixed (Branch + L3 Cache) | Memory Latency |
| 16+ | L3 Cache Contention (38%) | Synchronization |

**Key Insight**: At high thread counts, the 38MB dataset causes L3 cache thrashing between cores competing for the same cache lines. The algorithm is NOT DRAM-bandwidth bound (only 7.8% of max bandwidth used).

**Recommendation**: For this workload size (10M integers), **8 threads** provides the best performance/efficiency tradeoff. For larger datasets that exceed L3 cache size, DRAM bandwidth would become a factor.

---

## Appendix: VTune Commands Used

```powershell
# Compile benchmark
g++ -std=c++20 -O2 -g -pthread -I../include vtune_profile.cpp -o vtune_profile.exe

# Microarchitecture Exploration (primary analysis)
vtune -collect uarch-exploration -result-dir vtune_<N>threads -- ./vtune_profile.exe <N>

# Memory Access Analysis (for bandwidth metrics)
vtune -collect memory-access -result-dir vtune_memory_results -- ./vtune_profile.exe

# Hotspots Analysis (for function-level profiling)
vtune -collect hotspots -result-dir vtune_hotspots_results -- ./vtune_profile.exe

# Generate Summary Report
vtune -report summary -result-dir <result_dir>
```

## Appendix: Raw Data

### Performance by Thread Count
```
Threads  Time(ms)  Speedup   CPI    MemBound%  L3Bound%  BranchMiss%
----------------------------------------------------------------------
   1       508      1.00x   0.889      2.0%      0.1%      35.0%
   2       265      1.92x   0.855      2.6%      0.8%      36.4%
   4       154      3.30x   0.919     10.0%      5.0%      31.9%
   8       110      4.62x   1.134     23.8%     19.1%      28.7%
  16        98      5.18x   1.729     41.1%     37.9%      18.6%
```

### Scaling Trend Visualization
```
Speedup vs Threads:
16 ┤
   │                                              Ideal
14 ┤                                          ╱
   │                                       ╱
12 ┤                                    ╱
   │                                 ╱
10 ┤                              ╱
   │                           ╱
 8 ┤                        ╱          Actual
   │                     ╱        ■
 6 ┤                  ╱      ■
   │               ╱    ■
 4 ┤            ╱   ■
   │         ╱  ■
 2 ┤      ╱ ■
   │   ╱■
 0 ├─■─────────────────────────────────────────
   1    2    4    8   16
                Threads
```
