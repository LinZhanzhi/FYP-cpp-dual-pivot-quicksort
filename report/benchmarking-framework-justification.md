# Benchmarking Framework Justification

## 1. Introduction
This report documents the evaluation process and decision-making regarding the benchmarking infrastructure for the Dual-Pivot Quicksort project. Specifically, it addresses the choice between adopting the industry-standard **Google Benchmark** library versus maintaining the current **Custom Benchmarking Harness**.

## 2. Comparative Analysis

### 2.1 Google Benchmark
Google Benchmark is a widely used library for measuring the performance of C++ code snippets. It handles test registration, iteration scaling, and statistical reporting automatically.

**Pros:**
*   **Standardization:** Recognized in academia and industry; results are immediately trusted.
*   **Statistical Rigor:** Automatically calculates standard deviation and runs enough iterations to minimize noise.
*   **Tooling:** Built-in support for asymptotic complexity (Big-O) analysis and hardware counter integration (e.g., cache misses).

**Cons:**
*   **Destructive Operation Handling:** Designed primarily for operations that can be repeated in a tight loop without side effects. Sorting is inherently "destructive" (it modifies the input array).
*   **Dependency Management:** Requires integration into the build system (CMake/Bazel), adding complexity to the project setup.

### 2.2 Custom Benchmarking Harness
The current project utilizes a custom C++ runner (`benchmark_runner.cpp`) orchestrated by a Python script (`benchmark_manager.py`).

**Pros:**
*   **Control:** Offers absolute control over the execution flow, specifically the separation of data preparation and algorithm execution.
*   **Simplicity:** Zero external dependencies; relies only on the C++ Standard Library.
*   **Transparency:** The timing logic is explicit and easily auditable.

**Cons:**
*   **Manual Statistics:** Aggregation (median, average) and outlier filtering must be implemented manually in the analysis scripts.
*   **Boilerplate:** Requires writing manual loops for warmup and iteration.

## 3. The "Destructive Testing" Challenge
The primary technical factor influencing this decision is the **destructive nature of sorting algorithms**.

When benchmarking a sort function:
1.  Input: An unsorted array $A$.
2.  Operation: `sort(A)`.
3.  Result: $A$ is now sorted.

To measure the performance accurately over multiple iterations, $A$ must be reset to its original unsorted state before every single run.

### Google Benchmark Approach
In Google Benchmark, handling this requires pausing the timer to reset the state:
```cpp
for (auto _ : state) {
    state.PauseTiming();
    std::vector<int> data = original_data; // Reset
    state.ResumeTiming();
    
    sort(data);
}
```
**Issue:** The `PauseTiming()` and `ResumeTiming()` calls introduce system overhead. For high-performance sorting (especially on smaller arrays), this overhead can skew microsecond-level measurements. Alternatively, copying inside the timed region measures `Copy + Sort` time, which is inaccurate.

### Custom Harness Approach
The custom implementation explicitly separates these phases:
```cpp
for (int i = 0; i < iterations; ++i) {
    // 1. Data Reset (Untimed)
    auto test_data = original_data; 

    // 2. Algorithm Execution (Timed)
    auto start = std::chrono::high_resolution_clock::now();
    sort(test_data);
    auto end = std::chrono::high_resolution_clock::now();
    
    durations.push_back(end - start);
}
```
This approach ensures that the cost of memory allocation and data copying is strictly excluded from the performance metric, providing a purer measurement of the sorting algorithm itself.

## 4. Decision
**Decision:** Retain the **Custom Benchmarking Harness**.

**Justification:**
1.  **Measurement Accuracy:** The custom harness provides a more straightforward and accurate method for excluding data-reset costs from the sorting time, which is critical for this specific workload.
2.  **Project Scope:** The current harness is already implemented, debugged, and integrated with the Python analysis pipeline. Switching to Google Benchmark would incur significant refactoring costs for marginal gain in this specific context.
3.  **Sufficiency:** The current setup already implements necessary best practices, including:
    *   **Warmup Runs:** To prime the CPU cache.
    *   **Volatile Sinks:** To prevent compiler dead-code elimination.
    *   **Median Filtering:** The Python analysis script uses median values to reject OS-induced noise/spikes.

## 5. Conclusion
While Google Benchmark is a superior tool for general-purpose microbenchmarking, the custom harness is better optimized for the specific constraints of benchmarking destructive sorting algorithms within the scope of this project. The current methodology ensures valid, reproducible, and noise-free results without the overhead of managing timer states during data resets.
