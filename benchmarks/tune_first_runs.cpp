#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <numeric>
#include "dual_pivot_quicksort.hpp"

// Generates an array of size N composed of sorted runs of length L
std::vector<int> generate_runs(size_t n, size_t run_len, int seed) {
    std::vector<int> data(n);
    std::mt19937 rng(seed);

    // Fill with random numbers first
    for (size_t i = 0; i < n; ++i) {
        data[i] = rng();
    }

    // Sort segments of length run_len
    for (size_t i = 0; i < n; i += run_len) {
        size_t end = std::min(i + run_len, n);
        std::sort(data.begin() + i, data.begin() + end);
    }

    return data;
}

int main() {
    // Increase size to 10M to match other benchmarks and get stable timings
    const size_t N = 10000000;
    const int ITER = 10;

    std::vector<size_t> run_lengths = {32, 64, 80, 96, 128};

    for (size_t len : run_lengths) {
        long long total_time = 0;
        for (int i = 0; i < ITER; ++i) {
            auto data = generate_runs(N, len, i);
            auto start = std::chrono::high_resolution_clock::now();
            dual_pivot::sort(data);
            auto end = std::chrono::high_resolution_clock::now();
            total_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }
        std::cout << len << "," << (total_time / ITER) << std::endl;
    }
    return 0;
}
