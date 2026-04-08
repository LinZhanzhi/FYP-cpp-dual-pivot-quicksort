#ifndef DPQS_CHASE_LEV_DEQUE_HPP
#define DPQS_CHASE_LEV_DEQUE_HPP

#include <atomic>
#include <array>
#include <memory>
#include <functional>
#include <cstdint>

namespace dual_pivot {

/**
 * @brief Lock-free Chase-Lev Work-Stealing Deque
 *
 * Implementation based on the paper:
 * "Dynamic Circular Work-Stealing Deque" by Chase and Lev, SPAA 2005
 *
 * Key properties:
 * - Owner operations (push/pop from bottom) are lock-free and usually wait-free
 * - Thief operations (steal from top) are lock-free using CAS
 * - No mutex contention - uses atomic operations only
 *
 * Memory ordering:
 * - push: release fence to ensure task visibility before bottom increment
 * - pop: seq_cst fence to ensure correct synchronization with thieves
 * - steal: acquire-release for proper happens-before relationships
 *
 * @tparam CAPACITY Maximum number of tasks (must be power of 2)
 */
template<size_t CAPACITY = 4096>
class ChaseLevDeque {
    static_assert((CAPACITY & (CAPACITY - 1)) == 0, "CAPACITY must be power of 2");

private:
    // The task type we store (pointer to std::function to avoid object lifetime issues)
    using Task = std::function<void()>*;

    // Separate cache lines to prevent false sharing
    alignas(64) std::atomic<int64_t> top_{0};
    alignas(64) std::atomic<int64_t> bottom_{0};

    // Circular buffer
    alignas(64) std::array<std::atomic<Task>, CAPACITY> buffer_;

    // Mask for circular indexing
    static constexpr size_t MASK = CAPACITY - 1;

public:
    ChaseLevDeque() {
        for (auto& slot : buffer_) {
            slot.store(nullptr, std::memory_order_relaxed);
        }
    }

    ~ChaseLevDeque() {
        // Clean up any remaining tasks
        int64_t t = top_.load(std::memory_order_relaxed);
        int64_t b = bottom_.load(std::memory_order_relaxed);
        for (int64_t i = t; i < b; ++i) {
            Task task = buffer_[i & MASK].load(std::memory_order_relaxed);
            delete task;
        }
    }

    /**
     * @brief Push a task to the bottom (Owner only, wait-free)
     *
     * Only the owner thread calls this. No synchronization with other push/pop needed.
     */
    void push(std::function<void()> f) {
        // Allocate task on heap (required for lock-free correctness)
        Task task = new std::function<void()>(std::move(f));

        int64_t b = bottom_.load(std::memory_order_relaxed);
        buffer_[b & MASK].store(task, std::memory_order_relaxed);

        // CRITICAL: Ensure task is visible before incrementing bottom
        // This release fence pairs with the acquire in steal()
        std::atomic_thread_fence(std::memory_order_release);

        bottom_.store(b + 1, std::memory_order_relaxed);
    }

    /**
     * @brief Pop a task from the bottom (Owner only)
     *
     * Only the owner thread calls this. May contend with thieves on last element.
     *
     * @return Pointer to task function, or nullptr if empty
     */
    std::function<void()>* pop() {
        int64_t b = bottom_.load(std::memory_order_relaxed) - 1;
        bottom_.store(b, std::memory_order_relaxed);

        // CRITICAL: Full fence to ensure bottom decrement is visible to thieves
        // before we read top. This prevents the ABA problem on the last element.
        std::atomic_thread_fence(std::memory_order_seq_cst);

        int64_t t = top_.load(std::memory_order_relaxed);

        if (t <= b) {
            // Queue not empty
            Task task = buffer_[b & MASK].load(std::memory_order_relaxed);

            if (t == b) {
                // Last element - race with thieves!
                // Try to claim it with CAS on top
                if (!top_.compare_exchange_strong(
                        t, t + 1,
                        std::memory_order_seq_cst,
                        std::memory_order_relaxed)) {
                    // Lost race to a thief
                    bottom_.store(b + 1, std::memory_order_relaxed);
                    return nullptr;
                }
                // Won the race
                bottom_.store(b + 1, std::memory_order_relaxed);
            }
            return task;
        }

        // Queue was empty
        bottom_.store(b + 1, std::memory_order_relaxed);
        return nullptr;
    }

    /**
     * @brief Steal a task from the top (Thieves only)
     *
     * Any non-owner thread can call this. Lock-free using CAS.
     *
     * @return Pointer to task function, or nullptr if empty or contention
     */
    std::function<void()>* steal() {
        int64_t t = top_.load(std::memory_order_acquire);

        // CRITICAL: Fence to ensure we see consistent view of top and bottom
        std::atomic_thread_fence(std::memory_order_seq_cst);

        int64_t b = bottom_.load(std::memory_order_acquire);

        if (t < b) {
            // Queue not empty
            Task task = buffer_[t & MASK].load(std::memory_order_relaxed);

            // Try to claim this task with CAS
            if (top_.compare_exchange_strong(
                    t, t + 1,
                    std::memory_order_seq_cst,
                    std::memory_order_relaxed)) {
                return task;  // Success!
            }
            // CAS failed - another thief won or owner popped
        }
        return nullptr;
    }

    /**
     * @brief Check if deque is empty (approximate, for heuristics only)
     */
    bool empty() const {
        int64_t t = top_.load(std::memory_order_relaxed);
        int64_t b = bottom_.load(std::memory_order_relaxed);
        return t >= b;
    }

    /**
     * @brief Get approximate size (for heuristics only)
     */
    int64_t size() const {
        int64_t t = top_.load(std::memory_order_relaxed);
        int64_t b = bottom_.load(std::memory_order_relaxed);
        return b - t;
    }
};

} // namespace dual_pivot

#endif // DPQS_CHASE_LEV_DEQUE_HPP
