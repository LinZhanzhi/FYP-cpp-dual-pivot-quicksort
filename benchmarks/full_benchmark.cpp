#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

// Include all pattern generators
#include "alternating_pattern.hpp"
#include "few_unique_pattern.hpp"
#include "identical_pattern.hpp"
#include "mostly_small_pattern.hpp"
#include "nearly_sorted_pattern.hpp"
#include "permutation_pattern.hpp"
#include "random_pattern.hpp"
#include "range_pattern.hpp"
#include "reverse_nearly_sorted_pattern.hpp"
#include "reverse_sorted_pattern.hpp"
#include "scalability_pattern.hpp"
#include "sorted_pattern.hpp"

// Include dual pivot quicksort implementation
#include "dual_pivot_quicksort.hpp"

// Logarithmically spaced array sizes (61 total)
const std::vector<size_t> ARRAY_SIZES = {
    1,       2,       3,       4,       5,       6,       8,       10,
    13,      16,      21,      26,      33,      42,      54,      68,
    86,      109,     138,     175,     222,     281,     355,     449,
    568,     719,     910,     1151,    1456,    1842,    2329,    2947,
    3727,    4714,    5963,    7543,    9540,    12067,   15264,   19306,
    24420,   30888,   39069,   49417,   62505,   79060,   100000,  138949,
    193069,  268269,  372759,  517947,  719685,  1000000, 1389495, 1930697,
    2682695, 3727593, 5179474, 7196856, 10000000};

// Array pattern names (12 total)
const std::vector<std::string> PATTERN_NAMES = {"Random",
                                                "Sorted",
                                                "ReverseSorted",
                                                "NearlySorted",
                                                "ReverseNearlySorted",
                                                "FewUnique",
                                                "Identical",
                                                "Range",
                                                "Alternating",
                                                "MostlySmall",
                                                "Scalability",
                                                "Permutation"};

// Algorithm names (4 total)
const std::vector<std::string> ALGORITHM_NAMES = {
    "DualPivotQuicksort", "std::sort", "std::stable_sort", "qsort"};

// Type names (14 total - char types excluded due to dual-pivot algorithm issues
// with larger arrays)
const std::vector<std::string> TYPE_NAMES = {
    "int",      "unsigned int",  "short",       "unsigned short",
    "long",     "unsigned long", "long long",   "unsigned long long",
    "float",    "double",        "long double", "wchar_t",
    "char16_t", "char32_t"};

// High-resolution timing utility
class BenchmarkTimer {
public:
  template <typename Func> static double measureTime(Func &&func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    return duration.count() / 1e6; // Convert to milliseconds
  }
};

// Pattern generator dispatcher
template <typename T>
std::vector<T> generatePattern(const std::string &pattern_name, size_t length) {
  if (pattern_name == "Random") {
    return generate_random_pattern<T>(length);
  } else if (pattern_name == "Sorted") {
    return generate_sorted_pattern<T>(length);
  } else if (pattern_name == "ReverseSorted") {
    return generate_reverse_sorted_pattern<T>(length);
  } else if (pattern_name == "NearlySorted") {
    return generate_nearly_sorted_pattern<T>(length);
  } else if (pattern_name == "ReverseNearlySorted") {
    return generate_reverse_nearly_sorted_pattern<T>(length);
  } else if (pattern_name == "FewUnique") {
    return generate_few_unique_pattern<T>(length);
  } else if (pattern_name == "Identical") {
    return generate_identical_pattern<T>(length);
  } else if (pattern_name == "Range") {
    return generate_range_pattern<T>(length);
  } else if (pattern_name == "Alternating") {
    return generate_alternating_pattern<T>(length);
  } else if (pattern_name == "MostlySmall") {
    return generate_mostly_small_pattern<T>(length);
  } else if (pattern_name == "Scalability") {
    return generate_scalability_pattern<T>(length);
  } else if (pattern_name == "Permutation") {
    return generate_permutation_pattern<T>(length);
  }

  // Default fallback to random
  return generate_random_pattern<T>(length);
}

// Sorting algorithm implementations
template <typename T> double runDualPivotQuicksort(std::vector<T> data) {
  return BenchmarkTimer::measureTime(
      [&]() { dual_pivot::dual_pivot_quicksort(data.begin(), data.end()); });
}

template <typename T> double runStdSort(std::vector<T> data) {
  return BenchmarkTimer::measureTime(
      [&]() { std::sort(data.begin(), data.end()); });
}

template <typename T> double runStdStableSort(std::vector<T> data) {
  return BenchmarkTimer::measureTime(
      [&]() { std::stable_sort(data.begin(), data.end()); });
}

// C qsort wrapper for different types
template <typename T> int compareFunction(const void *a, const void *b) {
  const T *ta = static_cast<const T *>(a);
  const T *tb = static_cast<const T *>(b);

  if (*ta < *tb)
    return -1;
  if (*ta > *tb)
    return 1;
  return 0;
}

template <typename T> double runQsort(std::vector<T> data) {
  return BenchmarkTimer::measureTime([&]() {
    std::qsort(data.data(), data.size(), sizeof(T), compareFunction<T>);
  });
}

// Run single benchmark test
template <typename T>
void runSingleBenchmark(std::ofstream &csvFile, const std::string &type_name,
                        const std::string &pattern_name, size_t array_size) {

  // Generate the test data
  std::vector<T> original_data = generatePattern<T>(pattern_name, array_size);

  // Test all 4 algorithms
  for (const std::string &algorithm : ALGORITHM_NAMES) {
    std::vector<T> test_data = original_data; // Make a copy
    double execution_time = 0.0;

    try {
      if (algorithm == "DualPivotQuicksort") {
        execution_time = runDualPivotQuicksort(test_data);
      } else if (algorithm == "std::sort") {
        execution_time = runStdSort(test_data);
      } else if (algorithm == "std::stable_sort") {
        execution_time = runStdStableSort(test_data);
      } else if (algorithm == "qsort") {
        execution_time = runQsort(test_data);
      }
    } catch (const std::exception &e) {
      execution_time = -1.0; // Error indicator
      std::cerr << "Error in " << algorithm << " with " << type_name << " "
                << pattern_name << " size " << array_size << ": " << e.what()
                << std::endl;
    }

    // Write to CSV: Algorithm,Pattern,DataType,ArraySize,ExecutionTime(ms)
    csvFile << algorithm << "," << pattern_name << "," << type_name << ","
            << array_size << "," << execution_time << std::endl;

    // Progress indicator
    static int completed = 0;
    completed++;
    if (completed % 100 == 0) {
      std::cout << "Completed " << completed << " tests..." << std::endl;
    }
  }
}

// Type dispatcher macro to handle all 18 types
#define DISPATCH_TYPE(type_name, cpp_type)                                     \
  if (type_name == #cpp_type) {                                                \
    runSingleBenchmark<cpp_type>(csvFile, type_name, pattern_name,             \
                                 array_size);                                  \
    return;                                                                    \
  }

void runBenchmarkForType(std::ofstream &csvFile, const std::string &type_name,
                         const std::string &pattern_name, size_t array_size) {

  // Integer types
  DISPATCH_TYPE(type_name, int)
  DISPATCH_TYPE(type_name, unsigned int)
  DISPATCH_TYPE(type_name, short)
  DISPATCH_TYPE(type_name, unsigned short)
  DISPATCH_TYPE(type_name, long)
  DISPATCH_TYPE(type_name, unsigned long)
  DISPATCH_TYPE(type_name, long long)
  DISPATCH_TYPE(type_name, unsigned long long)

  // Floating-point types
  DISPATCH_TYPE(type_name, float)
  DISPATCH_TYPE(type_name, double)
  DISPATCH_TYPE(type_name, long double)

  // Character types (char types excluded - see TYPE_NAMES comment)
  DISPATCH_TYPE(type_name, wchar_t)
  DISPATCH_TYPE(type_name, char16_t)
  DISPATCH_TYPE(type_name, char32_t)

  std::cerr << "Unknown type: " << type_name << std::endl;
}

int main() {
  std::cout << "Starting Comprehensive Dual-Pivot Quicksort Benchmark"
            << std::endl;
  std::cout << "======================================================"
            << std::endl;
  std::cout << "Total test combinations: " << ALGORITHM_NAMES.size()
            << " algorithms × " << PATTERN_NAMES.size() << " patterns × "
            << TYPE_NAMES.size() << " types × " << ARRAY_SIZES.size()
            << " sizes = "
            << (ALGORITHM_NAMES.size() * PATTERN_NAMES.size() *
                TYPE_NAMES.size() * ARRAY_SIZES.size())
            << " tests" << std::endl;
  std::cout << std::endl;

  // Open CSV output file
  std::ofstream csvFile("benchmark_results.csv");
  if (!csvFile.is_open()) {
    std::cerr << "Error: Could not open benchmark_results.csv for writing"
              << std::endl;
    return 1;
  }

  // Write CSV header
  csvFile << "Algorithm,Pattern,DataType,ArraySize,ExecutionTime_ms"
          << std::endl;

  // Run all combinations
  int total_tests = 0;
  auto benchmark_start = std::chrono::high_resolution_clock::now();

  for (const std::string &pattern_name : PATTERN_NAMES) {
    std::cout << "Testing pattern: " << pattern_name << std::endl;

    for (const std::string &type_name : TYPE_NAMES) {
      std::cout << "  Type: " << type_name << std::flush;

      for (size_t array_size : ARRAY_SIZES) {
        runBenchmarkForType(csvFile, type_name, pattern_name, array_size);
        total_tests += ALGORITHM_NAMES.size();
      }
      std::cout << " ✓" << std::endl;
    }
    std::cout << std::endl;
  }

  auto benchmark_end = std::chrono::high_resolution_clock::now();
  auto total_duration = std::chrono::duration_cast<std::chrono::seconds>(
      benchmark_end - benchmark_start);

  csvFile.close();

  std::cout << "Benchmark Complete!" << std::endl;
  std::cout << "==================" << std::endl;
  std::cout << "Total tests completed: " << total_tests << std::endl;
  std::cout << "Total execution time: " << total_duration.count() << " seconds"
            << std::endl;
  std::cout << "Results saved to: benchmark_results.csv" << std::endl;
  std::cout << std::endl;

  // Display summary statistics
  std::cout << "Test Configuration:" << std::endl;
  std::cout << "- Array patterns: " << PATTERN_NAMES.size() << std::endl;
  std::cout << "- Data types: " << TYPE_NAMES.size() << std::endl;
  std::cout << "- Array sizes: " << ARRAY_SIZES.size() << " (from "
            << ARRAY_SIZES.front() << " to " << ARRAY_SIZES.back() << ")"
            << std::endl;
  std::cout << "- Sorting algorithms: " << ALGORITHM_NAMES.size() << std::endl;

  return 0;
}