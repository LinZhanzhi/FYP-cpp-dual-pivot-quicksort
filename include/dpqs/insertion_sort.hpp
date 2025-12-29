#ifndef DPQS_INSERTION_SORT_HPP
#define DPQS_INSERTION_SORT_HPP

#include "utils.hpp"

namespace dual_pivot {

/**
 * @brief Cache-friendly insertion sort with prefetching optimizations.
 *
 * This is an optimized implementation of insertion sort that serves as the base case
 * for small arrays in the dual-pivot quicksort algorithm. It includes several
 * performance optimizations based on modern CPU characteristics.
 *
 * Key optimizations:
 * - Memory prefetching to reduce cache misses.
 * - Branch prediction hints to reduce pipeline stalls.
 * - Optimized inner loop for better instruction scheduling.
 *
 * The algorithm threshold is carefully tuned: arrays smaller than MAX_INSERTION_SORT_SIZE
 * (44 elements) benefit from this approach over more complex algorithms.
 *
 * Time complexity: O(n^2) in worst case, O(n) for nearly sorted data.
 * Space complexity: O(1).
 *
 * @tparam T Element type (must support comparison and assignment).
 * @param a Pointer to the array to sort.
 * @param low Starting index (inclusive).
 * @param high Ending index (exclusive).
 */
template<typename T, typename Compare>
DPQS_FORCE_INLINE void insertion_sort(T* a, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp) {
    // Phase 6: Cache-friendly insertion sort with prefetching
    for (std::ptrdiff_t i, k = low; ++k < high; ) {
        T ai = a[i = k];

        // Prefetch next elements to improve cache performance
        // This is crucial for the "memory wall" - CPU-memory speed gap
        if (DPQS_LIKELY(k + 1 < high)) {
            DPQS_PREFETCH_READ(&a[k + 1]);
        }

        // Use branch prediction hints for the common case (already sorted)
        if (DPQS_UNLIKELY(comp(ai, a[i - 1]))) {
            // Element is out of place - shift elements to make room
            while (--i >= low && comp(ai, a[i])) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

/**
 * @brief Advanced mixed insertion sort with pin and pair insertion strategies.
 *
 * This sophisticated insertion sort variant is used for medium-sized arrays
 * (up to MAX_MIXED_INSERTION_SORT_SIZE = 65 elements). It combines multiple
 * optimization strategies to achieve better performance than simple insertion sort:
 *
 * Strategy 1 - Pin Insertion Sort:
 * - Uses a "pin" element to separate small and large elements.
 * - Reduces the number of comparisons for elements that are already roughly positioned.
 * - Handles small elements first, then processes large elements separately.
 *
 * Strategy 2 - Pair Insertion Sort:
 * - Processes elements in pairs for better cache utilization.
 * - Reduces the constant factor in the O(n^2) complexity.
 * - Optimizes for the common case of nearly sorted data.
 *
 * The algorithm dynamically switches between strategies based on array size,
 * using simple insertion for tiny arrays and the mixed approach for larger ones.
 *
 * @tparam T Element type (must support comparison and assignment).
 * @param a Pointer to the array to sort.
 * @param low Starting index (inclusive).
 * @param high Ending index (exclusive).
 */
template<typename T, typename Compare>
void mixed_insertion_sort(T* a, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp) {
    std::ptrdiff_t start = low;
    std::ptrdiff_t size = high - low;
    std::ptrdiff_t end = high - 3 * ((size >> 5) << 3);  // Calculate transition point

    if (end == high) {
        // Tiny array: use simple insertion sort
        for (std::ptrdiff_t i; ++low < end; ) {
            T ai = a[i = low];

            while (--i >= start && comp(ai, a[i])) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Mixed strategy: pin insertion sort + pair insertion sort

        // Phase 1: Pin insertion sort on the initial part
        T pin = a[end];  // Use pin element to separate small/large values

        for (std::ptrdiff_t i, p = high; ++low < end; ) {
            T ai = a[i = low];

            if (comp(ai, a[i - 1])) { // Small element - needs insertion
                // Insert small element into sorted part
                a[i] = a[i - 1];
                --i;

                while (--i >= start && comp(ai, a[i])) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;

            } else if (p > i && comp(pin, ai)) { // Large element - move to end
                // Find position for large element
                while (comp(pin, a[--p]));

                // Swap large element to proper position
                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }

                // Insert the swapped element (now small) into sorted part
                while (--i >= start && comp(ai, a[i])) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }

        // Phase 2: Pair insertion sort on remaining part
        // Process two elements at a time for better cache efficiency
        for (std::ptrdiff_t i; low < high; ++low) {
            T a1 = a[i = low], a2 = a[++low];

            // Insert pair of elements efficiently
            if (comp(a2, a1)) {
                // First element is larger - insert in reverse order
                while (--i >= start && comp(a1, a[i])) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;

                while (--i >= start && comp(a2, a[i])) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;

            } else if (comp(a1, a[i - 1])) {
                // Both elements need insertion
                while (--i >= start && comp(a2, a[i])) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;

                while (--i >= start && comp(a1, a[i])) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

/**
 * @brief Specialized insertion sort for int.
 */
static void insertion_sort_int(int* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    for (std::ptrdiff_t i, k = low; ++k < high; ) {
        int ai = a[i = k];

        if (DPQS_LIKELY(k + 1 < high)) {
            DPQS_PREFETCH_READ(&a[k + 1]);
        }

        if (DPQS_UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

/**
 * @brief Specialized insertion sort for long.
 */
static void insertion_sort_long(long* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    for (std::ptrdiff_t i, k = low; ++k < high; ) {
        long ai = a[i = k];

        if (DPQS_LIKELY(k + 1 < high)) {
            DPQS_PREFETCH_READ(&a[k + 1]);
        }

        if (DPQS_UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

/**
 * @brief Specialized insertion sort for float.
 */
static void insertion_sort_float(float* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    for (std::ptrdiff_t i, k = low; ++k < high; ) {
        float ai = a[i = k];

        if (DPQS_LIKELY(k + 1 < high)) {
            DPQS_PREFETCH_READ(&a[k + 1]);
        }

        if (DPQS_UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

/**
 * @brief Specialized insertion sort for double.
 */
static void insertion_sort_double(double* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    for (std::ptrdiff_t i, k = low; ++k < high; ) {
        double ai = a[i = k];

        if (DPQS_LIKELY(k + 1 < high)) {
            DPQS_PREFETCH_READ(&a[k + 1]);
        }

        if (DPQS_UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

} // namespace dual_pivot

#endif // DPQS_INSERTION_SORT_HPP
