#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>
#include <limits>
#include <type_traits>
#include "../include/dpqs/counting_sort.hpp"

// Mocking sort for sort_specialized if necessary, but we won't call it.
// Since counting_sort.hpp includes core_sort.hpp, we rely on that.

using namespace dual_pivot;

template <typename T>
void verify_sort(std::vector<T>& arr, int start, int end) {
    std::vector<T> copy = arr;

    // Run the algorithm
    counting_sort(arr.data(), start, end);

    // Run std::sort on the copy for verification
    std::sort(copy.begin() + start, copy.begin() + end);

    // Check if sorted
    bool sorted = true;
    for (int i = start; i < end - 1; ++i) {
        if (arr[i] > arr[i+1]) {
            sorted = false;
            break;
        }
    }

    if (!sorted) {
        std::cerr << "Test failed: Array not sorted!" << std::endl;
        std::cerr << "Expected: ";
        for (auto x : copy) std::cerr << (int)x << " ";
        std::cerr << "\nActual:   ";
        for (auto x : arr) std::cerr << (int)x << " ";
        std::cerr << std::endl;
        exit(1);
    }

    // Check if content matches std::sort (stability/correctness)
    for (int i = start; i < end; ++i) {
        if (arr[i] != copy[i]) {
            std::cerr << "Test failed: Result does not match std::sort!" << std::endl;
            exit(1);
        }
    }
}

template <typename T>
void test_random(int size, int start, int end) {
    std::cout << "Testing random array of size " << size << " [" << start << ", " << end << ")... ";
    std::vector<T> arr(size);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

    for (auto& val : arr) {
        val = static_cast<T>(dist(rng));
    }

    verify_sort(arr, start, end);
    std::cout << "Passed." << std::endl;
}

template <typename T>
void test_sorted(int size) {
    std::cout << "Testing sorted array of size " << size << "... ";
    std::vector<T> arr(size);
    for (int i = 0; i < size; ++i) {
        arr[i] = static_cast<T>(i % (std::numeric_limits<T>::max() - std::numeric_limits<T>::min() + 1) + std::numeric_limits<T>::min());
    }
    std::sort(arr.begin(), arr.end());
    verify_sort(arr, 0, size);
    std::cout << "Passed." << std::endl;
}

template <typename T>
void test_reverse_sorted(int size) {
    std::cout << "Testing reverse sorted array of size " << size << "... ";
    std::vector<T> arr(size);
    for (int i = 0; i < size; ++i) {
        arr[i] = static_cast<T>(i % (std::numeric_limits<T>::max() - std::numeric_limits<T>::min() + 1) + std::numeric_limits<T>::min());
    }
    std::sort(arr.rbegin(), arr.rend());
    verify_sort(arr, 0, size);
    std::cout << "Passed." << std::endl;
}

template <typename T>
void test_duplicates(int size) {
    std::cout << "Testing array with duplicates of size " << size << "... ";
    std::vector<T> arr(size);
    std::mt19937 rng(42);
    // Small range to force duplicates
    std::uniform_int_distribution<int> dist(0, 10);

    for (auto& val : arr) {
        val = static_cast<T>(dist(rng));
    }

    verify_sort(arr, 0, size);
    std::cout << "Passed." << std::endl;
}

template <typename T>
void run_tests_for_type(const std::string& type_name) {
    std::cout << "=== Testing type: " << type_name << " ===" << std::endl;

    // Small arrays (sparse case for counting sort)
    test_random<T>(10, 0, 10);
    test_random<T>(100, 0, 100);
    test_random<T>(128, 0, 128); // Boundary for sparse/dense optimization

    // Large arrays (dense case for counting sort)
    test_random<T>(129, 0, 129);
    test_random<T>(1000, 0, 1000);
    test_random<T>(66000, 0, 66000); // Larger than 2^16 for short tests

    // Sub-ranges
    test_random<T>(100, 20, 80);

    // Patterns
    test_sorted<T>(1000);
    test_reverse_sorted<T>(1000);
    test_duplicates<T>(1000);

    std::cout << std::endl;
}

int main() {
    // 1-byte types
    run_tests_for_type<char>("char");
    run_tests_for_type<unsigned char>("unsigned char");
    run_tests_for_type<int8_t>("int8_t");
    run_tests_for_type<uint8_t>("uint8_t");

    // 2-byte types
    run_tests_for_type<short>("short");
    run_tests_for_type<unsigned short>("unsigned short");
    run_tests_for_type<int16_t>("int16_t");
    run_tests_for_type<uint16_t>("uint16_t");

    std::cout << "All tests passed successfully!" << std::endl;
    return 0;
}
