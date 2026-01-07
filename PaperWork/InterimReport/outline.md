# Interim Report Outline
**Project Title:** Implementing Dual Pivot Quicksort in C++23 and Comparing its Performance with Standard Sorting Libraries
**Student:** LIN Zhanzhi (22097456D)
**Supervisor:** CAO Yixin

---

## 1. Cover Page (Appendix A)
*   Standard Capstone Project Cover Page.

## 2. Abstract
*   Summary of the move from Single-Pivot to Dual-Pivot Quicksort.
*   Goal: Investigate if C++ standard libraries can benefit from Dual-Pivot strategies used in Java.
*   Current Status: Generic C++23 implementation complete; benchmarking framework established; preliminary results show promise on large datasets.

## 3. Table of Contents

## 4. List of Tables and Figures
*   List of Figures (e.g., Partitioning diagrams, Performance graphs).
*   List of Tables (e.g., Complexity comparisons, Benchmark results).

## 5. Introduction
*   **5.1 Background**
    *   The dominance of Quicksort in system libraries.
    *   Yaroslavskiy's 2009 Dual-Pivot innovation and its adoption in Java 7 (Ref: `docs/DualPivotQuicksort.md`).
    *   The current state of C++ `std::sort` (Introsort) (Ref: `report/introsort-strategy.md`).
*   **5.2 Problem Statement**
    *   Lack of a modern, standard-compliant C++ implementation of Dual-Pivot Quicksort (Ref: `docs/implementation_report.md`).
    *   Need to verify if theoretical gains (reduced cache misses) translate to C++ performance.
*   **5.3 Objectives**
    *   1. Implement a robust, generic Dual-Pivot Quicksort using C++23 Concepts (Ref: `docs/cppPlan.md`).
    *   2. Develop a reliable benchmarking framework for destructive sorting tests.
    *   3. Compare performance against `std::sort` and `std::stable_sort`.
    *   4. Analyze memory bandwidth and cache behavior (Ref: `report/memory_strategy_analysis.md`).
*   **5.4 Scope**
    *   **In-Scope:** In-memory array sorting, primitive types (int, double), sequential execution (Semester 1), parallel execution (Semester 2) (Ref: `report/different-datatype-duplicate.md`, `docs/multiTypePlan.md`).
    *   **Out-of-Scope:** Disk-based sorting, distributed sorting, GPU sorting.

## 6. Literature Review
*   **6.1 Classical Quicksort & Introsort**
    *   Hoare's original algorithm (Ref: `PaperWork/source/Quicksort-Hoare.md`).
    *   Musser's Introsort (hybrid approach used in STL) (Ref: `report/introsort-strategy.md`).
*   **6.2 Dual-Pivot Quicksort**
    *   Yaroslavskiy's partitioning scheme (3 regions) (Ref: `docs/DualPivotQuicksort.md`, `report/dual-pivot-implementation.md`).
    *   Aumüller & Dietzfelbinger’s complexity analysis ($0.8 n \ln n$ comparisons).
*   **6.3 Memory Hierarchy & Sorting**
    *   Wild’s "Scanned Elements Model" (explaining why fewer memory accesses matter more than comparisons) (Ref: `docs/Why Is Dual-Pivot Quicksort Fast.md`).
    *   The "Memory Wall" concept.

## 7. Methodology & System Design
*   **7.1 Development Environment**
    *   Language: C++23 (GCC 13+).
    *   Tools: CMake, Python (for analysis), Linux (WSL) (Ref: `docs/implementation_report.md`).
*   **7.2 Algorithm Implementation Details**
    *   **Generic Interface:** Using `std::sortable` and `std::random_access_iterator` concepts (Ref: `report/custom-comparator-implementation.md`, `report/namespace-safety.md`).
    *   **Partitioning Logic:** Detailed explanation of the 5-point pivot selection and 3-way partitioning (Ref: `report/dual-pivot-implementation.md`).
    *   **Fallback Mechanism:** Switching to Insertion Sort for small arrays (threshold tuning) and Introsort strategy (Ref: `report/introsort-strategy.md`).
    *   **Optimizations:** Integer limit fixes, recursion depth capping, and non-contiguous optimization (Ref: `report/integer-limit-fix.md`, `report/recursion_depth_analysis.md`, `report/non-contiguous-optimization.md`).
*   **7.3 Benchmarking Framework Design**
    *   **Justification:** Why a custom harness was chosen over Google Benchmark (referencing *Benchmarking Framework Justification Report*) (Ref: `report/benchmarking-framework-justification.md`).
    *   **Metrics:** Execution time (ms), Speedup (x), Standard Deviation (Ref: `docs/benchmark.md`).
    *   **Sampling Strategy:** Determining appropriate array sizes and sample counts (Ref: `report/array-size-sampling.md`, `report/sample_size_analysis.md`).
    *   **Test Patterns:** Random, Sorted, Reverse, Duplicates (Organ Pipe) (Ref: `report/dual-pivot-reverse-sorted.md`).

## 8. Results and Achievements
*   **8.1 Implementation Status**
    *   Completed: Generic Dual-Pivot Quicksort (Sequential) (Ref: `docs/completion_summary.md`, `docs/implementation_report.md`).
    *   Completed: Custom Benchmarking Harness with Python automation.
    *   Completed: Initial Parallel prototype (Thread Pool) (Ref: `report/parallel_implementation_comparison.md`).
*   **8.2 Performance Analysis**
    *   **Experiment 1: Large Random Arrays:** Comparison of DPQS vs `std::sort`. (Graph: Time vs Size) (Ref: `docs/scaling_analysis_v3.md`).
    *   **Experiment 2: Memory Bandwidth & Mutex Contention:** Analysis of why scaling diminishes after 4 threads (Ref: `docs/scaling_analysis_report.md`, `docs/mutex_contention_analysis.md`).
    *   **Experiment 3: Sequential vs Parallel (1 Thread):** Investigation of overhead costs (Ref: `docs/sequential_vs_parallel_1thread.md`).
    *   **Experiment 4: Scalability Analysis:** Detailed study of parallel speedup (Ref: `report/parallel_scalability_analysis.md`, `report/parallel_performance_investigation.md`).
    *   **Experiment 5: Corner Cases:** Performance on sorted/reverse arrays (Ref: `report/dual-pivot-reverse-sorted.md`) and duplicates (Ref: `report/different-datatype-duplicate.md`).
*   **8.3 Key Findings**
    *   DPQS shows slight improvement over `std::sort` on large random inputs due to cache locality.
    *   Memory bandwidth is the primary bottleneck for parallel scaling on the test machine.

## 9. Discussion & Future Improvements
*   **9.1 Parallelization Refinement**
    *   Strategies for optimizing Thread Pool implementation (Ref: `docs/parallel_optimization_plan.md`, `docs/parallel_implementation_spec.md`).
    *   Potential for work-stealing to address load imbalance (Ref: `docs/work_stealing_implementation_plan.md`).
*   **9.2 Advanced Optimizations**
    *   **Vectorization & Memory:** Feasibility of SIMD with **Non-Temporal Stores** and **Block-based partitioning** to mitigate bandwidth bottlenecks (Ref: `report/simd-parallelism-feasibility.md`, `report/memory_strategy_analysis.md`).
    *   **Task Management:** Investigation into **Explicit Memory Management** (pre-allocated stacks) to reduce thread-pool allocation overhead.
    *   **Hybrid Strategies:**
        *   Adaptive Run Merging for partially sorted data (Ref: `report/adaptive-run-merging.md`).
        *   Specialized strategies like Counting Sort for specific distributions (Ref: `report/counting-sort-optimization.md`).
*   **9.3 Extended Comparison**
    *   Planned evaluation against state-of-the-art PDQSort (Pattern-Defeating Quicksort) (Ref: `docs/improvementPlan.md`).

## 10. Conclusion
*   Summary of Semester 1 achievements: Successful implementation of the core algorithm and a rigorous testing framework.
*   Readiness for Semester 2: Clear path identified for parallelization and optimization.

## 11. References/Bibliography
*   [List of key papers: Yaroslavskiy, Wild, Aumüller, etc.]