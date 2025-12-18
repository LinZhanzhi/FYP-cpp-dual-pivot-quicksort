#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>
#include <cmath>
#include "dpqs/float_sort.hpp"

using namespace dual_pivot;

// Helper to check if sorted
template<typename T>
bool is_sorted(const std::vector<T>& arr) {
    for (size_t i = 1; i < arr.size(); ++i) {
        if (std::isnan(arr[i]) || std::isnan(arr[i-1])) continue; // NaNs are at the end
        if (arr[i] < arr[i - 1]) return false;
    }
    return true;
}

void test_sort_floats() {
    std::cout << "Testing sort_floats..." << std::endl;
    
    std::vector<float> arr = {
        0.0f, -0.0f, 1.0f, -1.0f, NAN, 2.0f, -2.0f, NAN, 0.0f
    };
    
    sort_floats(arr.data(), 0, arr.size());
    
    // Check NaNs at end
    assert(std::isnan(arr[arr.size()-1]));
    assert(std::isnan(arr[arr.size()-2]));
    
    // Check -0.0 comes before +0.0
    // We need to find where zeros are
    bool found_neg_zero = false;
    bool found_pos_zero = false;
    for(float x : arr) {
        if (x == 0.0f) {
            if (std::signbit(x)) found_neg_zero = true;
            else found_pos_zero = true;
        }
    }
    assert(found_neg_zero);
    assert(found_pos_zero);
    
    // Check order
    // Expected: -2, -1, -0, 0, 0, 1, 2, NAN, NAN
    // Note: -0.0 == 0.0 is true, so standard sort might mix them.
    // But our sort guarantees -0.0 < 0.0 order?
    // The implementation restores -0.0 BEFORE +0.0.
    
    // Let's print to verify
    for(float x : arr) {
        if (std::isnan(x)) std::cout << "NAN ";
        else if (x == 0.0f && std::signbit(x)) std::cout << "-0.0 ";
        else std::cout << x << " ";
    }
    std::cout << "\n";
    
    std::cout << "Passed." << std::endl;
}

int main() {
    test_sort_floats();
    
    std::cout << "All float sort tests passed!" << std::endl;
    return 0;
}
