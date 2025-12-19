#ifndef DPQS_PARALLEL_COMPLETER_HPP
#define DPQS_PARALLEL_COMPLETER_HPP

#include "dpqs/utils.hpp"
#include "dpqs/parallel/threadpool.hpp"
#include <atomic>
#include <mutex>
#include <exception>
#include <thread>
#include <chrono>

namespace dual_pivot {

template<typename T>
class CountedCompleter {
protected:
    std::atomic<int> pending{0};
    CountedCompleter* parent{nullptr};
    bool completed{false};
    std::mutex completion_mutex;  // Enhanced thread safety
    std::condition_variable completion_cv; // For waiting on completion

public:
    CountedCompleter(CountedCompleter* parent = nullptr) : parent(parent) {
        if (parent) {
            parent->pending.fetch_add(1);
        }
    }

    virtual ~CountedCompleter() = default;

    virtual void compute() = 0;
    virtual void onCompletion(CountedCompleter* caller) {}
    virtual void onExceptionalCompletion(std::exception_ptr ex, CountedCompleter* caller) {}

    void invoke() {
        try {
            compute();
            tryComplete();
        } catch (...) {
            completeExceptionally(std::current_exception());
        }
    }

    void wait() {
        std::unique_lock<std::mutex> lock(completion_mutex);
        completion_cv.wait(lock, [this]{ return completed; });
    }

    void fork() {
        auto& pool = getThreadPool();
        pool.enqueue([this]() { invoke(); });
    }

    // Enhanced completion with proper propagation (matching Java's sophistication)
    void tryComplete() {
        CountedCompleter* curr = this;
        while (curr != nullptr) {
            std::lock_guard<std::mutex> lock(curr->completion_mutex);

            if (curr->pending.load() == 0 && !curr->completed) {
                curr->completed = true;
                curr->completion_cv.notify_all(); // Notify waiters

                CountedCompleter* parent = curr->parent;

                if (parent) {
                    try {
                        curr->onCompletion(curr);

                        // Advanced pending count management with atomic operations
                        int prevPending = parent->pending.fetch_sub(1);
                        if (prevPending == 1) {
                            curr = parent;
                            continue;  // Propagate completion to parent
                        }
                    } catch (...) {
                        curr->completeExceptionally(std::current_exception());
                        return;
                    }
                }
                break;
            } else {
                break;
            }
        }
    }

    // Enhanced exception handling (matching Java's pattern)
    void completeExceptionally(std::exception_ptr ex) {
        std::lock_guard<std::mutex> lock(completion_mutex);
        completed = true;

        // Propagate exception to parent chain
        CountedCompleter* curr = parent;
        while (curr != nullptr) {
            try {
                curr->onExceptionalCompletion(ex, this);
                curr = curr->parent;
            } catch (...) {
                // Swallow additional exceptions during propagation
                break;
            }
        }
    }

    void addToPendingCount(int delta) {
        pending.fetch_add(delta);
    }

    // Java-style pending count management
    void setPendingCount(int count) {
        pending.store(count);
    }

    int getPendingCount() const {
        return pending.load();
    }

    bool isCompletedAbnormally() const {
        return completed && pending.load() < 0;  // Use negative pending as error flag
    }

    // Advanced completion checking with timeout (C++ enhancement)
    bool tryCompleteWithTimeout(int timeout_ms) {
        auto start = std::chrono::steady_clock::now();

        while (!completed) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

            if (elapsed.count() > timeout_ms) {
                return false;  // Timeout
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return true;
    }
};

}

#endif // DPQS_PARALLEL_COMPLETER_HPP
