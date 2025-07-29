#include <iostream>
#include <vector>
#include <algorithm>
#include "include/dual_pivot_quicksort.hpp"

int main() {
    std::cout << "Testing minimal dual-pivot quicksort..." << std::endl;
    
    try {
        // Simple test
        std::vector<int> data = {64, 34, 25, 12, 22, 11, 90};
        std::cout << "Original: ";
        for (int x : data) std::cout << x << " ";
        std::cout << std::endl;
        
        // Use the algorithm
        dual_pivot::dual_pivot_quicksort(data.begin(), data.end());
        
        std::cout << "Sorted: ";
        for (int x : data) std::cout << x << " ";
        std::cout << std::endl;
        
        // Verify it's sorted
        bool is_sorted = std::is_sorted(data.begin(), data.end());
        std::cout << "Is sorted: " << (is_sorted ? "Yes" : "No") << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Test completed successfully!" << std::endl;
    return 0;
}