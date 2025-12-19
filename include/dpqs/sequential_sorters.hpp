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
 * @param a Pointer to the array.
 * @param e1 Index of the 1st element.
 * @param e2 Index of the 2nd element.
 * @param e3 Index of the 3rd element.
 * @param e4 Index of the 4th element.
 * @param e5 Index of the 5th element.
 */
template<typename T>
FORCE_INLINE void sort5_network(T* a, int e1, int e2, int e3, int e4, int e5) {
    if (a[e1] > a[e2]) std::swap(a[e1], a[e2]);
    if (a[e4] > a[e5]) std::swap(a[e4], a[e5]);
    if (a[e1] > a[e3]) std::swap(a[e1], a[e3]);
    if (a[e2] > a[e3]) std::swap(a[e2], a[e3]);
    if (a[e1] > a[e4]) std::swap(a[e1], a[e4]);
    if (a[e3] > a[e4]) std::swap(a[e3], a[e4]);
    if (a[e2] > a[e5]) std::swap(a[e2], a[e5]);
    if (a[e2] > a[e3]) std::swap(a[e2], a[e3]);
    if (a[e4] > a[e5]) std::swap(a[e4], a[e5]);
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
 * @param sorter Pointer to the Sorter object for parallel execution (can be nullptr).
 * @param a Pointer to the array to sort.
 * @param bits Recursion depth and mode bits.
 * @param low Starting index (inclusive).
 * @param high Ending index (exclusive).
 */
template<typename T>
void sort_sequential(Sorter<T>* sorter, T* a, int bits, int low, int high) {
    while (true) {
        int end = high - 1;
        int size = high - low;

        // Use mixed insertion sort on small non-leftmost parts
        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            mixed_insertion_sort(a, low, high);
            return;
        }

        // Use insertion sort on small leftmost parts
        if (size < MAX_INSERTION_SORT_SIZE) {
            insertion_sort(a, low, high);
            return;
        }

        // Try merge runs for nearly sorted data
        if (size > MIN_TRY_MERGE_SIZE && try_merge_runs(a, low, size, sorter != nullptr)) {
            return;
        }

        // Switch to heap sort if execution time is becoming quadratic
        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            heap_sort(a, low, high);
            return;
        }

        // Five-element pivot selection
        int step = (size >> 3) * 3 + 3;
        int e1 = low + step;
        int e5 = end - step;
        int e3 = (e1 + e5) >> 1;
        int e2 = (e1 + e3) >> 1;
        int e4 = (e3 + e5) >> 1;

        // Sort 5-element sample
        sort5_network(a, e1, e2, e3, e4, e5);

        int lower, upper;

        // Dual-pivot partitioning
        if (a[e1] < a[e2] && a[e2] < a[e3] && a[e3] < a[e4] && a[e4] < a[e5]) {
            auto pivotIndices = partition_dual_pivot(a, low, high, e1, e5);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            // Verify partitioning
            // Fork parallel tasks if sorter available
            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                // sorter->addToPendingCount(1); // Prevent premature completion
                sorter->forkSorter(bits | 1, lower + 1, upper);
                sorter->forkSorter(bits | 1, upper + 1, high);
                // sorter->addToPendingCount(-1); // Release hold
            } else {
                sort_sequential(sorter, a, bits | 1, lower + 1, upper);
                sort_sequential(sorter, a, bits | 1, upper + 1, high);
            }
        } else {
            // Single-pivot partitioning
            auto pivotIndices = partition_single_pivot(a, low, high, e3, e3);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, upper + 1, high);
            } else {
                sort_sequential(sorter, a, bits | 1, upper + 1, high);
            }
        }

        high = lower; // Continue with left part (tail recursion elimination)
    }
}

} // namespace dual_pivot

#endif // DPQS_SEQUENTIAL_SORTERS_HPP
