/**
 * VTune Profiling Benchmark
 *
 * Simple, focused benchmark for memory-bound vs CPU-bound analysis.
 * Runs parallel sorting with 16 threads on 10M random integers.
 */

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "../include/dual_pivot_quicksort.hpp"

int main() {
    constexpr size_t SIZE = 10'000'000;  // 10M integers
    constexpr int THREADS = 16;
    constexpr int ITERATIONS = 20;  // More iterations for reliable profiling

    std::cout << "VTune Profiling Benchmark\n";
    std::cout << "Array size: " << SIZE << " integers (" << (SIZE * sizeof(int) / 1024 / 1024) << " MB)\n";
    std::cout << "Threads: " << THREADS << "\n";
    std::cout << "Iterations: " << ITERATIONS << "\n\n";

    // Pre-allocate
    std::vector<int> data(SIZE);
    std::vector<int> original(SIZE);

    // Generate random data
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> dist(0, SIZE);
    for (size_t i = 0; i < SIZE; ++i) {
        original[i] = dist(gen);
    }

    std::cout << "Starting profiling warmup...\n";

    // Warmup (1 iteration)
    data = original;
    dual_pivot::sort(data, THREADS);

    std::cout << "Starting profiled iterations...\n";

    // Main profiling loop
    auto total_start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; ++i) {
        data = original;  // Reset data

        auto start = std::chrono::high_resolution_clock::now();
        dual_pivot::sort(data, THREADS);
        auto end = std::chrono::high_resolution_clock::now();

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "  Iteration " << (i + 1) << ": " << ms << " ms\n";
    }

    auto total_end = std::chrono::high_resolution_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_end - total_start).count();

    std::cout << "\nTotal time: " << total_ms << " ms\n";
    std::cout << "Average: " << (total_ms / ITERATIONS) << " ms per sort\n";

    // Verify correctness
    bool sorted = std::is_sorted(data.begin(), data.end());
    std::cout << "\nResult verified: " << (sorted ? "CORRECT" : "FAILED") << "\n";

    return sorted ? 0 : 1;
}
