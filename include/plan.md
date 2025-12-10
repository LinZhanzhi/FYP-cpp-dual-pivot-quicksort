# Refactoring Plan for `dual_pivot_quicksort.hpp`

## 1. Motivation
The current `dual_pivot_quicksort.hpp` is a monolithic header file (~5700 lines) containing:
-   Core sorting logic (Dual-Pivot Partitioning)
-   Base cases (Insertion Sort, Mixed Insertion Sort, Heap Sort)
-   Parallel processing infrastructure (`Sorter`, `Merger`, `CountedCompleter`)
-   Java-style type erasure (`ArrayPointer`, `ArrayVariant`)
-   Public API overloads

This structure makes it difficult to:
-   Understand individual components.
-   Test specific parts (e.g., just the partitioning logic).
-   Maintain and optimize (e.g., adding SIMD without cluttering the main file).
-   Demonstrate "command of the code" as required for the FYP.

## 2. Proposed Modular Structure
We will split the monolithic file into a set of cohesive, single-responsibility headers.

### 2.1. Core Utilities
-   **`include/dpqs/utils.hpp`**:
    -   Macros: `LIKELY`, `UNLIKELY`, `FORCE_INLINE`, `PREFETCH_READ/WRITE`.
    -   Helper functions: `getDepth`, `checkNotNull`.
-   **`include/dpqs/constants.hpp`**:
    -   Thresholds: `MAX_INSERTION_SORT_SIZE`, `MIN_PARALLEL_SORT_SIZE`, etc.

### 2.2. Base Sorting Algorithms
-   **`include/dpqs/insertion_sort.hpp`**:
    -   `insertionSort` (basic)
    -   `mixedInsertionSort` (pin/pair optimization)
    -   `countingSort` (for byte/char/short)
-   **`include/dpqs/heap_sort.hpp`**:
    -   `heapSort` implementation.

### 2.3. Core Partitioning
-   **`include/dpqs/partition.hpp`**:
    -   `partitionDualPivot`: The heart of the algorithm.
    -   `partitionSinglePivot`: Fallback/comparison.
    -   *Future*: SIMD optimizations can be isolated here or in `partition_simd.hpp`.

### 2.4. Parallel Infrastructure
-   **`include/dpqs/parallel/completer.hpp`**:
    -   `CountedCompleter` base class (if needed).
-   **`include/dpqs/parallel/merger.hpp`**:
    -   `Merger` and `GenericMerger` classes.
-   **`include/dpqs/parallel/sorter.hpp`**:
    -   `Sorter` and `AdvancedSorter` classes.

### 2.5. Type Handling (Legacy/Port Support)
-   **`include/dpqs/types.hpp`**:
    -   `ArrayVariant`, `ArrayPointer` (if strictly necessary).
    -   *Goal*: Phase this out in favor of C++ templates if possible.

### 2.6. Main Entry Point
-   **`include/dual_pivot_quicksort.hpp`**:
    -   Includes all the above.
    -   Exposes the public `sort()` API.

## 3. Execution Strategy

### Phase 1: Mechanical Extraction (Safe)
*Goal: Split files without changing logic to ensure nothing breaks.*
1.  Create the directory structure `include/dpqs/`.
2.  Move code block by block into new files.
3.  Add necessary `#include` guards and dependencies to each new file.
4.  Update `dual_pivot_quicksort.hpp` to just include the new files.
5.  **Verify**: Run existing tests after every move.

### Phase 2: C++ Modernization (Refactoring)
*Goal: Make the code idiomatic C++.*
1.  **Templatization**: Replace manual overloads (e.g., `sort_int`, `sort_long`) with templates where possible.
2.  **Remove Java-isms**: Evaluate if `ArrayPointer` can be replaced by `std::span<T>` or simple `T*` + size.
3.  **Namespace Cleanup**: Ensure everything is cleanly inside `namespace dual_pivot`.

### Phase 3: Optimization & SIMD (Performance)
*Goal: Accelerate specific hotspots.*
1.  Isolate `partitionDualPivot` in `partition.hpp`.
2.  Create `partition_avx2.hpp` with intrinsics.
3.  Use `if constexpr` or SFINAE to select the best implementation at compile time.

## 4. Feasibility Assessment
-   **Feasibility**: High. The code is already structured into classes and functions, just physically located in one file.
-   **Necessity**: High. For an FYP, you need to show you understand the architecture. A modular design demonstrates this better than a 5000-line port.
-   **Risk**: Moderate. The main risk is breaking the complex dependencies between `Sorter`, `Merger`, and the recursive calls.
    -   *Mitigation*: Strict adherence to "move, don't rewrite" in Phase 1. Run the benchmark suite constantly.

## 5. Next Steps
1.  Approve this plan.
2.  Create the `include/dpqs` directory.
3.  Begin Phase 1 with `utils.hpp` and `constants.hpp`.
