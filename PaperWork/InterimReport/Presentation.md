---
marp: true
theme: default
paginate: true
size: 16:9
style: |
  section {
    font-size: 24px;
    padding: 40px;
  }
  h1 {
    font-size: 40px;
    color: #0056b3;
  }
  h2 {
    font-size: 32px;
    color: #333;
  }
  strong {
    color: #d63384;
  }
---

<!-- _class: lead -->

# Implementing Dual Pivot Quicksort in C++23
## Validating Theoretically Superior Sorting Strategies in Modern Systems

**Student:** LIN Zhanzhi
**Supervisor:** Dr. CAO Yixin
**Project:** Capstone Interim Assessment 2025/26

---

# The Status Quo vs. The Innovation

| **Standard C++ (Introsort)** | **Our Project (Dual-Pivot)** |
| :--- | :--- |
| Single Pivot | **Dual Pivots** ($P_1, P_2$) |
| 2 Partitions ($<P$, $\ge P$) | **3 Partitions** ($<P_1$, $P_1..P_2$, $>P_2$) |
| Optimized for CPU Cycles | Optimized for **Memory Bandwidth** |

> **"Scanned Elements Model"**:
> Moving elements is expensive (Memory Write). Checking them is cheap (CPU Read).
> **3-Way Partitioning = Fewer Swaps = Better Cache Efficiency.**

---

# Project Objectives

1.  ### :wrench: Modernize
    *   Create a **Generic C++23 Library**.
    *   Use Concepts (`std::sortable`) for type safety.

2.  ### :rocket: Accelerate
    *   Outperform `std::sort` on sequential benchmarks.
    *   Achieve scalable parallel performance.

3.  ### :microscope: Analyze
    *   Identify hardware limits.
    *   Investigate the **"Memory Wall"** in parallel sorting.

---

# Baseline Results: Random Data

*   **Metric:** 64-bit Integers, $N = 10^3$ to $10^7$.
*   **Result:** Consistent **10-15% Speedup** vs `std::sort`.

<div align="center">
<!-- Placeholder for graph -->
<p style="font-size: 60px; color: #aaa;"> 📈 </p>
<p><b>DPQS (Blue) < std::sort (Red)</b></p>
</div>

---

# The "Run Merger" Optimization

**Why sort what is already sorted?**

*   **Strategy:** Adaptive Run Merging (TimSort-style).
*   **Structured Data Performance:**
    *   **Organ Pipe** Distribution: **27x Speedup**
    *   **Sawtooth** Distribution: **8x Speedup**

> "We detect existing order and merge in $O(N)$ instead of re-sorting."

---

# Robustness: The "Fat Partition" Problem

*   **Challenge:** Duplicate pivots usually degrade Quicksort to $O(N^2)$.
*   **Solution:** 3-Way Partitioning clusters duplicates:
    *   Region 2 ($P_1 \le x \le P_2$) naturally absorbs equal keys.
*   **Result:** Performance is **invariant** across 10%, 50%, or 90% duplicates.

---

# Parallel Architecture: V3 Work-Stealing

**Thread Pool Design:**
*   **Global Queue?** :x: Too much locking contention.
*   **Distributed Deques?** :white_check_mark: Scalable.

**Mechanism:**
1.  Each thread has a local deque.
2.  **LIFO** processing for cache locality.
3.  **Work Stealing:** Idle threads steal from the *tail* (FIFO) of busy threads.
4.  **"Fire-and-Forget"** tasks.

---

# Strong Scaling Results

**Setup:** $N = 10,000,000$ Integers.

| Threads | Speedup (vs Sequential) | Efficiency | Phase |
| :--- | :--- | :--- | :--- |
| **2** | **2.03x** | 101% | Super-Linear |
| **4** | **3.52x** | 88% | Linear |
| **8** | **4.98x** | 62% | Diminishing |
| **16** | **5.24x** | 33% | **Saturation** |

---

# The Bottleneck: Hitting the "Memory Wall"

**Why does speedup plateau at 5.2x?**

*   **It's NOT** Software Overhead (Locks are minimal).
*   **It IS** Memory Bandwidth Saturation.
    *   Sorting is $O(N)$ Read/Write intensive.
    *   16 Threads starve the memory controller.
    *   *Adding more CPU cores cannot fix a data supply shortage.*

---

# Engineering Quality: C++23 Standard

We rely on **Modern C++ Concepts** to ensure type safety and API compatibility.

```cpp
// include/dual_pivot_quicksort.hpp

template<std::random_access_iterator RandomAccessIterator>
void dual_pivot_quicksort(RandomAccessIterator first, RandomAccessIterator last) {
    // Validated at compile-time by C++20 Concepts

    if (first >= last) return;
    // ...
```

*   **Generic:** Works with `std::vector`, `std::array`, `std::deque`.
*   **Safe:** 64-bit support preventing integer overflow.

---

# Roadmap: Semester 2

**Goal: Break the Memory Wall.**

1.  **:zap: SIMD Vectorization (AVX-512)**
    *   Use **Non-Temporal Stores** to bypass cache and write directly to RAM.
    *   Expected to double effective bandwidth.

2.  **:brain: Memory-Aware Scheduling**
    *   Prioritize task stealing based on memory locality (NUMA awareness).

3.  **:bar_chart: Advanced Benchmarking**
    *   Compare against state-of-the-art **PDQSort**.

---

# Conclusion

1.  **Sequential Success**:
    *   **15%** faster on Random Data.
    *   **27x** faster on Structured Data.
2.  **Parallel Foundation**:
    *   **5.24x** Scalability achieved.
3.  **Rigorous Analysis**:
    *   Identified **Memory Bandwidth** as the true limit.

**Next Step:** Hardware-native optimizations to push beyond the wall.

**Thank You.**
