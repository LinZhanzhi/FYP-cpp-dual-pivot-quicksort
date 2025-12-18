# Test Suite Documentation

## Counting Sort Test (`test_counting_sort.cpp`)

This test verifies the correctness of the Counting Sort implementation in `include/dpqs/counting_sort.hpp`. It covers 1-byte and 2-byte integral types (signed and unsigned).

### How to Run

From the project root directory:

```bash
g++ -std=c++17 -Iinclude test/test_counting_sort.cpp -o test_counting_sort
./test_counting_sort
```

### Coverage
- **Types**: `char`, `unsigned char`, `int8_t`, `uint8_t`, `short`, `unsigned short`, `int16_t`, `uint16_t`.
- **Scenarios**: Random arrays, sorted arrays, reverse sorted arrays, arrays with duplicates, edge cases.

## Heap Sort Test (`test_heap_sort.cpp`)

This test verifies the correctness of the Heap Sort implementation in `include/dpqs/heap_sort.hpp`. It covers the generic template and specialized implementations for `int`, `long`, `float`, and `double`.

### How to Run

From the project root directory:

```bash
g++ -std=c++17 -Iinclude test/test_heap_sort.cpp -o test_heap_sort
./test_heap_sort
```

### Coverage
- **Generic Template**: Tested with `int` and `double`.
- **Specialized Functions**: `heapSort_int`, `heapSort_long`, `heapSort_float`, `heapSort_double`.
- **Scenarios**: Random arrays, sorted arrays, reverse sorted arrays, arrays with duplicates, partial ranges.

## Float Sort Test (`test_float_sort.cpp`)

This test verifies the correctness of the Float Sort implementation in `include/dpqs/float_sort.hpp`. It specifically targets floating-point edge cases.

### How to Run

From the project root directory:

```bash
g++ -std=c++17 -Iinclude test/test_float_sort.cpp -o test_float_sort
./test_float_sort
```

### Coverage
- **Types**: `float`, `double`.
- **Scenarios**:
    - **NaN Handling**: Verifies that `NaN` values are moved to the end of the array.
    - **Signed Zeros**: Verifies that `-0.0` is placed before `+0.0`.
    - **Mixed Values**: Arrays containing negative numbers, positive numbers, zeros, and NaNs.

## Insertion Sort Test (`test_insertion_sort.cpp`)

This test verifies the correctness of the Insertion Sort implementation in `include/dpqs/insertion_sort.hpp`.

### How to Run

From the project root directory:

```bash
g++ -std=c++17 -Iinclude test/test_insertion_sort.cpp -o test_insertion_sort
./test_insertion_sort
```

### Coverage
- **Templates**: `insertion_sort`, `mixed_insertion_sort`.
- **Specialized Functions**: `insertion_sort_int`, `insertion_sort_long`, `insertion_sort_float`, `insertion_sort_double`.
- **Scenarios**: Random arrays of various types (`int`, `long`, `float`, `double`).

## Partition Test (`test_partition.cpp`)

This test verifies the correctness of the Partitioning implementation in `include/dpqs/partition.hpp`.

### How to Run

From the project root directory:

```bash
g++ -std=c++17 -Iinclude test/test_partition.cpp -o test_partition
./test_partition
```

### Coverage
- **Functions**: `partition_dual_pivot`, `partition_single_pivot`.
- **Scenarios**:
    - **Dual Pivot**: Verifies 3-way partitioning around two pivots (P1, P2). Checks regions `< P1`, `P1 <= x <= P2`, and `> P2`.
    - **Single Pivot**: Verifies 3-way partitioning around one pivot. Checks regions `< P`, `== P`, and `> P`.



