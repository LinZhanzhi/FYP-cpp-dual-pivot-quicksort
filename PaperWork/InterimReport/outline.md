# Interim Report Outline
**Project Title:** Implementing Dual Pivot Quicksort in C++23 and Comparing its Performance with Standard Sorting Libraries
**Student:** LIN Zhanzhi (22097456D)
**Supervisor:** CAO Yixin

---

## 1. Preamble
*   **Title Page**: Standard Capstone Project Cover.
*   **Abstract**:
    *   Summary of the move from Single-Pivot to Dual-Pivot Quicksort.
    *   Goal: Investigate if C++ standard libraries can benefit from Dual-Pivot strategies used in Java.
    *   Current Status: Generic C++23 implementation complete; benchmarking framework established; preliminary results show promise on large datasets.
*   **Table of Contents**

## 2. Introduction
*   **2.1 Background**
    *   The dominance of Quicksort in system libraries.
    *   Yaroslavskiy's 2009 Dual-Pivot innovation and its adoption in Java 7.
    *   The current state of C++ `std::sort` (Introsort).
*   **2.2 Problem Statement**
    *   Lack of a modern, standard-compliant C++ implementation of Dual-Pivot Quicksort.
    *   Need to verify if theoretical gains (reduced cache misses) translate to C++ performance.
*   **2.3 Objectives**
    *   1. Implement a robust, generic Dual-Pivot Quicksort using C++23 Concepts.
    *   2. Develop a reliable benchmarking framework for destructive sorting tests.
    *   3. Compare performance against `std::sort` and `std::stable_sort`.
    *   4. Analyze memory bandwidth and cache behavior.
*   **2.4 Scope**
    *   **In-Scope:** In-memory array sorting, primitive types (int, double), sequential execution (Semester 1), parallel execution (Semester 2).
    *   **Out-of-Scope:** Disk-based sorting, distributed sorting, GPU sorting.

## 3. Literature Review
*   **3.1 Classical Quicksort & Introsort**
    *   Hoare's original algorithm.
    *   Musser's Introsort (hybrid approach used in STL).
*   **3.2 Dual-Pivot Quicksort**
    *   Yaroslavskiy's partitioning scheme (3 regions).
    *   Aumüller & Dietzfelbinger’s complexity analysis (.8 n \ln n$ comparisons).
*   **3.3 Memory Hierarchy & Sorting**
    *   Wild’s "Scanned Elements Model" (explaining why fewer memory accesses matter more than comparisons).
    *   The "Memory Wall" concept.

## 4. Methodology & System Design
*   **4.1 Development Environment**
    *   Language: C++23 (GCC 13+).
    *   Tools: CMake, Python (for analysis), Linux (WSL).
*   **4.2 Algorithm Implementation Details**
    *   **Generic Interface:** Using `std::sortable` and `std::random_access_iterator` concepts.
    *   **Partitioning Logic:** Detailed explanation of the 5-point pivot selection and 3-way partitioning.
    *   **Fallback Mechanism:** Switching to Insertion Sort for small arrays (threshold tuning).
*   **4.3 Benchmarking Framework Design**
    *   **Justification:** Why a custom harness was chosen over Google Benchmark (referencing *Benchmarking Framework Justification Report*).
    *   **Metrics:** Execution time (ms), Speedup (x), Standard Deviation.
    *   **Test Patterns:** Random, Sorted, Reverse, Duplicates (Organ Pipe).

## 5. Progress & Preliminary Results
*   **5.1 Implementation Status**
    *   Completed: Generic Dual-Pivot Quicksort (Sequential).
    *   Completed: Custom Benchmarking Harness with Python automation.
    *   Completed: Initial Parallel prototype (Thread Pool).
*   **5.2 Performance Analysis**
    *   **Experiment 1: Large Random Arrays:** Comparison of DPQS vs `std::sort`. (Graph: Time vs Size).
    *   **Experiment 2: Memory Bandwidth Saturation:** Analysis of why scaling diminishes after 4 threads (referencing *Scaling Analysis Report*).
    *   **Experiment 3: Sequential vs Parallel (1 Thread):** Investigation of overhead costs (referencing *Sequential vs Parallel Report*).
*   **5.3 Key Findings**
    *   DPQS shows slight improvement over `std::sort` on large random inputs due to cache locality.
    *   Memory bandwidth is the primary bottleneck for parallel scaling on the test machine.

## 6. Project Plan (Semester 2)
*   **6.1 Parallelization Refinement**
    *   Optimize the Thread Pool implementation.
    *   Implement work-stealing for load balancing.
*   **6.2 Advanced Optimizations**
    *   Vectorization (SIMD) for partitioning.
    *   Block-based partitioning to further reduce cache misses.
*   **6.3 Final Evaluation**
    *   Comprehensive comparison against PDQSort (Pattern-Defeating Quicksort).
    *   Final Thesis writing.
*   **6.4 Timeline (Gantt Chart Placeholder)**
    *   Jan: Parallel Optimization.
    *   Feb: SIMD & Advanced Tuning.
    *   Mar: Final Benchmarks & Report Writing.
    *   Apr: Submission.

## 7. Conclusion
*   Summary of Semester 1 achievements: Successful implementation of the core algorithm and a rigorous testing framework.
*   Readiness for Semester 2: Clear path identified for parallelization and optimization.

## 8. References
*   [List of key papers: Yaroslavskiy, Wild, Aumüller, etc.]
