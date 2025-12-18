#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>
#include <string>
#include "dpqs/insertion_sort.hpp"

using namespace dual_pivot;

// Helper to print array
template<typename T>
void print_array(const std::vector<T>& arr) {
    for (const auto& x : arr) {
        std::cout << x << " ";
    }
    std::cout << "\n";
}

// Helper to check if sorted
template<typename T>
bool is_sorted(const std::vector<T>& arr) {
    for (size_t i = 1; i < arr.size(); ++i) {
        if (arr[i] < arr[i - 1]) return false;
    }
    return true;
}

void test_insertion_sort_template() {
    std::cout << "Testing insertion_sort template..." << std::endl;
    
    // Random int
    {
        std::vector<int> arr(50);
        std::iota(arr.begin(), arr.end(), 0);
        std::mt19937 g(123);
        std::shuffle(arr.begin(), arr.end(), g);
        
        insertion_sort(arr.data(), 0, arr.size());
        assert(is_sorted(arr));
    }

    // Random double
    {
        std::vector<double> arr(50);
        std::mt19937 g(123);
        std::uniform_real_distribution<double> dis(0.0, 100.0);
        for(auto& x : arr) x = dis(g);
        
        insertion_sort(arr.data(), 0, arr.size());
        assert(is_sorted(arr));
    }
    
    std::cout << "Passed." << std::endl;
}

void test_mixed_insertion_sort_template() {
    std::cout << "Testing mixed_insertion_sort template..." << std::endl;
    
    // Random int - larger size to trigger mixed strategy
    {
        std::vector<int> arr(100);
        std::iota(arr.begin(), arr.end(), 0);
        std::mt19937 g(123);
        std::shuffle(arr.begin(), arr.end(), g);
        
        mixed_insertion_sort(arr.data(), 0, arr.size());
        assert(is_sorted(arr));
    }
    
    std::cout << "Passed." << std::endl;
}

void test_specialized_insertion_sort() {
    std::cout << "Testing specialized insertion_sort functions..." << std::endl;

    // int
    {
        std::vector<int> arr(50);
        std::iota(arr.begin(), arr.end(), 0);
        std::mt19937 g(123);
        std::shuffle(arr.begin(), arr.end(), g);
        
        insertion_sort_int(arr.data(), 0, arr.size());
        assert(is_sorted(arr));
    }

    // long
    {
        std::vector<long> arr(50);
        std::iota(arr.begin(), arr.end(), 0);
        std::mt19937 g(123);
        std::shuffle(arr.begin(), arr.end(), g);
        
        insertion_sort_long(arr.data(), 0, arr.size());
        assert(is_sorted(arr));
    }

    // float
    {
        std::vector<float> arr(50);
        std::mt19937 g(123);
        std::uniform_real_distribution<float> dis(0.0f, 100.0f);
        for(auto& x : arr) x = dis(g);
        
        insertion_sort_float(arr.data(), 0, arr.size());
        assert(is_sorted(arr));
    }

    // double
    {
        std::vector<double> arr(50);
        std::mt19937 g(123);
        std::uniform_real_distribution<double> dis(0.0, 100.0);
        for(auto& x : arr) x = dis(g);
        
        insertion_sort_double(arr.data(), 0, arr.size());
        assert(is_sorted(arr));
    }

    std::cout << "Passed." << std::endl;
}

int main() {
    test_insertion_sort_template();
    test_mixed_insertion_sort_template();
    test_specialized_insertion_sort();
    
    std::cout << "All insertion sort tests passed!" << std::endl;
    return 0;
}
