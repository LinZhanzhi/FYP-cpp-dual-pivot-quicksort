# Chapter 5: Results and Evaluation

This chapter presents comprehensive benchmarking results comparing our dual-pivot quicksort implementation against `std::sort` across diverse data patterns, array sizes, and thread configurations. The goal is twofold: validate that adaptive optimizations deliver their promised speedups, and characterize the parallel scaling behavior to guide practical usage recommendations.

---

## 5.1 Experimental Setup

### 5.1.1 Hardware Platform

All benchmarks were conducted on a single workstation to ensure consistent thermal and power conditions:

| Component | Specification |
|-----------|---------------|
| **CPU** | Intel Core i5-12600KF (Alder Lake), Intel 7 (10 nm) process |
| **Architecture** | Hybrid: 6 Performance-cores + 4 Efficiency-cores |
| **Threads** | 16 hardware threads (P-cores: 2-way SMT; E-cores: 1 thread each) |
| **P-core L2 Cache** | 6 × 1.25 MB (private per core) |
| **E-core L2 Cache** | 2 MB (shared across 4 E-cores) |
| **L3 Cache** | 20 MB (shared across all cores) |
| **RAM** | 32 GB DDR5-6000 (dual-channel) |
| **TDP** | 125 W |

**Table 5.1**: Hardware specifications for benchmark system.

### 5.1.2 Software Environment

| Component | Version |
|-----------|---------|
| **Operating System** | Windows 11 Pro 23H2 |
| **Compiler** | g++ 13.3.0 (MinGW-w64 UCRT) |
| **C++ Standard** | C++17 |
| **Optimization Flags** | `-O2 -march=native -DNDEBUG` |
| **Profiler** | Intel VTune Profiler 2025.1 |

**Table 5.2**: Software environment for benchmarks.

**Compiler Flag Selection**: We evaluated 12 GCC optimization flag combinations to determine the optimal build configuration:

| Flag Combination | Relative Performance | Notes |
|------------------|---------------------|-------|
| `-O2` | Baseline | Standard optimization |
| `-O3` | +0.2% | No benefit for branch-heavy code |
| `-O2 -flto` | −2.5% | Regression; templates already inline |
| `-O2 -march=native` | **+1.5%** | Best; enables AVX2 memory ops |
| `-O3 -flto -march=native` | −1.8% | LTO overhead negates gains |

**Table 5.3**: Compiler flag benchmark results on 10M random integers.

Key findings:
- **`-O3` offers no benefit over `-O2`**: Sorting code is branch-heavy; `-O3`'s aggressive loop unrolling provides no advantage when branches dominate.
- **`-flto` causes 2-5% regression**: Our header-only implementation already achieves full inlining within each translation unit. Link-time optimization adds compilation overhead without enabling additional optimizations.
- **`-march=native` provides ~1.5% improvement**: Enables processor-specific instructions (AVX2 for memory operations), though the memory-bound nature of sorting limits further gains.

**Final choice**: `-O2 -march=native -DNDEBUG` — the simplest flag combination achieving best measured performance.

### 5.1.3 Benchmark Protocol

Each benchmark configuration follows a rigorous measurement protocol:

1. **Warmup Phase**: 3 iterations executed and discarded
   - Populates instruction and data caches
   - Triggers JIT compilation of any lazy-initialized code paths
   - Allows CPU frequency to stabilize (turbo boost ramp-up)

2. **Measurement Phase**: 30 timed iterations
   - Fresh data generated for each iteration (prevents sorted-data caching artifacts)
   - Timer: `std::chrono::high_resolution_clock` (nanosecond resolution on Windows)

3. **Representative Runtime**: Minimum of 30 timed iterations
   - The benchmark pipeline records all 30 samples, then uses the minimum observed runtime as the representative value
   - This follows the minimum-estimator approach: the fastest run is treated as the least perturbed measurement of the algorithm itself, with incidental system interference filtered out rather than averaged in

4. **Memory Allocation**: Pre-allocated buffers reused across iterations
   - Keeps allocation overhead outside the timed region
   - Auxiliary space (for merge operations) allocated once before timing begins

**Timing Granularity**: For small arrays (<10K elements), we batch multiple sorts per timing measurement to ensure timer resolution does not dominate. Reported time is per-sort after dividing by batch count.

### 5.1.4 Test Matrix

| Parameter | Values |
|-----------|--------|
| **Array Sizes** | 41 logarithmic steps: 1,000 → 10,000,000 |
| **Size Distribution** | 10 steps per decade (×1.26 multiplier) |
| **Data Types** | `int`, `int8_t`, `int16_t`, `double` |
| **Data Patterns** | 8 patterns (see Table 5.5) |
| **Algorithms** | 6 variants (see Table 5.6) |

**Table 5.4**: Benchmark parameter ranges.

**Total configurations**: 6 algorithms × 4 types × 8 patterns × 41 sizes = **7,872 unique benchmarks**

| Pattern | Description | Generator |
|---------|-------------|-----------|
| RANDOM | Uniform random distribution | `std::uniform_int_distribution` |
| NEARLY_SORTED | 99% sorted, 1% random swaps | Sorted then perturbed |
| REVERSE_SORTED | Descending order | `n-1, n-2, ..., 0` |
| MANY_DUPLICATES_10 | 10% unique values | `rand() % (n/10)` |
| MANY_DUPLICATES_50 | 50% unique values | `rand() % (n/2)` |
| MANY_DUPLICATES_90 | 90% unique values | `rand() % (9n/10)` |
| ORGAN_PIPE | Ascending then descending | `0,1,...,n/2,...,1,0` |
| SAWTOOTH | k ascending runs of length n/k | `i % run_length` scaled |

**Table 5.5**: Data pattern definitions.

| Algorithm | Description |
|-----------|-------------|
| `std::sort` | libstdc++ Introsort (baseline) |
| `dpqs_sequential` | Our implementation, single-threaded |
| `dpqs_parallel_2t` | Parallel with 2 threads |
| `dpqs_parallel_4t` | Parallel with 4 threads |
| `dpqs_parallel_8t` | Parallel with 8 threads |
| `dpqs_parallel_16t` | Parallel with 16 threads |

**Table 5.6**: Algorithms included in benchmark suite.

**Note on Data Types**: Performance results are presented primarily for `int`, with a dedicated subsection for `int8_t` and `int16_t` because these types activate the counting-sort optimization from Chapter 3. Benchmarks with `double` show similar scaling trends to `int` in the comparison-based path, since sorting remains dominated by data movement rather than arithmetic cost. Floating-point-specific correctness handling (NaN, −0.0) is discussed in Chapter 3, §3.3.

**Note on Baseline Selection**: We compare primarily against `std::sort` (Introsort), the canonical C++ standard library sorting algorithm. It serves as the main external baseline for the experimental results in this chapter.

### 5.1.5 Data Pattern Relevance

Each test pattern represents real-world data characteristics:

| Pattern | Real-World Source | Practical Example |
|---------|-------------------|-------------------|
| RANDOM | Hash table outputs | User IDs after hashing |
| NEARLY_SORTED | Incremental updates | Database with appended records |
| REVERSE_SORTED | Opposite sort key | Price high→low when low→high needed |
| MANY_DUPLICATES | Categorical data | Star ratings (1-5), status codes |
| ORGAN_PIPE | Time series peaks | Stock prices: morning rise, afternoon fall |
| SAWTOOTH | Pre-sorted chunks | Merging sorted log files by timestamp |

**Table 5.7**: Real-world relevance of benchmark patterns.

**Pattern Selection Rationale**: These patterns specifically target the adaptive optimizations described in Chapter 3:
- REVERSE_SORTED and ORGAN_PIPE test run detection (§3.1)
- SAWTOOTH tests adaptive merge vs. quicksort selection (§3.1)
- MANY_DUPLICATES tests Dutch National Flag partitioning (§2.3)
- NEARLY_SORTED tests the boundary between merge-friendly and random data

### 5.1.6 Reproducibility

All benchmark artifacts are included in the repository:

| Artifact | Location |
|----------|----------|
| **Implementation** | `include/dual_pivot_quicksort.hpp` |
| **Benchmark runner** | `benchmarks/src/benchmark_runner.cpp` |
| **Pattern generators** | `benchmarks/include/data_generator.hpp` |
| **Raw CSV results** | `benchmarks/results/aggregate/` |
| **Analysis scripts** | `benchmarks/analyze_report_data.py` |
| **Build instructions** | `benchmarks/README.md` |

**Table 5.8**: Reproducibility artifacts.

To reproduce benchmarks:
```bash
cd benchmarks
make benchmark_runner
./build/benchmark_runner.exe --all --output results/
```

Results are written as CSV files with columns: `algorithm`, `data_type`, `pattern`, `size`, `iteration`, `time_ns`. Analysis scripts aggregate these into summary statistics.

---

## 5.2 Sequential Performance

This section compares `dpqs_sequential` against `std::sort` across all data patterns. The algorithm paths and optimization mechanisms are detailed in Chapter 3; here we present measured results.

**Key Finding**: Adaptive optimizations provide up to **30× speedup** on structured data (organ-pipe) and ~1.1× on random data. Small integers achieve **40-70× speedup** via counting sort. One exception: nearly-sorted data shows ~0.75× due to scattered perturbations defeating run detection.

### 5.2.1 Small Integer Types (`int8_t`, `int16_t`)

For 1-byte and 2-byte integers, counting sort (§3.2) achieves near-linear runtime, while `std::sort` remains $O(n \log n)$. The gap widens with array size.

**[PLACEHOLDER: Figure 5.1 — `int8_t` Performance vs `std::sort`]**

**[PLACEHOLDER: Figure 5.2 — `int16_t` Performance vs `std::sort`]**

| Type | Array Size | `std::sort` (ms) | `dpqs_sequential` (ms) | Speedup |
|------|------------|------------------|------------------------|---------|
| `int8_t` | 1M | 26.33 | 0.38 | 69.9× |
| `int8_t` | 10M | 266.98 | 3.98 | 67.1× |
| `int16_t` | 1M | 42.66 | 1.02 | 41.8× |
| `int16_t` | 10M | 421.91 | 7.18 | 58.7× |

**Table 5.9**: Small integer type performance summary.

### 5.2.2 Random Data

Dual-pivot partitioning achieves consistent ~1.1× speedup due to reduced recursion depth ($\log_3 n$ vs $\log_2 n$).

**[PLACEHOLDER: Figure 5.3 — Random Data Performance]**

| Array Size | `std::sort` (ms) | `dpqs_sequential` (ms) | Speedup |
|------------|------------------|------------------------|---------|
| 100K | 4.38 | 3.89 | 1.13× |
| 1M | 53.21 | 45.41 | 1.17× |
| 10M | 618.51 | 552.95 | 1.12× |

**Table 5.10**: Random data performance.

### 5.2.3 Reverse-Sorted Data

O(n) in-place reversal vs O(n log n) partitioning yields **~8-10× speedup**.

**[PLACEHOLDER: Figure 5.4 — Reverse-Sorted Data Performance]**

| Array Size | `std::sort` (ms) | `dpqs_sequential` (ms) | Speedup |
|------------|------------------|------------------------|---------|
| 100K | 0.35 | 0.05 | 7.6× |
| 1M | 4.08 | 0.54 | 7.6× |
| 10M | 53.16 | 5.50 | 9.7× |

**Table 5.11**: Reverse-sorted data performance.

### 5.2.4 Organ-Pipe Data

Two-run detection and O(n) merge yields **27-45× speedup**—the largest across all patterns.

**[PLACEHOLDER: Figure 5.5 — Organ-Pipe Data Performance]**

| Array Size | `std::sort` (ms) | `dpqs_sequential` (ms) | Speedup |
|------------|------------------|------------------------|---------|
| 100K | 4.43 | 0.10 | 45.2× |
| 1M | 49.13 | 1.82 | 27.1× |
| 10M | 587.11 | 19.44 | 30.2× |

**Table 5.12**: Organ-pipe data performance.

### 5.2.5 Sawtooth Data

O(n log k) k-way merge yields ~10× speedup; best parallel scaling potential.

**[PLACEHOLDER: Figure 5.6 — Sawtooth Data Performance]**

| Array Size | Runs (k) | `std::sort` (ms) | `dpqs_sequential` (ms) | Speedup |
|------------|----------|------------------|------------------------|---------|
| 100K | ~316 | 1.87 | 0.16 | 12.0× |
| 1M | ~1000 | 21.29 | 2.32 | 9.2× |
| 10M | ~3162 | 241.52 | 24.91 | 9.7× |

**Table 5.13**: Sawtooth data performance.

### 5.2.6 Nearly-Sorted Data

**Finding**:

**Explanation**: The 1% random perturbation creates short runs that fail the quality heuristics, triggering fallback to dual-pivot quicksort. Meanwhile, `std::sort`'s Introsort uses insertion sort for small partitions, which excels on nearly-sorted segments. This represents a known limitation: our run merger requires contiguous runs, while `std::sort`'s insertion sort handles scattered disorder more gracefully.

**[PLACEHOLDER: Figure 5.7 — Nearly-Sorted Data Performance]**

| Array Size | `std::sort` (ms) | `dpqs_sequential` (ms) | Ratio |
|------------|------------------|------------------------|-------|
| 100K | 1.62 | 2.15 | 0.75× |
| 1M | 19.72 | 26.19 | 0.75× |
| 10M | 230.37 | 310.12 | 0.74× |

**Table 5.14**: Nearly-sorted data performance (DPQS slower).

### 5.2.7 Duplicate-Heavy Data

Dutch National Flag partitioning handles duplicates efficiently; no degradation at any duplicate level.

**[PLACEHOLDER: Figure 5.8 — Duplicate-Heavy Data Performance]**

| Unique % | Array Size | `std::sort` (ms) | `dpqs_sequential` (ms) | Speedup |
|----------|------------|------------------|------------------------|---------|
| 10% | 10M | 589.49 | 542.38 | 1.09× |
| 50% | 10M | 631.75 | 551.87 | 1.14× |
| 90% | 10M | 640.84 | 542.18 | 1.18× |

**Table 5.15**: Duplicate-heavy data performance.

### 5.2.8 Sequential Performance Summary

| Pattern | Complexity | Speedup vs `std::sort` |
|---------|------------|------------------------|
| RANDOM | O(n log n) | **1.1×** |
| NEARLY_SORTED | O(n log n) | **0.75×** (regression) |
| REVERSE_SORTED | O(n) | **8-10×** |
| ORGAN_PIPE | O(n) | **27-45×** |
| SAWTOOTH | O(n log k) | **9-12×** |
| MANY_DUPLICATES | O(n log n) | **1.1×** |
| `int8_t` | O(n) | **42-70×** |
| `int16_t` | O(n) | **42-59×** |

**Table 5.16**: Sequential performance summary.

**Note on NEARLY_SORTED regression**: This is the only pattern where `std::sort` outperforms DPQS. The scattered perturbation defeats run detection but suits Introsort's insertion-sort finisher. Future work could add a local-disorder detector to trigger insertion sort on nearly-sorted segments.

---

## 5.3 Parallel Scaling Analysis

This section presents parallel speedup measurements on 10M random integers. VTune Profiler identifies bottlenecks at each thread count.

**Key Finding**: 5.76× speedup at 16 threads; bottleneck shifts from branch misprediction (1-4T) to L3 cache contention (8-16T).

### 5.3.1 Speedup Results

| Threads | Runtime (ms) | Speedup | Efficiency | Primary Bottleneck |
|---------|--------------|---------|------------|-------------------|
| 1 | 553 | 1.00× | 100% | Branch Mispredict (35%) |
| 2 | 279 | 1.98× | 99% | Branch Mispredict (36%) |
| 4 | 162 | 3.41× | 85% | Branch Mispredict (32%) |
| 8 | 108 | 5.12× | 64% | L3 Cache (19%) + Branch (29%) |
| 16 | 96 | 5.76× | 36% | L3 Cache (38%) + Sync (48%) |

**Table 5.17**: Parallel scaling results on 10M random integers.

**[PLACEHOLDER: Figure 5.9 — Parallel Speedup Curve]**

### 5.3.2 VTune Bottleneck Analysis (16 Threads)

Intel VTune Profiler uses a **Top-Down Microarchitecture Analysis** methodology to identify CPU bottlenecks. The analysis is based on **pipeline slots**—opportunities for the CPU to complete useful work. A modern superscalar processor can retire multiple instructions per cycle (4–6 on Intel Alder Lake). Each slot represents one instruction that *could* have completed; VTune classifies what actually happened to each slot.

| Category | Slots | Definition | Impact on Parallel Sorting |
|----------|-------|------------|---------------------------|
| **Memory Bound** | 41.1% | Slot stalled waiting for data from cache or RAM | 16 threads exhaust 20 MB L3 cache; each thread evicts lines needed by others |
| **Bad Speculation** | 25.7% | Slot wasted on instructions later discarded (branch misprediction) | Element comparisons are data-dependent; branch predictors fail on random data |
| **Front-End Bound** | 19.8% | Slot stalled because CPU could not fetch/decode instructions fast enough | Many threads cause instruction cache pressure; complex template code hurts decoding |
| **Retiring** | 9.8% | Slot performed useful work—instruction completed successfully | Only ~1 in 10 slots actually sorts data |

**Table 5.18**: VTune pipeline slot breakdown at 16 threads with category definitions.

**[PLACEHOLDER: Figure 5.10 — VTune Pipeline Breakdown]**

**Detailed Impact Analysis**:

- **Memory Bound (41.1%)**: The dominant bottleneck. Sorting 10M integers (40 MB) with 16 threads means each thread processes ~2.5 MB partitions. The 20 MB shared L3 cache cannot hold all active working sets simultaneously. When thread A accesses its partition, it evicts cache lines that thread B needs, forcing B to re-fetch from RAM (100+ cycle penalty). This "cache thrashing" worsens non-linearly with thread count—doubling threads more than doubles contention.

- **Bad Speculation (25.7%)**: Comparison-based sorting has inherent branch unpredictability. Each `if (a[i] < pivot)` depends on data values unknown until runtime. Modern branch predictors achieve ~95% accuracy on predictable patterns, but random data reduces this to ~50%—no better than guessing. Each misprediction costs 15–20 cycles to flush the pipeline and restart. This overhead is unavoidable for comparison sorts on random data.

- **Front-End Bound (19.8%)**: Two factors contribute: (1) 16 threads compete for the instruction cache (32 KB per P-core, shared by 2 hyperthreads), and (2) C++ template-heavy code generates larger instruction footprints. When the CPU cannot fetch the next instruction in time, the entire pipeline stalls waiting for code rather than data.

- **Retiring (9.8%)**: The productive fraction. Only ~10% of the CPU's theoretical throughput performs actual sorting operations (comparisons, swaps, pointer arithmetic). The remaining 90% is overhead from the three stall categories above.

**Critical Finding**: At 16 threads, 90% of CPU capacity is wasted on stalls and mispredictions. This quantitatively explains the diminishing returns: going from 8 to 16 threads doubles the hardware resources but increases memory contention faster than it adds useful parallelism, yielding only +12% speedup.

### 5.3.3 Amdahl's Law

From 5.76× speedup at 16 threads: serial fraction ≈ 11.2%, maximum theoretical speedup ≈ 8.92×.

| Serial Component | Estimated Impact |
|------------------|------------------|
| Initial partitioning | ~4% |
| Synchronization spin | ~6% |
| Memory serialization | ~3% |
| Work-stealing overhead | ~1% |

**Table 5.19**: Serial overhead breakdown.

### 5.3.4 Practical Recommendation

| Configuration | Speedup | Use Case |
|---------------|---------|----------|
| 4 threads | 3.41× | Shared server—leaves CPU headroom |
| **8 threads** | **5.12×** | **Recommended**—best efficiency |
| 16 threads | 5.76× | Latency-critical, dedicated system |

**Table 5.20**: Thread configuration guide.

Beyond 8 threads, diminishing returns: 4T→8T gains +50%; 8T→16T gains only +12% while doubling L3 contention.

---

## 5.4 Pattern-Specific Results

*[Section to be completed with benchmark data]*

---

## 5.5 Space Complexity Validation

*[Section to be completed]*

---

## 5.6 Correctness Verification

*[Section to be completed]*

---

## 5.7 Chapter Summary

*[Section to be completed]*
