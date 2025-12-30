#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <thread>
#include "dual_pivot_quicksort.hpp"

// Simple timer
class Timer {
    std::chrono::high_resolution_clock::time_point start;
public:
    Timer() : start(std::chrono::high_resolution_clock::now()) {}
    double elapsed() {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start).count();
    }
};

void run_test(size_t size, int threads) {
    // Generate data
    std::vector<int> data(size);
    std::mt19937 rng(42); // Fixed seed for reproducibility
    std::uniform_int_distribution<int> dist(0, 1000000000);
    for(auto& x : data) x = dist(rng);

    std::cout << "Sorting " << size << " ints with " << threads << " threads... " << std::flush;

    // Reset stats
    auto& pool = dual_pivot::getThreadPool(threads);
    pool.reset_stats();

    Timer t;
    dual_pivot::dual_pivot_quicksort_parallel(data.begin(), data.end(), threads);
    double time = t.elapsed();

    std::cout << "Time: " << std::fixed << std::setprecision(4) << time << "s";
    std::cout << " | Tasks: " << pool.get_tasks_executed() << std::endl;
}

int main() {
    const size_t SIZE = 50000000; // 50 Million
    std::cout << "Diagnostic Benchmark (Size: " << SIZE << ")" << std::endl;
    std::cout << "Hardware Concurrency: " << std::thread::hardware_concurrency() << std::endl;
    std::cout << "------------------------------------------------" << std::endl;

    // Baseline std::sort
    {
        std::vector<int> data(SIZE);
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> dist(0, 1000000000);
        for(auto& x : data) x = dist(rng);

        std::cout << "std::sort baseline... " << std::flush;
        Timer t;
        std::sort(data.begin(), data.end());
        std::cout << "Time: " << t.elapsed() << "s" << std::endl;
    }

    std::vector<int> thread_counts = {1, 2, 4, 8, 16, 24};

    for (int t : thread_counts) {
        run_test(SIZE, t);
    }

    return 0;
}
