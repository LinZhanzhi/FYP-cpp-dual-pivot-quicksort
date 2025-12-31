#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include "dual_pivot_quicksort.hpp"
#include "dpqs/parallel/threadpool.hpp"

using namespace dual_pivot;

int main() {
    const size_t SIZE = 100000000; // 100 Million
    const int THREADS = 2;

    std::cout << "Crash Repro Test (Size: " << SIZE << ", Threads: " << THREADS << ")" << std::endl;

    // Re-initialize thread pool
    getThreadPool(THREADS);

    // Generate data
    std::vector<int> data(SIZE);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 1000000);
    for (auto& x : data) x = dist(rng);

    std::cout << "Sorting..." << std::endl;
    dual_pivot::sort(data, THREADS);
    std::cout << "Done." << std::endl;

    return 0;
}
