#ifndef DPQS_PARALLEL_THREADPOOL_HPP
#define DPQS_PARALLEL_THREADPOOL_HPP

#include <vector>
#include <deque>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <random>
#include <chrono>

namespace dual_pivot {

// Thread-local index to identify worker threads
// -1 indicates an external thread (e.g., main thread)
inline thread_local int thread_index = -1;

/**
 * @brief Work Stealing Thread Pool (V3)
 *
 * Implements a distributed queue architecture where each thread has its own
 * double-ended queue (deque).
 * - Owner pushes/pops from bottom (LIFO) for cache locality.
 * - Thieves steal from top (FIFO) to take largest tasks.
 * - Eliminates global mutex contention.
 */
class ThreadPool {
private:
    struct WorkStealingQueue {
        std::deque<std::function<void()>> q;
        std::mutex mtx; // Protects ONLY this specific queue

        // Push a task to the bottom (Owner only)
        void push(std::function<void()> task) {
            std::lock_guard<std::mutex> lock(mtx);
            q.push_back(std::move(task));
        }

        // Pop from bottom (Owner only)
        bool try_pop(std::function<void()>& task) {
            std::lock_guard<std::mutex> lock(mtx);
            if (q.empty()) return false;
            task = std::move(q.back()); // LIFO
            q.pop_back();
            return true;
        }

        // Steal from top (Thieves only)
        bool try_steal(std::function<void()>& task) {
            // CRITICAL: Use try_lock to avoid blocking on contention
            std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
            if (!lock || q.empty()) return false;
            task = std::move(q.front()); // FIFO
            q.pop_front();
            return true;
        }

        bool empty() {
            std::lock_guard<std::mutex> lock(mtx);
            return q.empty();
        }
    };

    std::vector<std::unique_ptr<WorkStealingQueue>> queues;
    std::vector<std::thread> workers;
    std::atomic<bool> stop{false};
    std::atomic<int> active_tasks{0};

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

    ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) {
        queues.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            queues.push_back(std::make_unique<WorkStealingQueue>());
        }

        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this, i, num_threads] {
                thread_index = static_cast<int>(i);

                while (!stop) {
                    std::function<void()> task;
                    bool found = false;

                    // 1. Try Local Pop (LIFO)
                    if (queues[i]->try_pop(task)) {
                        found = true;
                        local_pops++;
                    }
                    // 2. Try Steal (FIFO)
                    else {
                        steal_attempts++;
                        // Random victim selection strategy
                        for (size_t offset = 1; offset < num_threads; ++offset) {
                            size_t victim = (i + offset) % num_threads;
                            if (queues[victim]->try_steal(task)) {
                                found = true;
                                steal_successes++;
                                break;
                            }
                        }
                    }

                    if (found) {
                        active_tasks++;
                        try {
                            task();
                        } catch (...) {
                            // Ensure active_tasks is decremented even if task throws
                            active_tasks--;
                            if (active_tasks == 0) {
                                wait_cv.notify_all();
                            }
                            throw; // Re-throw or handle? Ideally log and continue, but for now let's just ensure cleanup
                        }
                        active_tasks--;
                        tasks_executed++;

                        if (active_tasks == 0) {
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

    template<typename F>
    void submit(F&& f) {
        int idx = thread_index;
        if (idx == -1) idx = 0;
        if (idx >= static_cast<int>(queues.size())) idx = 0;

        queues[idx]->push(std::forward<F>(f));
        tasks_pushed++;
    }

    void wait_for_completion() {
        while (true) {
            if (active_tasks == 0) {
                bool all_empty = true;
                for (const auto& q : queues) {
                    if (!q->empty()) {
                        all_empty = false;
                        break;
                    }
                }
                // Double-check active_tasks to avoid race condition where a task was popped
                // while we were checking queues.
                if (all_empty && active_tasks == 0) return;
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
