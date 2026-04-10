/**
 * Threshold Sweep Benchmark
 *
 * Tests intermediate values between 8192 and 65536 at 16 threads
 * Goal: Find optimal balance between parallelism and L3 cache contention
 *
 * Compile:
 *   g++ -O2 -march=native -std=c++17 -I../../include -o threshold_sweep.exe threshold_sweep.cpp -pthread
 */

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <array>

// We include the sorter but override the threshold at runtime via a modified sort
#include "dpqs/parallel/parallel_sort.hpp"
#include "dpqs/sequential_sorters.hpp"
#include "dpqs/run_merger.hpp"
#include "dpqs/completer.hpp"
#include "dpqs/threadpool.hpp"

namespace sweep_test {

// Modified parallel sort that accepts threshold as parameter
template <typename RandomIt, typename Comp = std::less<>>
void sort_with_threshold(RandomIt first, RandomIt last, int parallelism, std::ptrdiff_t threshold, Comp comp = Comp{}) {
    using T = typename std::iterator_traits<RandomIt>::value_type;
    
    std::ptrdiff_t size = last - first;
    if (size <= 1) return;
    
    // For small arrays or single thread, use sequential
    if (parallelism <= 1 || size <= threshold) {
        dual_pivot::sequential_int_sort(first, size, comp);
        return;
    }
    
    // Use thread pool with custom threshold
    dual_pivot::ThreadPool pool(parallelism);
    auto root = std::make_shared<dual_pivot::SortTask<RandomIt, Comp>>(
        first, 0, size, nullptr, comp, pool, threshold);
    
    pool.push_task([root]() { root->compute(); });
    pool.wait_for_task(root);
}

}  // namespace sweep_test

double benchmark_threshold(std::vector<int>& data, const std::vector<int>& original, 
                           int threads, std::ptrdiff_t threshold, int iterations) {
    std::vector<double> times;
    times.reserve(iterations);
    
    for (int i = 0; i < iterations; ++i) {
        data = original;  // Reset data
        
        auto start = std::chrono::high_resolution_clock::now();
        sweep_test::sort_with_threshold(data.begin(), data.end(), threads, threshold);
        auto end = std::chrono::high_resolution_clock::now();
        
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        times.push_back(ms);
    }
    
    // Return median
    std::sort(times.begin(), times.end());
    return times[iterations / 2];
}

int main() {
    constexpr size_t SIZE = 10'000'000;  // 10M integers
    constexpr int ITERATIONS = 10;
    constexpr int THREADS = 16;
    
    // Thresholds to test (finer granularity between 8192 and 65536)
    std::array<std::ptrdiff_t, 10> thresholds = {
        8192, 12288, 16384, 20480, 24576, 32768, 40960, 49152, 57344, 65536
    };
    
    std::cout << "=== Threshold Sweep Benchmark ===\n";
    std::cout << "Array size: " << SIZE << " integers (" << (SIZE * sizeof(int) / 1024 / 1024) << " MB)\n";
    std::cout << "Threads: " << THREADS << "\n";
    std::cout << "Iterations: " << ITERATIONS << " (median reported)\n\n";
    
    // Pre-allocate
    std::vector<int> data(SIZE);
    std::vector<int> original(SIZE);
    
    // Generate random data with fixed seed
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> dist(0, SIZE);
    for (size_t i = 0; i < SIZE; ++i) {
        original[i] = dist(gen);
    }
    
    // Warmup
    data = original;
    dual_pivot::sort(data, 4);
    
    std::cout << "| Threshold | Tasks (est.) | Runtime (ms) | vs 8192 |\n";
    std::cout << "|-----------|--------------|--------------|--------|\n";
    
    double baseline = 0.0;
    
    for (auto threshold : thresholds) {
        double median_ms = benchmark_threshold(data, original, THREADS, threshold, ITERATIONS);
        
        // Estimate task count
        int est_tasks = static_cast<int>(SIZE / threshold);
        
        if (threshold == 8192) {
            baseline = median_ms;
        }
        
        double vs_baseline = ((median_ms - baseline) / baseline) * 100.0;
        
        std::cout << "| " << std::setw(9) << threshold 
                  << " | " << std::setw(12) << "~" << est_tasks
                  << " | " << std::setw(12) << std::fixed << std::setprecision(2) << median_ms
                  << " | " << (vs_baseline >= 0 ? "+" : "") << std::setprecision(1) << vs_baseline << "% |\n";
        
        // Verify correctness
        if (!std::is_sorted(data.begin(), data.end())) {
            std::cerr << "ERROR: Sort failed at threshold " << threshold << "\n";
            return 1;
        }
    }
    
    std::cout << "\nAll tests passed.\n";
    return 0;
}
