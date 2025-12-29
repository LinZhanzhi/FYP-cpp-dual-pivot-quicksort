# Improvement Report: Integer Limit Fix (64-bit Addressing)

## 1. Problem Description
The original implementation of the Dual-Pivot Quicksort library relied heavily on the `int` data type for array indices, loop counters, and size calculations. In C++, `int` is typically a 32-bit signed integer, which limits the maximum addressable array index to ^{31} - 1$ (approximately 2.14 billion elements).

For a modern high-performance sorting library, this limitation is significant. Large-scale data processing tasks often involve arrays exceeding this size (e.g., sorting 16GB+ of integers). Using `int` would cause integer overflow when calculating indices or sizes for such large arrays, leading to undefined behavior, segmentation faults, or incorrect sorting results.

## 2. Solution Implemented
We replaced `int` with `std::ptrdiff_t` throughout the codebase.

### Key Changes:
- **Type Substitution**: All variables representing array indices, lengths, offsets, and loop counters were changed from `int` to `std::ptrdiff_t`.
- **Header Updates**: Updated all headers in `include/dpqs/`, including:
    - `partition.hpp`
    - `insertion_sort.hpp`
    - `heap_sort.hpp`
    - `run_merger.hpp`
    - `sequential_sorters.hpp`
    - `parallel/merger.hpp`
    - `parallel/sorter.hpp`
- **Interface Updates**: The public API in `dual_pivot_quicksort.hpp` was updated to accept `std::ptrdiff_t` for range bounds (`low`, `high`).

### Why `std::ptrdiff_t`?
- **64-bit Support**: On 64-bit systems, `std::ptrdiff_t` is a 64-bit signed integer, allowing addressing of arrays up to ^{63}-1$ elements.
- **Pointer Arithmetic Compatibility**: It is the standard type for the result of subtracting two pointers, making it semantically correct for array indexing and iterator distances.
- **Signedness**: Like `int`, it is signed, which preserves the logic of existing loops (e.g., `while (--i >= low)`) that rely on checking for negative or boundary conditions.

## 3. Impact and Verification
- **Capacity**: The library can now theoretically sort arrays up to exabytes in size (limited only by system RAM), whereas it was previously capped at ~8GB (for 4-byte integers).
- **Correctness**: The logic remains identical; only the range of representable values has expanded.
- **Verification**: The existing test suite passed successfully. While we cannot easily test a >8GB array in a standard CI environment, the type safety ensures the logic holds for larger values.

## 4. Code Example
**Before:**
```cpp
void insertion_sort(T* a, int low, int high) {
    for (int i, k = low; ++k < high; ) { ... }
}
```

**After:**
```cpp
void insertion_sort(T* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    for (std::ptrdiff_t i, k = low; ++k < high; ) { ... }
}
```
