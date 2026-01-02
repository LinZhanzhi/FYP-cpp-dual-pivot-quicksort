#ifndef DPQS_SEQUENTIAL_SORTERS_HPP
#define DPQS_SEQUENTIAL_SORTERS_HPP

#include "dpqs/utils.hpp"
#include "dpqs/parallel/sorter.hpp"
#include "dpqs/partition.hpp"
#include "dpqs/insertion_sort.hpp"
#include "dpqs/heap_sort.hpp"
#include "dpqs/run_merger.hpp"
#include <vector>
#include <utility>
#include <algorithm>

namespace dual_pivot {

/**
 * @brief Sorts a 5-element network for pivot selection.
 *
 * This helper function sorts 5 elements at specified indices using a sorting network.
 * It is used to select pivots for the dual-pivot quicksort.
 *
 * @tparam T Element type.
 * @tparam Compare Comparator type.
 * @param a Pointer to the array.
 * @param e1 Index of the 1st element.
 * @param e2 Index of the 2nd element.
 * @param e3 Index of the 3rd element.
 * @param e4 Index of the 4th element.
 * @param e5 Index of the 5th element.
 * @param comp Comparator instance.
 */
template<typename T, typename Compare>
DPQS_FORCE_INLINE void sort5_network(T* a, std::ptrdiff_t e1, std::ptrdiff_t e2, std::ptrdiff_t e3, std::ptrdiff_t e4, std::ptrdiff_t e5, Compare comp) {
    if (comp(a[e2], a[e1])) std::swap(a[e1], a[e2]);
    if (comp(a[e5], a[e4])) std::swap(a[e4], a[e5]);
    if (comp(a[e3], a[e1])) std::swap(a[e1], a[e3]);
    if (comp(a[e3], a[e2])) std::swap(a[e2], a[e3]);
    if (comp(a[e4], a[e1])) std::swap(a[e1], a[e4]);
    if (comp(a[e4], a[e3])) std::swap(a[e3], a[e4]);
    if (comp(a[e5], a[e2])) std::swap(a[e2], a[e5]);
    if (comp(a[e3], a[e2])) std::swap(a[e2], a[e3]);
    if (comp(a[e5], a[e4])) std::swap(a[e4], a[e5]);
}

/**
 * @brief Generic sequential implementation of Dual-Pivot Quicksort.
 *
 * This function implements the core recursive logic of the Dual-Pivot Quicksort algorithm.
 * It coordinates various sorting strategies based on array size and recursion depth:
 * - Mixed Insertion Sort for small arrays.
 * - Insertion Sort for very small arrays.
 * - Run Merging for nearly sorted data.
 * - Heap Sort for deep recursion (fallback).
 * - Dual-Pivot Partitioning for the general case.
 * - Single-Pivot Partitioning when elements are equal.
 *
 * It also supports parallel execution by forking tasks if a Sorter is provided.
 *
 * @tparam T Element type.
 * @tparam Compare Comparator type.
 * @param sorter Pointer to the Sorter object for parallel execution (can be nullptr).
 * @param a Pointer to the array to sort.
 * @param bits Recursion depth and mode bits.
 * @param low Starting index (inclusive).
 * @param high Ending index (exclusive).
 * @param comp Comparator instance.
 */
template<typename T, typename Compare>
void sort_sequential(Sorter<T, Compare>* sorter, T* a, int bits, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp) {
    while (true) {
        std::ptrdiff_t end = high - 1;
        std::ptrdiff_t size = high - low;

        // Use mixed insertion sort on small non-leftmost parts
        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            mixed_insertion_sort(a, low, high, comp);
            return;
        }

        // Use insertion sort on small leftmost parts
        if (size < MAX_INSERTION_SORT_SIZE) {
            insertion_sort(a, low, high, comp);
            return;
        }

        // Try merge runs for nearly sorted data
        if (size > MIN_TRY_MERGE_SIZE && try_merge_runs(a, low, size, comp, sorter != nullptr)) {
            return;
        }

        // Switch to heap sort if execution time is becoming quadratic
        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            heap_sort(a, low, high, comp);
            return;
        }

        // Five-element pivot selection
        std::ptrdiff_t step = (size >> 3) * 3 + 3;
        std::ptrdiff_t e1 = low + step;
        std::ptrdiff_t e5 = end - step;
        std::ptrdiff_t e3 = (e1 + e5) >> 1;
        std::ptrdiff_t e2 = (e1 + e3) >> 1;
        std::ptrdiff_t e4 = (e3 + e5) >> 1;

        // Sort 5-element sample
        sort5_network(a, e1, e2, e3, e4, e5, comp);

        std::ptrdiff_t lower, upper;

        // Dual-pivot partitioning
        if (comp(a[e1], a[e2]) && comp(a[e2], a[e3]) && comp(a[e3], a[e4]) && comp(a[e4], a[e5])) {
            auto pivotIndices = partition_dual_pivot(a, low, high, e1, e5, comp);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            // Optimized Tail Recursion: Recurse on smaller parts, loop on largest
            std::ptrdiff_t left_len = lower - low;
            std::ptrdiff_t mid_len = upper - (lower + 1);
            std::ptrdiff_t right_len = high - (upper + 1);

            // PARALLEL STRATEGY: Offload largest, keep smallest (Load Balancing)
            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                if (left_len <= mid_len && left_len <= right_len) {
                    // Left is smallest. Fork Mid and Right. Loop Left.
                    sorter->forkSorter(bits | 1, lower + 1, upper);
                    sorter->forkSorter(bits | 1, upper + 1, high);
                    high = lower;
                } else if (mid_len <= right_len) {
                    // Mid is smallest. Fork Left and Right. Loop Mid.
                    sorter->forkSorter(bits, low, lower);
                    sorter->forkSorter(bits | 1, upper + 1, high);
                    low = lower + 1;
                    high = upper;
                    bits |= 1;
                } else {
                    // Right is smallest. Fork Left and Mid. Loop Right.
                    sorter->forkSorter(bits, low, lower);
                    sorter->forkSorter(bits | 1, lower + 1, upper);
                    low = upper + 1;
                    bits |= 1;
                }
            }
            // SEQUENTIAL STRATEGY: Recurse smallest, loop largest (Stack Optimization)
            else {
                if (left_len >= mid_len && left_len >= right_len) {
                    // Left is largest. Recurse Mid and Right. Loop Left.
                    sort_sequential(sorter, a, bits | 1, lower + 1, upper, comp);
                    sort_sequential(sorter, a, bits | 1, upper + 1, high, comp);
                    high = lower;
                } else if (mid_len >= right_len) {
                    // Mid is largest. Recurse Left and Right. Loop Mid.
                    sort_sequential(sorter, a, bits, low, lower, comp);
                    sort_sequential(sorter, a, bits | 1, upper + 1, high, comp);
                    low = lower + 1;
                    high = upper;
                    bits |= 1;
                } else {
                    // Right is largest. Recurse Left and Mid. Loop Right.
                    sort_sequential(sorter, a, bits, low, lower, comp);
                    sort_sequential(sorter, a, bits | 1, lower + 1, upper, comp);
                    low = upper + 1;
                    bits |= 1;
                }
            }
        } else {
            // Single-pivot partitioning
            auto pivotIndices = partition_single_pivot(a, low, high, e3, e3, comp);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            std::ptrdiff_t left_len = lower - low;
            std::ptrdiff_t right_len = high - (upper + 1);

            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                // Parallel: Loop Smaller
                if (left_len <= right_len) {
                    // Left is smaller. Fork Right. Loop Left.
                    sorter->forkSorter(bits | 1, upper + 1, high);
                    high = lower;
                } else {
                    // Right is smaller. Fork Left. Loop Right.
                    sorter->forkSorter(bits, low, lower);
                    low = upper + 1;
                    bits |= 1;
                }
            } else {
                // Sequential: Loop Larger
                if (left_len >= right_len) {
                    // Left is larger. Recurse Right. Loop Left.
                    sort_sequential(sorter, a, bits | 1, upper + 1, high, comp);
                    high = lower;
                } else {
                    // Right is larger. Recurse Left. Loop Right.
                    sort_sequential(sorter, a, bits, low, lower, comp);
                    low = upper + 1;
                    bits |= 1;
                }
            }
        }
    }
}

} // namespace dual_pivot

#endif // DPQS_SEQUENTIAL_SORTERS_HPP
