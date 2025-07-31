# Benchmark Technical Specification

## Overview

This document describes the comprehensive benchmarking system for evaluating the dual-pivot quicksort implementation against standard sorting algorithms. The benchmark evaluates performance across three critical variables: array patterns, data types, and array lengths.

## Benchmark Variables

### 1. Array Patterns (12 Total)

The benchmark tests against diverse input patterns that reveal different algorithmic characteristics:

#### 1.1 Random/Unsorted Array
- **Description**: Elements are unordered and randomly distributed
- **Purpose**: Provides baseline performance measurement for the generic case
- **Expected Behavior**: Should demonstrate typical O(n log n) performance

#### 1.2 Sorted Array (Ascending)
- **Description**: Elements are already in ascending order
- **Purpose**: Tests algorithm adaptiveness to pre-sorted input
- **Expected Behavior**: Optimized algorithms should approach O(n) performance

#### 1.3 Reverse-Sorted Array (Descending)
- **Description**: Elements are in exact descending order
- **Purpose**: Identifies worst-case performance scenarios
- **Expected Behavior**: May trigger worst-case behavior in poorly implemented quicksort variants

#### 1.4 Nearly Sorted (Mostly Sorted) Array
- **Description**: Most elements are in correct order with few out-of-place elements
- **Purpose**: Tests algorithm adaptiveness to small disorder
- **Expected Behavior**: Adaptive algorithms should perform significantly better than O(n log n)

#### 1.5 Reverse Nearly Sorted
- **Description**: Elements are mostly in descending order with minor shuffling
- **Purpose**: Tests handling of reverse-ordered input with small perturbations
- **Expected Behavior**: Should reveal algorithm's ability to handle reverse patterns efficiently

#### 1.6 Array with Few Unique Elements
- **Description**: Many repeated elements/duplicates throughout the array
- **Purpose**: Tests efficiency with duplicate handling
- **Expected Behavior**: Dual-pivot quicksort should show >20% improvement due to three-way partitioning

#### 1.7 Array with All Identical Elements
- **Description**: Every element has the same value
- **Purpose**: Tests degenerate case handling and unnecessary operations
- **Expected Behavior**: Should approach O(n) performance in optimized implementations

#### 1.8 Array with Small Range vs. Large Range
- **Description**: Elements span either narrow or wide value ranges
- **Purpose**: Tests algorithm behavior with different value distributions
- **Expected Behavior**: May affect cache performance and comparison overhead

#### 1.9 Array with Alternating High-Low Pattern
- **Description**: Pattern like [1,100,2,99,3,98,...]
- **Purpose**: Tests handling of maximum number of inversions
- **Expected Behavior**: Challenges partitioning efficiency and swap operations

#### 1.10 Array with Only Small Values vs. Mostly Small with Few Large Values
- **Description**: Skewed value distributions
- **Purpose**: Tests partitioning behavior with uneven value distributions
- **Expected Behavior**: May affect pivot selection effectiveness

#### 1.11 Short Arrays and Very Long Arrays
- **Description**: Tests scalability across different array sizes
- **Purpose**: Reveals algorithmic overhead and scalability characteristics
- **Expected Behavior**: Should demonstrate consistent algorithmic complexity

#### 1.12 Permutation Patterns
- **Description**: Arrays requiring only few swaps to reach sorted order
- **Purpose**: Tests efficiency with nearly-sorted permutations
- **Expected Behavior**: Adaptive algorithms should show significant performance gains

### 2. Data Types (18 Total)

The benchmark evaluates performance across all C++ primitive types to assess type-specific overhead:

#### 2.1 Integer Types (8 types)
- `int` (signed integer)
- `unsigned int` (unsigned integer)
- `short` (signed short integer)
- `unsigned short` (unsigned short integer)
- `long` (signed long integer)
- `unsigned long` (unsigned long integer)
- `long long` (signed long long integer)
- `unsigned long long` (unsigned long long integer)

#### 2.2 Floating-Point Types (3 types)
- `float` (single-precision floating-point)
- `double` (double-precision floating-point)
- `long double` (extended-precision floating-point)

#### 2.3 Character Types (5 types)
- `char` (character type)
- `unsigned char` (unsigned character type)
- `signed char` (signed character type)
- `wchar_t` (wide character)
- `char16_t` (16-bit Unicode character, C++11 and later)

#### 2.4 Special Types (2 types)
- `char32_t` (32-bit Unicode character, C++11 and later)
- `bool` (boolean type)

### 3. Array Lengths (61 Total)

Array sizes are logarithmically spaced to provide comprehensive performance analysis across different scales:

```
1, 2, 3, 4, 5, 6, 8, 10, 13, 16, 21, 26, 33, 42, 54, 68, 86, 109, 138, 175, 222,
281, 355, 449, 568, 719, 910, 1,151, 1,456, 1,842, 2,329, 2,947, 3,727, 4,714,
5,963, 7,543, 9,540, 12,067, 15,264, 19,306, 24,420, 30,888, 39,069, 49,417,
62,505, 79,060, 100,000, 138949, 193069, 268269, 372759, 517947, 719685, 1000000, 1389495, 1930697, 2682695, 
3727593, 5179474, 7196856, 10000000
```

**Rationale**: Logarithmic spacing provides:
- Dense sampling for small arrays where overhead effects dominate
- Adequate coverage for large arrays where algorithmic complexity becomes apparent
- Smooth transition points for threshold-based optimizations

## Algorithm Comparison

The benchmark compares four sorting implementations:

### 1. Dual-Pivot Quicksort
- **Implementation**: Custom dual-pivot quicksort based on Yaroslavskiy's algorithm
- **Location**: `include/dual_pivot_quicksort.hpp`
- **Features**: Three-way partitioning, median-of-5 pivot selection, threshold switching

### 2. std::sort
- **Implementation**: STL standard sort (typically introsort)
- **Purpose**: Primary performance baseline
- **Expected Performance**: Highly optimized hybrid algorithm

### 3. std::stable_sort
- **Implementation**: STL stable sorting algorithm (typically merge sort)
- **Purpose**: Stability comparison benchmark
- **Expected Performance**: O(n log n) guaranteed with stability overhead

### 4. qsort (C Library)
- **Implementation**: C standard library quicksort
- **Purpose**: Legacy comparison baseline
- **Expected Performance**: Traditional quicksort with function pointer overhead

## Benchmark Execution

### Total Test Combinations
- **Array Patterns**: 12
- **Data Types**: 18
- **Array Lengths**: 50
- **Total Combinations**: 12 × 18 × 50 = **10,800 test cases**

### Execution Parameters
- **Iterations per test**: Multiple runs for statistical significance
- **Warmup runs**: Performed to minimize cold-start effects
- **Statistical measures**: Mean, median, standard deviation collected
- **Timing precision**: High-resolution timing using `std::chrono`

### Output Format
- **File**: CSV format for structured data analysis
- **Columns**: Algorithm name, array pattern, data type, array size, execution time, statistical measures
- **Future Extensions**: Plotting capabilities planned for visualization

### Performance Expectations

Based on theoretical analysis and implementation optimizations:

- **Overall improvement**: 10-12% over std::sort for random data
- **Duplicate handling**: >20% improvement for arrays with many duplicates  
- **Swap efficiency**: 20% fewer swaps than classic quicksort (0.8×n×ln(n) vs 1.0×n×ln(n))
- **Cache performance**: Better memory access patterns due to three-way partitioning
- **Threshold optimization**: Automatic switching to insertion sort for arrays < 17 elements

### Implementation Notes

- **Thread safety**: Single-threaded execution for consistent timing
- **Memory management**: In-place sorting to minimize allocation overhead
- **Compiler optimization**: Built with `-O3 -march=native` for maximum performance
- **Statistical validation**: Multiple iterations ensure measurement reliability

This comprehensive benchmark provides thorough evaluation of the dual-pivot quicksort implementation across diverse scenarios, enabling detailed performance analysis and algorithm validation.