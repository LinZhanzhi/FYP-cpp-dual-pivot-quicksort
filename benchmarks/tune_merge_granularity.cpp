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
    const size_t N = 50000000; // 50M integers
    const int ITER = 3; 
    const size_t RUN_LEN = 256; 
    
    // Warmup
    {
        auto data = generate_runs(100000, RUN_LEN, 0);
        dual_pivot::sort(data);
    }

    long long total_time = 0;
    for (int i = 0; i < ITER; ++i) {
        auto data = generate_runs(N, RUN_LEN, i);
        
        auto start = std::chrono::high_resolution_clock::now();
        dual_pivot::sort(data); 
        auto end = std::chrono::high_resolution_clock::now();
        total_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }
    std::cout << (total_time / ITER) << std::endl;
    return 0;
}
