#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>
#include <string>
#include <limits>
#include <functional>
#include <type_traits>
#include <iomanip>
#include "dual_pivot_quicksort.hpp"

// ANSI Color Codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"

int g_tests_passed = 0;
int g_tests_failed = 0;

template <typename T>
bool is_sorted(const std::vector<T>& v) {
    for (size_t i = 1; i < v.size(); ++i) {
        if (v[i] < v[i-1]) return false;
    }
    return true;
}

// Specialization for float/double to handle NaNs if necessary,
// but for standard sorting we expect standard behavior.
// The dual_pivot implementation claims to handle -0.0 and NaNs.
// For now, we stick to standard strict weak ordering checks.

template <typename T>
void print_array(const std::vector<T>& v, size_t max_len = 10) {
    std::cout << "[";
    for (size_t i = 0; i < std::min(v.size(), max_len); ++i) {
        std::cout << v[i] << (i < std::min(v.size(), max_len) - 1 ? ", " : "");
    }
    if (v.size() > max_len) std::cout << ", ...";
    std::cout << "]";
}

template <typename T>
int find_unsorted_index(const std::vector<T>& v) {
    for (size_t i = 1; i < v.size(); ++i) {
        if (v[i] < v[i-1]) return i;
    }
    return -1;
}

template <typename T>
void run_test(const std::string& test_name, std::vector<T> input, int parallelism = -1) {
    std::cout << "Running " << std::left << std::setw(50) << test_name << "... ";

    std::vector<T> expected = input;
    std::sort(expected.begin(), expected.end());

    try {
        if (parallelism == -1) {
            dual_pivot::sort(input);
        } else {
            dual_pivot::sort(input, parallelism);
        }

        bool sorted = is_sorted(input);
        bool match = (input == expected);

        if (sorted && match) {
            std::cout << GREEN << "PASS" << RESET << std::endl;
            g_tests_passed++;
        } else {
            std::cout << RED << "FAIL" << RESET << std::endl;
            if (!sorted) {
                std::cout << "  Error: Array is not sorted." << std::endl;
                int idx = find_unsorted_index(input);
                if (idx != -1) {
                    std::cout << "  First unsorted index: " << idx << std::endl;
                    std::cout << "  input[" << idx-1 << "] = " << input[idx-1] << ", input[" << idx << "] = " << input[idx] << std::endl;
                }
            }
            if (!match) std::cout << "  Error: Array content mismatch (permutation error)." << std::endl;
            std::cout << "  Expected: "; print_array(expected); std::cout << std::endl;
            std::cout << "  Actual:   "; print_array(input); std::cout << std::endl;
            g_tests_failed++;
        }
    } catch (const std::exception& e) {
        std::cout << RED << "FAIL (Exception: " << e.what() << ")" << RESET << std::endl;
        g_tests_failed++;
    }
}

// Data Generators
template <typename T>
std::vector<T> generate_random(size_t size, T min, T max) {
    std::vector<T> v(size);
    std::mt19937 gen(42); // Fixed seed for reproducibility
    if constexpr (std::is_floating_point_v<T>) {
        std::uniform_real_distribution<T> dist(min, max);
        for (auto& x : v) x = dist(gen);
    } else {
        std::uniform_int_distribution<T> dist(min, max);
        for (auto& x : v) x = dist(gen);
    }
    return v;
}

template <typename T>
std::vector<T> generate_sorted(size_t size) {
    std::vector<T> v(size);
    for (size_t i = 0; i < size; ++i) v[i] = static_cast<T>(i);
    return v;
}

template <typename T>
std::vector<T> generate_reverse_sorted(size_t size) {
    std::vector<T> v(size);
    for (size_t i = 0; i < size; ++i) v[i] = static_cast<T>(size - i);
    return v;
}

template <typename T>
std::vector<T> generate_duplicates(size_t size, T value) {
    return std::vector<T>(size, value);
}

template <typename T>
std::vector<T> generate_sawtooth(size_t size, int period) {
    std::vector<T> v(size);
    for (size_t i = 0; i < size; ++i) v[i] = static_cast<T>(i % period);
    return v;
}

int main() {
    std::cout << "========================================================" << std::endl;
    std::cout << "       Dual-Pivot Quicksort Comprehensive Test Suite    " << std::endl;
    std::cout << "========================================================" << std::endl;

    // -------------------------------------------------------------------------
    // 1. Basic Integer Types (int)
    // -------------------------------------------------------------------------
    std::cout << "\n--- Integer Tests (int) ---\n";
    run_test<int>("Int Empty", {});
    run_test<int>("Int Single Element", {42});
    run_test<int>("Int Small Random (Size 10)", generate_random<int>(10, -100, 100));
    run_test<int>("Int Medium Random (Size 1000)", generate_random<int>(1000, -10000, 10000));
    run_test<int>("Int Sorted", generate_sorted<int>(1000));
    run_test<int>("Int Reverse Sorted", generate_reverse_sorted<int>(1000));
    run_test<int>("Int All Duplicates", generate_duplicates<int>(1000, 7));
    run_test<int>("Int Sawtooth", generate_sawtooth<int>(1000, 50));
    run_test<int>("Int Sawtooth Large", generate_sawtooth<int>(5012, 501));

    // -------------------------------------------------------------------------
    // 2. Large Integer Tests (Parallelism Check)
    // -------------------------------------------------------------------------
    std::cout << "\n--- Large Integer Tests (Parallelism) ---\n";
    // Force sequential
    run_test<int>("Int Large Random (Seq)", generate_random<int>(100000, 0, 1000000), 0);
    // Force parallel
    run_test<int>("Int Large Random (Parallel)", generate_random<int>(100000, 0, 1000000), 4);

    // -------------------------------------------------------------------------
    // 3. Floating Point Types (double, float)
    // -------------------------------------------------------------------------
    std::cout << "\n--- Floating Point Tests ---\n";
    run_test<double>("Double Random", generate_random<double>(1000, -100.0, 100.0));
    run_test<float>("Float Random", generate_random<float>(1000, -100.0f, 100.0f));
    run_test<double>("Double With Duplicates", generate_duplicates<double>(1000, 3.14159));

    // -------------------------------------------------------------------------
    // 4. Small Integral Types (char, short) - Triggers Counting Sort
    // -------------------------------------------------------------------------
    std::cout << "\n--- Small Integral Types (Counting Sort Path) ---\n";
    run_test<char>("Char Random", generate_random<char>(1000, -120, 120));
    run_test<unsigned char>("UChar Random", generate_random<unsigned char>(1000, 0, 255));
    run_test<short>("Short Random", generate_random<short>(1000, -30000, 30000));
    run_test<unsigned short>("UShort Random", generate_random<unsigned short>(1000, 0, 60000));

    // -------------------------------------------------------------------------
    // 5. Long Types
    // -------------------------------------------------------------------------
    std::cout << "\n--- Long Integer Tests ---\n";
    run_test<long long>("Long Long Random", generate_random<long long>(1000, -1000000LL, 1000000LL));

    // -------------------------------------------------------------------------
    // 6. Edge Cases
    // -------------------------------------------------------------------------
    std::cout << "\n--- Edge Cases ---\n";
    std::vector<int> min_max = {std::numeric_limits<int>::max(), std::numeric_limits<int>::min(), 0, -1, 1};
    run_test<int>("Int Min/Max", min_max);

    // -------------------------------------------------------------------------
    // Summary
    // -------------------------------------------------------------------------
    std::cout << "\n========================================================" << std::endl;
    std::cout << "Test Summary: ";
    if (g_tests_failed == 0) {
        std::cout << GREEN << "ALL TESTS PASSED (" << g_tests_passed << "/" << g_tests_passed << ")" << RESET << std::endl;
    } else {
        std::cout << RED << "SOME TESTS FAILED (" << g_tests_failed << " failed, " << g_tests_passed << " passed)" << RESET << std::endl;
    }
    std::cout << "========================================================" << std::endl;

    return (g_tests_failed == 0) ? 0 : 1;
}
