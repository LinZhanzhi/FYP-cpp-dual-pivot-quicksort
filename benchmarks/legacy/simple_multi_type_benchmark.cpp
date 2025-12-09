#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>

// Include our implementations
#include "../include/dual_pivot_quicksort.hpp"
#include "../include/dual_pivot_optimized.hpp"
#include "data_generator.hpp"

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

template<typename T>
double time_sort(std::vector<T> data, std::function<void(std::vector<T>&)> sort_func) {
    auto start = std::chrono::high_resolution_clock::now();
    sort_func(data);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / 1000.0; // Convert to milliseconds
}

template<typename T>
void test_type() {
    std::string type_name = get_type_name<T>();
    std::cout << "Testing type: " << type_name << " (" << sizeof(T) << " bytes)\n";
    
    std::ofstream results("multi_type_benchmark_results.csv", std::ios::app);
    
    std::vector<size_t> sizes = {1000, 10000};
    std::vector<benchmark_data::DataPattern> patterns = {
        benchmark_data::DataPattern::RANDOM,
        benchmark_data::DataPattern::NEARLY_SORTED
    };
    
    for (size_t size : sizes) {
        for (auto pattern : patterns) {
            std::cout << "  Size: " << size << ", Pattern: " << benchmark_data::pattern_name(pattern) << "\n";
            
            auto data = benchmark_data::generate_data<T>(size, pattern);
            
            // Test std::sort
            double std_time = time_sort<T>(data, [](std::vector<T>& arr) {
                std::sort(arr.begin(), arr.end());
            });
            
            // Test dual-pivot
            double dp_time = time_sort<T>(data, [](std::vector<T>& arr) {
                dual_pivot::dual_pivot_quicksort(arr.begin(), arr.end());
            });
            
            // Test dual-pivot optimized
            double dpo_time = time_sort<T>(data, [](std::vector<T>& arr) {
                dual_pivot_optimized::dual_pivot_introsort(arr.begin(), arr.end());
            });
            
            // Write results
            results << type_name << "," << sizeof(T) << "," << size << "," 
                   << benchmark_data::pattern_name(pattern) << ","
                   << "std::sort," << std_time << "\n";
            results << type_name << "," << sizeof(T) << "," << size << "," 
                   << benchmark_data::pattern_name(pattern) << ","
                   << "dual_pivot," << dp_time << "\n";
            results << type_name << "," << sizeof(T) << "," << size << "," 
                   << benchmark_data::pattern_name(pattern) << ","
                   << "dual_pivot_optimized," << dpo_time << "\n";
            
            std::cout << "    std::sort: " << std_time << "ms, dual_pivot: " << dp_time 
                     << "ms, optimized: " << dpo_time << "ms\n";
        }
    }
    
    results.close();
}

int main() {
    std::cout << "Starting Simplified Multi-Type Benchmark\n";
    std::cout << "========================================\n\n";
    
    // Create CSV header
    std::ofstream results("multi_type_benchmark_results.csv");
    results << "Type,Type_Size_Bytes,Size,Pattern,Algorithm,Time_ms\n";
    results.close();
    
    // Test all types
    test_type<char>();
    test_type<short>();
    test_type<int>();
    test_type<long>();
    test_type<float>();
    test_type<double>();
    
    std::cout << "\nBenchmark completed! Results saved to multi_type_benchmark_results.csv\n";
    return 0;
}