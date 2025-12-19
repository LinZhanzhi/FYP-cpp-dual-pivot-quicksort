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
            merge_parts(dst.asIntArray(), k, a1.asIntArray(), lo1, hi1, a2.asIntArray(), lo2, hi2);
        } else if (dst.isLongArray()) {
            merge_parts(dst.asLongArray(), k, a1.asLongArray(), lo1, hi1, a2.asLongArray(), lo2, hi2);
        } else if (dst.isFloatArray()) {
            merge_parts(dst.asFloatArray(), k, a1.asFloatArray(), lo1, hi1, a2.asFloatArray(), lo2, hi2);
        } else if (dst.isDoubleArray()) {
            merge_parts(dst.asDoubleArray(), k, a1.asDoubleArray(), lo1, hi1, a2.asDoubleArray(), lo2, hi2);
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
            parallel_merge_parts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
        } else {
            // Sequential merge for small parts
            merge_parts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
        }
    }
};

// RunMerger class for parallel run merging (matching Java's RunMerger extends RecursiveTask)
template<typename T>
class RunMerger : public CountedCompleter<void> {
public:
    T* result = nullptr;

private:
    T* a;
    T* b;
    int offset;
    int aim;
    std::vector<int> run;
    int lo, hi;
    int mi; // Store split point

    RunMerger* leftChild = nullptr;
    RunMerger* rightChild = nullptr;

public:
    RunMerger(CountedCompleter<void>* parent, T* a, T* b, int offset, int aim, const std::vector<int>& run, int lo, int hi)
        : CountedCompleter<void>(parent), a(a), b(b), offset(offset), aim(aim), run(run), lo(lo), hi(hi) {}

    ~RunMerger() {
        if (leftChild) delete leftChild;
        if (rightChild) delete rightChild;
    }

    void compute() override {
        if (hi - lo == 1) {
            // Base case: single run
            if (aim >= 0) {
                result = a;
            } else {
                // Copy elements in reverse order (matching Java's approach)
                for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
                    b[--j] = a[--i];
                }
                result = b;
            }
            tryComplete();
            return;
        }

        // Advanced parallel subdivision (matching Java's sophisticated approach)
        mi = lo;
        int rmi = (run[lo] + run[hi]) >> 1; // Unsigned right shift equivalent
        while (run[++mi + 1] <= rmi);

        // Create parallel tasks for left and right parts
        leftChild = new RunMerger(this, a, b, offset, -aim, run, lo, mi);
        rightChild = new RunMerger(this, a, b, offset, 0, run, mi, hi);

        leftChild->fork();
        rightChild->fork();
    }

    void onCompletion(CountedCompleter<void>* caller) override {
        if (leftChild && rightChild) {
            T* a1 = leftChild->result;
            T* a2 = rightChild->result;

            // Advanced destination calculation (matching Java's sophisticated logic)
            T* dst = (a1 == a) ? b : a;

            // Complex offset calculations (matching Java's approach)
            int k   = (a1 == a) ? run[lo] - offset : run[lo];
            int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
            int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
            int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
            int hi2 = (a2 == b) ? run[hi] - offset : run[hi];

            // Advanced merge with parallel coordination
            merge_parts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
            result = dst;
        }
    }
};

}

#endif // DPQS_PARALLEL_MERGER_HPP
