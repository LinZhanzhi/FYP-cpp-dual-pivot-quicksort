# Appendix D: Build and Usage Instructions

This appendix provides comprehensive instructions for building, testing, and using the dual-pivot quicksort library.

---

## D.1 Prerequisites

| Requirement | Minimum Version | Recommended |
|-------------|-----------------|-------------|
| **C++ Compiler** | C++17 support | GCC 13+, Clang 15+, MSVC 2022 |
| **Build System** | Make or CMake 3.15+ | CMake 3.20+ |
| **Python** | 3.8+ (benchmarks only) | 3.10+ |
| **OS** | Linux, Windows, macOS | Ubuntu 22.04+, Windows 11 |

---

## D.2 Installation

This is a **header-only library**. No compilation required for integration.

### Method 1: Direct Include
```bash
# Clone repository
git clone https://github.com/LinZhanzhi/FYP-cpp-dual-pivot-quicksort.git

# Copy include directory to your project
cp -r FYP-cpp-dual-pivot-quicksort/include /path/to/your/project/
```

### Method 2: Git Submodule
```bash
cd your_project
git submodule add https://github.com/LinZhanzhi/FYP-cpp-dual-pivot-quicksort.git external/dpqs
```

### Method 3: CMake FetchContent
```cmake
include(FetchContent)
FetchContent_Declare(
    dpqs
    GIT_REPOSITORY https://github.com/LinZhanzhi/FYP-cpp-dual-pivot-quicksort.git
    GIT_TAG main
)
FetchContent_MakeAvailable(dpqs)

target_include_directories(your_target PRIVATE ${dpqs_SOURCE_DIR}/include)
```

---

## D.3 Compilation Flags

### Recommended Flags (Production)
```bash
g++ -std=c++17 -O2 -march=native -DNDEBUG -Iinclude your_code.cpp -o your_program -pthread
```

| Flag | Purpose |
|------|---------|
| `-std=c++17` | Required for `if constexpr`, structured bindings |
| `-O2` | Optimal optimization level (see §D.9) |
| `-march=native` | Enable CPU-specific instructions |
| `-DNDEBUG` | Disable assertions |
| `-pthread` | Required for parallel execution |

### Debug Flags
```bash
g++ -std=c++17 -O0 -g -fsanitize=address,undefined -Iinclude your_code.cpp -o debug_program -pthread
```

---

## D.4 Basic Usage Examples

### Example 1: Sort a Vector
```cpp
#include "dual_pivot_quicksort.hpp"
#include <vector>
#include <iostream>

int main() {
    std::vector<int> data = {64, 34, 25, 12, 22, 11, 90};

    // Sort using container adapter (auto-parallel)
    dual_pivot::sort(data);

    for (int x : data) {
        std::cout << x << " ";
    }
    // Output: 11 12 22 25 34 64 90
    return 0;
}
```

### Example 2: Sort with Custom Comparator
```cpp
#include "dual_pivot_quicksort.hpp"
#include <vector>
#include <string>

struct Person {
    std::string name;
    int age;
};

int main() {
    std::vector<Person> people = {
        {"Alice", 30}, {"Bob", 25}, {"Charlie", 35}
    };

    // Sort by age descending
    dual_pivot::sort(people, [](const Person& a, const Person& b) {
        return a.age > b.age;
    });

    // Result: Charlie(35), Alice(30), Bob(25)
    return 0;
}
```

### Example 3: Explicit Thread Control
```cpp
#include "dual_pivot_quicksort.hpp"
#include <vector>

int main() {
    std::vector<double> data(10'000'000);
    // ... fill data ...

    // Use exactly 8 threads
    dual_pivot::sort(data, 8);

    // Or use sequential mode
    dual_pivot::sort(data, 1);

    return 0;
}
```

### Example 4: Iterator Interface (STL Style)
```cpp
#include "dual_pivot_quicksort.hpp"
#include <vector>
#include <array>

int main() {
    std::vector<int> vec = {5, 2, 8, 1, 9};
    std::array<int, 5> arr = {5, 2, 8, 1, 9};

    // Sort using iterators
    dual_pivot::dual_pivot_quicksort(vec.begin(), vec.end());
    dual_pivot::dual_pivot_quicksort(arr.begin(), arr.end());

    // Parallel version
    dual_pivot::dual_pivot_quicksort_parallel(vec.begin(), vec.end(), 4);

    return 0;
}
```

### Example 5: Raw Array Sorting
```cpp
#include "dual_pivot_quicksort.hpp"

int main() {
    int data[] = {64, 34, 25, 12, 22, 11, 90};
    size_t n = sizeof(data) / sizeof(data[0]);

    // Sort raw array
    dual_pivot::sort(data, n);

    // With explicit parallelism
    dual_pivot::sort(data, 4, 0, n);  // 4 threads, range [0, n)

    return 0;
}
```

---

## D.5 API Reference

### Container Functions

```cpp
namespace dual_pivot {

// Auto-parallel sort (uses hardware_concurrency threads)
template<typename Container>
void sort(Container& container);

// Sort with custom comparator
template<typename Container, typename Compare>
void sort(Container& container, Compare comp);

// Sort with explicit thread count
template<typename Container>
void sort(Container& container, int parallelism);

// Full control (comparator + parallelism)
template<typename Container, typename Compare>
void sort(Container& container, int parallelism, Compare comp);

}
```

### Pointer Functions

```cpp
namespace dual_pivot {

// Sort array by length
template<typename T>
void sort(T* array, std::ptrdiff_t length);

// Sort with range
template<typename T>
void sort(T* array, int parallelism, std::ptrdiff_t low, std::ptrdiff_t high);

// With comparator
template<typename T, typename Compare>
void sort(T* array, int parallelism, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp);

}
```

### Iterator Functions

```cpp
namespace dual_pivot {

// Sequential (STL-style)
template<typename RandomAccessIterator>
void dual_pivot_quicksort(RandomAccessIterator first, RandomAccessIterator last);

// With comparator
template<typename RandomAccessIterator, typename Compare>
void dual_pivot_quicksort(RandomAccessIterator first, RandomAccessIterator last, Compare comp);

// Parallel
template<typename RandomAccessIterator>
void dual_pivot_quicksort_parallel(RandomAccessIterator first, RandomAccessIterator last,
                                   int parallelism = std::thread::hardware_concurrency());

}
```

---

## D.6 Running Tests

### Compile and Run Individual Tests
```bash
cd test

# Core algorithm test
g++ -std=c++17 -O2 -Iinclude test_dual_pivot_quicksort.cpp -o test_dpqs -pthread
./test_dpqs

# Counting sort test
g++ -std=c++17 -O2 -Iinclude test_counting_sort.cpp -o test_counting -pthread
./test_counting

# Float handling test
g++ -std=c++17 -O2 -Iinclude test_float_sort.cpp -o test_float -pthread
./test_float

# Partitioning test
g++ -std=c++17 -O2 -Iinclude test_partition.cpp -o test_partition -pthread
./test_partition
```

### Run All Tests
```bash
# Using make
make test

# Or manually
for test in test/*.cpp; do
    g++ -std=c++17 -O2 -Iinclude "$test" -o "${test%.cpp}" -pthread
    "./${test%.cpp}" || echo "FAILED: $test"
done
```

---

## D.7 Running Benchmarks

### Web Dashboard (Recommended)
```bash
cd benchmarks
python server.py
# Open http://localhost:8000
```

### Command Line
```bash
# Build benchmark runner
cd benchmarks
mkdir -p build
g++ -std=c++17 -O2 -march=native -DNDEBUG -Iinclude -I../include \
    src/benchmark_runner.cpp -o build/benchmark_runner -pthread

# Run specific benchmark
./build/benchmark_runner --algorithm dpqs_sequential --type int --pattern RANDOM --size 1000000

# Run full benchmark suite
make run
```

### Benchmark Manager
```bash
cd benchmarks
python benchmark_manager.py --algorithms all --types int,double --patterns RANDOM,ORGAN_PIPE --sizes 1000000,10000000
```

---

## D.8 Constant Override at Compile Time

Override tuning constants without modifying source:

```bash
# Custom insertion sort threshold
g++ -DMAX_INSERTION_SORT_SIZE=80 -std=c++17 -O2 ... your_code.cpp

# Custom parallel granularity
g++ -DMIN_PARALLEL_SORT_SIZE=65536 -std=c++17 -O2 ... your_code.cpp

# Multiple overrides
g++ -DMAX_INSERTION_SORT_SIZE=50 -DMIN_FIRST_RUNS_FACTOR=7 -std=c++17 -O2 ... your_code.cpp
```

**Available Constants**:
| Constant | Default | Description |
|----------|---------|-------------|
| `MAX_INSERTION_SORT_SIZE` | 60 | Insertion sort threshold |
| `MIN_PARALLEL_SORT_SIZE` | 8192 | Min size for parallel tasks |
| `MIN_FIRST_RUNS_FACTOR` | 6 | Run quality heuristic |
| `MIN_BYTE_COUNTING_SORT_SIZE` | 64 | Counting sort threshold (1B) |
| `MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE` | 1750 | Counting sort threshold (2B) |

---

## D.9 Compiler Optimization Notes

### Why `-O2` Not `-O3`?

Our benchmarks show `-O3` provides **no benefit** for sorting:

| Flags | Runtime (10M int) | Notes |
|-------|-------------------|-------|
| `-O2` | 190.2 ms | Baseline |
| `-O3` | 191.5 ms | +0.7% (worse) |
| `-O3 -flto` | 195.8 ms | +2.9% (worse) |

**Reason**: Sorting is branch-heavy. `-O3` aggressive loop unrolling hurts branch prediction.

### Why `-march=native`?

Enables CPU-specific optimizations:

| Flags | Runtime (10M int) | Notes |
|-------|-------------------|-------|
| Without | 193.1 ms | Generic x86-64 |
| `-march=native` | 190.2 ms | -1.5% |

**Enables**: Better prefetch instructions, optimized memory barriers.

---

## D.10 Integration Examples

### CMake Integration
```cmake
cmake_minimum_required(VERSION 3.15)
project(my_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=native -pthread")

# Add include path
include_directories(path/to/dpqs/include)

add_executable(my_app main.cpp)
```

### Makefile Integration
```makefile
CXX = g++
CXXFLAGS = -std=c++17 -O2 -march=native -DNDEBUG -Ipath/to/dpqs/include
LDFLAGS = -pthread

my_app: main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)
```

---

## D.11 Troubleshooting

### Common Issues

| Issue | Cause | Solution |
|-------|-------|----------|
| `error: 'if constexpr'` | Old compiler | Use GCC 7+, Clang 5+, or MSVC 2017+ |
| Linker errors with `pthread` | Missing flag | Add `-pthread` to both compile and link |
| Slow performance | Debug mode | Ensure `-O2 -DNDEBUG` is set |
| Deadlock on small arrays | Rare race | Update to latest version |

### Debugging Parallel Issues
```bash
# Enable thread sanitizer
g++ -std=c++17 -O1 -g -fsanitize=thread -Iinclude your_code.cpp -o debug -pthread
./debug
```

---

## D.12 Platform-Specific Notes

### Windows (MinGW-w64)
```bash
g++ -std=c++17 -O2 -march=native -DNDEBUG -Iinclude your_code.cpp -o your_program.exe
# Note: -pthread not needed on Windows (uses native threads)
```

### Windows (MSVC)
```powershell
cl /std:c++17 /O2 /EHsc /Iinclude your_code.cpp
```

### macOS
```bash
clang++ -std=c++17 -O2 -march=native -DNDEBUG -Iinclude your_code.cpp -o your_program -pthread
```

### Linux
```bash
g++ -std=c++17 -O2 -march=native -DNDEBUG -Iinclude your_code.cpp -o your_program -pthread
```

---

## D.13 License

This project is released under the MIT License. See `LICENSE` file for details.

```
MIT License

Copyright (c) 2025-2026 Lin Zhanzhi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software...
```
