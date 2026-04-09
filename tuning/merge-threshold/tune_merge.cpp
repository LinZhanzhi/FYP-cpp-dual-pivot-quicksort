#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <numeric>
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

std::vector<int> generate_sorted(size_t size) {
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 0);
    return data;
}

long long bench_large_random() {
    auto data = generate_random(10000000, 42); // 10M
    auto start = std::chrono::high_resolution_clock::now();
    dual_pivot::sort(data);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

long long bench_small_arrays(size_t arr_size, bool sorted) {
    const int iterations = 20000; // 20k iterations
    std::vector<std::vector<int>> datasets(iterations);

    // Pre-generate
    for(int i=0; i<iterations; ++i) {
        if(sorted) datasets[i] = generate_sorted(arr_size);
        else datasets[i] = generate_random(arr_size, i);
    }

    auto start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<iterations; ++i) {
        dual_pivot::sort(datasets[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

int main(int argc, char** argv) {
    // 1. Large Random (The "Overhead" penalty check)
    long long t_large_random = bench_large_random();

    // 2. Small Random (The "Micro Overhead" penalty check)
    // Testing around the threshold boundaries of 64...300
    long long t_rnd_100 = bench_small_arrays(100, false);
    long long t_rnd_200 = bench_small_arrays(200, false);
    long long t_rnd_300 = bench_small_arrays(300, false);

    // 3. Small Sorted (The "Benefit" check)
    long long t_srt_100 = bench_small_arrays(100, true);
    long long t_srt_200 = bench_small_arrays(200, true);
    long long t_srt_300 = bench_small_arrays(300, true);
    long long t_srt_500 = bench_small_arrays(500, true);

    std::cout << t_large_random << ","
              << t_rnd_100 << "," << t_rnd_200 << "," << t_rnd_300 << ","
              << t_srt_100 << "," << t_srt_200 << "," << t_srt_300 << ","
              << t_srt_500 << std::endl;
}