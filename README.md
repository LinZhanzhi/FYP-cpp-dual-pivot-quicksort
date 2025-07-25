# Dual-Pivot Quicksort C++ Implementation

A comprehensive C++ implementation of Vladimir Yaroslavskiy's dual-pivot quicksort algorithm, featuring STL compatibility, advanced optimizations, and extensive benchmarking capabilities.

## 🚀 Quick Start

```cpp
#include "include/dual_pivot_quicksort.hpp"

std::vector<int> data = {64, 34, 25, 12, 22, 11, 90};
dual_pivot::dual_pivot_quicksort(data.begin(), data.end());
// data is now sorted: {11, 12, 22, 25, 34, 64, 90}
```

## 📁 Project Structure

```
├── include/                          # Header files
│   ├── dual_pivot_quicksort.hpp      # Main interface
│   ├── dual_pivot_quicksort_impl.hpp # Core implementation
│   ├── classic_quicksort.hpp         # Comparison baseline
│   ├── dual_pivot_optimized.hpp      # Advanced optimizations
│   └── stl_compatible.hpp            # STL-style wrappers
├── benchmarks/                       # Performance testing
│   ├── data_generator.hpp            # Test data generation
│   ├── timer.hpp                     # Performance measurement
│   └── main_benchmark.cpp            # Comprehensive benchmarks
├── tests/                            # Unit tests
│   ├── unit_tests.hpp                # Correctness verification
│   └── run_tests.cpp                 # Test runner
├── docs/                            # Documentation
│   ├── cppPlan.md                   # Implementation plan
│   ├── implementation_report.md      # Detailed analysis
│   ├── DualPivotQuicksort.md        # Original algorithm paper
│   └── Why Is Dual-Pivot Quicksort Fast.md # Performance analysis
├── results/                         # Benchmark results (generated)
└── Makefile                         # Build configuration
```

## 🎯 Key Features

### Core Algorithm
- **Three-way partitioning** using two pivot elements
- **Optimized pivot selection** with median-of-5 sorting networks
- **Smart threshold switching** to insertion sort for small arrays
- **Efficient duplicate handling** for arrays with repeated elements

### C++ Specific Optimizations
- **Template-based design** for type safety and performance
- **STL iterator compatibility** for seamless integration
- **Exception safety** following modern C++ practices
- **Memory access optimization** based on cache-aware analysis

### Advanced Features
- **Introsort-style hybrid** with depth limiting and fallback to heapsort
- **Adaptive algorithm selection** for nearly-sorted data
- **Enhanced sampling** for large arrays
- **Quickselect variant** for partial sorting

## 🔧 Building and Running

### Prerequisites
- C++17 compatible compiler (GCC, Clang, or MSVC)
- Make utility (optional, for convenient building)

#### Windows: Quick Install with winget

You can install a GCC toolchain and Make utility using [WinLibs](https://winlibs.com/) via Windows Package Manager:

```bash
winget install BrechtSanders.WinLibs.POSIX.UCRT
```

### Build Commands

```bash
# Build optimized benchmark
make benchmark_optimized

# Build debug version
make benchmark_debug

# Run comprehensive benchmarks
make run_benchmark

# Run tests with different optimization levels
make run_all_optimizations

# Run unit tests
make test

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

## 📊 Performance Characteristics

### Theoretical Improvements
Based on Yaroslavskiy's analysis:
- **20% fewer swaps** compared to classic quicksort (0.8×n×ln(n) vs 1.0×n×ln(n))
- **Better cache performance** due to optimized memory access patterns
- **Superior handling** of arrays with many duplicate elements

### Expected Performance Gains
- **10-12% improvement** over std::sort for random data
- **>20% improvement** for arrays with many duplicates
- **Competitive performance** across all input patterns

## 🧪 Usage Examples

### Basic Sorting
```cpp
#include "include/dual_pivot_quicksort.hpp"

// Sort integers
std::vector<int> numbers = {5, 2, 8, 1, 9, 3};
dual_pivot::dual_pivot_quicksort(numbers.begin(), numbers.end());

// Sort with custom comparator
dual_pivot::dual_pivot_quicksort(numbers.begin(), numbers.end(), std::greater<int>());
```

### STL-Compatible Interface
```cpp
#include "include/stl_compatible.hpp"

std::vector<std::string> words = {"zebra", "apple", "banana"};
std_compatible::sort(words);  // Drop-in replacement for std::sort
```

### Advanced Optimized Version
```cpp
#include "include/dual_pivot_optimized.hpp"

std::vector<int> large_data = generate_large_dataset();
dual_pivot_optimized::dual_pivot_introsort(large_data.begin(), large_data.end());
```

### Different Data Types
```cpp
// Works with any comparable type
std::vector<double> floats = {3.14, 2.71, 1.41};
dual_pivot::dual_pivot_quicksort(floats.begin(), floats.end());

// Custom objects with comparison operators
struct Person { std::string name; int age; };
std::vector<Person> people = /* ... */;
dual_pivot::dual_pivot_quicksort(people.begin(), people.end(), 
    [](const Person& a, const Person& b) { return a.age < b.age; });
```

## 📈 Benchmarking

### Test Data Patterns
The benchmarking framework tests against multiple data patterns:
- **Random permutations**: Baseline performance
- **Nearly sorted**: 90% sorted with random swaps  
- **Reverse sorted**: Worst-case scenario for many algorithms
- **Many duplicates**: 10%, 50%, 90% unique values
- **Organ pipe**: Ascending then descending pattern
- **Sawtooth**: Repeating ascending patterns

### Array Sizes
- **Small**: 10, 100, 1,000 elements
- **Medium**: 10,000, 100,000 elements
- **Large**: 1,000,000, 10,000,000 elements

### Comparison Algorithms
- `std::sort` (typically introsort)
- `std::stable_sort` (merge sort)
- Classic quicksort implementation
- Dual-pivot quicksort variants

## ✅ Testing and Verification

### Correctness Tests
- Empty and single-element arrays
- Basic sorting functionality
- Edge cases (all same elements, two elements)
- Large arrays (10,000+ elements)
- Different data types (int, double, string)
- Custom comparators
- Algorithm equivalence verification

### Running Tests
```bash
make test  # Run all unit tests
```

## 📚 Algorithm Background

### Dual-Pivot Quicksort Advantages
1. **Reduced swap operations**: 20% fewer swaps than classic quicksort
2. **Better cache locality**: Optimized memory access patterns
3. **Efficient duplicate handling**: Superior performance on real-world data
4. **Proven effectiveness**: Adopted in Java 7 standard library

### Key Innovation
Instead of the classic two-way partitioning:
```
[< pivot] [>= pivot]
```

Dual-pivot uses three-way partitioning:
```
[< P1] [P1 <= && <= P2] [> P2]
```

This reduces the number of elements that need to be moved and improves cache performance.

## 🤝 Contributing

Contributions are welcome! Areas for improvement:
- Additional platform-specific optimizations
- Parallel sorting variants
- SIMD optimizations
- Enhanced benchmarking tools

## 📄 License

This implementation is provided for educational and research purposes. Please refer to the original papers for theoretical background and citations.

## 📖 References

1. Yaroslavskiy, V. "Dual-Pivot Quicksort" (2009)
2. Wild, S. "Why Is Dual-Pivot Quicksort Fast?"
3. Java 7 Arrays.sort() implementation
4. Modern C++ sorting algorithm best practices

## 🎉 Acknowledgments

This implementation is based on the groundbreaking work of Vladimir Yaroslavskiy and the theoretical analysis by Sebastian Wild, adapted for modern C++ with additional optimizations and comprehensive testing framework.