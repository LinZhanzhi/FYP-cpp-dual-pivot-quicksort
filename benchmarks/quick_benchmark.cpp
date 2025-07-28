#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstdlib>

// Include our implementations
#include "../include/dual_pivot_quicksort.hpp"
#include "../include/classic_quicksort.hpp"
#include "../include/dual_pivot_optimized.hpp"
#include "data_generator.hpp"
#include "timer.hpp"

class QuickBenchmarkSuite {
private:
    benchmark_timer::BenchmarkTimer timer;
    std::ofstream results_file;
    std::string output_file_path;
    
    // Quick benchmark with smaller sizes and fewer iterations
    std::vector<size_t> quick_test_sizes = {100, 1000, 10000};
    std::vector<benchmark_data::DataPattern> quick_patterns = {
        benchmark_data::DataPattern::RANDOM,
        benchmark_data::DataPattern::NEARLY_SORTED,
        benchmark_data::DataPattern::REVERSE_SORTED
    };
    
public:
    QuickBenchmarkSuite(const std::string& output_file = "benchmark_results.csv") 
        : output_file_path(output_file) {
        results_file.open(output_file);
        write_csv_header();
    }
    
    ~QuickBenchmarkSuite() {
        if (results_file.is_open()) {
            results_file.close();
        }
        
        // Generate plots after benchmark completion
        generate_plots();
    }
    
    void run_quick_benchmark() {
        std::cout << "Starting Quick Dual-Pivot Quicksort Benchmark\n";
        std::cout << "==============================================\n\n";
        
        for (size_t size : quick_test_sizes) {
            std::cout << "Testing array size: " << size << "\n";
            
            for (auto pattern : quick_patterns) {
                std::cout << "  Pattern: " << benchmark_data::pattern_name(pattern) << std::flush;
                
                auto data = benchmark_data::generate_data<int>(size, pattern);
                run_algorithm_comparison(data, size, pattern);
                
                std::cout << " ✓\n";
            }
            std::cout << "\n";
        }
        
        std::cout << "Benchmark completed. Results saved to " << output_file_path << "\n";
        std::cout << "Generating performance plots...\n";
    }
    
private:
    void write_csv_header() {
        results_file << "Size,Pattern,Algorithm,Mean_ms,Median_ms,StdDev_ms,Min_ms,Max_ms\n";
    }
    
    void run_algorithm_comparison(const std::vector<int>& data, size_t size, benchmark_data::DataPattern pattern) {
        const int iterations = 10;  // Fewer iterations for quick demo
        
        // Test std::sort
        auto std_sort_result = timer.benchmark_sort(data, [](std::vector<int>& arr) {
            std::sort(arr.begin(), arr.end());
        }, iterations);
        write_result(size, pattern, "std::sort", std_sort_result);
        
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
    
    void generate_plots() {
        std::cout << "Generating performance plots...\n";
        
        // Construct the Python script path relative to the executable
        std::string python_script = "../scripts/plot_benchmark.py";
        std::string command = "python " + python_script + " " + output_file_path;
        
        // Try to run the Python plotting script
        int result = std::system(command.c_str());
        
        if (result == 0) {
            std::cout << "Performance plots generated successfully in results/plots/\n";
        } else {
            std::cout << "Warning: Could not generate plots. Make sure Python and matplotlib are installed.\n";
            std::cout << "  You can manually generate plots by running: " << command << "\n";
        }
    }
};

int main() {
    // Run quick performance benchmarks
    QuickBenchmarkSuite benchmark;
    benchmark.run_quick_benchmark();
    
    return 0;
}