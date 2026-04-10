/**
 * False Sharing Benchmark
 * 
 * This microbenchmark isolates the effect of cache-line padding on
 * concurrent queue operations, removing the sorting data access noise.
 * 
 * Compile with:
 *   WITH padding:    g++ -std=c++17 -O2 -pthread -DWITH_PADDING false_sharing_benchmark.cpp -o fs_padded.exe
 *   WITHOUT padding: g++ -std=c++17 -O2 -pthread false_sharing_benchmark.cpp -o fs_packed.exe
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>
#include <functional>
#include <deque>

// Number of operations per thread
constexpr size_t OPS_PER_THREAD = 1'000'000;

#ifdef WITH_PADDING
// Version A: Cache-line padded (no false sharing)
struct alignas(64) PaddedQueue {
    std::deque<int> q;
    std::mutex mtx;
    std::atomic<int> counter{0};
    
    void increment() {
        std::lock_guard<std::mutex> lock(mtx);
        counter.fetch_add(1, std::memory_order_relaxed);
        q.push_back(counter.load());
        if (q.size() > 10) q.pop_front();
    }
};
#else
// Version B: Packed layout (false sharing possible)
struct PackedQueue {
    std::deque<int> q;
    std::mutex mtx;
    std::atomic<int> counter{0};
    
    void increment() {
        std::lock_guard<std::mutex> lock(mtx);
        counter.fetch_add(1, std::memory_order_relaxed);
        q.push_back(counter.load());
        if (q.size() > 10) q.pop_front();
    }
};
#endif

#ifdef WITH_PADDING
using Queue = PaddedQueue;
#else
using Queue = PackedQueue;
#endif

void print_queue_layout(const std::vector<std::unique_ptr<Queue>>& queues) {
    std::cout << "\n=== Memory Layout Analysis ===\n";
    std::cout << "sizeof(Queue) = " << sizeof(Queue) << " bytes\n";
    
    if (queues.size() >= 2) {
        uintptr_t addr0 = reinterpret_cast<uintptr_t>(queues[0].get());
        uintptr_t addr1 = reinterpret_cast<uintptr_t>(queues[1].get());
        std::cout << "Queue[0] address: 0x" << std::hex << addr0 << std::dec << "\n";
        std::cout << "Queue[1] address: 0x" << std::hex << addr1 << std::dec << "\n";
        std::cout << "Distance: " << (addr1 - addr0) << " bytes\n";
        
        bool same_line = ((addr0 / 64) == (addr1 / 64));
        std::cout << "Same cache line: " << (same_line ? "YES (potential false sharing!)" : "NO (isolated)") << "\n";
    }
    std::cout << "\n";
}

double run_benchmark(int num_threads) {
    std::vector<std::unique_ptr<Queue>> queues;
    queues.reserve(num_threads);
    for (int i = 0; i < num_threads; ++i) {
        queues.push_back(std::make_unique<Queue>());
    }
    
    // Print layout for analysis
    if (num_threads >= 2) {
        print_queue_layout(queues);
    }
    
    std::atomic<bool> start{false};
    std::vector<std::thread> threads;
    
    // Each thread hammers its own queue
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }
            
            for (size_t i = 0; i < OPS_PER_THREAD; ++i) {
                queues[t]->increment();
            }
        });
    }
    
    // Start all threads simultaneously
    auto t1 = std::chrono::high_resolution_clock::now();
    start.store(true, std::memory_order_release);
    
    for (auto& th : threads) {
        th.join();
    }
    
    auto t2 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
    
    return ms;
}

int main() {
#ifdef WITH_PADDING
    std::cout << "=== FALSE SHARING BENCHMARK (WITH PADDING - alignas(64)) ===\n";
#else
    std::cout << "=== FALSE SHARING BENCHMARK (WITHOUT PADDING - packed) ===\n";
#endif
    
    std::cout << "Operations per thread: " << OPS_PER_THREAD << "\n";
    std::cout << "Cache line size: 64 bytes\n";
    
    std::vector<int> thread_counts = {2, 4, 8, 16};
    
    for (int num_threads : thread_counts) {
        std::cout << "\n--- " << num_threads << " threads ---\n";
        
        // Warmup
        run_benchmark(num_threads);
        
        // Actual runs
        constexpr int RUNS = 5;
        double total_ms = 0;
        double min_ms = 1e9, max_ms = 0;
        
        for (int r = 0; r < RUNS; ++r) {
            double ms = run_benchmark(num_threads);
            total_ms += ms;
            min_ms = std::min(min_ms, ms);
            max_ms = std::max(max_ms, ms);
        }
        
        double avg_ms = total_ms / RUNS;
        double ops_per_sec = (num_threads * OPS_PER_THREAD) / (avg_ms / 1000.0);
        
        std::cout << "Average time: " << avg_ms << " ms\n";
        std::cout << "Min/Max: " << min_ms << " / " << max_ms << " ms\n";
        std::cout << "Throughput: " << (ops_per_sec / 1e6) << " M ops/sec\n";
    }
    
    return 0;
}
