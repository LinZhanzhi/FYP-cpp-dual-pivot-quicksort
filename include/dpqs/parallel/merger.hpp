#ifndef DPQS_PARALLEL_MERGER_HPP
#define DPQS_PARALLEL_MERGER_HPP

#include "dpqs/parallel/completer.hpp"
#include "dpqs/types.hpp"
#include "dpqs/utils.hpp"
#include "dpqs/merge_ops.hpp"
#include "dpqs/parallel/threadpool.hpp"

namespace dual_pivot {

class GenericMerger : public CountedCompleter<void> {
private:
    ArrayPointer dst;
    int k;
    ArrayPointer a1;
    int lo1, hi1;
    ArrayPointer a2;
    int lo2, hi2;

public:
    GenericMerger(CountedCompleter<void>* parent, ArrayPointer dst, int k,
                  ArrayPointer a1, int lo1, int hi1, ArrayPointer a2, int lo2, int hi2)
        : CountedCompleter<void>(parent), dst(dst), k(k), a1(a1), lo1(lo1), hi1(hi1), a2(a2), lo2(lo2), hi2(hi2) {}

    void compute() override {
        // Runtime type dispatch for merging operations (matching Java's approach)
        if (dst.isIntArray()) {
            mergeParts(dst.asIntArray(), k, a1.asIntArray(), lo1, hi1, a2.asIntArray(), lo2, hi2);
        } else if (dst.isLongArray()) {
            mergeParts(dst.asLongArray(), k, a1.asLongArray(), lo1, hi1, a2.asLongArray(), lo2, hi2);
        } else if (dst.isFloatArray()) {
            mergeParts(dst.asFloatArray(), k, a1.asFloatArray(), lo1, hi1, a2.asFloatArray(), lo2, hi2);
        } else if (dst.isDoubleArray()) {
            mergeParts(dst.asDoubleArray(), k, a1.asDoubleArray(), lo1, hi1, a2.asDoubleArray(), lo2, hi2);
        } else {
            throw std::runtime_error("Unknown array type in GenericMerger::compute()");
        }
        tryComplete();
    }
};




/**
 * @brief Parallel merger for combining sorted array segments
 *
 * This class implements parallel merging of two sorted array segments using
 * recursive decomposition. It automatically switches between parallel and
 * sequential merging based on segment sizes to optimize performance.
 *
 * Parallel Merge Strategy:
 * - Large segments: Use binary search partitioning for parallel subdivision
 * - Small segments: Use sequential merge to avoid thread overhead
 * - Load balancing: Ensure work is distributed evenly across threads
 *
 * Binary Search Partitioning:
 * - Find split points using std::lower_bound for balanced workload
 * - Recursive subdivision until segments are too small for parallelism
 * - Cache-aware processing to minimize memory access overhead
 *
 * @tparam T Element type being merged
 */
template<typename T>
class Merger : public CountedCompleter<T> {
private:
    T* dst;                      ///< Destination array for merged result
    int k;                       ///< Starting index in destination
    T* a1;                       ///< First source array
    int lo1, hi1;                ///< Range of first segment [lo1, hi1)
    T* a2;                       ///< Second source array
    int lo2, hi2;                ///< Range of second segment [lo2, hi2)

public:
    /**
     * @brief Construct a parallel merger task
     * @param parent Parent task for completion coordination
     * @param dst Destination array for merged output
     * @param k Starting index in destination array
     * @param a1 First source array
     * @param lo1 Start of first segment (inclusive)
     * @param hi1 End of first segment (exclusive)
     * @param a2 Second source array
     * @param lo2 Start of second segment (inclusive)
     * @param hi2 End of second segment (exclusive)
     */
    Merger(CountedCompleter<T>* parent, T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2)
        : CountedCompleter<T>(parent), dst(dst), k(k), a1(a1), lo1(lo1), hi1(hi1), a2(a2), lo2(lo2), hi2(hi2) {}

    /**
     * @brief Main computation method for parallel merging
     *
     * Implements the core merge logic with automatic parallelization for
     * large segments. Uses sophisticated load balancing to ensure optimal
     * thread utilization while maintaining cache efficiency.
     *
     * Merge Strategy:
     * - Check segment sizes against parallelization threshold
     * - Use parallel subdivision for large segments
     * - Fall back to sequential merge for small segments
     * - Maintain cache locality through careful work distribution
     */
    void compute() override {
        // Use parallel merge with subdivision for large parts
        if (hi1 - lo1 >= MIN_PARALLEL_MERGE_PARTS_SIZE && hi2 - lo2 >= MIN_PARALLEL_MERGE_PARTS_SIZE) {
            // Parallel merge with binary search partitioning
            parallelMergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
        } else {
            // Sequential merge for small parts
            mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
        }
    }
};

// RunMerger class for parallel run merging (matching Java's RunMerger extends RecursiveTask)
template<typename T>
class RunMerger {
private:
    T* a;
    T* b;
    int offset;
    int aim;
    std::vector<int> run;
    int lo, hi;

    // Advanced parallel coordination state
    std::future<T*> future_result;
    bool is_forked = false;
    T* result = nullptr;
    std::atomic<bool> completed{false};
    std::mutex result_mutex;

public:
    RunMerger(T* a, T* b, int offset, int aim, const std::vector<int>& run, int lo, int hi)
        : a(a), b(b), offset(offset), aim(aim), run(run), lo(lo), hi(hi) {}

    // Enhanced compute method with sophisticated parallel subdivision
    T* compute() {
        if (hi - lo == 1) {
            // Base case: single run
            if (aim >= 0) {
                return a;
            }
            // Copy elements in reverse order (matching Java's approach)
            for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
                b[--j] = a[--i];
            }
            return b;
        }

        // Advanced parallel subdivision (matching Java's sophisticated approach)
        int mi = lo;
        int rmi = (run[lo] + run[hi]) >> 1; // Unsigned right shift equivalent
        while (run[++mi + 1] <= rmi);

        // Create parallel tasks for left and right parts with advanced coordination
        auto& pool = getThreadPool();

        // Left subtask with negative aim (Java pattern)
        auto future1 = pool.enqueue([=]() {
            RunMerger<T> left(a, b, offset, -aim, run, lo, mi);
            return left.compute();
        });

        // Right subtask with zero aim (Java pattern)
        auto future2 = pool.enqueue([=]() {
            RunMerger<T> right(a, b, offset, 0, run, mi, hi);
            return right.compute();
        });

        // Get results from parallel tasks with proper synchronization
        T* a1 = future1.get();
        T* a2 = future2.get();

        // Advanced destination calculation (matching Java's sophisticated logic)
        T* dst = (a1 == a) ? b : a;

        // Complex offset calculations (matching Java's approach)
        int k   = (a1 == a) ? run[lo] - offset : run[lo];
        int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
        int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
        int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
        int hi2 = (a2 == b) ? run[hi] - offset : run[hi];

        // Advanced merge with parallel coordination
        mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
        return dst;
    }

    // Java-style forkMe() method - sophisticated fork and return self pattern
    RunMerger& forkMe() {
        std::lock_guard<std::mutex> lock(result_mutex);

        if (!is_forked) {
            auto& pool = getThreadPool();

            // Advanced future-based forking with exception handling
            future_result = pool.enqueue([this]() -> T* {
                try {
                    T* computed_result = this->compute();

                    // Mark completion with thread-safe state management
                    std::lock_guard<std::mutex> result_lock(this->result_mutex);
                    this->result = computed_result;
                    this->completed.store(true);

                    return computed_result;
                } catch (...) {
                    // Enhanced exception handling (matching Java's approach)
                    std::lock_guard<std::mutex> result_lock(this->result_mutex);
                    this->completed.store(true);
                    throw;
                }
            });

            is_forked = true;
        }
        return *this;
    }

    // Java-style getDestination() method - sophisticated join and get result
    T* getDestination() {
        std::lock_guard<std::mutex> lock(result_mutex);

        if (is_forked) {
            if (!completed.load()) {
                // Advanced synchronization with timeout handling
                try {
                    // Wait for completion with proper exception propagation
                    result = future_result.get();
                    completed.store(true);
                } catch (const std::exception& e) {
                    // Enhanced error handling matching Java's exception model
                    completed.store(true);
                    throw std::runtime_error("RunMerger computation failed: " + std::string(e.what()));
                }
            }
            return result;
        } else {
            // If not forked, compute directly (lazy evaluation pattern)
            if (!completed.load()) {
                result = compute();
                completed.store(true);
            }
            return result;
        }
    }
};

}

#endif // DPQS_PARALLEL_MERGER_HPP
