# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a comprehensive C++ implementation of Vladimir Yaroslavskiy's dual-pivot quicksort algorithm, featuring STL compatibility, advanced optimizations, and extensive benchmarking capabilities. The implementation is based on the algorithm adopted in Java 7 and includes theoretical performance analysis from Sebastian Wild's research.

## Build and Development Commands

### Essential Build Commands
```bash
# Build optimized benchmark (default)
make benchmark_optimized

# Build debug version for development
make benchmark_debug

# Run comprehensive performance benchmarks
make run_benchmark

# Run unit tests
make test

# Run benchmarks with all optimization levels (-O0, -O2, -O3)
make run_all_optimizations

# Clean build artifacts
make clean
```

### Manual Compilation
```bash
# Optimized build
g++ -std=c++17 -O3 -march=native -I./include benchmarks/main_benchmark.cpp -o benchmark

# Debug build
g++ -std=c++17 -g -O0 -I./include benchmarks/main_benchmark.cpp -o benchmark_debug
```

## Architecture Overview

### Core Algorithm Hierarchy

The implementation follows a layered architecture with three main sorting algorithm variants:

1. **`dual_pivot::dual_pivot_quicksort`** - Core implementation following Yaroslavskiy's algorithm exactly
2. **`dual_pivot_optimized::dual_pivot_introsort`** - Enhanced version with introsort-style depth limiting and adaptive optimizations
3. **`std_compatible::sort`** - STL drop-in replacement wrapper

### Key Algorithmic Components

**Three-Way Partitioning**: Unlike classic quicksort's two-way partitioning, dual-pivot uses two pivot elements (P1 ≤ P2) to create three partitions:
- Part I: Elements < P1
- Part II: P1 ≤ Elements ≤ P2  
- Part III: Elements > P2

**Pivot Selection**: Uses median-of-5 with a 5-element sorting network for optimal pivot selection, implemented in `sort_5_network()`.

**Threshold Switching**: Arrays smaller than `INSERTION_SORT_THRESHOLD` (17 elements) automatically switch to insertion sort.

### Template Design Pattern

All sorting functions use template metaprogramming for:
- **Type safety**: `static_assert` for random access iterators
- **Performance**: Compile-time optimization for different types
- **Flexibility**: Custom comparator support with `std::less<>` default

### Benchmarking Architecture

**Data Generation**: `benchmark_data::generate_data()` creates 8 different test patterns (random, nearly sorted, reverse sorted, organ pipe, sawtooth, and three levels of duplicates).

**Statistical Analysis**: `benchmark_timer::BenchmarkTimer` performs 50+ iterations with warmup runs and calculates mean, median, standard deviation.

**Comparison Framework**: Tests against `std::sort`, `std::stable_sort`, and classic quicksort implementations across multiple array sizes (10 to 10,000,000 elements).

## Implementation Constants

- `INSERTION_SORT_THRESHOLD = 17` - Switch to insertion sort for small arrays
- `DIST_SIZE = 13` - Threshold for equal elements optimization
- Default iterations: 50 for small/medium arrays, 10 for large arrays (>100,000 elements)

## Testing Strategy

The correctness verification (`tests/unit_tests.hpp`) covers:
- Edge cases (empty arrays, single elements, all duplicates)
- Algorithm equivalence across all implementations
- Type compatibility (int, double, string, custom types)
- Performance pattern handling (problematic cases for classic quicksort)

## Performance Expectations

Based on theoretical analysis:
- **10-12% improvement** over std::sort for random data
- **>20% improvement** for arrays with many duplicates
- **20% fewer swaps** than classic quicksort (0.8×n×ln(n) vs 1.0×n×ln(n))
- **Better cache performance** due to optimized memory access patterns

## Key Files for Algorithm Modification

- `include/dual_pivot_quicksort_impl.hpp` - Core partitioning logic and recursion
- `include/dual_pivot_optimized.hpp` - Advanced optimizations and hybrid algorithms
- `benchmarks/data_generator.hpp` - Test pattern generation for performance analysis
- `benchmarks/timer.hpp` - Statistical measurement framework