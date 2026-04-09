/**
 * Quick test for Chase-Lev threadpool
 */

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "../include/dual_pivot_quicksort.hpp"

int main(int argc, char* argv[]) {
    constexpr size_t SIZE = 1'000'000;  // 1M integers (smaller for quick test)
    const int THREADS = (argc > 1) ? std::atoi(argv[1]) : 4;
    constexpr int ITERATIONS = 3;

    std::cout << "Quick Test - Chase-Lev Threadpool\n";
    std::cout << "Array size: " << SIZE << " integers\n";
    std::cout << "Threads: " << THREADS << "\n\n";

    std::vector<int> data(SIZE);
    std::vector<int> original(SIZE);

    // Generate random data
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> dist(0, SIZE);
    for (size_t i = 0; i < SIZE; ++i) {
        original[i] = dist(gen);
    }

    for (int i = 0; i < ITERATIONS; ++i) {
        data = original;

        std::cout << "Iteration " << (i + 1) << ": ";
        std::cout.flush();

        auto start = std::chrono::high_resolution_clock::now();
        dual_pivot::sort(data, THREADS);
        auto end = std::chrono::high_resolution_clock::now();

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << ms << " ms";

        if (!std::is_sorted(data.begin(), data.end())) {
            std::cout << " - FAILED!\n";
            return 1;
        }
        std::cout << " - OK\n";
    }

    std::cout << "\nAll tests PASSED\n";
    return 0;
}
