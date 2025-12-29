#include "../include/dual_pivot_quicksort.hpp"
#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <functional>
#include <random>
#include <cmath>
#include <string>

// Helper to print vectors on failure
template<typename T>
void print_vec(const std::vector<T>& v, const std::string& name) {
    std::cerr << name << ": ";
    for (size_t i = 0; i < std::min(v.size(), size_t(20)); ++i) std::cerr << v[i] << " ";
    if (v.size() > 20) std::cerr << "...";
    std::cerr << std::endl;
}

int main() {
    std::cout << "Running Comprehensive Custom Comparator Tests..." << std::endl;

    // Test 1: Descending sort using std::greater
    {
        std::cout << "Test 1: Small Descending Sort (std::greater)... ";
        std::vector<int> data = {5, 2, 9, 1, 5, 6};
        std::vector<int> expected = {9, 6, 5, 5, 2, 1};
        
        dual_pivot::sort(data, std::greater<int>());
        
        if (data != expected) {
            std::cout << "FAILED" << std::endl;
            print_vec(data, "Got");
            print_vec(expected, "Expected");
            return 1;
        }
        std::cout << "PASSED" << std::endl;
    }

    // Test 2: Custom struct with custom comparator
    {
        std::cout << "Test 2: Custom Struct (Point)... ";
        struct Point {
            int x, y;
            bool operator==(const Point& other) const { return x == other.x && y == other.y; }
            bool operator!=(const Point& other) const { return !(*this == other); }
        };
        
        std::vector<Point> points = {{1, 2}, {3, 1}, {1, 1}, {2, 2}};
        // Sort by x, then by y
        auto comp = [](const Point& a, const Point& b) {
            if (a.x != b.x) return a.x < b.x;
            return a.y < b.y;
        };
        
        std::vector<Point> expected = {{1, 1}, {1, 2}, {2, 2}, {3, 1}};
        
        dual_pivot::sort(points, comp);
        
        if (points != expected) {
            std::cout << "FAILED" << std::endl;
            return 1;
        }
        std::cout << "PASSED" << std::endl;
    }

    // Test 3: Parallel Descending Sort (Large)
    {
        std::cout << "Test 3: Large Parallel Descending Sort (1M ints)... " << std::flush;
        int size = 1000000;
        std::vector<int> data(size);
        for (int i = 0; i < size; ++i) data[i] = i;
        
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(data.begin(), data.end(), g);
        
        dual_pivot::sort(data, 4, std::greater<int>());
        
        bool sorted = true;
        for (int i = 0; i < size - 1; ++i) {
            if (data[i] < data[i+1]) {
                sorted = false;
                std::cerr << "\nUnsorted at " << i << ": " << data[i] << " < " << data[i+1] << std::endl;
                break;
            }
        }
        
        if (!sorted) {
            std::cout << "FAILED" << std::endl;
            return 1;
        }
        std::cout << "PASSED" << std::endl;
    }

    // Test 4: Sequential Descending Sort (Large)
    {
        std::cout << "Test 4: Large Sequential Descending Sort (1M ints)... " << std::flush;
        int size = 1000000;
        std::vector<int> data(size);
        for (int i = 0; i < size; ++i) data[i] = i;
        
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(data.begin(), data.end(), g);
        
        dual_pivot::sort(data, 0, std::greater<int>());
        
        bool sorted = true;
        for (int i = 0; i < size - 1; ++i) {
            if (data[i] < data[i+1]) {
                sorted = false;
                break;
            }
        }
        
        if (!sorted) {
            std::cout << "FAILED" << std::endl;
            return 1;
        }
        std::cout << "PASSED" << std::endl;
    }

    // Test 5: Absolute Value Ascending
    {
        std::cout << "Test 5: Absolute Value Ascending... ";
        std::vector<int> data = {-5, 2, -9, 1, -5, 6, 0, -1};
        
        auto abs_less = [](int a, int b) {
            return std::abs(a) < std::abs(b);
        };

        dual_pivot::sort(data, abs_less);

        bool sorted = true;
        for (size_t i = 0; i < data.size() - 1; ++i) {
            if (std::abs(data[i]) > std::abs(data[i+1])) {
                sorted = false;
                std::cerr << "Failed at " << i << ": abs(" << data[i] << ") > abs(" << data[i+1] << ")" << std::endl;
                break;
            }
        }
        
        if (!sorted) {
            std::cout << "FAILED" << std::endl;
            print_vec(data, "Data");
            return 1;
        }
        std::cout << "PASSED" << std::endl;
    }

    // Test 6: Absolute Value Descending
    {
        std::cout << "Test 6: Absolute Value Descending... ";
        std::vector<int> data = {-5, 2, -9, 1, -5, 6, 0, -1};
        
        auto abs_greater = [](int a, int b) {
            return std::abs(a) > std::abs(b);
        };

        dual_pivot::sort(data, abs_greater);

        bool sorted = true;
        for (size_t i = 0; i < data.size() - 1; ++i) {
            if (std::abs(data[i]) < std::abs(data[i+1])) {
                sorted = false;
                std::cerr << "Failed at " << i << ": abs(" << data[i] << ") < abs(" << data[i+1] << ")" << std::endl;
                break;
            }
        }
        
        if (!sorted) {
            std::cout << "FAILED" << std::endl;
            print_vec(data, "Data");
            return 1;
        }
        std::cout << "PASSED" << std::endl;
    }

    // Test 7: String Length Sort
    {
        std::cout << "Test 7: String Length Sort... ";
        std::vector<std::string> data = {"apple", "b", "cat", "banana", "dog", "elephant"};
        
        // Sort by length, then lexicographically
        auto len_comp = [](const std::string& a, const std::string& b) {
            if (a.length() != b.length()) return a.length() < b.length();
            return a < b;
        };

        std::vector<std::string> expected = {"b", "cat", "dog", "apple", "banana", "elephant"};
        
        dual_pivot::sort(data, len_comp);

        if (data != expected) {
            std::cout << "FAILED" << std::endl;
            print_vec(data, "Got");
            return 1;
        }
        std::cout << "PASSED" << std::endl;
    }

    // Test 8: Double Descending
    {
        std::cout << "Test 8: Double Descending... ";
        std::vector<double> data = {1.1, 5.5, 2.2, 9.9, 3.3};
        std::vector<double> expected = {9.9, 5.5, 3.3, 2.2, 1.1};
        
        dual_pivot::sort(data, std::greater<double>());
        
        if (data != expected) {
            std::cout << "FAILED" << std::endl;
            print_vec(data, "Got");
            return 1;
        }
        std::cout << "PASSED" << std::endl;
    }

    // Test 9: Already Sorted (Descending) - Triggers Run Merger
    {
        std::cout << "Test 9: Already Sorted Descending (Run Merger)... ";
        int size = 10000;
        std::vector<int> data(size);
        for (int i = 0; i < size; ++i) data[i] = size - i; // 10000, 9999, ...
        
        // It is already sorted according to std::greater
        std::vector<int> expected = data;
        
        dual_pivot::sort(data, std::greater<int>());
        
        if (data != expected) {
            std::cout << "FAILED" << std::endl;
            return 1;
        }
        std::cout << "PASSED" << std::endl;
    }

    // Test 10: Reverse Sorted (Ascending input, Descending sort)
    {
        std::cout << "Test 10: Reverse Sorted (Ascending input, Descending sort)... ";
        int size = 10000;
        std::vector<int> data(size);
        for (int i = 0; i < size; ++i) data[i] = i; // 0, 1, 2...
        
        // We want to sort descending, so expected is size-1...0
        std::vector<int> expected(size);
        for (int i = 0; i < size; ++i) expected[i] = size - 1 - i;
        
        dual_pivot::sort(data, std::greater<int>());
        
        if (data != expected) {
            std::cout << "FAILED" << std::endl;
            return 1;
        }
        std::cout << "PASSED" << std::endl;
    }

    std::cout << "All tests passed successfully!" << std::endl;
    return 0;
}
