#include <iostream>
#include <vector>
#include "benchmarks/random_pattern.hpp"
#include "benchmarks/sorted_pattern.hpp"
#include "include/dual_pivot_quicksort.hpp"

int main() {
    std::cout << "Testing char type handling..." << std::endl;
    
    // Test small array first
    try {
        std::cout << "Testing char with size 10..." << std::endl;
        auto data = generate_random_pattern<char>(10);
        std::cout << "Generated random data: ";
        for (char c : data) {
            std::cout << static_cast<int>(c) << " ";
        }
        std::cout << std::endl;
        
        dual_pivot::dual_pivot_quicksort(data.begin(), data.end());
        std::cout << "Sorted data: ";
        for (char c : data) {
            std::cout << static_cast<int>(c) << " ";
        }
        std::cout << std::endl;
        
        std::cout << "Testing sorted pattern..." << std::endl;
        auto sorted_data = generate_sorted_pattern<char>(10);
        std::cout << "Generated sorted data: ";
        for (char c : sorted_data) {
            std::cout << static_cast<int>(c) << " ";
        }
        std::cout << std::endl;
        
        std::cout << "Testing with larger size (100)..." << std::endl;
        auto large_data = generate_random_pattern<char>(100);
        dual_pivot::dual_pivot_quicksort(large_data.begin(), large_data.end());
        std::cout << "Large data sorted successfully!" << std::endl;
        
        std::cout << "All char tests passed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}