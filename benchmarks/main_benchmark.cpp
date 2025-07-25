#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>

// Include our implementations
#include "../include/dual_pivot_quicksort.hpp"
#include "../include/classic_quicksort.hpp"
#include "../include/dual_pivot_optimized.hpp"
#include "data_generator.hpp"
#include "timer.hpp"

class BenchmarkSuite {
private:
    benchmark_timer::BenchmarkTimer timer;
    std::ofstream results_file;
    
public:
    BenchmarkSuite(const std::string& output_file = "benchmark_results.csv") {
        results_file.open(output_file);
        write_csv_header();
    }
    
    ~BenchmarkSuite() {
        if (results_file.is_open()) {
            results_file.close();
        }
    }
    
    void run_comprehensive_benchmark() {
        std::cout << "Starting Comprehensive Dual-Pivot Quicksort Benchmark\n";
        std::cout << "======================================================\n\n";
        
        for (size_t size : benchmark_data::test_sizes) {
            std::cout << "Testing array size: " << size << "\n";
            
            for (auto pattern : benchmark_data::all_patterns) {
                std::cout << "  Pattern: " << benchmark_data::pattern_name(pattern) << std::flush;
                
                auto data = benchmark_data::generate_data<int>(size, pattern);
                run_algorithm_comparison(data, size, pattern);
                
                std::cout << " ✓\n";
            }
            std::cout << "\n";
        }
        
        std::cout << "Benchmark completed. Results saved to benchmark_results.csv\n";
    }
    
private:
    void write_csv_header() {
        results_file << "Size,Pattern,Algorithm,Mean_ms,Median_ms,StdDev_ms,Min_ms,Max_ms\n";
    }
    
    void run_algorithm_comparison(const std::vector<int>& data, size_t size, benchmark_data::DataPattern pattern) {
        const int iterations = (size > 100000) ? 10 : 50;  // Fewer iterations for large arrays
        
        // Test std::sort
        auto std_sort_result = timer.benchmark_sort(data, [](std::vector<int>& arr) {
            std::sort(arr.begin(), arr.end());
        }, iterations);
        write_result(size, pattern, "std::sort", std_sort_result);
        
        // Test std::stable_sort
        auto std_stable_sort_result = timer.benchmark_sort(data, [](std::vector<int>& arr) {
            std::stable_sort(arr.begin(), arr.end());
        }, iterations);
        write_result(size, pattern, "std::stable_sort", std_stable_sort_result);
        
        // Test classic quicksort
        auto classic_qs_result = timer.benchmark_sort(data, [](std::vector<int>& arr) {
            classic_quicksort::quicksort(arr.begin(), arr.end());
        }, iterations);
        write_result(size, pattern, "classic_quicksort", classic_qs_result);
        
        // Test dual-pivot quicksort
        auto dual_pivot_result = timer.benchmark_sort(data, [](std::vector<int>& arr) {
            dual_pivot::dual_pivot_quicksort(arr.begin(), arr.end());
        }, iterations);
        write_result(size, pattern, "dual_pivot_quicksort", dual_pivot_result);
        
        // Test optimized dual-pivot
        auto optimized_result = timer.benchmark_sort(data, [](std::vector<int>& arr) {
            dual_pivot_optimized::dual_pivot_introsort(arr.begin(), arr.end());
        }, iterations);
        write_result(size, pattern, "dual_pivot_optimized", optimized_result);
    }
    
    void write_result(size_t size, benchmark_data::DataPattern pattern, 
                     const std::string& algorithm, const benchmark_timer::TimingResult& result) {
        results_file << size << ","
                    << benchmark_data::pattern_name(pattern) << ","
                    << algorithm << ","
                    << std::fixed << std::setprecision(3)
                    << result.mean_ms << ","
                    << result.median_ms << ","
                    << result.std_dev_ms << ","
                    << result.min_ms << ","
                    << result.max_ms << "\n";
        results_file.flush();
    }
};

// Correctness testing
class CorrectnessTest {
public:
    bool run_all_tests() {
        std::cout << "Running Correctness Tests\n";
        std::cout << "========================\n";
        
        bool all_passed = true;
        
        all_passed &= test_basic_sorting();
        all_passed &= test_edge_cases();
        all_passed &= test_all_patterns();
        all_passed &= test_different_types();
        
        if (all_passed) {
            std::cout << "All correctness tests PASSED ✓\n\n";
        } else {
            std::cout << "Some correctness tests FAILED ✗\n\n";
        }
        
        return all_passed;
    }
    
private:
    bool test_basic_sorting() {
        std::cout << "  Basic sorting test... ";
        
        std::vector<int> data = {64, 34, 25, 12, 22, 11, 90};
        std::vector<int> expected = {11, 12, 22, 25, 34, 64, 90};
        
        auto test_data = data;
        dual_pivot::dual_pivot_quicksort(test_data.begin(), test_data.end());
        
        bool passed = (test_data == expected);
        std::cout << (passed ? "PASS" : "FAIL") << "\n";
        return passed;
    }
    
    bool test_edge_cases() {
        std::cout << "  Edge cases test... ";
        
        // Empty array
        std::vector<int> empty;
        dual_pivot::dual_pivot_quicksort(empty.begin(), empty.end());
        
        // Single element
        std::vector<int> single = {42};
        dual_pivot::dual_pivot_quicksort(single.begin(), single.end());
        
        // Two elements
        std::vector<int> two = {2, 1};
        dual_pivot::dual_pivot_quicksort(two.begin(), two.end());
        
        // All same elements
        std::vector<int> same = {5, 5, 5, 5, 5};
        dual_pivot::dual_pivot_quicksort(same.begin(), same.end());
        
        bool passed = (single == std::vector<int>{42}) && 
                     (two == std::vector<int>{1, 2}) &&
                     (same == std::vector<int>{5, 5, 5, 5, 5});
        
        std::cout << (passed ? "PASS" : "FAIL") << "\n";
        return passed;
    }
    
    bool test_all_patterns() {
        std::cout << "  All patterns test... ";
        
        bool all_passed = true;
        
        for (auto pattern : benchmark_data::all_patterns) {
            auto data = benchmark_data::generate_data<int>(1000, pattern);
            auto original = data;
            
            dual_pivot::dual_pivot_quicksort(data.begin(), data.end());
            
            // Check if sorted
            bool is_sorted = std::is_sorted(data.begin(), data.end());
            
            // Check if it's a permutation of original
            std::sort(original.begin(), original.end());
            bool is_permutation = (data == original);
            
            if (!is_sorted || !is_permutation) {
                all_passed = false;
                break;
            }
        }
        
        std::cout << (all_passed ? "PASS" : "FAIL") << "\n";
        return all_passed;
    }
    
    bool test_different_types() {
        std::cout << "  Different types test... ";
        
        // Test with doubles
        std::vector<double> doubles = {3.14, 2.71, 1.41, 1.73, 0.57};
        dual_pivot::dual_pivot_quicksort(doubles.begin(), doubles.end());
        bool doubles_sorted = std::is_sorted(doubles.begin(), doubles.end());
        
        // Test with strings
        std::vector<std::string> strings = {"zebra", "apple", "banana", "cherry"};
        dual_pivot::dual_pivot_quicksort(strings.begin(), strings.end());
        bool strings_sorted = std::is_sorted(strings.begin(), strings.end());
        
        bool passed = doubles_sorted && strings_sorted;
        std::cout << (passed ? "PASS" : "FAIL") << "\n";
        return passed;
    }
};

int main() {
    // First run correctness tests
    CorrectnessTest correctness;
    if (!correctness.run_all_tests()) {
        std::cerr << "Correctness tests failed. Aborting benchmark.\n";
        return 1;
    }
    
    // Run performance benchmarks
    BenchmarkSuite benchmark;
    benchmark.run_comprehensive_benchmark();
    
    return 0;
}