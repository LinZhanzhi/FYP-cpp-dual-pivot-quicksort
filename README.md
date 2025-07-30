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
│   ├── data_generator.hpp            # Test data generation (multi-type support)
│   ├── timer.hpp                     # Performance measurement
│   ├── main_benchmark.cpp            # Comprehensive benchmarks
│   ├── multi_type_benchmark.cpp      # Multi-type performance analysis (NEW!)
│   └── quick_benchmark.cpp           # Quick demo benchmarks
├── scripts/                          # Plotting and analysis tools
│   ├── plot_benchmark.py             # Performance visualization
│   ├── plot_multi_type_benchmark.py  # Multi-type analysis plots (NEW!)
│   ├── demo_benchmark.py             # Demo data generation
│   └── test_plotting.py              # Plotting functionality tests
├── tests/                            # Unit tests
│   ├── unit_tests.hpp                # Correctness verification
│   └── run_tests.cpp                 # Test runner
├── docs/                            # Documentation
│   ├── cppPlan.md                   # Implementation plan
│   ├── implementation_report.md      # Detailed analysis
│   ├── multiTypePlan.md             # Multi-type benchmark plan (NEW!)
│   ├── DualPivotQuicksort.md        # Original algorithm paper
│   └── Why Is Dual-Pivot Quicksort Fast.md # Performance analysis
├── results/                         # Benchmark results (generated)
│   ├── benchmark_results.csv        # Raw performance data
│   ├── multi_type_benchmark_results.csv # Multi-type analysis data (NEW!)
│   └── plots/                       # Performance visualizations
├── run_benchmark_demo.bat           # Windows demo script
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
- **Multi-type performance analysis** testing primitive data types (NEW!)

### Advanced Features
- **Introsort-style hybrid** with depth limiting and fallback to heapsort
- **Adaptive algorithm selection** for nearly-sorted data
- **Enhanced sampling** for large arrays
- **Quickselect variant** for partial sorting

## 🔧 Building and Running

### Prerequisites
- C++17 compatible compiler (GCC, Clang, or MSVC)
- Make utility (optional, for convenient building)
- **Python 3.x with matplotlib and pandas** (for performance visualization)

#### Install Python Dependencies
```bash
pip install matplotlib pandas
```

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

# Run comprehensive benchmarks with automatic plot generation
make run_benchmark

# Run quick benchmark demonstration (recommended for testing)
make run_quick_benchmark

# Run multi-type data analysis benchmarks (NEW!)
make run_multi_type_benchmark

# Run tests with different optimization levels
make run_all_optimizations

# Run unit tests
make test

# Clean build artifacts
make clean

# Show all available commands
make help
```

### 🚀 Quick Benchmark Demo

For a quick demonstration of the plotting functionality:

```bash
# Option 1: Windows Batch Script (Recommended)
run_benchmark_demo.bat

# Option 2: Manual plotting with existing data
cd results
python ../scripts/plot_benchmark.py benchmark_results.csv

# Option 3: Generate new demo data and plots
python scripts/demo_benchmark.py
```

### Manual Compilation

```bash
# Optimized build
g++ -std=c++17 -O3 -march=native -I./include benchmarks/main_benchmark.cpp -o benchmark

# Debug build
g++ -std=c++17 -g -O0 -I./include benchmarks/main_benchmark.cpp -o benchmark_debug
```

### 🔬 Multi-Type Performance Analysis (NEW!)

A new benchmarking system to analyze how different primitive data types affect dual-pivot quicksort performance:

#### Data Types Tested
- **char** (1 byte) - Minimal memory footprint, maximum cache efficiency
- **short** (2 bytes) - Small integer type
- **int** (4 bytes) - Standard integer baseline
- **long** (4 bytes/8 bytes) - Platform-dependent integer type
- **float** (4 bytes) - Single-precision IEEE 754
- **double** (8 bytes) - Double-precision IEEE 754

#### Key Analysis Features
- **Memory hierarchy effects** - How type size impacts cache performance
- **Comparison overhead analysis** - Type-specific comparison costs
- **Memory bandwidth utilization** - Throughput analysis per type
- **Algorithm consistency** - Performance stability across types

#### Running Multi-Type Benchmarks
```bash
# Build and run multi-type analysis
make run_multi_type_benchmark

# Manual execution
make multi_type_benchmark
cd results && ./multi_type_benchmark.exe
```

#### Generated Multi-Type Plots
1. **Performance by Pattern** - Algorithm comparison across types and patterns
2. **Type Size Correlation** - Performance vs memory footprint analysis  
3. **Algorithm Performance Heatmap** - Comprehensive type×pattern×algorithm view
4. **Relative Performance** - Dual-pivot efficiency compared to std::sort
5. **Analysis Report** - Detailed findings and performance insights

#### Key Findings (Sample Results)
- **Smaller types** (char, short) show ~5-10% better performance due to cache efficiency
- **Consistent algorithm behavior** across all primitive types
- **Memory access patterns** scale predictably with type size
- **Dual-pivot maintains advantage** regardless of data type

### 📊 Performance Visualization

### 🎯 Automatic Plot Generation

The benchmark system now automatically generates comprehensive performance visualizations alongside CSV results:

#### Generated Plot Types

1. **Individual Pattern Plots** (`performance_*.png`)
   - Time vs. array size for each data pattern
   - Compare all algorithms on the same graph
   - Log scales for better visualization of large ranges

2. **Summary Comparison** (`performance_summary.png`)
   - All data patterns in a single multi-subplot view
   - Quick overview of algorithm performance across patterns

3. **Speedup Analysis** (`speedup_analysis.png`)
   - Performance relative to `std::sort` baseline
   - Values >1.0 indicate faster performance
   - Identify which algorithms excel on specific patterns

#### Plot Features

- **Professional styling** with distinct colors per algorithm
- **High resolution** (300 DPI) PNG output
- **Automatic log scaling** for large data ranges
- **Clear legends and labels** for easy interpretation
- **Grid lines** for precise value reading

### 📈 Understanding the Results

#### Performance Patterns to Look For

- **Random Data**: Generally shows theoretical performance characteristics
- **Nearly Sorted**: Dual-pivot should show significant advantages
- **Reverse Sorted**: Classic quicksort often performs poorly
- **Many Duplicates**: Dual-pivot excels with optimized duplicate handling
- **Organ Pipe & Sawtooth**: Tests pattern-specific optimizations

#### Algorithm Comparison

- **std::sort**: Typically introsort hybrid (quicksort + heapsort)
- **classic_quicksort**: Traditional single-pivot implementation
- **dual_pivot_quicksort**: Core Yaroslavskiy algorithm
- **dual_pivot_optimized**: Enhanced with introsort-style improvements

### 📊 Performance Characteristics

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

### 🎯 Quick Start - Running Benchmarks

#### Option 1: Windows Batch Script (Easiest)
```bash
# Run this from the project root directory
run_benchmark_demo.bat
```

#### Option 2: Make Commands (Cross-platform)
```bash
# Quick demo benchmark (recommended for first run)
make run_quick_benchmark

# Full comprehensive benchmark (takes longer)
make full_benchmark

# Generate plots from existing data
cd results
python ../scripts/plot_benchmark.py benchmark_results.csv
```

#### Option 3: Manual Python Demo
```bash
# Generate demo data and plots
python scripts/demo_benchmark.py
```

### 📊 Output Files

After running benchmarks, you'll find:

```
results/
├── benchmark_results.csv          # Raw timing data
└── plots/                         # Performance visualizations
    ├── performance_random.png     # Random data performance
    ├── performance_nearly_sorted.png
    ├── performance_reverse_sorted.png
    ├── performance_summary.png    # All patterns overview
    └── speedup_analysis.png       # Relative performance
```

### 📋 Benchmark Details

#### Test Data Patterns
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

### Running Tests
```bash
make test  # Run all unit tests
```

## 🔧 Troubleshooting

### Common Issues and Solutions

#### Python/Plotting Issues
```bash
# If plotting fails with "module not found"
pip install matplotlib pandas

# If using conda
conda install matplotlib pandas

# Test plotting functionality
cd results
python ../scripts/plot_benchmark.py benchmark_results.csv
```

#### Windows Build Issues
```bash
# If make command not found
winget install BrechtSanders.WinLibs.POSIX.UCRT

# Alternative: Use manual compilation
g++ -std=c++17 -O3 -march=native -I./include benchmarks/main_benchmark.cpp -o benchmark.exe

# Run Windows demo script
run_benchmark_demo.bat
```

#### Benchmark Execution Issues
```bash
# If benchmark crashes or takes too long, use quick demo
make run_quick_benchmark

# Or generate plots from existing data
cd results && python ../scripts/plot_benchmark.py benchmark_results.csv

# Manual demo with sample data
python scripts/demo_benchmark.py
```

#### Plot Generation Problems
```bash
# Test with demo data first
python scripts/demo_benchmark.py

# Check if plots directory exists
ls results/plots/

# Manually run plotting script
cd results
python ../scripts/plot_benchmark.py benchmark_results.csv
```

### Performance Tips

- Use `run_quick_benchmark` for quick testing
- Full benchmarks can take 10+ minutes on large datasets
- Plot generation requires Python with matplotlib and pandas
- Windows users: Use the provided batch script for easiest setup

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