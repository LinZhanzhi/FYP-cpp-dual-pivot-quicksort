#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include "dual_pivot_quicksort.hpp"

using namespace dual_pivot;

// ----- 1. Byte (char) Tuning -----
// Testing range: 32 - 128
long long bench_char_arrays(size_t arr_size) {
    const int iterations = 50000;
    std::vector<std::vector<char>> datasets(iterations);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(-128, 127);

    // Pre-generate
    for(int i=0; i<iterations; ++i) {
        datasets[i].resize(arr_size);
        for(auto& x : datasets[i]) x = (char)dist(rng);
    }

    auto start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<iterations; ++i) {
        dual_pivot::sort(datasets[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

// ----- 2. Short Tuning -----
// Testing range: 500 - 4000
long long bench_short_arrays(size_t arr_size) {
    const int iterations = 5000;
    std::vector<std::vector<short>> datasets(iterations);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(-32768, 32767);

    // Pre-generate
    for(int i=0; i<iterations; ++i) {
        datasets[i].resize(arr_size);
        for(auto& x : datasets[i]) x = (short)dist(rng);
    }

    auto start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<iterations; ++i) {
        dual_pivot::sort(datasets[i]);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

int main(int argc, char** argv) {
    // We output a CSV-like line: Byte_Time_at_64, Short_Time_at_2000
    // But since the python script changes one constant at a time, we run fixed workload sizes.

    // For Byte tuning: Fixed size = 80 (Currently near split point)
    long long t_byte = bench_char_arrays(80);

    // For Short tuning: Fixed size = 2000 (Currently near split point)
    long long t_short = bench_short_arrays(2000);

    std::cout << t_byte << "," << t_short << std::endl;
    return 0;
}