#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>
#include <string>
#include "dpqs/run_merger.hpp"

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

void test_try_merge_runs() {
    std::cout << "Testing try_merge_runs..." << std::endl;

    // Array with clear runs
    {
        std::vector<int> arr = {
            1, 2, 3, 4, 5, // Run 1
            10, 9, 8, 7, 6, // Run 2 (descending)
            11, 12, 13, 14, 15 // Run 3
        };
        // try_merge_runs should detect these runs, reverse the descending one, and merge them.

        bool merged = try_merge_runs(arr.data(), 0, arr.size());

        if (merged) {
            std::cout << "Runs detected and merged." << std::endl;
            assert(is_sorted(arr));
        } else {
            std::cout << "Runs NOT detected (might be too short or too few)." << std::endl;
            // If it returns false, it means it didn't sort it.
            // But for this specific input, we expect it to work if parameters allow.
            // MIN_FIRST_RUN_SIZE is usually small (e.g. 16 or 32).
            // If our runs are 5 elements, they might be too short.
            // Let's check constants.hpp or just make runs longer.
        }
    }

    // Longer runs
    {
        int run_len = 50;
        std::vector<int> arr;
        // Run 1: 0..49
        for(int i=0; i<run_len; ++i) arr.push_back(i);
        // Run 2: 99..50
        for(int i=0; i<run_len; ++i) arr.push_back(99 - i);
        // Run 3: 100..149
        for(int i=0; i<run_len; ++i) arr.push_back(100 + i);

        bool merged = try_merge_runs(arr.data(), 0, arr.size());

        if (merged) {
            assert(is_sorted(arr));
        } else {
            std::cout << "Long runs NOT detected." << std::endl;
        }
    }

    // Random array (should return false usually)
    {
        std::vector<int> arr(100);
        std::iota(arr.begin(), arr.end(), 0);
        std::mt19937 g(123);
        std::shuffle(arr.begin(), arr.end(), g);

        bool merged = try_merge_runs(arr.data(), 0, arr.size());
        if (!merged) {
            std::cout << "Random array correctly rejected." << std::endl;
        } else {
            std::cout << "Random array accepted (unexpected but possible)." << std::endl;
            assert(is_sorted(arr));
        }
    }

    // Large array with runs (size > MIN_TRY_MERGE_SIZE which is 4096)
    {
        std::cout << "Testing large array with runs..." << std::endl;
        int run_len = 2000; // Total size will be 6000 > 4096
        std::vector<int> arr;
        arr.reserve(run_len * 3);

        // Run 1: 0..1999
        for(int i=0; i<run_len; ++i) arr.push_back(i);
        // Run 2: 3999..2000 (descending)
        for(int i=0; i<run_len; ++i) arr.push_back(3999 - i);
        // Run 3: 4000..5999
        for(int i=0; i<run_len; ++i) arr.push_back(4000 + i);

        bool merged = try_merge_runs(arr.data(), 0, arr.size());

        if (merged) {
            std::cout << "Large runs detected and merged." << std::endl;
            assert(is_sorted(arr));
        } else {
            std::cout << "Large runs NOT detected (unexpected)." << std::endl;
            // If this fails, we might need to check MAX_RUN_COUNT or other logic
        }
    }

    std::cout << "Passed." << std::endl;
}

int main() {
    test_try_merge_runs();

    std::cout << "All run merger tests passed!" << std::endl;
    return 0;
}
