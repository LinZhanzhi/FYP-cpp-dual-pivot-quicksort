#ifndef DPQS_RUN_MERGER_HPP
#define DPQS_RUN_MERGER_HPP

#include <vector>
#include <algorithm>
#include "dpqs/constants.hpp"
#include "dpqs/merge_ops.hpp"
#include "dpqs/parallel/merger.hpp"

namespace dual_pivot {

// =============================================================================
// RUN DETECTION AND MERGING IMPLEMENTATION (Phase 3)
// =============================================================================

/**
 * @brief Advanced run detection and merging for naturally ordered subsequences
 *
 * This section implements the sophisticated run detection and merging system
 * that identifies naturally occurring sorted subsequences (runs) in the input
 * array and merges them efficiently. This optimization is crucial for achieving
 * excellent performance on real-world data that often contains partial order.
 *
 * Key Concepts:
 * - Run: A maximal sequence of consecutive elements that are already sorted
 * - Ascending runs: Elements in non-decreasing order (a[i] <= a[i+1])
 * - Descending runs: Elements in non-increasing order (reversed to ascending)
 * - Constant runs: All elements equal (treated as sorted)
 *
 * Performance Benefits:
 * - O(n) performance on already sorted data
 * - Significant speedup on partially sorted data
 * - Better cache utilization through sequential access patterns
 * - Reduced number of comparisons and swaps
 */

// Forward declarations for run merging functions
template<typename T>
T* mergeRuns(T* a, T* b, int offset, int aim,
             const std::vector<int>& run, int lo, int hi);

template<typename T>
void mergeParts(T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2);

/**
 * @brief Attempts to detect and merge sorted runs for optimized sorting
 *
 * This function implements an advanced run detection algorithm that identifies
 * naturally occurring sorted subsequences in the array. If sufficient runs
 * are found with adequate length, it merges them using an optimized merge
 * strategy, potentially achieving O(n) or near-O(n) performance.
 *
 * Run Detection Strategy:
 * 1. Scan array to identify ascending, descending, and constant sequences
 * 2. Reverse descending sequences to make them ascending
 * 3. Validate that initial runs are long enough to justify merge overhead
 * 4. Track run boundaries in a compact integer array
 * 5. Merge runs using recursive divide-and-conquer if beneficial
 *
 * Quality Heuristics:
 * - Initial runs must be at least MIN_FIRST_RUN_SIZE elements
 * - Total run count limited to MAX_RUN_CAPACITY to avoid overhead
 * - First runs factor validates that runs are long enough relative to total size
 * - Early termination if runs are too short or too numerous
 *
 * Parallel Optimization:
 * - Uses parallel run merging for large run counts (>= MIN_RUN_COUNT)
 * - Falls back to sequential merging for smaller run sets
 * - Maintains cache efficiency through localized processing
 *
 * @tparam T Element type (must support comparison and assignment)
 * @param a Pointer to the array to analyze and potentially sort
 * @param low Starting index of the range to process
 * @param size Number of elements in the range
 * @param parallel Whether to use parallel merging (default: false)
 * @return true if runs were detected and merged (array is now sorted)
 * @return false if run detection failed (caller should use different algorithm)
 */
template<typename T>
bool tryMergeRuns(T* a, int low, int size, bool parallel = false) {
    // Run array stores start indices of sorted subsequences
    // Only constructed if initial analysis shows promising run structure
    // run[i] holds the starting index of the i-th run
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;

    // Identify all possible runs
    for (int k = low + 1; k < high; ) {

        // Find the end index of the current run
        if (a[k - 1] < a[k]) {
            // Identify ascending sequence
            while (++k < high && a[k - 1] <= a[k]);

        } else if (a[k - 1] > a[k]) {
            // Identify descending sequence
            while (++k < high && a[k - 1] >= a[k]);

            // Reverse into ascending order
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                T temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        } else { // Identify constant sequence
            T ak = a[k];
            while (++k < high && ak == a[k]);

            if (k < high) {
                continue;
            }
        }

        // Check special cases
        if (run.empty()) {
            if (k == high) {
                // The array is monotonous sequence,
                // and therefore already sorted.
                return true;
            }

            if (k - low < MIN_FIRST_RUN_SIZE) {
                // The first run is too small
                // to proceed with scanning.
                return false;
            }

            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);

        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                // The first runs are not long
                // enough to continue scanning.
                return false;
            }

            if (++count == MAX_RUN_CAPACITY) {
                // Array is not highly structured.
                return false;
            }
        }
        run.push_back(last = k);
    }

    // Merge runs of highly structured array
    if (count > 1) {
        std::vector<T> b(size);

        if (parallel && count >= MIN_RUN_COUNT) {
            // Use parallel run merging for large run counts
            RunMerger<T> merger(a, b.data(), low, 1, run, 0, count);
            T* result = merger.compute();

            // Copy back to main array if needed
            if (result != a) {
                std::copy(result + low, result + low + size, a + low);
            }
        } else {
            // Use sequential merging
            mergeRuns(a, b.data(), low, 1, run, 0, count);
        }
    }
    return true;
}

template<typename T>
T* mergeRuns(T* a, T* b, int offset, int aim,
             const std::vector<int>& run, int lo, int hi) {

    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }

    // Split into approximately equal parts
    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);

    // Merge the left and right parts
    T* a1 = mergeRuns(a, b, offset, -aim, run, lo, mi);
    T* a2 = mergeRuns(a, b, offset, 0, run, mi, hi);

    T* dst = (a1 == a) ? b : a;

    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];

    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}



} // namespace dual_pivot

#endif // DPQS_RUN_MERGER_HPP