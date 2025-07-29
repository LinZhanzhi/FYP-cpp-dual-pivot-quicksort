#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <random>

// Include our implementation
#include "dual_pivot_quicksort.hpp"

class SimpleBenchmark {
private:
    std::mt19937 gen{42};
    
public:
    // Generate test data
    std::vector<int> generateRandomData(size_t size) {
        std::vector<int> data;
        data.reserve(size);
        std::uniform_int_distribution<int> dis(1, static_cast<int>(size));
        
        for (size_t i = 0; i < size; ++i) {
            data.push_back(dis(gen));
        }
        return data;
    }
    
    // Simple timing function
    template<typename SortFunc>
    double timeSort(const std::vector<int>& original_data, SortFunc sort_func) {
        std::vector<int> data = original_data;
        
        auto start = std::chrono::high_resolution_clock::now();
        sort_func(data);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        return duration.count() / 1e6;  // Convert to milliseconds
    }
    
    void runBenchmark() {
        std::cout << "Running Simple Dual-Pivot Quicksort Benchmark\n";
        std::cout << "==============================================\n\n";
        
        // Create results file
        std::ofstream results("benchmark_results.csv");
        results << "Size,Algorithm,Time_ms\n";
        
        std::vector<size_t> test_sizes = {100, 1000, 10000, 50000};
        
        for (size_t size : test_sizes) {
            std::cout << "Testing size: " << size << std::endl;
            
            // Generate test data
            auto data = generateRandomData(size);
            
            // Test std::sort
            double std_sort_time = timeSort(data, [](std::vector<int>& arr) {
                std::sort(arr.begin(), arr.end());
            });
            
            // Test our dual-pivot quicksort
            double dual_pivot_time = timeSort(data, [](std::vector<int>& arr) {
                dual_pivot::dual_pivot_quicksort(arr.begin(), arr.end());
            });
            
            // Write results
            results << size << ",std::sort," << std::fixed << std::setprecision(3) << std_sort_time << "\n";
            results << size << ",dual_pivot_quicksort," << std::fixed << std::setprecision(3) << dual_pivot_time << "\n";
            
            std::cout << "  std::sort: " << std_sort_time << " ms\n";
            std::cout << "  dual_pivot: " << dual_pivot_time << " ms\n";
            std::cout << "  Speedup: " << (std_sort_time / dual_pivot_time) << "x\n\n";
        }
        
        results.close();
        std::cout << "Benchmark completed. Results saved to benchmark_results.csv\n";
    }
};

int main() {
    try {
        SimpleBenchmark benchmark;
        benchmark.runBenchmark();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}