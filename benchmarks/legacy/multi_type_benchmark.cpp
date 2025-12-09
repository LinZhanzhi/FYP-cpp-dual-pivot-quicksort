#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <tuple>

// Include our implementations
#include "../include/dual_pivot_quicksort.hpp"
#include "../include/dual_pivot_optimized.hpp"
#include "data_generator.hpp"
#include "timer.hpp"

// Type information helper
template<typename T>
std::string get_type_name() {
    if constexpr (std::is_same_v<T, char>) return "char";
    else if constexpr (std::is_same_v<T, short>) return "short";
    else if constexpr (std::is_same_v<T, int>) return "int";
    else if constexpr (std::is_same_v<T, long>) return "long";
    else if constexpr (std::is_same_v<T, long long>) return "long_long";
    else if constexpr (std::is_same_v<T, float>) return "float";
    else if constexpr (std::is_same_v<T, double>) return "double";
    else return "unknown";
}

class MultiTypeBenchmarkSuite {
private:
    benchmark_timer::BenchmarkTimer timer;
    std::ofstream results_file;
    std::string output_file_path;
    
public:
    MultiTypeBenchmarkSuite(const std::string& output_file = "multi_type_benchmark_results.csv") 
        : output_file_path(output_file) {
        results_file.open(output_file);
        write_csv_header();
    }
    
    ~MultiTypeBenchmarkSuite() {
        if (results_file.is_open()) {
            results_file.close();
        }
        generate_plots();
    }
    
    void run_comprehensive_benchmark() {
        std::cout << "Starting Multi-Type Dual-Pivot Quicksort Benchmark\n";
        std::cout << "==================================================\n\n";
        
        // Test all primitive types
        run_type_benchmark<char>();
        run_type_benchmark<short>();
        run_type_benchmark<int>();
        run_type_benchmark<long>();
        run_type_benchmark<long long>();
        run_type_benchmark<float>();
        run_type_benchmark<double>();
        
        std::cout << "Multi-type benchmark completed. Results saved to " << output_file_path << "\n";
        std::cout << "Generating performance plots...\n";
    }
    
private:
    void write_csv_header() {
        results_file << "Type,Type_Size_Bytes,Size,Pattern,Algorithm,Mean_ms,Median_ms,StdDev_ms,Min_ms,Max_ms,Elements_Per_Second\n";
    }
    
    template<typename T>
    void run_type_benchmark() {
        std::string type_name = get_type_name<T>();
        std::cout << "Testing type: " << type_name << " (" << sizeof(T) << " bytes)\n";
        
        // Use smaller subset of sizes and patterns for multi-type testing
        std::vector<size_t> type_test_sizes = {100, 1000};  // Start with smaller sizes
        std::vector<benchmark_data::DataPattern> key_patterns = {
            benchmark_data::DataPattern::RANDOM,
            benchmark_data::DataPattern::NEARLY_SORTED
        };
        
        for (size_t size : type_test_sizes) {
            std::cout << "  Array size: " << size << "\n";
            
            for (auto pattern : key_patterns) {
                std::cout << "    Pattern: " << benchmark_data::pattern_name(pattern) << std::flush;
                
                try {
                    auto data = benchmark_data::generate_data<T>(size, pattern);
                    run_algorithm_comparison<T>(data, size, pattern);
                    std::cout << " ✓\n";
                } catch (const std::exception& e) {
                    std::cout << " ✗ Error: " << e.what() << "\n";
                }
            }
        }
        std::cout << "\n";
    }
    
    template<typename T>
    void run_algorithm_comparison(const std::vector<T>& data, size_t size, benchmark_data::DataPattern pattern) {
        const int iterations = 5;  // Use fewer iterations to reduce runtime
        std::string type_name = get_type_name<T>();
        
        try {
            // Test std::sort
            auto std_sort_result = timer.benchmark_sort_with_type_info(data, [](std::vector<T>& arr) {
                std::sort(arr.begin(), arr.end());
            }, type_name, iterations);
            write_result<T>(size, pattern, "std::sort", std_sort_result);
            
            // Test dual-pivot quicksort (unoptimized)
            auto dual_pivot_result = timer.benchmark_sort_with_type_info(data, [](std::vector<T>& arr) {
                dual_pivot::dual_pivot_quicksort(arr.begin(), arr.end());
            }, type_name, iterations);
            write_result<T>(size, pattern, "dual_pivot_quicksort", dual_pivot_result);
            
            // Test dual-pivot quicksort (optimized)
            auto dual_pivot_optimized_result = timer.benchmark_sort_with_type_info(data, [](std::vector<T>& arr) {
                dual_pivot_optimized::dual_pivot_introsort(arr.begin(), arr.end());
            }, type_name, iterations);
            write_result<T>(size, pattern, "dual_pivot_optimized", dual_pivot_optimized_result);
        } catch (const std::exception& e) {
            std::cout << " ✗ Algorithm comparison error: " << e.what() << "\n";
        }
    }
    
    template<typename T>
    void write_result(size_t size, benchmark_data::DataPattern pattern, 
                     const std::string& algorithm, const benchmark_timer::TimingResult& result) {
        double elements_per_second = (result.mean_ms > 0) ? (size / (result.mean_ms / 1000.0)) : 0.0;
        
        results_file << get_type_name<T>() << ","
                    << sizeof(T) << ","
                    << size << ","
                    << benchmark_data::pattern_name(pattern) << ","
                    << algorithm << ","
                    << std::fixed << std::setprecision(3)
                    << result.mean_ms << ","
                    << result.median_ms << ","
                    << result.std_dev_ms << ","
                    << result.min_ms << ","
                    << result.max_ms << ","
                    << std::fixed << std::setprecision(0)
                    << elements_per_second << "\n";
        results_file.flush();
    }
    
    void generate_plots() {
        std::cout << "Generating multi-type performance plots...\n";
        
        // Construct the Python script path relative to the executable
        std::string python_script = "../scripts/plot_multi_type_benchmark.py";
        std::string command = "python " + python_script + " " + output_file_path;
        
        // Try to run the Python plotting script
        int result = std::system(command.c_str());
        
        if (result == 0) {
            std::cout << "✓ Multi-type performance plots generated successfully in results/plots/\n";
        } else {
            std::cout << "⚠ Warning: Could not generate plots. Make sure Python and matplotlib are installed.\n";
            std::cout << "  You can manually generate plots by running: " << command << "\n";
        }
    }
};

// Multi-type correctness testing
class MultiTypeCorrectnessTest {
public:
    bool run_all_tests() {
        std::cout << "Running Multi-Type Correctness Tests\n";
        std::cout << "===================================\n";
        
        bool all_passed = true;
        
        all_passed &= test_type<char>("char");
        all_passed &= test_type<short>("short");
        all_passed &= test_type<int>("int");
        all_passed &= test_type<long>("long");
        all_passed &= test_type<long long>("long long");
        all_passed &= test_type<float>("float");
        all_passed &= test_type<double>("double");
        
        if (all_passed) {
            std::cout << "All multi-type correctness tests PASSED ✓\n\n";
        } else {
            std::cout << "Some multi-type correctness tests FAILED ✗\n\n";
        }
        
        return all_passed;
    }
    
private:
    template<typename T>
    bool test_type(const std::string& type_name) {
        std::cout << "  Testing " << type_name << "... ";
        
        bool all_passed = true;
        
        // Test all patterns with this type
        for (auto pattern : benchmark_data::all_patterns) {
            auto data = benchmark_data::generate_data<T>(500, pattern);
            auto original = data;
            
            // Test dual-pivot quicksort
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
        
        // Test edge cases
        std::vector<T> empty;
        dual_pivot::dual_pivot_quicksort(empty.begin(), empty.end());
        
        std::vector<T> single = {static_cast<T>(42)};
        dual_pivot::dual_pivot_quicksort(single.begin(), single.end());
        
        if (single.size() != 1) {
            all_passed = false;
        }
        
        std::cout << (all_passed ? "PASS" : "FAIL") << "\n";
        return all_passed;
    }
};

int main() {
    // First run correctness tests
    MultiTypeCorrectnessTest correctness;
    if (!correctness.run_all_tests()) {
        std::cerr << "Multi-type correctness tests failed. Aborting benchmark.\n";
        return 1;
    }
    
    // Run multi-type performance benchmarks
    MultiTypeBenchmarkSuite benchmark;
    benchmark.run_comprehensive_benchmark();
    
    return 0;
}