#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include "dual_pivot_quicksort.hpp"

using namespace dual_pivot;

// Utils
std::vector<int> generate_random(size_t size, int seed) {
    std::vector<int> data(size);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(-1000000, 1000000);
    for(auto& x : data) x = dist(rng);
    return data;
}

long long bench_workload() {
    // We sort a large array (10M elements).
    // Since Mixed IS affects the leaves of the recursion tree on non-left paths,
    // this cumulative test should show the impact on total throughput.
    auto data = generate_random(10000000, 12345);

    auto start = std::chrono::high_resolution_clock::now();
    dual_pivot::sort(data);
    auto end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

int main() {
    // Run multiple iterations to reduce noise
    long long total_time = 0;
    const int ITERATIONS = 20; // Increased to 20 for stability

    // Warmup
    bench_workload();

    for(int i=0; i<ITERATIONS; ++i) {
        total_time += bench_workload();
    }

    std::cout << (total_time / ITERATIONS) << std::endl;
    return 0;
}