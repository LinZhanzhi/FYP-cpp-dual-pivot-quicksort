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
    -   `partitionSinglePivot` (Fallback/comparison).
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
    -   `ArrayVariant`, `ArrayPointer`.

## 3. Execution Plan

### Phase 1: Mechanical Extraction (Low Risk)
- [x] Create `include/dpqs/` directory.
- [x] Extract `constants.hpp` (Thresholds).
- [x] Extract `utils.hpp` (Macros, `checkNotNull`, `checkEarlyTermination`).
- [x] Update `dual_pivot_quicksort.hpp` to include these new files.
- [x] Verify compilation and basic functionality.

### Phase 2: Base Algorithms (Medium Risk)
- [x] Extract `insertion_sort.hpp`.
- [x] Extract `heap_sort.hpp`.
- [x] Verify.

### Phase 3: Parallel Infrastructure (High Risk)
- [x] Extract `parallel/completer.hpp`.
- [x] Extract `parallel/merger.hpp`.
- [x] Extract `parallel/sorter.hpp`.
- [x] Verify (requires careful dependency management).

### Phase 4: Core Logic & Cleanup (Medium Risk)
- [ ] Extract `partition.hpp`.
- [ ] Clean up `dual_pivot_quicksort.hpp` to be just an entry point.
