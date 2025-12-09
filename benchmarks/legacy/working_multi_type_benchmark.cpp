#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>

#include "../include/dual_pivot_quicksort.hpp"
#include "data_generator.hpp"

template<typename T>
std::string get_type_name() {
    if constexpr (std::is_same_v<T, char>) return "char";
    else if constexpr (std::is_same_v<T, short>) return "short";
    else if constexpr (std::is_same_v<T, int>) return "int";
    else if constexpr (std::is_same_v<T, long>) return "long";
    else if constexpr (std::is_same_v<T, float>) return "float";
    else if constexpr (std::is_same_v<T, double>) return "double";
    else return "unknown";
}

template<typename T>
void run_benchmark() {
    std::string type_name = get_type_name<T>();
    std::cout << "Testing " << type_name << " (" << sizeof(T) << " bytes)\n";
    
    std::ofstream results("multi_type_benchmark_results.csv", std::ios::app);
    
    std::vector<size_t> sizes = {1000};
    std::vector<benchmark_data::DataPattern> patterns = {
        benchmark_data::DataPattern::RANDOM,
        benchmark_data::DataPattern::NEARLY_SORTED
    };
    
    for (size_t size : sizes) {
        for (auto pattern : patterns) {
            std::cout << "  Size: " << size << ", Pattern: " << benchmark_data::pattern_name(pattern) << "\n";
            
            try {
                auto data = benchmark_data::generate_data<T>(size, pattern);
                std::cout << "    Generated data successfully\n";
                
                // Test std::sort
                auto data_copy = data;
                auto start = std::chrono::high_resolution_clock::now();
                std::sort(data_copy.begin(), data_copy.end());
                auto end = std::chrono::high_resolution_clock::now();
                double std_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
                
                // Test dual-pivot
                data_copy = data;
                start = std::chrono::high_resolution_clock::now();
                dual_pivot::dual_pivot_quicksort(data_copy.begin(), data_copy.end());
                end = std::chrono::high_resolution_clock::now();
                double dp_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
                
                results << type_name << "," << sizeof(T) << "," << size << "," 
                       << benchmark_data::pattern_name(pattern) << ","
                       << "std_sort," << std_time << "\n";
                results << type_name << "," << sizeof(T) << "," << size << "," 
                       << benchmark_data::pattern_name(pattern) << ","
                       << "dual_pivot," << dp_time << "\n";
                
                std::cout << "    std::sort: " << std_time << "ms, dual_pivot: " << dp_time << "ms\n";
                
            } catch (const std::exception& e) {
                std::cout << "    Error: " << e.what() << "\n";
            }
        }
    }
    
    results.close();
}

int main() {
    std::cout << "Multi-Type Benchmark (Working Version)\n";
    std::cout << "======================================\n\n";
    
    // Create CSV header
    std::ofstream results("multi_type_benchmark_results.csv");
    results << "Type,Type_Size_Bytes,Size,Pattern,Algorithm,Time_ms\n";
    results.close();
    
    run_benchmark<char>();
    run_benchmark<short>();
    run_benchmark<int>();
    run_benchmark<long>();
    run_benchmark<float>();
    run_benchmark<double>();
    
    std::cout << "\nBenchmark completed successfully!\n";
    return 0;
}