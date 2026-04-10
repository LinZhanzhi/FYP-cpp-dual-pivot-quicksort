/**
 * Threshold Comparison Benchmark
 *
 * Compares MIN_PARALLEL_SORT_SIZE = 8192 vs 65536
 * Tests across all thread counts: 1, 2, 4, 8, 16
 * Measures runtime for 10M random integers
 *
 * Compile with:
 *   g++ -O2 -march=native -std=c++17 -o threshold_8192 threshold_comparison.cpp -DMIN_PARALLEL_SORT_SIZE=8192 -pthread
 *   g++ -O2 -march=native -std=c++17 -o threshold_65536 threshold_comparison.cpp -DMIN_PARALLEL_SORT_SIZE=65536 -pthread
 *
 * For VTune profiling (single thread count):
 *   vtune -collect uarch-exploration -result-dir vtune_8192_16t ./threshold_8192 16
 *   vtune -collect uarch-exploration -result-dir vtune_65536_16t ./threshold_65536 16
 */

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include "../../include/dual_pivot_quicksort.hpp"

// Capture the threshold at compile time
#ifndef MIN_PARALLEL_SORT_SIZE
#define MIN_PARALLEL_SORT_SIZE 65536
#endif
constexpr int THRESHOLD_VALUE = MIN_PARALLEL_SORT_SIZE;

// Report the threshold being used
void report_threshold() {
    std::cout << "MIN_PARALLEL_SORT_SIZE = " << THRESHOLD_VALUE << "\n";
}

double benchmark_sort(std::vector<int>& data, const std::vector<int>& original, int threads, int iterations) {
    std::vector<double> times;
    times.reserve(iterations);

    for (int i = 0; i < iterations; ++i) {
        data = original;  // Reset data

        auto start = std::chrono::high_resolution_clock::now();
        dual_pivot::sort(data, threads);
        auto end = std::chrono::high_resolution_clock::now();

        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        times.push_back(ms);
    }

    // Return median
    std::sort(times.begin(), times.end());
    return times[iterations / 2];
}

int main(int argc, char* argv[]) {
    constexpr size_t SIZE = 10'000'000;  // 10M integers
    constexpr int ITERATIONS = 10;

    // If a thread count is specified, only test that count (for VTune)
    bool single_thread_mode = (argc > 1);
    int single_thread_count = single_thread_mode ? std::atoi(argv[1]) : 0;

    std::cout << "=== Threshold Comparison Benchmark ===\n";
    std::cout << "Array size: " << SIZE << " integers (" << (SIZE * sizeof(int) / 1024 / 1024) << " MB)\n";
    report_threshold();
    std::cout << "Iterations: " << ITERATIONS << " (median reported)\n\n";

    // Pre-allocate
    std::vector<int> data(SIZE);
    std::vector<int> original(SIZE);

    // Generate random data with fixed seed for reproducibility
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> dist(0, SIZE);
    for (size_t i = 0; i < SIZE; ++i) {
        original[i] = dist(gen);
    }

    // Warmup
    data = original;
    dual_pivot::sort(data, 4);

    if (single_thread_mode) {
        // Single thread mode for VTune profiling
        std::cout << "Running " << ITERATIONS << " iterations with " << single_thread_count << " threads...\n\n";

        double median_ms = benchmark_sort(data, original, single_thread_count, ITERATIONS);

        std::cout << "Threads: " << single_thread_count << "\n";
        std::cout << "Median time: " << std::fixed << std::setprecision(2) << median_ms << " ms\n";

        // Verify correctness
        bool sorted = std::is_sorted(data.begin(), data.end());
        std::cout << "Result: " << (sorted ? "CORRECT" : "FAILED") << "\n";

        return sorted ? 0 : 1;
    }

    // Full comparison mode
    std::cout << "| Threads | Runtime (ms) | Speedup |\n";
    std::cout << "|---------|--------------|--------|\n";

    int thread_counts[] = {1, 2, 4, 8, 16};
    double baseline = 0.0;

    for (int threads : thread_counts) {
        double median_ms = benchmark_sort(data, original, threads, ITERATIONS);

        if (threads == 1) {
            baseline = median_ms;
        }

        double speedup = baseline / median_ms;

        std::cout << "| " << std::setw(7) << threads
                  << " | " << std::setw(12) << std::fixed << std::setprecision(2) << median_ms
                  << " | " << std::setw(6) << std::fixed << std::setprecision(2) << speedup << "x |\n";
    }

    // Verify correctness
    bool sorted = std::is_sorted(data.begin(), data.end());
    std::cout << "\nResult verified: " << (sorted ? "CORRECT" : "FAILED") << "\n";

    return sorted ? 0 : 1;
}
