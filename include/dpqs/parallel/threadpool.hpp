#ifndef DPQS_PARALLEL_THREADPOOL_HPP
#define DPQS_PARALLEL_THREADPOOL_HPP

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <atomic>

namespace dual_pivot {

/**
 * @brief Thread pool for parallel sorting operations
 *
 * This thread pool implementation provides work distribution for parallel
 * sorting algorithms. It uses a producer-consumer pattern with condition
 * variables for efficient thread coordination and work stealing.
 *
 * The design follows modern C++ best practices for thread management and
 * provides RAII-style automatic cleanup. Tasks are executed asynchronously
 * with future-based result handling.
 *
 * Key features:
 * - Automatic thread count detection based on hardware capabilities
 * - Efficient task queuing with condition variable synchronization
 * - Future-based result handling for async operations
 * - RAII-style cleanup with proper thread joining
 */
class ThreadPool {
private:
    std::vector<std::thread> workers;           ///< Worker threads
    std::queue<std::function<void()>> tasks;   ///< Task queue
    std::mutex queue_mutex;                     ///< Mutex for thread-safe queue access
    std::condition_variable condition;          ///< Condition variable for worker synchronization
    std::atomic<bool> stop;                     ///< Atomic flag for clean shutdown

    std::atomic<int> active_tasks{0};           ///< Count of currently executing tasks
    std::condition_variable wait_cv;            ///< CV for waiting for all tasks to complete

    // Profiling
    std::atomic<long> tasks_pushed{0};
    std::atomic<long> tasks_executed{0};

public:
    void reset_stats() {
        tasks_pushed = 0;
        tasks_executed = 0;
    }

    void print_stats() {
        // Use printf to avoid iostream includes if possible, or just use std::cout since we likely have it
        // For now, let's assume std::cout is available or just expose getters
    }

    long get_tasks_pushed() const { return tasks_pushed; }
    long get_tasks_executed() const { return tasks_executed; }

    /**
     * @brief Construct a thread pool with specified number of threads
     *
     * Creates a thread pool with the given number of worker threads. Each worker
     * runs in a loop, waiting for tasks to be enqueued and executing them.
     *
     * @param num_threads Number of worker threads (default: hardware concurrency)
     */
    ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) : stop(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty()) return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    this->active_tasks++;
                    task();
                    this->active_tasks--;
                    this->tasks_executed++;

                    if (this->active_tasks == 0) {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        if (this->tasks.empty()) {
                            this->wait_cv.notify_all();
                        }
                    }
                }
            });
        }
    }

    /**
     * @brief Submit a fire-and-forget task
     *
     * Adds a task to the queue without returning a future.
     * Used for the non-blocking parallel sort implementation.
     */
    template<typename F>
    void submit(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
            tasks_pushed++;
        }
        condition.notify_one();
    }

    /**
     * @brief Wait until all tasks are completed
     *
     * Blocks the calling thread until the task queue is empty AND
     * no workers are currently executing a task.
     * Also processes tasks while waiting to help out (Work Stealing-ish).
     */
    void wait_for_completion() {
        // Try to help while waiting
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                if (tasks.empty()) {
                    if (active_tasks == 0) return; // Done
                    // Wait for others to finish
                    wait_cv.wait(lock, [this] { return this->tasks.empty() && this->active_tasks == 0; });
                    return;
                }
                task = std::move(tasks.front());
                tasks.pop();
            }

            // Execute stolen task
            active_tasks++;
            task();
            active_tasks--;
            tasks_executed++;

            if (active_tasks == 0) {
                std::unique_lock<std::mutex> lock(queue_mutex);
                if (tasks.empty()) {
                    wait_cv.notify_all();
                }
            }
        }
    }

    /**
     * @brief Enqueue a task for asynchronous execution
     *
     * Adds a callable task to the thread pool's work queue. The task will be
     * executed by one of the worker threads when available. Returns a future
     * that can be used to retrieve the result.
     *
     * @tparam F Function type
     * @tparam Args Argument types
     * @param f Function to execute
     * @param args Arguments to pass to the function
     * @return std::future for retrieving the result
     * @throws std::runtime_error if the thread pool has been stopped
     */
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace([task](){ (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    /**
     * @brief Get the number of worker threads
     */
    size_t size() const {
        return workers.size();
    }

    /**
     * @brief Destructor - ensures clean shutdown of all threads
     *
     * Signals all worker threads to stop, wakes them up, and waits for
     * them to complete their current tasks before destruction.
     */
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker: workers) {
            if (worker.joinable()) worker.join();
        }
    }
};

/**
 * @brief Singleton thread pool accessor for parallel sorting
 *
 * Provides a global thread pool instance for use in parallel sorting operations.
 * The singleton pattern ensures that only one thread pool exists per process,
 * avoiding resource waste and thread proliferation.
 *
 * Supports resizing if a specific thread count is requested.
 *
 * @param num_threads Optional number of threads to resize the pool to.
 *                    If 0 (default), returns existing pool or creates with hardware concurrency.
 * @return Reference to the global ThreadPool instance
 */
inline ThreadPool& getThreadPool(int num_threads = 0) {
    static std::unique_ptr<ThreadPool> pool;
    static std::mutex init_mutex;

    std::lock_guard<std::mutex> lock(init_mutex);
    if (!pool || (num_threads > 0 && pool->size() != static_cast<size_t>(num_threads))) {
        if (num_threads <= 0) num_threads = std::thread::hardware_concurrency();
        pool = std::make_unique<ThreadPool>(num_threads);
    }
    return *pool;
}

} // namespace dual_pivot

#endif // DPQS_PARALLEL_THREADPOOL_HPP
