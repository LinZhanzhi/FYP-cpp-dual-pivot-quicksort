/**
 * False Sharing Benchmark V2 - Contiguous Array Allocation
 *
 * This version uses a contiguous array (not heap-allocated pointers)
 * to force queues onto adjacent/same cache lines.
 *
 * Compile with:
 *   WITH padding:    g++ -std=c++17 -O2 -pthread -DWITH_PADDING false_sharing_v2.cpp -o fs2_padded.exe
 *   WITHOUT padding: g++ -std=c++17 -O2 -pthread false_sharing_v2.cpp -o fs2_packed.exe
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <chrono>
#include <cstdint>

constexpr size_t OPS_PER_THREAD = 5'000'000;

//=============================================================================
// Test 1: Simple atomic counter (clearest demonstration)
//=============================================================================

#ifdef WITH_PADDING
struct alignas(64) PaddedCounter {
    std::atomic<int64_t> value{0};
};
#else
struct PackedCounter {
    std::atomic<int64_t> value{0};  // 8 bytes - multiple fit in one cache line
};
#endif

#ifdef WITH_PADDING
using Counter = PaddedCounter;
#else
using Counter = PackedCounter;
#endif

void print_counter_layout(Counter* counters, int num) {
    std::cout << "\n=== Counter Memory Layout ===\n";
    std::cout << "sizeof(Counter) = " << sizeof(Counter) << " bytes\n";

    for (int i = 0; i < std::min(num, 4); ++i) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(&counters[i]);
        std::cout << "Counter[" << i << "] @ 0x" << std::hex << addr
                  << " (cache line " << std::dec << (addr / 64) % 1024 << ")\n";
    }

    if (num >= 2) {
        uintptr_t addr0 = reinterpret_cast<uintptr_t>(&counters[0]);
        uintptr_t addr1 = reinterpret_cast<uintptr_t>(&counters[1]);
        bool same_line = ((addr0 / 64) == (addr1 / 64));
        std::cout << "Counter[0] and Counter[1] on same cache line: "
                  << (same_line ? "YES (FALSE SHARING!)" : "NO (isolated)") << "\n";
    }
}

double run_counter_benchmark(int num_threads) {
    // Contiguous array allocation - THIS is where false sharing occurs
    std::vector<Counter> counters(num_threads);

    std::atomic<bool> start{false};
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            // Each thread repeatedly increments ONLY its own counter
            for (size_t i = 0; i < OPS_PER_THREAD; ++i) {
                counters[t].value.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    start.store(true, std::memory_order_release);

    for (auto& th : threads) {
        th.join();
    }

    auto t2 = std::chrono::high_resolution_clock::now();

    // Verify correctness
    for (int t = 0; t < num_threads; ++t) {
        if (counters[t].value.load() != OPS_PER_THREAD) {
            std::cerr << "ERROR: Counter[" << t << "] = " << counters[t].value.load() << "\n";
        }
    }

    return std::chrono::duration<double, std::milli>(t2 - t1).count();
}

//=============================================================================
// Test 2: Mutex + counter (more realistic)
//=============================================================================

#ifdef WITH_PADDING
struct alignas(64) PaddedMutexCounter {
    std::mutex mtx;
    int64_t value{0};

    void increment() {
        std::lock_guard<std::mutex> lock(mtx);
        ++value;
    }
};
#else
struct PackedMutexCounter {
    std::mutex mtx;  // ~40 bytes on Windows
    int64_t value{0};

    void increment() {
        std::lock_guard<std::mutex> lock(mtx);
        ++value;
    }
};
#endif

#ifdef WITH_PADDING
using MutexCounter = PaddedMutexCounter;
#else
using MutexCounter = PackedMutexCounter;
#endif

void print_mutex_layout(MutexCounter* counters, int num) {
    std::cout << "\n=== MutexCounter Memory Layout ===\n";
    std::cout << "sizeof(MutexCounter) = " << sizeof(MutexCounter) << " bytes\n";
    std::cout << "sizeof(std::mutex) = " << sizeof(std::mutex) << " bytes\n";

    for (int i = 0; i < std::min(num, 4); ++i) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(&counters[i]);
        std::cout << "MutexCounter[" << i << "] @ 0x" << std::hex << addr
                  << " (cache line " << std::dec << (addr / 64) % 1024 << ")\n";
    }

    if (num >= 2) {
        uintptr_t addr0 = reinterpret_cast<uintptr_t>(&counters[0]);
        uintptr_t addr1 = reinterpret_cast<uintptr_t>(&counters[1]);
        bool same_line = ((addr0 / 64) == (addr1 / 64));
        std::cout << "MutexCounter[0] and MutexCounter[1] on same cache line: "
                  << (same_line ? "YES (FALSE SHARING!)" : "NO (isolated)") << "\n";
    }
}

double run_mutex_benchmark(int num_threads) {
    std::vector<MutexCounter> counters(num_threads);

    std::atomic<bool> start{false};
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            for (size_t i = 0; i < OPS_PER_THREAD / 10; ++i) {  // Fewer ops (mutex is slow)
                counters[t].increment();
            }
        });
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    start.store(true, std::memory_order_release);

    for (auto& th : threads) {
        th.join();
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(t2 - t1).count();
}

int main() {
#ifdef WITH_PADDING
    std::cout << "========================================\n";
    std::cout << "  FALSE SHARING TEST - WITH PADDING    \n";
    std::cout << "  (alignas(64) - each counter isolated)\n";
    std::cout << "========================================\n";
#else
    std::cout << "========================================\n";
    std::cout << "  FALSE SHARING TEST - WITHOUT PADDING \n";
    std::cout << "  (packed - counters may share lines)  \n";
    std::cout << "========================================\n";
#endif

    std::cout << "\nCache line size: 64 bytes\n";
    std::cout << "Operations per thread: " << OPS_PER_THREAD << "\n";

    //=========================================================================
    // Test 1: Atomic counters
    //=========================================================================
    std::cout << "\n\n########## TEST 1: Atomic Counters ##########\n";

    std::vector<int> thread_counts = {2, 4, 8, 16};

    // Print layout once
    {
        std::vector<Counter> temp(16);
        print_counter_layout(temp.data(), 16);
    }

    std::cout << "\nResults:\n";
    std::cout << "Threads | Time (ms) | Ops/sec (M)\n";
    std::cout << "--------|-----------|------------\n";

    for (int num_threads : thread_counts) {
        // Warmup
        run_counter_benchmark(num_threads);

        // Actual runs
        constexpr int RUNS = 5;
        double total_ms = 0;

        for (int r = 0; r < RUNS; ++r) {
            total_ms += run_counter_benchmark(num_threads);
        }

        double avg_ms = total_ms / RUNS;
        double ops_per_sec = (num_threads * OPS_PER_THREAD) / (avg_ms / 1000.0);

        printf("   %2d   | %9.2f | %10.2f\n", num_threads, avg_ms, ops_per_sec / 1e6);
    }

    //=========================================================================
    // Test 2: Mutex counters
    //=========================================================================
    std::cout << "\n\n########## TEST 2: Mutex Counters ##########\n";
    std::cout << "(Each thread locks its OWN mutex - no true contention,\n";
    std::cout << " only false sharing if mutexes share cache lines)\n";

    // Print layout once
    {
        std::vector<MutexCounter> temp(16);
        print_mutex_layout(temp.data(), 16);
    }

    std::cout << "\nResults:\n";
    std::cout << "Threads | Time (ms) | Ops/sec (M)\n";
    std::cout << "--------|-----------|------------\n";

    for (int num_threads : thread_counts) {
        // Warmup
        run_mutex_benchmark(num_threads);

        constexpr int RUNS = 5;
        double total_ms = 0;

        for (int r = 0; r < RUNS; ++r) {
            total_ms += run_mutex_benchmark(num_threads);
        }

        double avg_ms = total_ms / RUNS;
        double ops = num_threads * (OPS_PER_THREAD / 10);
        double ops_per_sec = ops / (avg_ms / 1000.0);

        printf("   %2d   | %9.2f | %10.2f\n", num_threads, avg_ms, ops_per_sec / 1e6);
    }

    return 0;
}
