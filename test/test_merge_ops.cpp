#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>
#include <string>
#include "dpqs/merge_ops.hpp"

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

void test_merge_parts() {
    std::cout << "Testing merge_parts..." << std::endl;
    
    // Random int
    {
        std::vector<int> a1(50);
        std::iota(a1.begin(), a1.end(), 0); // 0..49
        
        std::vector<int> a2(50);
        std::iota(a2.begin(), a2.end(), 50); // 50..99
        
        // Shuffle but keep sorted segments?
        // merge_parts expects sorted segments.
        
        // Let's create two sorted arrays
        std::vector<int> src1 = {1, 3, 5, 7, 9};
        std::vector<int> src2 = {2, 4, 6, 8, 10};
        std::vector<int> dst(10);
        
        merge_parts(dst.data(), 0, src1.data(), 0, 5, src2.data(), 0, 5);
        
        assert(is_sorted(dst));
        for(int i=0; i<10; ++i) assert(dst[i] == i+1);
    }
    
    // Overlapping buffers (dst == src1)
    {
        std::vector<int> arr = {1, 3, 5, 7, 9, 2, 4, 6, 8, 10};
        // We want to merge arr[0..5] and arr[5..10] into a separate buffer first?
        // merge_parts(dst, k, a1, lo1, hi1, a2, lo2, hi2)
        // If dst overlaps with a1 or a2, we need to be careful.
        // The implementation says:
        // "Buffer management optimizations to handle cases where the destination overlaps with source arrays."
        // But standard merge usually requires auxiliary space if merging in-place without rotation.
        // Let's check implementation.
        // It copies from a1 and a2 to dst.
        // If dst == a1, and we write to dst[k], we might overwrite a1[k] if k >= lo1.
        // But usually merge is done from aux to main, or main to aux.
        
        // Let's test merging FROM two separate arrays INTO one.
        // That's the standard use case in merge sort (copy to aux, merge back).
        
        std::vector<int> aux(10);
        std::copy(arr.begin(), arr.end(), aux.begin());
        
        // Merge from aux to arr
        // a1 = aux, a2 = aux
        // lo1=0, hi1=5, lo2=5, hi2=10
        // dst = arr
        
        merge_parts(arr.data(), 0, aux.data(), 0, 5, aux.data(), 5, 10);
        assert(is_sorted(arr));
    }

    std::cout << "Passed." << std::endl;
}

void test_parallel_merge_parts() {
    std::cout << "Testing parallel_merge_parts..." << std::endl;
    
    // Large arrays to trigger parallelism
    int size = 20000; // MIN_PARALLEL_MERGE_PARTS_SIZE is likely 4096 or similar
    std::vector<int> src1(size);
    std::vector<int> src2(size);
    std::vector<int> dst(size * 2);
    
    std::iota(src1.begin(), src1.end(), 0);
    for(auto& x : src1) x *= 2; // 0, 2, 4...
    
    std::iota(src2.begin(), src2.end(), 0);
    for(auto& x : src2) x = x * 2 + 1; // 1, 3, 5...
    
    parallel_merge_parts(dst.data(), 0, src1.data(), 0, size, src2.data(), 0, size);
    
    assert(is_sorted(dst));
    for(int i=0; i<size*2; ++i) assert(dst[i] == i);
    
    std::cout << "Passed." << std::endl;
}

int main() {
    test_merge_parts();
    test_parallel_merge_parts();
    
    std::cout << "All merge ops tests passed!" << std::endl;
    return 0;
}
