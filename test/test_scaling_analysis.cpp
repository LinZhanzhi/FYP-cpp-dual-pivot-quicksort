#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include "dual_pivot_quicksort.hpp"
#include "dpqs/parallel/threadpool.hpp"

using namespace dual_pivot;

void run_test(int num_threads, size_t size) {
    // Re-initialize thread pool with specific thread count
    auto& pool = getThreadPool(num_threads);
    pool.reset_stats();

    // Generate data
    std::vector<int> data(size);
    std::mt19937 rng(42); // Fixed seed for reproducibility
    std::uniform_int_distribution<int> dist(0, 1000000);
    for (auto& x : data) x = dist(rng);

    std::cout << "Running with " << std::setw(2) << num_threads << " threads on " << size << " elements..." << std::flush;

    auto start = std::chrono::high_resolution_clock::now();
    dual_pivot::sort(data, num_threads);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;

    // Verify
    bool sorted = std::is_sorted(data.begin(), data.end());
    if (!sorted) {
        std::cerr << " ERROR: Not sorted!" << std::endl;
        return;
    }

    std::cout << " Done in " << std::fixed << std::setprecision(4) << elapsed.count() << "s" << std::endl;

    long pushed = pool.get_tasks_pushed();
    long executed = pool.get_tasks_executed();
    long local = pool.get_local_pops();
    long attempts = pool.get_steal_attempts();
    long successes = pool.get_steal_successes();

    std::cout << "  Stats:" << std::endl;
    std::cout << "    Tasks Pushed:    " << pushed << std::endl;
    std::cout << "    Tasks Executed:  " << executed << std::endl;
    std::cout << "    Local Pops:      " << local << " (" << (executed > 0 ? 100.0 * local / executed : 0) << "%)" << std::endl;
    std::cout << "    Steal Attempts:  " << attempts << std::endl;
    std::cout << "    Steal Successes: " << successes << " (" << (attempts > 0 ? 100.0 * successes / attempts : 0) << "%)" << std::endl;
    std::cout << "    Steal/Exec Ratio:" << (executed > 0 ? 100.0 * successes / executed : 0) << "%" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
}

int main() {
    const size_t SIZE = 100000000; // 100 Million
    std::cout << "Scaling Analysis Test (Size: " << SIZE << ")" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;

    std::vector<int> threads = {1, 2, 4, 8, 12, 16};

    for (int t : threads) {
        run_test(t, SIZE);
    }

    return 0;
}
