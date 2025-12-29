# Improvement Report: Optimized Non-Contiguous Sorting

## 1. Problem Description
The original implementation of `dual_pivot::sort` was highly optimized for **contiguous memory** (e.g., C-arrays, `std::vector`, `std::array`). It relied on pointer arithmetic for performance.

When a user attempted to sort a **non-contiguous container** (like `std::deque`), the library used a fallback mechanism:
1.  Allocate a temporary `std::vector`.
2.  Copy all elements from the non-contiguous container to the vector.
3.  Sort the vector using the optimized pointer-based algorithm.
4.  Copy the sorted elements back to the original container.

**Drawbacks:**
- **Memory Overhead**: Requires (N)$ extra memory to store the copy.
- **Performance Overhead**: Two (N)$ copy operations (copy-in and copy-out) are added to the (N \log N)$ sort time. For large datasets or complex objects, this copy cost is substantial.

## 2. Solution Implemented
We implemented a generic, iterator-based version of the Dual-Pivot Quicksort algorithm that operates directly on any container satisfying the `RandomAccessIterator` concept (e.g., `std::deque`).

### Key Changes:
- **New Header**: Created `include/dpqs/iterator_sort.hpp`.
- **Iterator Logic**:
    - Replaced pointer arithmetic (e.g., `ptr + index`) with iterator arithmetic (e.g., `std::next(iter, index)` or `iter + index`).
    - Replaced `std::swap` on values with `std::iter_swap` on iterators.
    - Implemented iterator-based versions of `partition` and `insertion_sort`.
- **Dispatch Logic**: Updated `dual_pivot_quicksort.hpp` to detect the iterator type at compile time:
    - If **Contiguous** (pointer, vector iterator): Use the highly optimized pointer-based implementation.
    - If **Non-Contiguous** (deque iterator): Use the new iterator-based implementation.

## 3. Impact and Verification
- **Zero Copy**: Sorting a `std::deque` is now performed in-place. No temporary vector is allocated, and no elements are copied unnecessarily.
- **Memory Efficiency**: Memory usage for sorting `std::deque` dropped from (N)$ to (\log N)$ (stack recursion depth).
- **Performance**: While iterator abstraction adds a small overhead compared to raw pointers, avoiding the N$ copy operations results in a net performance gain for large datasets and complex types.
- **Verification**:
    - A specific test case for `std::deque` was created and passed, verifying that the iterator-based logic correctly sorts data.
    - The existing tests for `std::vector` (contiguous) passed, confirming that the dispatch logic correctly selects the pointer-based path for them.
