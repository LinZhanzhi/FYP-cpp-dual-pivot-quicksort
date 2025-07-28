#include <iostream>
#include <vector>
#include "data_generator.hpp"

int main() {
    std::cout << "Testing data generation...\n";
    
    try {
        std::cout << "Testing int data generation...";
        auto int_data = benchmark_data::generate_data<int>(100, benchmark_data::DataPattern::RANDOM);
        std::cout << " OK (size: " << int_data.size() << ")\n";
        
        std::cout << "Testing char data generation...";
        auto char_data = benchmark_data::generate_data<char>(100, benchmark_data::DataPattern::RANDOM);
        std::cout << " OK (size: " << char_data.size() << ")\n";
        
        std::cout << "Testing float data generation...";
        auto float_data = benchmark_data::generate_data<float>(100, benchmark_data::DataPattern::RANDOM);
        std::cout << " OK (size: " << float_data.size() << ")\n";
        
        std::cout << "All data generation tests passed!\n";
        
    } catch (const std::exception& e) {
        std::cout << " Error: " << e.what() << "\n";
    }
    
    return 0;
}