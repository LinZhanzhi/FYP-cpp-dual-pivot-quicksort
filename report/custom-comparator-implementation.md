# Custom Comparator Support Implementation Report

## 1. Objective
The primary goal of this task was to extend the Dual-Pivot Quicksort library to support **custom comparators**. This feature allows users to define arbitrary sorting orders (e.g., descending, absolute value, custom struct fields) instead of being limited to the default ascending order (`<` operator).

## 2. Implementation Details

The implementation involved a comprehensive refactoring of the entire library to replace hardcoded comparison operators with a generic `Compare` template argument.

### 2.1 Core Algorithm Refactoring
All internal sorting algorithms were updated to accept a `Compare comp` functor:
-   **Partitioning (`partition.hpp`)**: Updated `partition_dual_pivot` and `partition_single_pivot` to use `comp(a, b)` instead of `a < b`.
-   **Insertion Sort (`insertion_sort.hpp`)**: Refactored `insertion_sort` and `mixed_insertion_sort`.
    -   *Critical Fix*: Fixed a boundary check issue in `mixed_insertion_sort` where the loop condition `i > low` was insufficient when `low` was an offset in a parallel task, leading to memory corruption.
-   **Heap Sort (`heap_sort.hpp`)**: Updated `push_down` and `heap_sort` to maintain heap properties using the custom comparator.
-   **Merge Operations (`merge_ops.hpp`)**: Updated `merge_parts` and `parallel_merge_parts` to merge runs based on the custom order.

### 2.2 Parallel Framework Updates
The parallel sorting framework required significant updates to propagate the comparator object to all worker threads:
-   **`Sorter` Class**: Added a `Compare comp` member variable. The comparator is passed by value to all child tasks (`forkSorter`).
-   **`Merger` Class**: Updated to store and use the comparator when merging sorted segments in parallel.
-   **`AdvancedSorter`**: Updated to initialize the `Sorter` base class with the user-provided comparator.

### 2.3 Run Merger Integration
The "Run Merger" optimization (which detects already-sorted sequences) was updated to respect the custom comparator:
-   **Run Detection**: Now detects "ascending" runs based on `comp(a[i], a[i+1])`.
-   **Reverse Handling**: Correctly identifies and reverses "descending" runs (relative to the custom order).

### 2.4 Public API
A new overload was added to `dual_pivot_quicksort.hpp`:

```cpp
template<typename T, typename Compare>
void sort(T* a, int parallelism, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp);
```

This allows users to pass any standard functor (e.g., `std::greater`), lambda, or function pointer.

## 3. Challenges and Solutions

### 3.1 Memory Corruption in Parallel Sort
**Issue**: During testing, a "double free or corruption" error occurred when sorting large arrays (N=1,000,000) in parallel mode.
**Root Cause**: The `mixed_insertion_sort` function, used for small sub-tasks, had a loop condition `while (--i >= low && ...)` that assumed `low` was the absolute start of the buffer. In parallel tasks, `low` is relative to the current segment.
**Solution**: Introduced a `start` variable to track the absolute boundary of the current insertion sort range, ensuring the loop never underflows the buffer.

### 3.2 Run Merger Logic
**Issue**: The run merger relies on detecting "ascending" and "descending" sequences.
**Solution**: Abstracted the logic to define "ascending" as `!comp(b, a)` and "descending" as `comp(b, a)`. This ensures that if a user requests a descending sort, the run merger correctly identifies sequences that match that order.

## 4. Verification

A comprehensive test suite (`test/test_custom_comparator.cpp`) was developed to verify correctness across all scenarios.

### 4.1 Test Cases
| Test Case | Description | Result |
| :--- | :--- | :--- |
| **Descending Sort** | `std::greater<int>` on small and large arrays. | ✅ PASSED |
| **Custom Structs** | Sorting `Point` structs by `x` then `y` using a lambda. | ✅ PASSED |
| **Parallel Execution** | Sorting 1,000,000 elements with 4 threads. | ✅ PASSED |
| **Sequential Execution** | Sorting 1,000,000 elements on a single thread. | ✅ PASSED |
| **Complex Logic** | Sorting by Absolute Value (`abs(a) < abs(b)`). | ✅ PASSED |
| **String Sorting** | Sorting strings by length, then lexicographically. | ✅ PASSED |
| **Edge Cases** | Already sorted and reverse sorted inputs (triggering Run Merger). | ✅ PASSED |

## 5. Usage Examples

### 5.1 Descending Sort
```cpp
std::vector<int> data = {1, 5, 2, 9, 3};
dual_pivot::sort(data, std::greater<int>());
// Result: {9, 5, 3, 2, 1}
```

### 5.2 Custom Struct Sort
```cpp
struct Player { std::string name; int score; };
std::vector<Player> players = ...;

// Sort by score descending
dual_pivot::sort(players, [](const Player& a, const Player& b) {
    return a.score > b.score;
});
```

## 6. Conclusion
The custom comparator support is now fully implemented and robust. It seamlessly integrates with the library's advanced features, including parallel execution and run merging, without compromising performance or correctness.
