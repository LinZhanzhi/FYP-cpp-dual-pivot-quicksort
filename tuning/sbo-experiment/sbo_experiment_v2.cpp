/**
 * SBO Experiment v2 - With Realistic Thread Pool Overhead
 *
 * This version simulates the actual overhead in a work-stealing thread pool:
 * 1. Lock contention when pushing/popping tasks
 * 2. Atomic operations for coordination
 * 3. Memory barriers
 *
 * The goal is to understand why our ring buffer showed REGRESSION despite
 * eliminating std::function allocation.
 */

#include <functional>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <deque>
#include <mutex>
#include <atomic>
#include <cstring>
#include <thread>

static std::atomic<int64_t> global_sink{0};

//=============================================================================
// Approach 1: std::function with std::deque (original implementation)
//=============================================================================
class StdFunctionQueue {
    std::deque<std::function<void()>> queue_;
    mutable std::mutex mutex_;

public:
    template<typename F>
    void push(F&& func) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.emplace_back(std::forward<F>(func));
    }

    bool pop(std::function<void()>& task) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) return false;
        task = std::move(queue_.front());
        queue_.pop_front();
        return true;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.clear();
    }
};

//=============================================================================
// Approach 2: Pre-allocated Ring Buffer with POD tasks
//=============================================================================
struct alignas(64) SortTask {
    using ExecuteFunc = void(*)(void*);
    ExecuteFunc execute;
    void* array_ptr;
    int64_t low;
    int64_t high;
    uint32_t bits;
    alignas(8) std::byte comparator_storage[24];

    void run() {
        if (execute) execute(this);
    }
};

class RingBufferQueue {
    static constexpr size_t CAPACITY = 8192;
    std::vector<SortTask> buffer_;
    size_t head_ = 0;
    size_t tail_ = 0;
    mutable std::mutex mutex_;

public:
    RingBufferQueue() : buffer_(CAPACITY) {}

    bool push(const SortTask& task) {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t next_tail = (tail_ + 1) % CAPACITY;
        if (next_tail == head_) return false;  // Full
        buffer_[tail_] = task;  // 64-byte copy
        tail_ = next_tail;
        return true;
    }

    bool pop(SortTask& task) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (head_ == tail_) return false;  // Empty
        task = buffer_[head_];  // 64-byte copy
        head_ = (head_ + 1) % CAPACITY;
        return true;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        head_ = tail_ = 0;
    }
};

//=============================================================================
// Simulate sorting task captures
//=============================================================================

// Small capture (fits in SBO) - like a simple lambda
struct SmallCapture {
    int64_t value;
    void execute() { global_sink.fetch_add(value, std::memory_order_relaxed); }
};

// Medium capture (at SBO boundary) - like our actual sort lambda
struct MediumCapture {
    void* array_ptr;    // 8 bytes
    int64_t low;        // 8 bytes
    int64_t high;       // 8 bytes
    void* comparator;   // 8 bytes = 32 bytes total

    void execute() {
        global_sink.fetch_add(low + high, std::memory_order_relaxed);
    }
};

// Large capture (definitely exceeds SBO)
struct LargeCapture {
    char data[64];
    int64_t value;
    void execute() { global_sink.fetch_add(value, std::memory_order_relaxed); }
};

//=============================================================================
// Benchmark Functions
//=============================================================================

template<typename Capture>
void bench_std_function(size_t iterations, const char* label) {
    StdFunctionQueue queue;

    auto start = std::chrono::high_resolution_clock::now();

    // Push phase
    for (size_t i = 0; i < iterations; ++i) {
        Capture cap{};
        if constexpr (std::is_same_v<Capture, SmallCapture>) {
            cap.value = static_cast<int64_t>(i);
        } else if constexpr (std::is_same_v<Capture, MediumCapture>) {
            cap.low = static_cast<int64_t>(i);
            cap.high = static_cast<int64_t>(i + 1);
        } else {
            cap.value = static_cast<int64_t>(i);
        }
        queue.push([cap]() mutable { cap.execute(); });
    }

    auto push_end = std::chrono::high_resolution_clock::now();

    // Pop & execute phase
    std::function<void()> task;
    while (queue.pop(task)) {
        task();
    }

    auto end = std::chrono::high_resolution_clock::now();

    double push_ns = std::chrono::duration<double, std::nano>(push_end - start).count();
    double total_ns = std::chrono::duration<double, std::nano>(end - start).count();

    std::cout << "  std::function " << std::setw(8) << label << ": "
              << "push=" << std::setw(8) << std::fixed << std::setprecision(2)
              << (push_ns / iterations) << " ns, "
              << "total=" << std::setw(8) << (total_ns / iterations) << " ns/task\n";
}

template<typename Capture>
void bench_ring_buffer(size_t iterations, const char* label) {
    RingBufferQueue queue;

    auto start = std::chrono::high_resolution_clock::now();

    // Push phase
    for (size_t i = 0; i < iterations; ++i) {
        SortTask task{};
        task.execute = [](void* self) {
            // Simulate work
            global_sink.fetch_add(1, std::memory_order_relaxed);
        };
        task.low = static_cast<int64_t>(i);
        task.high = static_cast<int64_t>(i + 1);
        queue.push(task);
    }

    auto push_end = std::chrono::high_resolution_clock::now();

    // Pop & execute phase
    SortTask task;
    while (queue.pop(task)) {
        task.run();
    }

    auto end = std::chrono::high_resolution_clock::now();

    double push_ns = std::chrono::duration<double, std::nano>(push_end - start).count();
    double total_ns = std::chrono::duration<double, std::nano>(end - start).count();

    std::cout << "  RingBuffer    " << std::setw(8) << label << ": "
              << "push=" << std::setw(8) << std::fixed << std::setprecision(2)
              << (push_ns / iterations) << " ns, "
              << "total=" << std::setw(8) << (total_ns / iterations) << " ns/task\n";
}

//=============================================================================
// Multi-threaded contention test (realistic scenario)
//=============================================================================

void bench_contention_std_function(size_t total_tasks, size_t num_producers) {
    StdFunctionQueue queue;
    std::atomic<size_t> tasks_pushed{0};
    std::atomic<size_t> tasks_executed{0};
    std::atomic<bool> done{false};

    auto start = std::chrono::high_resolution_clock::now();

    // Start consumer
    std::thread consumer([&]() {
        std::function<void()> task;
        while (!done.load(std::memory_order_relaxed) || tasks_executed < total_tasks) {
            if (queue.pop(task)) {
                task();
                tasks_executed.fetch_add(1, std::memory_order_relaxed);
            }
        }
    });

    // Producers
    std::vector<std::thread> producers;
    size_t per_producer = total_tasks / num_producers;

    for (size_t p = 0; p < num_producers; ++p) {
        producers.emplace_back([&, p]() {
            for (size_t i = 0; i < per_producer; ++i) {
                MediumCapture cap{};
                cap.low = static_cast<int64_t>(p * per_producer + i);
                queue.push([cap]() mutable { cap.execute(); });
                tasks_pushed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : producers) t.join();
    done = true;
    consumer.join();

    auto end = std::chrono::high_resolution_clock::now();
    double total_ns = std::chrono::duration<double, std::nano>(end - start).count();

    std::cout << "  std::function (" << num_producers << " producers): "
              << std::setw(8) << std::fixed << std::setprecision(2)
              << (total_ns / total_tasks) << " ns/task\n";
}

void bench_contention_ring_buffer(size_t total_tasks, size_t num_producers) {
    RingBufferQueue queue;
    std::atomic<size_t> tasks_pushed{0};
    std::atomic<size_t> tasks_executed{0};
    std::atomic<bool> done{false};

    auto start = std::chrono::high_resolution_clock::now();

    // Start consumer
    std::thread consumer([&]() {
        SortTask task;
        while (!done.load(std::memory_order_relaxed) || tasks_executed < total_tasks) {
            if (queue.pop(task)) {
                task.run();
                tasks_executed.fetch_add(1, std::memory_order_relaxed);
            }
        }
    });

    // Producers
    std::vector<std::thread> producers;
    size_t per_producer = total_tasks / num_producers;

    for (size_t p = 0; p < num_producers; ++p) {
        producers.emplace_back([&, p]() {
            for (size_t i = 0; i < per_producer; ++i) {
                SortTask task{};
                task.execute = [](void* self) {
                    global_sink.fetch_add(1, std::memory_order_relaxed);
                };
                task.low = static_cast<int64_t>(p * per_producer + i);
                queue.push(task);
                tasks_pushed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : producers) t.join();
    done = true;
    consumer.join();

    auto end = std::chrono::high_resolution_clock::now();
    double total_ns = std::chrono::duration<double, std::nano>(end - start).count();

    std::cout << "  RingBuffer    (" << num_producers << " producers): "
              << std::setw(8) << std::fixed << std::setprecision(2)
              << (total_ns / total_tasks) << " ns/task\n";
}

int main() {
    std::cout << "=== SBO Experiment v2: Realistic Thread Pool Simulation ===\n\n";

    std::cout << "Platform Info:\n";
    std::cout << "  sizeof(std::function<void()>): " << sizeof(std::function<void()>) << " bytes\n";
    std::cout << "  sizeof(SortTask): " << sizeof(SortTask) << " bytes\n";
    std::cout << "  sizeof(SmallCapture): " << sizeof(SmallCapture) << " bytes\n";
    std::cout << "  sizeof(MediumCapture): " << sizeof(MediumCapture) << " bytes\n";
    std::cout << "  sizeof(LargeCapture): " << sizeof(LargeCapture) << " bytes\n";
    std::cout << "\n";

    constexpr size_t ITERATIONS = 100000;

    std::cout << "--- Single-threaded (no contention) ---\n";
    bench_std_function<SmallCapture>(ITERATIONS, "small");
    bench_ring_buffer<SmallCapture>(ITERATIONS, "small");
    std::cout << "\n";

    bench_std_function<MediumCapture>(ITERATIONS, "medium");
    bench_ring_buffer<MediumCapture>(ITERATIONS, "medium");
    std::cout << "\n";

    bench_std_function<LargeCapture>(ITERATIONS, "large");
    bench_ring_buffer<LargeCapture>(ITERATIONS, "large");

    std::cout << "\n--- Multi-threaded (with contention) ---\n";

    std::cout << "\n2 Producers:\n";
    bench_contention_std_function(ITERATIONS, 2);
    bench_contention_ring_buffer(ITERATIONS, 2);

    std::cout << "\n4 Producers:\n";
    bench_contention_std_function(ITERATIONS, 4);
    bench_contention_ring_buffer(ITERATIONS, 4);

    std::cout << "\n8 Producers:\n";
    bench_contention_std_function(ITERATIONS, 8);
    bench_contention_ring_buffer(ITERATIONS, 8);

    std::cout << "\n=== Key Finding ===\n";
    std::cout << R"(
The ring buffer copies 64 bytes per task vs ~32 bytes for std::function SBO.
Under contention, the larger memory copy while holding the mutex
increases lock hold time, which can hurt parallel performance.

This explains why our "optimization" caused a 3-7% REGRESSION:
1. std::function SBO already avoided heap allocation for our ~28 byte captures
2. The 64-byte SortTask requires larger copies
3. Longer critical sections = worse scalability
)";

    return 0;
}
