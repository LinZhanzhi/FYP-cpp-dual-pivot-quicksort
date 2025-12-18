#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>
#include "dpqs/counting_sort.hpp"

using namespace dual_pivot;

// Helper to check if sorted
template<typename T>
bool is_sorted(const std::vector<T>& arr) {
    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i] < arr[i - 1]) return false;
    }
    return true;
}

void test_counting_sort_char() {
    std::cout << "Testing counting_sort (char)..." << std::endl;
    
    std::vector<char> arr(1000);
    std::mt19937 g(123);
    std::uniform_int_distribution<int> dis(-128, 127);
    
    for(auto& x : arr) x = static_cast<char>(dis(g));
    
    counting_sort(arr.data(), 0, arr.size());
    
    assert(is_sorted(arr));
    std::cout << "Passed." << std::endl;
}

void test_counting_sort_short() {
    std::cout << "Testing counting_sort (short)..." << std::endl;
    
    std::vector<short> arr(1000);
    std::mt19937 g(123);
    std::uniform_int_distribution<int> dis(-32768, 32767);
    
    for(auto& x : arr) x = static_cast<short>(dis(g));
    
    counting_sort(arr.data(), 0, arr.size());
    
    assert(is_sorted(arr));
    std::cout << "Passed." << std::endl;
}

int main() {
    test_counting_sort_char();
    test_counting_sort_short();
    
    std::cout << "All counting sort tests passed!" << std::endl;
    return 0;
}
