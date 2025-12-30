# Dual-Pivot Quicksort C++ Implementation

A high-performance, robust C++ implementation of Vladimir Yaroslavskiy's Dual-Pivot Quicksort algorithm. This project is part of a Final Year Project (FYP) focusing on advanced sorting algorithms and their optimization in C++.

## 🌟 Key Features

*   **Dual-Pivot Quicksort:** Implements the optimized three-way partitioning algorithm.
*   **Parallel Execution:** Automatic parallelization for large arrays using `std::thread`.
*   **Type-Specific Optimizations:**
    *   **Counting Sort:** Automatically used for small integral types (byte, short, char).
    *   **Float Sort:** Specialized handling for floating-point numbers (NaNs, -0.0).
*   **STL Compatibility:** Supports `std::vector`, arrays, and random-access iterators.
*   **Custom Comparators:** Fully supports custom comparison functions.
*   **Robustness:** Handles edge cases like duplicate elements, already sorted arrays, and different data types.

## 📦 Installation

This is a header-only library. To use it, simply include the `include/dual_pivot_quicksort.hpp` file in your project.

1.  Clone the repository:
    ```bash
    git clone https://github.com/LinZhanzhi/FYP-cpp-dual-pivot-quicksort.git
    ```
2.  Add the `include` directory to your include path.

## 🚀 Usage

### Basic Usage

```cpp
#include "include/dual_pivot_quicksort.hpp"
#include <vector>
#include <iostream>

int main() {
    std::vector<int> data = {64, 34, 25, 12, 22, 11, 90};
    
    // Sort using the container adapter
    dual_pivot::sort(data);
    
    for (int x : data) std::cout << x << " ";
    return 0;
}
```

### Iterator Interface (STL Style)

```cpp
#include "include/dual_pivot_quicksort.hpp"
#include <vector>

int main() {
    std::vector<int> data = {64, 34, 25, 12, 22, 11, 90};
    
    // Sort using iterators
    dual_pivot::dual_pivot_quicksort(data.begin(), data.end());
    
    return 0;
}
```

### Parallel Sorting

For large datasets, you can enable parallel execution. The library automatically switches to parallel mode for large arrays if parallelism is allowed.

```cpp
#include "include/dual_pivot_quicksort.hpp"
#include <vector>
#include <thread>

int main() {
    std::vector<int> large_data(1000000);
    // ... fill data ...

    // Explicitly request parallel execution
    int num_threads = std::thread::hardware_concurrency();
    dual_pivot::dual_pivot_quicksort_parallel(large_data.begin(), large_data.end(), num_threads);
    
    return 0;
}
```

### Custom Comparator

```cpp
struct CustomObj {
    int id;
    float value;
};

// Sort descending by value
dual_pivot::sort(objects, [](const CustomObj& a, const CustomObj& b) {
    return a.value > b.value; 
});
```

## 🧪 Running Tests

The project includes a comprehensive test suite located in the `test/` directory.

### Prerequisites
*   C++17 compatible compiler (GCC, Clang, MSVC)
*   Python 3.x (for benchmarks)

### Compiling and Running Tests

You can compile individual tests using `g++`. For example:

```bash
# Compile and run the main dual-pivot quicksort test
g++ -std=c++17 -Iinclude test/test_dual_pivot_quicksort.cpp -o test_dpqs
./test_dpqs
```

Other available tests in `test/`:
*   `test_counting_sort.cpp`: Tests for the counting sort optimization.
*   `test_float_sort.cpp`: Tests for floating-point handling.
*   `test_partition.cpp`: Tests for the partitioning logic.

## 📊 Benchmarking & Visualization

The project includes a sophisticated benchmarking suite with a web-based interface for managing tests and visualizing performance data.

### Interactive Web Interface

The easiest way to run benchmarks and view results is through the included web dashboard.

1.  Navigate to the `benchmarks` directory:
    ```bash
    cd benchmarks
    ```
2.  Start the server:
    ```bash
    python3 server.py
    ```
3.  Open your browser and go to `http://localhost:8000`.

From the dashboard, you can:
*   **Manage Tests:** Select specific algorithms, data types, patterns, and sizes to run.
*   **Visualize:** View interactive charts comparing the performance of different algorithms.
*   **Playground:** Experiment with different settings.

### Command Line Interface

You can also run benchmarks directly using the manager script:

1.  Build the benchmark runner:
    ```bash
    cd benchmarks
    mkdir -p build
    cd build
    cmake ..
    make
    cd ..
    ```

2.  Run the benchmark manager:
    ```bash
    python3 benchmark_manager.py
    ```

## 📂 Project Structure

*   `include/`: Header files for the library.
    *   `dual_pivot_quicksort.hpp`: Main entry point.
    *   `dpqs/`: Internal implementation details.
*   `test/`: Unit tests for various components.
*   `benchmarks/`: Benchmarking framework, web interface, and scripts.
*   `docs/`: Documentation, plans, and reports.
*   `report/`: Detailed implementation reports and analysis.

## 🎓 About This Project

This project was developed as a Final Year Project (FYP) to explore and optimize the Dual-Pivot Quicksort algorithm in C++. It demonstrates advanced C++ techniques, including template metaprogramming, multithreading, and cache optimization.

**Author:** Lin Zhanzhi
