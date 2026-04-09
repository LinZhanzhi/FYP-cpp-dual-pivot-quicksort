#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include "../include/dual_pivot_quicksort.hpp"

int main() {
    const int SIZE = 10'000'000;
    const int WARMUP = 2;
    const int RUNS = 5;

    // Generate random data
    std::vector<int> original(SIZE);
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> dist(0, SIZE);
    for (auto& x : original) x = dist(gen);

    std::vector<int> threads = {1, 2, 4, 8, 16};

    std::cout << "threads,median_ms" << std::endl;

    for (int t : threads) {
        std::vector<double> times;

        for (int i = 0; i < WARMUP + RUNS; i++) {
            std::vector<int> data = original;

            auto start = std::chrono::high_resolution_clock::now();
            dual_pivot::sort(data, t);
            auto end = std::chrono::high_resolution_clock::now();

            double ms = std::chrono::duration<double, std::milli>(end - start).count();

            if (i >= WARMUP) {
                times.push_back(ms);
            }
        }

        // Get median
        std::sort(times.begin(), times.end());
        double median = times[times.size() / 2];

        std::cout << t << "," << median << std::endl;
    }

    return 0;
}
