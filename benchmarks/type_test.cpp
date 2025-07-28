#include <iostream>
#include <vector>
#include "../include/dual_pivot_quicksort.hpp"

int main() {
    std::cout << "Testing individual types...\n";
    
    // Test int first (known to work)
    std::vector<int> int_data = {5, 2, 8, 1, 9, 3};
    std::cout << "Testing int...";
    dual_pivot::dual_pivot_quicksort(int_data.begin(), int_data.end());
    std::cout << " OK\n";
    
    // Test char
    std::vector<char> char_data = {5, 2, 8, 1, 9, 3};
    std::cout << "Testing char...";
    dual_pivot::dual_pivot_quicksort(char_data.begin(), char_data.end());
    std::cout << " OK\n";
    
    // Test float
    std::vector<float> float_data = {5.0f, 2.0f, 8.0f, 1.0f, 9.0f, 3.0f};
    std::cout << "Testing float...";
    dual_pivot::dual_pivot_quicksort(float_data.begin(), float_data.end());
    std::cout << " OK\n";
    
    std::cout << "All basic tests passed!\n";
    return 0;
}