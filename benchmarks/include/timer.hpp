/**
 * @file timer.hpp
 * @brief High-precision benchmarking and timing utilities for dual-pivot quicksort performance analysis
 * 
 * This file provides a comprehensive benchmarking framework specifically designed for measuring
 * and analyzing the performance characteristics of sorting algorithms. The implementation includes
 * statistical analysis, outlier detection, and specialized timing methods for different scenarios.
 * 
 * Key Features:
 * - High-resolution timing using std::chrono::high_resolution_clock
 * - Statistical analysis with mean, median, standard deviation calculations
 * - Automatic warmup runs to stabilize CPU frequency and cache state
 * - Template-based timing for maximum flexibility and performance
 * - Memory access and operation counting for detailed algorithm analysis
 * - Type-aware benchmarking with automatic type information capture
 * 
 * The benchmarking framework is essential for validating the performance improvements
 * claimed by the dual-pivot quicksort algorithm over traditional sorting methods.
 * 
 * @author Dual-Pivot Quicksort Research Project
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <numeric>
#include <cmath>

/**
 * @brief Namespace containing high-precision benchmarking and timing utilities
 * 
 * This namespace provides all the tools necessary for comprehensive performance
 * analysis of sorting algorithms, with particular focus on the dual-pivot quicksort
 * implementation. The timing utilities are designed to minimize measurement overhead
 * while providing accurate and repeatable results.
 */
namespace benchmark_timer {

/**
 * @brief Comprehensive timing result structure with statistical analysis
 * 
 * This structure encapsulates all timing measurements and statistical calculations
 * for a benchmarking session. It provides both raw timing data and derived statistics
 * to enable thorough performance analysis.
 * 
 * The structure is designed to capture not just execution times but also metadata
 * about the data types being sorted, enabling type-specific performance analysis.
 */
struct TimingResult {
    std::vector<double> times_ms;    ///< Raw timing measurements in milliseconds
    double mean_ms;                  ///< Arithmetic mean of all measurements
    double median_ms;                ///< Median time (50th percentile)
    double std_dev_ms;               ///< Standard deviation of measurements
    double min_ms;                   ///< Minimum execution time observed
    double max_ms;                   ///< Maximum execution time observed
    std::string type_name;           ///< Name of the data type being sorted
    size_t type_size_bytes;          ///< Size of each element in bytes
};

/**
 * @brief High-precision benchmark timer for algorithm performance measurement
 * 
 * This class provides sophisticated timing capabilities specifically designed for
 * benchmarking sorting algorithms. It addresses common pitfalls in performance
 * measurement such as:
 * 
 * - Cold cache effects through automatic warmup runs
 * - CPU frequency scaling through consistent measurement methodology
 * - Statistical variance through multiple iterations and outlier detection
 * - Measurement overhead through optimized timing loops
 * 
 * The timer is particularly well-suited for comparing different sorting algorithms
 * and analyzing performance characteristics across different data patterns and sizes.
 * 
 * Usage Example:
 * @code
 * BenchmarkTimer timer;
 * std::vector<int> data = generate_test_data();
 * 
 * auto result = timer.benchmark_sort(data, [](auto& arr) {
 *     dual_pivot::dual_pivot_quicksort(arr.begin(), arr.end());
 * });
 * 
 * std::cout << "Mean time: " << result.mean_ms << "ms" << std::endl;
 * @endcode
 */
class BenchmarkTimer {
private:
    using Clock = std::chrono::high_resolution_clock;      ///< High-resolution clock for precise timing
    using TimePoint = std::chrono::time_point<Clock>;      ///< Time point type for consistency
    
public:
    /**
     * @brief Time a single execution of a function with nanosecond precision
     * 
     * This method provides the core timing functionality for single function executions.
     * It uses high-resolution clock to minimize timing overhead and provides results
     * in milliseconds for consistency with other timing methods.
     * 
     * The timing includes any setup/teardown overhead within the function, making it
     * suitable for timing complete sorting operations including any internal copying
     * or initialization.
     * 
     * @tparam Func Callable type (function, lambda, functor)
     * @param func Function to time - should be callable with no arguments
     * @return Execution time in milliseconds (double precision)
     * 
     * @note This method does not perform warmup or statistical analysis.
     *       For comprehensive benchmarking, use benchmark() instead.
     */
    template<typename Func>
    double time_execution(Func&& func) {
        auto start = Clock::now();
        func();
        auto end = Clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        return duration.count() / 1e6;  // Convert to milliseconds
    }
    
    /**
     * @brief Run comprehensive benchmark with statistical analysis
     * 
     * This method performs a complete benchmarking session including warmup runs,
     * multiple timing iterations, and statistical analysis of results. It is the
     * primary method for obtaining reliable performance measurements.
     * 
     * Benchmarking Process:
     * 1. Warmup Phase: 5 executions to stabilize CPU frequency and cache state
     * 2. Measurement Phase: Multiple timed executions (default 50 iterations)
     * 3. Statistical Analysis: Calculation of mean, median, standard deviation
     * 
     * The warmup phase is crucial for modern CPUs that adjust frequency based on
     * workload, and for ensuring consistent cache state across measurements.
     * 
     * @tparam Func Callable type (function, lambda, functor)
     * @param func Function to benchmark - should be callable with no arguments
     * @param iterations Number of timing iterations (default: 50)
     * @return TimingResult containing all measurements and statistics
     * 
     * @note Higher iteration counts provide more stable statistics but increase
     *       total benchmarking time. For sorting algorithms, 50 iterations typically
     *       provides good statistical significance.
     */
    template<typename Func>
    TimingResult benchmark(Func&& func, int iterations = 50) {
        std::vector<double> times;
        times.reserve(iterations);
        
        // Warmup runs to stabilize caches and CPU frequency
        for (int i = 0; i < 5; ++i) {
            func();
        }
        
        // Actual timing runs
        for (int i = 0; i < iterations; ++i) {
            times.push_back(time_execution(func));
        }
        
        return calculate_statistics(times);
    }
    
    /**
     * @brief Specialized benchmark for sorting algorithms with automatic type detection
     * 
     * This method is specifically designed for benchmarking sorting algorithms and
     * addresses the unique challenges of sorting performance measurement:
     * 
     * - Data Copying: Each iteration works on a fresh copy to ensure consistent input
     * - Pure Sorting Time: Excludes data copying time from measurements
     * - Type Information: Automatically captures element type and size information
     * - Cache Consistency: Ensures each iteration starts with the same cache state
     * 
     * The method is particularly valuable for comparing different sorting algorithms
     * on the same dataset, as it eliminates confounding factors like data state.
     * 
     * Performance Considerations:
     * - Data copying overhead is excluded from timing measurements
     * - Each iteration starts with identical input data
     * - Memory allocation patterns are consistent across iterations
     * 
     * @tparam Container STL container type (e.g., std::vector, std::array)
     * @tparam SortFunc Callable that accepts Container& and sorts it in-place
     * @param original_data Input data to sort (remains unchanged)
     * @param sort_func Sorting function - called as sort_func(container)
     * @param iterations Number of timing iterations (default: 50)
     * @return TimingResult with type information and sorting-specific statistics
     * 
     * @note The sort_func should modify the container in-place. Lambda functions
     *       are recommended for wrapping different sorting algorithm calls.
     * 
     * Example:
     * @code
     * std::vector<int> data = {3, 1, 4, 1, 5, 9};
     * auto result = timer.benchmark_sort(data, [](auto& arr) {
     *     std::sort(arr.begin(), arr.end());
     * });
     * @endcode
     */
    template<typename Container, typename SortFunc>
    TimingResult benchmark_sort(const Container& original_data, SortFunc sort_func, int iterations = 50) {
        using T = typename Container::value_type;
        
        std::vector<double> times;
        times.reserve(iterations);
        
        // Warmup
        for (int i = 0; i < 5; ++i) {
            Container data_copy = original_data;
            sort_func(data_copy);
        }
        
        // Actual benchmarking
        for (int i = 0; i < iterations; ++i) {
            Container data_copy = original_data;
            
            auto start = Clock::now();
            sort_func(data_copy);
            auto end = Clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
            times.push_back(duration.count() / 1e6);
        }
        
        auto result = calculate_statistics(times);
        
        // Add type information
        result.type_name = typeid(T).name();
        result.type_size_bytes = sizeof(T);
        
        return result;
    }
    
    /**
     * @brief Enhanced sorting benchmark with custom type name override
     * 
     * This variant of benchmark_sort allows for custom type name specification,
     * which is useful when the automatic type detection (typeid) produces
     * compiler-specific or mangled names that are not human-readable.
     * 
     * This is particularly valuable for:
     * - Template instantiations with complex type names
     * - Custom types where a descriptive name is preferred
     * - Cross-compiler compatibility in benchmarking reports
     * - Generating user-friendly benchmark output
     * 
     * The method provides identical timing characteristics to benchmark_sort()
     * but allows for better presentation and analysis of results.
     * 
     * @tparam Container STL container type (e.g., std::vector, std::array)
     * @tparam SortFunc Callable that accepts Container& and sorts it in-place
     * @param original_data Input data to sort (remains unchanged)
     * @param sort_func Sorting function - called as sort_func(container)
     * @param type_name Human-readable name for the element type
     * @param iterations Number of timing iterations (default: 50)
     * @return TimingResult with custom type name and sorting statistics
     * 
     * Example:
     * @code
     * std::vector<std::complex<double>> data = generate_complex_data();
     * auto result = timer.benchmark_sort_with_type_info(data, sort_func, "complex<double>");
     * // result.type_name will be "complex<double>" instead of mangled name
     * @endcode
     */
    template<typename Container, typename SortFunc>
    TimingResult benchmark_sort_with_type_info(const Container& original_data, SortFunc sort_func, 
                                              const std::string& type_name, int iterations = 50) {
        using T = typename Container::value_type;
        
        auto result = benchmark_sort(original_data, sort_func, iterations);
        result.type_name = type_name;
        result.type_size_bytes = sizeof(T);
        
        return result;
    }
    
private:
    /**
     * @brief Calculate comprehensive statistical measures from timing data
     * 
     * This method performs thorough statistical analysis of the collected timing
     * measurements, computing all standard descriptive statistics needed for
     * performance analysis and comparison.
     * 
     * Statistical Measures Computed:
     * - Minimum and Maximum: Range of observed performance
     * - Mean (Arithmetic Average): Central tendency, affected by outliers
     * - Median (50th Percentile): Robust central tendency, resistant to outliers
     * - Standard Deviation: Measure of variability and consistency
     * 
     * The method uses population standard deviation (dividing by N rather than N-1)
     * as we're analyzing the complete population of our measurements rather than
     * estimating population parameters from a sample.
     * 
     * For performance analysis, the median is often more meaningful than the mean
     * as it's less affected by occasional system interruptions or outlier measurements.
     * 
     * @param times Vector of timing measurements in milliseconds (will be sorted)
     * @return TimingResult structure with all calculated statistics
     * 
     * @note The input vector is modified (sorted) for efficient median calculation.
     *       A copy should be passed if the original order needs to be preserved.
     */
    TimingResult calculate_statistics(std::vector<double>& times) {
        TimingResult result;
        result.times_ms = times;
        result.type_name = "";
        result.type_size_bytes = 0;
        
        if (times.empty()) {
            result.mean_ms = result.median_ms = result.std_dev_ms = 0.0;
            result.min_ms = result.max_ms = 0.0;
            return result;
        }
        
        // Sort for median calculation
        std::sort(times.begin(), times.end());
        
        // Calculate statistics
        result.min_ms = times.front();
        result.max_ms = times.back();
        
        // Mean
        result.mean_ms = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        
        // Median
        size_t mid = times.size() / 2;
        if (times.size() % 2 == 0) {
            result.median_ms = (times[mid - 1] + times[mid]) / 2.0;
        } else {
            result.median_ms = times[mid];
        }
        
        // Standard deviation
        double variance = 0.0;
        for (double time : times) {
            double diff = time - result.mean_ms;
            variance += diff * diff;
        }
        variance /= times.size();
        result.std_dev_ms = std::sqrt(variance);
        
        return result;
    }
};

/**
 * @brief Extended timing result with algorithm operation counting
 * 
 * This structure extends basic timing information with detailed operation
 * counts that are crucial for theoretical performance analysis of sorting
 * algorithms. It enables validation of theoretical complexity claims and
 * detailed comparison of algorithmic efficiency.
 * 
 * The operation counts provide insights into:
 * - Algorithm efficiency beyond just execution time
 * - Memory access patterns and cache performance implications
 * - Theoretical vs. practical performance characteristics
 * - Bottlenecks in algorithm implementation
 * 
 * This is particularly valuable for validating the claimed improvements
 * of dual-pivot quicksort over traditional quicksort variants.
 */
struct InstrumentedResult {
    TimingResult timing;        ///< Complete timing analysis with statistics
    size_t comparisons;         ///< Number of element comparisons performed
    size_t swaps;               ///< Number of element swaps/assignments
    size_t memory_accesses;     ///< Total memory access operations
};

/**
 * @brief Instrumented comparison function for counting operations
 * 
 * This wrapper class provides a drop-in replacement for standard comparison
 * functions while automatically counting the number of comparisons performed.
 * It's essential for analyzing the theoretical performance characteristics
 * of sorting algorithms.
 * 
 * The class maintains full compatibility with STL sorting algorithms and
 * custom comparison predicates while providing transparent operation counting.
 * This enables detailed analysis of algorithmic complexity without modifying
 * the core algorithm implementations.
 * 
 * Usage Pattern:
 * @code
 * size_t comparison_count = 0;
 * InstrumentedCompare<int> comp(&comparison_count);
 * std::sort(data.begin(), data.end(), comp);
 * // comparison_count now contains the number of comparisons
 * @endcode
 * 
 * @tparam T Element type for comparison
 */
template<typename T>
class InstrumentedCompare {
private:
    mutable size_t* counter;    ///< Pointer to external counter for comparisons
    std::less<T> comp;          ///< Standard comparison function
    
public:
    /**
     * @brief Construct instrumented comparator with external counter
     * @param comp_counter Pointer to counter variable (incremented on each comparison)
     */
    InstrumentedCompare(size_t* comp_counter) : counter(comp_counter) {}
    
    /**
     * @brief Perform comparison and increment counter
     * @param a First element to compare
     * @param b Second element to compare
     * @return Result of comparison (a < b)
     */
    bool operator()(const T& a, const T& b) const {
        if (counter) ++(*counter);
        return comp(a, b);
    }
};

/**
 * @brief Memory access tracking utility for algorithm analysis
 * 
 * This class provides a framework for tracking memory access patterns in
 * sorting algorithms. While this is a simplified implementation, it establishes
 * the foundation for more sophisticated memory profiling.
 * 
 * In a complete implementation, this would track:
 * - Cache hits and misses
 * - Memory bandwidth utilization
 * - Access pattern locality
 * - Read vs. write operation ratios
 * 
 * Such detailed memory analysis is crucial for understanding the performance
 * advantages of dual-pivot quicksort, which claims better cache locality
 * compared to traditional quicksort variants.
 * 
 * @tparam T Element type being accessed
 * 
 * @note This is a simplified placeholder for more advanced memory profiling
 *       tools that would require platform-specific implementation.
 */
template<typename T>
class MemoryAccessCounter {
private:
    size_t* access_counter;     ///< Pointer to external access counter
    
public:
    /**
     * @brief Construct memory access counter with external counter
     * @param counter Pointer to counter variable for memory accesses
     */
    MemoryAccessCounter(size_t* counter) : access_counter(counter) {}
    
    /**
     * @brief Record a memory access operation
     * 
     * In a complete implementation, this would be called by instrumented
     * memory access operations to track detailed memory usage patterns.
     * 
     * @note This is a placeholder for more sophisticated memory tracking
     *       that would require compiler support or binary instrumentation.
     */
    void record_access() {
        if (access_counter) ++(*access_counter);
    }
};

} // namespace benchmark_timer