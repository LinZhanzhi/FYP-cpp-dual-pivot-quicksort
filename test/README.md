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
