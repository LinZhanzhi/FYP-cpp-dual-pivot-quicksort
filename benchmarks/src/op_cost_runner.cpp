#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <numeric>
#include <iomanip>
#include <string>

// Prevent optimizer from removing the code
void doNotOptimize(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}

struct Results {
    double comp_ns;
    double assign_ns;
    double swap_ns;
};

Results run_benchmark(size_t N, const std::string& label) {
    const size_t ITERATIONS = (N > 1000000) ? 50 : 5000;

    std::cout << "\n------------------------------------------------\n";
    std::cout << "Measuring: " << label << "\n";
    std::cout << "Array Size: " << N << " ints (" << (N * sizeof(int)) / 1024 << " KB)\n";

    std::vector<int> arr(N);
    std::iota(arr.begin(), arr.end(), 0);
    std::mt19937 gen(42);
    std::shuffle(arr.begin(), arr.end(), gen);

    // --- 1. Assignment (Copy) ---
    // Modeled as: dest[i] = src[i]
    std::vector<int> arr_copy(N);
    auto start = std::chrono::high_resolution_clock::now();
    for(size_t k=0; k<ITERATIONS; ++k) {
        // We use std::copy to simulate efficient block copy/assignment
        // But manual loop is often what we want to measure for specific item assignment
        // Let's stick to manual loop to represent "moving one item" cost
        for(size_t i=0; i<N; ++i) {
            arr_copy[i] = arr[i];
        }
        doNotOptimize(arr_copy.data());
    }
    auto end = std::chrono::high_resolution_clock::now();
    double total_ops = (double)N * ITERATIONS;
    double time_assign_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / total_ops;

    // --- 2. Comparison ---
    // Modeled as: if (arr[i] < random_val)
    size_t count = 0;
    int pivot = N/2;
    start = std::chrono::high_resolution_clock::now();
    for(size_t k=0; k<ITERATIONS; ++k) {
        for(size_t i=0; i<N; ++i) {
            if (arr[i] < pivot) count++;
        }
        doNotOptimize(&count);
    }
    end = std::chrono::high_resolution_clock::now();
    double time_comp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / total_ops;

    // --- 3. Swap ---
    // Modeled as: swap(arr[i], arr[j])
    start = std::chrono::high_resolution_clock::now();
    for(size_t k=0; k<ITERATIONS; ++k) {
        for(size_t i=0; i < N/2; ++i) {
            using std::swap;
            swap(arr[i], arr[N-1-i]);
        }
        doNotOptimize(arr.data());
    }
    end = std::chrono::high_resolution_clock::now();
    double time_swap_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / (total_ops / 2.0); // Correct ops count

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "  Comparison: " << time_comp_ns << " ns\n";
    std::cout << "  Assignment: " << time_assign_ns << " ns\n";
    std::cout << "  Swap:       " << time_swap_ns << " ns\n";

    return {time_comp_ns, time_assign_ns, time_swap_ns};
}

int main() {
    // 1. L2 Cache Size (~256 KB)
    // Most recursive steps of QuickSort happen here
    run_benchmark(65536, "L2 Cache Resident");

    // 2. Main Memory Size (~64 MB)
    // Initial partitioning happens here
    // 64MB = 16 * 1024 * 1024 ints
    run_benchmark(16 * 1024 * 1024, "Main Memory (RAM)");

    return 0;
}
