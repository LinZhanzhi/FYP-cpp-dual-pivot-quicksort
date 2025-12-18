#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>
#include <string>
#include <limits>
#include <iomanip>
#include <type_traits>
#include "../include/dpqs/heap_sort.hpp"

using namespace dual_pivot;

// --- Helper Functions ---

template <typename T>
bool is_sorted(const std::vector<T>& arr, int start, int end) {
    for (int i = start; i < end - 1; ++i) {
        if (arr[i] > arr[i + 1]) return false;
    }
    return true;
}

// --- Wrappers for Heap Sort Functions ---

// Wrapper to call the generic template heapSort
template <typename T>
void run_generic_heap_sort(std::vector<T>& arr, int start, int end) {
    heapSort(arr.data(), start, end);
}

// Wrappers for static specialized functions
void run_int_heap_sort(std::vector<int>& arr, int start, int end) {
    heapSort_int(arr.data(), start, end);
}

void run_long_heap_sort(std::vector<long>& arr, int start, int end) {
    heapSort_long(arr.data(), start, end);
}

void run_float_heap_sort(std::vector<float>& arr, int start, int end) {
    heapSort_float(arr.data(), start, end);
}

void run_double_heap_sort(std::vector<double>& arr, int start, int end) {
    heapSort_double(arr.data(), start, end);
}

// --- Test Runner ---

template <typename T, typename Func>
void test_scenario(const std::string& name, int size, int start, int end, Func sort_func, const std::string& type_name) {
    std::cout << "Testing " << name << " for " << type_name << " (size " << size << ", range [" << start << ", " << end << "))... ";
    
    std::vector<T> arr(size);
    std::mt19937 gen(42); // Fixed seed for reproducibility
    
    if (name == "random") {
        if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dist(-10000.0, 10000.0);
            for(auto& x : arr) x = dist(gen);
        } else {
            // Use a smaller range for integers to avoid overflow issues in simple checks if any
            std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min() / 2, std::numeric_limits<T>::max() / 2);
            for(auto& x : arr) x = dist(gen);
        }
    } else if (name == "sorted") {
        for(int i=0; i<size; ++i) arr[i] = static_cast<T>(i);
    } else if (name == "reverse_sorted") {
        for(int i=0; i<size; ++i) arr[i] = static_cast<T>(size - i);
    } else if (name == "duplicates") {
        for(int i=0; i<size; ++i) arr[i] = static_cast<T>(i % 10);
    }

    std::vector<T> copy = arr;
    
    // Run the sort
    sort_func(arr, start, end);

    // Verify 1: Check elements outside range are untouched
    bool untouched = true;
    for(int i=0; i<start; ++i) if(arr[i] != copy[i]) untouched = false;
    for(int i=end; i<size; ++i) if(arr[i] != copy[i]) untouched = false;

    if (!untouched) {
        std::cout << "Failed: Elements outside range modified!" << std::endl;
        return;
    }

    // Verify 2: Check range is sorted
    if (is_sorted(arr, start, end)) {
        // Verify 3: Check permutation (sort copy and compare)
        std::vector<T> sorted_copy = copy;
        std::sort(sorted_copy.begin() + start, sorted_copy.begin() + end);
        
        bool match = true;
        for(int i=start; i<end; ++i) {
            // For floating point, exact match might be tricky due to precision, but for sort it should be exact if values are preserved
            if (arr[i] != sorted_copy[i]) {
                match = false; 
                break;
            }
        }
        
        if (match) std::cout << "Passed." << std::endl;
        else std::cout << "Failed: Elements mismatch (permutation check)!" << std::endl;
    } else {
        std::cout << "Failed: Not sorted!" << std::endl;
        if (size <= 20) {
             std::cout << "Expected: ";
             std::vector<T> c = copy;
             std::sort(c.begin() + start, c.begin() + end);
             for(int i=start; i<end; ++i) std::cout << c[i] << " ";
             std::cout << "\nActual:   ";
             for(int i=start; i<end; ++i) std::cout << arr[i] << " ";
             std::cout << std::endl;
        }
    }
}

int main() {
    // 1. Test Generic Template
    std::cout << "=== Testing Generic Template heapSort<T> ===" << std::endl;
    test_scenario<int>("random", 100, 0, 100, run_generic_heap_sort<int>, "int");
    test_scenario<int>("random", 1000, 100, 900, run_generic_heap_sort<int>, "int"); // Partial range
    test_scenario<double>("random", 100, 0, 100, run_generic_heap_sort<double>, "double");

    // 2. Test Specialized int
    std::cout << "\n=== Testing Specialized heapSort_int ===" << std::endl;
    test_scenario<int>("random", 1000, 0, 1000, run_int_heap_sort, "int");
    test_scenario<int>("sorted", 1000, 0, 1000, run_int_heap_sort, "int");
    test_scenario<int>("reverse_sorted", 1000, 0, 1000, run_int_heap_sort, "int");
    test_scenario<int>("duplicates", 1000, 0, 1000, run_int_heap_sort, "int");
    test_scenario<int>("partial_range", 100, 20, 80, run_int_heap_sort, "int");

    // 3. Test Specialized long
    std::cout << "\n=== Testing Specialized heapSort_long ===" << std::endl;
    test_scenario<long>("random", 1000, 0, 1000, run_long_heap_sort, "long");
    test_scenario<long>("reverse_sorted", 1000, 0, 1000, run_long_heap_sort, "long");

    // 4. Test Specialized float
    std::cout << "\n=== Testing Specialized heapSort_float ===" << std::endl;
    test_scenario<float>("random", 1000, 0, 1000, run_float_heap_sort, "float");
    test_scenario<float>("duplicates", 1000, 0, 1000, run_float_heap_sort, "float");

    // 5. Test Specialized double
    std::cout << "\n=== Testing Specialized heapSort_double ===" << std::endl;
    test_scenario<double>("random", 1000, 0, 1000, run_double_heap_sort, "double");
    test_scenario<double>("reverse_sorted", 1000, 0, 1000, run_double_heap_sort, "double");

    std::cout << "\nAll tests completed successfully." << std::endl;
    return 0;
}
