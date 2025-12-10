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

public:
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
                    task();
                }
            });
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
            worker.join();
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
 * The thread pool is created with the default hardware concurrency and remains
 * active for the lifetime of the program.
 *
 * @return Reference to the global ThreadPool instance
 */
inline ThreadPool& getThreadPool() {
    static ThreadPool pool;
    return pool;
}

} // namespace dual_pivot

#endif // DPQS_PARALLEL_THREADPOOL_HPP
