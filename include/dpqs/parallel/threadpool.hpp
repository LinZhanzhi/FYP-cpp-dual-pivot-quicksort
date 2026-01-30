#ifndef DPQS_PARALLEL_THREADPOOL_HPP
#define DPQS_PARALLEL_THREADPOOL_HPP

#include <vector>
#include <deque>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <random>
#include <chrono>

#include <cstring>
#include <new>

namespace dual_pivot {

// Thread-local index to identify worker threads
// -1 indicates an external thread (e.g., main thread)
inline thread_local int thread_index = -1;

/**
 * @brief High-Performance Task Struct
 * Designed to fit in a cache line (64 bytes) and avoid dynamic allocation.
 * Replaces std::function for sorting tasks.
 */
struct SortTask {
    using ExecuteFunc = void (*)(SortTask&);

    ExecuteFunc executor;           // 8 bytes
    void* array_ptr;                // 8 bytes (pointer to T)
    std::ptrdiff_t low;             // 8 bytes
    std::ptrdiff_t high;            // 8 bytes
    int bits;                       // 4 bytes
    // 36 bytes used.

    // Inline storage for the Comparator object.
    // Most comparators are 1 byte (empty struct). Stateful ones fit here.
    // If a comparator is larger than 24 bytes, we can't support it in parallel mode without alloc.
    static constexpr size_t STORAGE_SIZE = 24;
    alignas(8) std::byte comparator_storage[STORAGE_SIZE];

    // Helper to constructing
    template <typename Compare>
    void set_comparator(const Compare& comp) {
        static_assert(sizeof(Compare) <= STORAGE_SIZE, "Comparator too large for manual task storage");
        new (comparator_storage) Compare(comp);
    }

    template <typename Compare>
    Compare& get_comparator() {
        return *reinterpret_cast<Compare*>(comparator_storage);
    }
};

/**
 * @brief Work Stealing Thread Pool (V4 - Ring Buffer & Explicit Memory)
 *
 * Implements a distributed queue architecture using fixed-size Ring Buffers.
 * - Eliminates all dynamic allocation during the sort.
 * - Uses a lightweight Task struct instead of std::function.
 * - Provides fallback to synchronous execution when queues are full.
 */
class ThreadPool {
private:
    // Fixed capacity per thread.
    // 8192 is large enough for deep recursion but small enough to fit in L3 cache partitions.
    static constexpr size_t RING_CAPACITY = 8192;

    struct WorkStealingQueue {
        std::vector<SortTask> buffer;
        std::mutex mtx; // Protects this queue

        // Circular Buffer Indices
        // Owner pushes/pops at 'bottom'
        // Thieves steal from 'top'
        size_t top = 0;
        size_t bottom = 0;
        size_t count = 0;

        WorkStealingQueue() : buffer(RING_CAPACITY) {}

        // Push to bottom (Owner only)
        bool push(const SortTask& task) {
            std::lock_guard<std::mutex> lock(mtx);
            if (count >= RING_CAPACITY) return false;

            buffer[bottom] = task; // Copy task data
            bottom = (bottom + 1) % RING_CAPACITY;
            count++;
            return true;
        }

        // Pop from bottom (Owner only - LIFO)
        bool try_pop(SortTask& task) {
            std::lock_guard<std::mutex> lock(mtx);
            if (count == 0) return false;

            // Decrement bottom cyclically
            if (bottom == 0) bottom = RING_CAPACITY - 1;
            else bottom--;

            task = buffer[bottom];
            count--;
            return true;
        }

        // Steal from top (Thieves - FIFO)
        bool try_steal(SortTask& task) {
            std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
            if (!lock || count == 0) return false;

            task = buffer[top];
            top = (top + 1) % RING_CAPACITY;
            count--;
            return true;
        }
    };

    std::vector<std::unique_ptr<WorkStealingQueue>> queues;
    std::vector<std::thread> workers;
    std::atomic<bool> stop{false};
    std::atomic<long> incomplete_tasks{0};

    // For wait_for_completion
    std::mutex wait_mutex;
    std::condition_variable wait_cv;

    // Profiling
    std::atomic<long> tasks_pushed{0};
    std::atomic<long> tasks_executed{0};
    std::atomic<long> steal_attempts{0};
    std::atomic<long> steal_successes{0};
    std::atomic<long> local_pops{0};

public:
    void reset_stats() {
        tasks_pushed = 0;
        tasks_executed = 0;
        steal_attempts = 0;
        steal_successes = 0;
        local_pops = 0;
    }

    long get_tasks_pushed() const { return tasks_pushed; }
    long get_tasks_executed() const { return tasks_executed; }
    long get_steal_attempts() const { return steal_attempts; }
    long get_steal_successes() const { return steal_successes; }
    long get_local_pops() const { return local_pops; }
    size_t get_thread_count() const { return workers.size(); }

    // Heuristic for Adaptive Granularity
    long get_active_task_count() const {
        return incomplete_tasks.load(std::memory_order_relaxed);
    }

    ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) {
        queues.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            queues.push_back(std::make_unique<WorkStealingQueue>());
        }

        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this, i, num_threads] {
                thread_index = static_cast<int>(i);

                // Memory-Awareness: Sticky Victim Strategy
                // We remember the last successful victim.
                // Rationale: If a thread has one task, it likely has more from the same subtree (data locality).
                size_t last_victim = (i + 1) % num_threads;

                while (!stop) {
                    SortTask task;
                    bool found = false;

                    // 1. Try Local Pop (LIFO)
                    if (queues[i]->try_pop(task)) {
                        found = true;
                        local_pops++;
                    }
                    // 2. Try Steal (FIFO)
                    else {
                        steal_attempts++;

                        // Scan for victims starting from the LAST successful victim (Sticky)
                        for (size_t k = 0; k < num_threads; ++k) {
                            // Calculate via offset from last_victim to maintain cycle
                            size_t victim = (last_victim + k) % num_threads;

                            if (victim == i) continue; // Don't steal from self

                            if (queues[victim]->try_steal(task)) {
                                found = true;
                                steal_successes++;
                                last_victim = victim; // STICK to this victim for next time
                                break;
                            }
                        }
                    }

                    if (found) {
                        try {
                            task.executor(task); // Execute
                        } catch (...) {
                           // Swallow
                        }

                        long remaining = --incomplete_tasks;
                        tasks_executed++;
                        if (remaining == 0) {
                            wait_cv.notify_all();
                        }
                    } else {
                        std::this_thread::yield();
                    }
                }
            });
        }
    }

    ~ThreadPool() {
        stop = true;
        for (auto& worker : workers) {
            if (worker.joinable()) worker.join();
        }
    }

    // Non-template enqueue that takes a constructed SortTask
    bool enqueue_task(const SortTask& task) {
        incomplete_tasks++;
        tasks_pushed++;

        int idx = thread_index;
        if (idx != -1) {
            if (queues[idx]->push(task)) {
                return true;
            }
        } else {
            // External injection (Round-Robin attempt)
            size_t n = queues.size();
            size_t start = tasks_pushed.load() % n;
            for (size_t i = 0; i < n; ++i) {
                if (queues[(start + i) % n]->push(task)) {
                    return true;
                }
            }
        }

        incomplete_tasks--;
        return false;
    }

    void wait_for_completion() {
        while (true) {
            if (incomplete_tasks == 0) {
                return;
            }

            // DISABLE HELPING for Main Thread to avoid "Queue 0 Bottleneck"
            // If Main Thread helps, it executes tasks with thread_index = -1,
            // pushing sub-tasks to Queue 0, causing centralization.

            std::unique_lock<std::mutex> lock(wait_mutex);
            wait_cv.wait_for(lock, std::chrono::microseconds(100));
        }
    }
};

// Singleton accessor with re-initialization support
inline std::unique_ptr<ThreadPool>& getThreadPoolInstance() {
    static std::unique_ptr<ThreadPool> pool;
    return pool;
}

inline ThreadPool& getThreadPool(int num_threads = 0) {
    auto& pool = getThreadPoolInstance();
    if (!pool) {
        pool = std::make_unique<ThreadPool>(num_threads > 0 ? num_threads : std::thread::hardware_concurrency());
    } else if (num_threads > 0 && pool->get_thread_count() != static_cast<size_t>(num_threads)) {
        pool = std::make_unique<ThreadPool>(num_threads);
    }
    return *pool;
}

} // namespace dual_pivot

#endif // DPQS_PARALLEL_THREADPOOL_HPP
