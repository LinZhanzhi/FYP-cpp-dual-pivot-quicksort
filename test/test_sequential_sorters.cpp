#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>
#include "dpqs/sequential_sorters.hpp"

using namespace dual_pivot;

// Helper to check if sorted
template<typename T>
bool is_sorted(const std::vector<T>& arr) {
    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i] < arr[i - 1]) return false;
    }
    return true;
}

void test_sort_int() {
    std::cout << "Testing sort_int_sequential..." << std::endl;
    
    std::vector<int> arr(1000);
    std::iota(arr.begin(), arr.end(), 0);
    std::mt19937 g(123);
    std::shuffle(arr.begin(), arr.end(), g);
    
    sort_int_sequential(nullptr, arr.data(), 0, 0, arr.size());
    
    assert(is_sorted(arr));
    std::cout << "Passed." << std::endl;
}

void test_sort_double() {
    std::cout << "Testing sort_double_sequential..." << std::endl;
    
    std::vector<double> arr(1000);
    std::mt19937 g(123);
    std::uniform_real_distribution<double> dis(0.0, 100.0);
    
    for(auto& x : arr) x = dis(g);
    
    sort_double_sequential(nullptr, arr.data(), 0, 0, arr.size());
    
    assert(is_sorted(arr));
    std::cout << "Passed." << std::endl;
}

int main() {
    test_sort_int();
    test_sort_double();
    
    std::cout << "All sequential sorter tests passed!" << std::endl;
    return 0;
}
