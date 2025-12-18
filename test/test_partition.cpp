#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>
#include <string>
#include "dpqs/partition.hpp"

using namespace dual_pivot;

// Helper to print array
template<typename T>
void print_array(const std::vector<T>& arr) {
    for (const auto& x : arr) {
        std::cout << x << " ";
    }
    std::cout << "\n";
}

void test_partition_dual_pivot() {
    std::cout << "Testing partition_dual_pivot..." << std::endl;
    
    // Random int
    {
        std::vector<int> arr(50);
        std::iota(arr.begin(), arr.end(), 0);
        std::mt19937 g(123);
        std::shuffle(arr.begin(), arr.end(), g);
        
        // Pick pivots (simple strategy for test)
        if (arr[0] > arr[1]) std::swap(arr[0], arr[1]);
        int p1 = arr[0];
        int p2 = arr[1];
        
        // partition_dual_pivot expects pivots at pivotIndex1 and pivotIndex2
        // and it moves them to boundaries.
        // But wait, the function signature is:
        // partition_dual_pivot(T* a, int low, int high, int pivotIndex1, int pivotIndex2)
        // It extracts pivots from a[pivotIndex1] and a[pivotIndex2].
        
        auto res = partition_dual_pivot(arr.data(), 0, arr.size(), 0, 1);
        int lower = res.first;
        int upper = res.second;
        
        // Verify partition property
        // [low, lower) < p1
        for (int i = 0; i < lower; ++i) {
            assert(arr[i] < p1);
        }
        
        // [lower, upper] >= p1 && <= p2
        // Note: pivots are placed at arr[lower] and arr[upper]
        assert(arr[lower] == p1);
        assert(arr[upper] == p2);
        
        for (int i = lower + 1; i < upper; ++i) {
            assert(arr[i] >= p1 && arr[i] <= p2);
        }
        
        // (upper, high) > p2
        for (int i = upper + 1; i < arr.size(); ++i) {
            assert(arr[i] > p2);
        }
    }
    
    std::cout << "Passed." << std::endl;
}

void test_partition_single_pivot() {
    std::cout << "Testing partition_single_pivot..." << std::endl;
    
    // Random int
    {
        std::vector<int> arr(50);
        std::iota(arr.begin(), arr.end(), 0);
        std::mt19937 g(123);
        std::shuffle(arr.begin(), arr.end(), g);
        
        int p = arr[0];
        
        // partition_single_pivot(T* a, int low, int high, int pivotIndex1, int)
        auto res = partition_single_pivot(arr.data(), 0, arr.size(), 0, 0);
        int lower = res.first;
        int upper = res.second;
        
        // Verify partition property
        // [low, lower) < p
        for (int i = 0; i < lower; ++i) {
            assert(arr[i] < p);
        }
        
        // [lower, upper) == p
        for (int i = lower; i < upper; ++i) {
            if (arr[i] != p) {
                std::cout << "Failed at middle part index " << i << ": " << arr[i] << " != " << p << std::endl;
                print_array(arr);
                std::cout << "lower=" << lower << ", upper=" << upper << std::endl;
                assert(arr[i] == p);
            }
        }
        
        // [upper, high) > p
        for (int i = upper; i < arr.size(); ++i) {
            if (arr[i] <= p) {
                std::cout << "Failed at right part index " << i << ": " << arr[i] << " <= " << p << std::endl;
                print_array(arr);
                std::cout << "lower=" << lower << ", upper=" << upper << std::endl;
                assert(arr[i] > p);
            }
        }
    }
    
    std::cout << "Passed." << std::endl;
}

int main() {
    test_partition_dual_pivot();
    test_partition_single_pivot();
    
    std::cout << "All partition tests passed!" << std::endl;
    return 0;
}
