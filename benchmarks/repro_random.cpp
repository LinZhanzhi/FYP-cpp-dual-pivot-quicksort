#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include "dual_pivot_quicksort.hpp"
#include "data_generator.hpp"

using namespace dual_pivot;

int main(int argc, char* argv[]) {
    size_t size = 79433;
    if (argc > 1) size = std::stoull(argv[1]);

    std::cout << "Generating RANDOM data with size " << size << "..." << std::endl;
    auto data = benchmark_data::generate_data<int>(size, benchmark_data::DataPattern::RANDOM, 42);

    std::cout << "Sorting with parallel dual-pivot quicksort..." << std::endl;
    // Use 24 threads
    for (int i = 0; i < 1000; ++i) {
        auto copy = data;
        dual_pivot::sort(copy.data(), 24, 0, size);
        if (!std::is_sorted(copy.begin(), copy.end())) {
            std::cout << "FAIL at iteration " << i << std::endl;
            return 1;
        }
        if (i % 100 == 0) std::cout << "." << std::flush;
    }
    std::cout << std::endl;

    std::cout << "Checking correctness..." << std::endl;
    auto copy = data;
    dual_pivot::sort(copy.data(), 24, 0, size);
    bool sorted = std::is_sorted(copy.begin(), copy.end());

    if (sorted) {
        std::cout << "PASS" << std::endl;
    } else {
        std::cout << "FAIL" << std::endl;
        for (size_t i = 0; i < size - 1; ++i) {
            if (data[i] > data[i+1]) {
                std::cout << "Failure at index " << i << ": " << data[i] << " > " << data[i+1] << std::endl;
                break;
            }
        }
        return 1;
    }

    return 0;
}
