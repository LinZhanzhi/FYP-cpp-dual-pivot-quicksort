# Meeting 04 Preparation: First Supervisor Meeting

**Date:** [Insert Date]
**Student:** [Your Name] ([Student ID])
**Project:** Optimized Dual-Pivot Quicksort (Sequential & Parallel)

---

## 2. Meeting Agenda

1.  **Project Overview**: Brief recap of objectives (Optimizing DPQS for modern hardware).
2.  **Comprehensive Progress Report (Completed Modules & Trials)**:
    *   **Algorithmic Core**:
        *   *Dual-Pivot Implementation*: 3-way partitioning with 20% fewer swaps.
        *   *Adaptive Run Merging*: TimSort-style optimization for pre-sorted data.
        *   *Introsort Strategy*: Fallback to HeapSort to guarantee $O(N \log N)$ worst-case.
        *   *Small Integer Optimization*: Counting Sort for `byte`/`short` types.
        *   *Threshold Tuning*: Empirical tuning of Insertion Sort thresholds (32/48).
    *   **Robustness & Safety**:
        *   *Namespace Safety*: Encapsulation to prevent symbol collisions.
        *   *Integer Overflow Fixes*: Safe arithmetic for large array indices.
        *   *Generic Support*: Custom comparators and non-contiguous iterator handling.
    *   **Parallel Architecture**:
        *   *Work-Stealing Thread Pool*: Dynamic load balancing.
        *   *Contention Analysis*: Mutex optimization for high-concurrency scenarios.
    *   **Benchmarking Infrastructure**:
        *   *Custom Harness*: Justification for "Destructive Testing" support (vs Google Benchmark).
        *   *Statistical Rigor*: Sample size analysis and outlier filtering.
3.  **Key Results**:
    *   Sequential DPQS beating `std::sort` on random integers.
    *   Parallel scaling analysis on multi-core systems.
4.  **Technical Decisions**: Justification for prioritizing Parallelism over SIMD for Semester 1.
5.  **Interim Report**: Review of proposed structure and timeline.

---

## 3. Progress Summary (The "One-Pager")

### A. Achievements to Date
*   **Core Implementation**:
    *   Implemented a robust Dual-Pivot Quicksort in C++ with **3-way partitioning**.
    *   Developed a hybrid Parallel Sort using `std::thread` and a **work-stealing queue**.
*   **Advanced Optimizations**:
    *   **Adaptive Run Merging**: Detects and merges existing runs, boosting performance on partially sorted data.
    *   **Counting Sort Integration**: Automatically switches to Counting Sort for small integral types (`char`, `short`), achieving $O(N)$ performance.
    *   **Tail Recursion Elimination**: Optimized the largest partition loop to minimize stack depth.
    *   **Introsort Fallback**: Implemented depth-tracking to switch to HeapSort, preventing recursion depth attacks.
*   **Infrastructure**:
    *   Built a comprehensive Benchmarking Suite with a Web UI (Vue.js) for real-time performance visualization.
    *   Added "Speedup Analysis" to automatically calculate performance relative to `std::sort`.
    *   Conducted **Sample Size Analysis** to determine the optimal number of iterations for statistical significance.

### B. Current Challenges
*   **SIMD Complexity**: Implementing AVX-512 vectorization is complex and may yield diminishing returns compared to multi-threading. *Proposal: Keep as a stretch goal for Semester 2.*
*   **Parallel Overhead**: Managing thread overhead for small-to-medium arrays (currently handled by falling back to sequential sort).

### C. Next Steps (Jan - Feb)
*   Finalize Interim Report (Drafting in progress).
*   Refine Parallel Load Balancing (Optimize the work-stealing deque).
*   Expand benchmarks to include skewed/sorted distributions (Pattern analysis).

---

## 4. Materials to Bring

1.  **Speedup Graphs**:
    *   Printout or laptop demo of the "Speedup Analysis" tab showing DPQS > 1.0x speedup vs `std::sort`.
2.  **Code Structure Diagram**:
    *   Simple block diagram showing: `Sorter` Class -> `Partition` Logic -> `Insertion Sort` Fallback.
3.  **Interim Report Outline**:
    *   Copy of `PaperWork/InterimReport/outline.md`.

---

## 5. Questions for Supervisor

1.  **Scope Validation**:
    *   "I have focused heavily on algorithmic tuning and parallelism. Is this sufficient for the Interim Assessment, or is the SIMD implementation strictly required *before* the interim report?"
2.  **Evaluation Metrics**:
    *   "I am currently benchmarking against `std::sort` and `std::stable_sort`. Should I also compare against other parallel libraries like Intel TBB or OpenMP `parallel_sort`?"
3.  **Report Structure**:
    *   "I have drafted an outline for the Interim Report. Does this structure align with your expectations for the 'Methodology' section?"

---

## 6. Draft Timeline (Semester 2)

*   **Jan 15**: Submit Interim Report.
*   **Jan 30**: Interim Presentation.
*   **Feb**: Deep dive into SIMD (AVX-512) feasibility.
*   **Mar**: Advanced Parallel optimizations (Lock-free structures).
*   **Apr**: Final Benchmarking & Final Report Writing.

---

## 7. Technical Deep Dive: The Implementation Workflow

*Use this section to explain the "Journey of a Sort Call" through the codebase, demonstrating how all files interact.*

### Step 1: The Entry Gate (API & Dispatch)
*Everything starts here. The user calls `dual_pivot::sort()`, and the system decides the best strategy.*
1.  **The Public Interface**:
    *   `include/dual_pivot_quicksort.hpp`: The main entry point. It checks array size and type.
    *   **Decision Logic**:
        *   If `char` or `short` -> Dispatch to **Counting Sort** (`include/dpqs/counting_sort.hpp`).
        *   If `float`/`double` -> Pre-process NaNs/Zeros (`include/dpqs/float_sort.hpp`).
        *   If Large Array -> Dispatch to **Parallel Engine**.
        *   Otherwise -> Dispatch to **Sequential Engine**.
    *   `include/dpqs/iterator_sort.hpp`: Provides the STL-compatible wrapper (`begin`, `end`) for standard usage.

### Step 2: The Sequential Engine (The Core Logic)
*This is the heart of the algorithm, running on a single thread.*
1.  **The Orchestrator**:
    *   `include/dpqs/sequential_sorters.hpp`: Contains the main recursive function `sort_sequential`. It manages the flow:
        *   Checks for **Run Merging** opportunities (`include/dpqs/run_merger.hpp`).
        *   Checks recursion depth for **Introsort** fallback (`include/dpqs/heap_sort.hpp`).
        *   Selects pivots using a 5-element network.
2.  **The Workhorse (Partitioning)**:
    *   `include/dpqs/partition.hpp`: Implements the actual 3-way partitioning logic (swapping elements around 2 pivots).
3.  **Base Case Handlers**:
    *   When the array slice gets small (< 48 elements), we stop recursing.
    *   `include/dpqs/insertion_sort.hpp`: Handles these small chunks using optimized Insertion Sort (with prefetching).

### Step 3: The Parallel Engine (Scaling Up)
*When the array is large, we distribute work across cores.*
1.  **The Task Manager**:
    *   `include/dpqs/parallel/threadpool.hpp`: Our custom **Work-Stealing Thread Pool**. Threads take tasks from their own deque (LIFO) or steal from others (FIFO).
2.  **The Parallel Logic**:
    *   `include/dpqs/parallel/parallel_sort.hpp`: The parallel version of the main loop. It forks the largest partitions to the thread pool and processes the smallest partition immediately (Hybrid Strategy).
3.  **Advanced Framework (Future-Proofing)**:
    *   *Infrastructure for complex parallel merge operations (currently optional but integrated):*
    *   `include/dpqs/parallel/completer.hpp`: Tracks task completion (Java ForkJoin port).
    *   `include/dpqs/parallel/sorter.hpp` & `merger.hpp`: Generic tasks for parallel merge sort.
    *   `include/dpqs/parallel/buffer_manager.hpp`: Manages thread-local memory to avoid allocation overhead.

### Step 4: The Foundation (Utilities & Config)
*These files support the entire system.*
1.  **Configuration**:
    *   `include/dpqs/constants.hpp`: Centralized tuning knobs (Thresholds: 32 for Insertion, 4096 for Parallel).
2.  **Type System**:
    *   `include/dpqs/types.hpp`: Defines `ArrayPointer` to handle generic types safely without void* casting.
3.  **Helpers**:
    *   `include/dpqs/utils.hpp`: Macros for branch prediction (`LIKELY`/`UNLIKELY`) and prefetching.
    *   `include/dpqs/merge_ops.hpp`: Low-level merge primitives used by both sequential and parallel mergers.
1.  **Safety & Generics**:
    *   Full STL compatibility (Iterators, Custom Comparators).
    *   Namespace encapsulation (`dual_pivot::`) to prevent symbol collisions.
2.  **Benchmarking Suite**:
    *   **Destructive Testing**: Custom harness resets arrays between runs (unlike Google Benchmark).
    *   **Visualization**: Web-based UI to plot Speedup vs `std::sort` in real-time.