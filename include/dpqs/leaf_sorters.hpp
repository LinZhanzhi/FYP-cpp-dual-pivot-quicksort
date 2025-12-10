#ifndef DPQS_LEAF_SORTERS_HPP
#define DPQS_LEAF_SORTERS_HPP

#include "dpqs/utils.hpp"

namespace dual_pivot {

/**
 * @brief Cache-friendly insertion sort with prefetching optimizations
 *
 * This is an optimized implementation of insertion sort that serves as the base case
 * for small arrays in the dual-pivot quicksort algorithm. It includes several
 * performance optimizations based on modern CPU characteristics:
 *
 * Key optimizations:
 * - Memory prefetching to reduce cache misses
 * - Branch prediction hints to reduce pipeline stalls
 * - Optimized inner loop for better instruction scheduling
 *
 * The algorithm threshold is carefully tuned: arrays smaller than MAX_INSERTION_SORT_SIZE
 * (44 elements) benefit from this approach over more complex algorithms.
 *
 * Time complexity: O(n²) in worst case, O(n) for nearly sorted data
 * Space complexity: O(1)
 *
 * @tparam T Element type (must support comparison and assignment)
 * @param a Pointer to the array to sort
 * @param low Starting index (inclusive)
 * @param high Ending index (exclusive)
 */
template<typename T>
FORCE_INLINE void insertionSort(T* a, int low, int high) {
    // Phase 6: Cache-friendly insertion sort with prefetching
    for (int i, k = low; ++k < high; ) {
        T ai = a[i = k];

        // Prefetch next elements to improve cache performance
        // This is crucial for the "memory wall" - CPU-memory speed gap
        if (LIKELY(k + 1 < high)) {
            PREFETCH_READ(&a[k + 1]);
        }

        // Use branch prediction hints for the common case (already sorted)
        if (UNLIKELY(ai < a[i - 1])) {
            // Element is out of place - shift elements to make room
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

/**
 * @brief Advanced mixed insertion sort with pin and pair insertion strategies
 *
 * This sophisticated insertion sort variant is used for medium-sized arrays
 * (up to MAX_MIXED_INSERTION_SORT_SIZE = 65 elements). It combines multiple
 * optimization strategies to achieve better performance than simple insertion sort:
 *
 * Strategy 1 - Pin Insertion Sort:
 * - Uses a "pin" element to separate small and large elements
 * - Reduces the number of comparisons for elements that are already roughly positioned
 * - Handles small elements first, then processes large elements separately
 *
 * Strategy 2 - Pair Insertion Sort:
 * - Processes elements in pairs for better cache utilization
 * - Reduces the constant factor in the O(n²) complexity
 * - Optimizes for the common case of nearly sorted data
 *
 * The algorithm dynamically switches between strategies based on array size,
 * using simple insertion for tiny arrays and the mixed approach for larger ones.
 *
 * This is a key optimization that significantly improves performance on
 * real-world data where arrays often contain partially sorted sequences.
 *
 * @tparam T Element type (must support comparison and assignment)
 * @param a Pointer to the array to sort
 * @param low Starting index (inclusive)
 * @param high Ending index (exclusive)
 */
template<typename T>
void mixedInsertionSort(T* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);  // Calculate transition point

    if (end == high) {
        // Tiny array: use simple insertion sort
        for (int i; ++low < end; ) {
            T ai = a[i = low];

            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Mixed strategy: pin insertion sort + pair insertion sort

        // Phase 1: Pin insertion sort on the initial part
        T pin = a[end];  // Use pin element to separate small/large values

        for (int i, p = high; ++low < end; ) {
            T ai = a[i = low];

            if (ai < a[i - 1]) { // Small element - needs insertion
                // Insert small element into sorted part
                a[i] = a[i - 1];
                --i;

                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;

            } else if (p > i && ai > pin) { // Large element - move to end
                // Find position for large element
                while (a[--p] > pin);

                // Swap large element to proper position
                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }

                // Insert the swapped element (now small) into sorted part
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }

        // Phase 2: Pair insertion sort on remaining part
        // Process two elements at a time for better cache efficiency
        for (int i; low < high; ++low) {
            T a1 = a[i = low], a2 = a[++low];

            // Insert pair of elements efficiently
            if (a1 > a2) {
                // First element is larger - insert in reverse order
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;

                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;

            } else if (a1 < a[i - 1]) {
                // Both elements need insertion
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;

                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

// =============================================================================
// TYPE-SPECIFIC MIXED INSERTION SORT VARIANTS (matching Java's per-type approach)
// =============================================================================

template<typename T>
void pushDown(T* a, int p, T value, int low, int high) {
    for (int k;;) {
        k = (p << 1) - low + 2; // Index of the right child

        if (k > high) {
            break;
        }
        if (k == high || a[k] < a[k - 1]) {
            --k;
        }
        if (a[k] <= value) {
            break;
        }
        a[p] = a[k];
        p = k;
    }
    a[p] = value;
}

template<typename T>
void heapSort(T* a, int low, int high) {
    for (int k = (low + high) >> 1; k > low; ) {
        --k;
        pushDown(a, k, a[k], low, high);
    }
    while (--high > low) {
        T max = a[low];
        pushDown(a, low, a[high], low, high);
        a[high] = max;
    }
}

} // namespace dual_pivot

#endif // DPQS_LEAF_SORTERS_HPP
