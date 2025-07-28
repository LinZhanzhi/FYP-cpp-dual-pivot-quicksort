#include <iostream>
#include <vector>
#include "../include/dual_pivot_quicksort.hpp"
#include "../include/dual_pivot_optimized.hpp"
#include "data_generator.hpp"

int main() {
    std::cout << "Testing basic functionality...\n";
    
    // Test basic int case first
    try {
        auto data = benchmark_data::generate_data<int>(100, benchmark_data::DataPattern::RANDOM);
        std::cout << "Generated int data of size: " << data.size() << "\n";
        
        dual_pivot::dual_pivot_quicksort(data.begin(), data.end());
        std::cout << "int sorting completed successfully\n";
        
        // Test char case
        auto char_data = benchmark_data::generate_data<char>(100, benchmark_data::DataPattern::RANDOM);
        std::cout << "Generated char data of size: " << char_data.size() << "\n";
        
        dual_pivot::dual_pivot_quicksort(char_data.begin(), char_data.end());
        std::cout << "char sorting completed successfully\n";
        
        std::cout << "Basic test passed!\n";
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}