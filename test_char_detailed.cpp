#include <iostream>
#include <vector>
#include <algorithm>
#include "benchmarks/random_pattern.hpp"
#include "include/dual_pivot_quicksort.hpp"

int main() {
    std::cout << "Testing different sorting algorithms with char..." << std::endl;
    
    try {
        std::cout << "Testing with size 50..." << std::endl;
        auto data1 = generate_random_pattern<char>(50);
        auto data2 = data1; // Copy for std::sort
        auto data3 = data1; // Copy for dual-pivot
        
        std::cout << "Testing std::sort..." << std::endl;
        std::sort(data2.begin(), data2.end());
        std::cout << "std::sort succeeded!" << std::endl;
        
        std::cout << "Testing dual_pivot_quicksort..." << std::endl;
        dual_pivot::dual_pivot_quicksort(data3.begin(), data3.end());
        std::cout << "dual_pivot_quicksort succeeded!" << std::endl;
        
        // Verify both give same result
        if (data2 == data3) {
            std::cout << "Both sorting methods give identical results!" << std::endl;
        } else {
            std::cout << "WARNING: Different sorting results!" << std::endl;
        }
        
        std::cout << "Testing with size 100..." << std::endl;
        auto large_data1 = generate_random_pattern<char>(100);
        auto large_data2 = large_data1;
        
        std::cout << "Testing std::sort on size 100..." << std::endl;
        std::sort(large_data1.begin(), large_data1.end());
        std::cout << "std::sort on size 100 succeeded!" << std::endl;
        
        std::cout << "Testing dual_pivot_quicksort on size 100..." << std::endl;
        dual_pivot::dual_pivot_quicksort(large_data2.begin(), large_data2.end());
        std::cout << "dual_pivot_quicksort on size 100 succeeded!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}