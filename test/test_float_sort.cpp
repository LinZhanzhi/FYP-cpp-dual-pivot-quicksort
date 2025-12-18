#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>
#include <string>
#include <limits>
#include <cmath>
#include <iomanip>
#include <cstring>
#include "../include/dpqs/float_sort.hpp"

// Mocking the sequential sorters if they are not fully linked or to isolate float_sort logic.
// However, since we include float_sort.hpp which includes sequential_sorters.hpp, 
// we rely on the actual implementation if it compiles. 
// If linking fails, we might need to define them here, but let's try to use the real ones first.
// The user mentioned "although the sequential sorter is not implemented in it", 
// which might mean the *definitions* are missing or empty in the header?
// Let's check if we need to provide definitions.
// Based on the grep, they are defined as 'inline void ...' in the header, so they should be fine.

using namespace dual_pivot;

// --- Helper Functions ---

template <typename T>
bool is_sorted(const std::vector<T>& arr, int start, int end) {
    for (int i = start; i < end - 1; ++i) {
        // NaNs should be at the end, so if we see a NaN followed by a non-NaN, it's wrong.
        // But wait, the sort function moves NaNs to the end *outside* the sorted range?
        // Let's check process_and_sort_floats logic:
        // "Move NaNs to the end of the active range" -> array[k] = array[--effective_end_index];
        // Then it sorts [start_index, effective_end_index).
        // So NaNs are at [effective_end_index, end_index).
        
        // This helper checks strict ordering for the sorted part.
        if (arr[i] > arr[i + 1]) return false;
    }
    return true;
}

// Helper to check if a value is -0.0
template <typename T>
bool check_negative_zero(T val) {
    return val == 0 && std::signbit(val);
}

// Helper to check if a value is +0.0
template <typename T>
bool check_positive_zero(T val) {
    return val == 0 && !std::signbit(val);
}

// --- Test Runner ---

template <typename T>
void test_float_sort_logic(const std::string& type_name) {
    std::cout << "Testing float sort logic for " << type_name << "..." << std::endl;

    const int size = 20;
    std::vector<T> arr(size);
    
    // Construct a tricky array:
    // - NaNs
    // - Negative Zeros
    // - Positive Zeros
    // - Negative Numbers
    // - Positive Numbers
    
    T nan = std::numeric_limits<T>::quiet_NaN();
    T pos_zero = 0.0;
    T neg_zero = -0.0;
    
    // Fill array
    arr = {
        T(5.0), T(-3.0), nan, neg_zero, T(2.0), 
        pos_zero, T(-10.0), nan, neg_zero, T(0.0),
        T(1.0), T(-1.0), T(3.14), nan, T(-0.0),
        T(100.0), T(-100.0), T(0.0), T(-0.0), nan
    };

    // Expected behavior:
    // 1. NaNs moved to the end.
    // 2. Remaining elements sorted.
    // 3. -0.0 comes before +0.0.

    // Run the sort
    sort_specialized(arr.data(), 0, size);

    // Verification
    
    // 1. Check NaNs
    int nan_count = 0;
    for (T val : arr) {
        if (std::isnan(val)) nan_count++;
    }
    assert(nan_count == 4); // We put 4 NaNs in

    // Verify NaNs are at the end
    for (int i = size - nan_count; i < size; ++i) {
        if (!std::isnan(arr[i])) {
            std::cout << "Failed: NaN not at the end at index " << i << std::endl;
            return;
        }
    }

    // 2. Check Sorting of non-NaNs
    int sorted_end = size - nan_count;
    bool sorted = true;
    for (int i = 0; i < sorted_end - 1; ++i) {
        if (arr[i] > arr[i+1]) {
            sorted = false;
            std::cout << "Failed: Array not sorted at index " << i << " (" << arr[i] << " > " << arr[i+1] << ")" << std::endl;
        }
    }
    
    // 3. Check Negative Zero placement
    // Find zeros
    bool seen_pos_zero = false;
    for (int i = 0; i < sorted_end; ++i) {
        if (arr[i] == 0.0) {
            if (std::signbit(arr[i])) {
                // Found -0.0
                if (seen_pos_zero) {
                    std::cout << "Failed: Found -0.0 after +0.0 at index " << i << std::endl;
                    sorted = false;
                }
            } else {
                // Found +0.0
                seen_pos_zero = true;
            }
        }
    }

    if (sorted) {
        std::cout << "Passed." << std::endl;
    } else {
        std::cout << "Array content: ";
        for (T val : arr) std::cout << val << " ";
        std::cout << std::endl;
    }
}

int main() {
    test_float_sort_logic<float>("float");
    test_float_sort_logic<double>("double");
    return 0;
}
