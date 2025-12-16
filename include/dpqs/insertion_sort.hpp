#ifndef DPQS_INSERTION_SORT_HPP
#define DPQS_INSERTION_SORT_HPP

#include "utils.hpp"

namespace dual_pivot {

/**
 * @brief Sorts a range of the array using Insertion Sort.
 *
 * This implementation is optimized for cache performance using prefetching
 * and branch prediction hints.
 *
 * @tparam T The type of elements in the array.
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
template<typename T>
FORCE_INLINE void insertionSort(T* array, int start_index, int end_index) {
    // Phase 6: Cache-friendly insertion sort with prefetching
    // Iterate through the array starting from the second element
    for (int insert_index, current_index = start_index; ++current_index < end_index; ) {
        T value_to_insert = array[insert_index = current_index];

        // Prefetch next elements to improve cache performance
        // This is crucial for the "memory wall" - CPU-memory speed gap
        if (LIKELY(current_index + 1 < end_index)) {
            PREFETCH_READ(&array[current_index + 1]);
        }

        // Use branch prediction hints for the common case (already sorted)
        // If the current element is smaller than the previous one, it needs to be moved
        if (UNLIKELY(value_to_insert < array[insert_index - 1])) {
            // Element is out of place - shift elements to the right to make room
            while (--insert_index >= start_index && value_to_insert < array[insert_index]) {
                array[insert_index + 1] = array[insert_index];
            }
            // Place the element in its correct sorted position
            array[insert_index + 1] = value_to_insert;
        }
    }
}

/**
 * @brief Sorts a range of the array using a Mixed Insertion Sort strategy.
 *
 * This strategy combines "Pin Insertion Sort" and "Pair Insertion Sort" to optimize
 * for different array sizes and element distributions.
 *
 * @tparam T The type of elements in the array.
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
template<typename T>
void mixedInsertionSort(T* array, int start_index, int end_index) {
    const int range_start = start_index;
    int size = end_index - start_index;
    // Calculate transition point for the mixed strategy
    // if size < 32, transition_point == end_index
    // if size >= 32, transition_point = end_index - 3 * ((size / 32) * 8) = end_index - 0.75 * size (approx)
    // so [start_index, transition_point), first quarter of array,  uses pin insertion sort
    // and [transition_point, end_index), last 3 quarters of array, uses pair insertion sort
    int transition_point = end_index - 3 * ((size >> 5) << 3);

    // This mean that  - 3 * ((size >> 5) << 3) is zero.
    // so (size >> 5) is zero
    // so size < 32
    if (transition_point == end_index) {
        // Tiny array: use simple insertion sort
        for (int insert_index; ++start_index < transition_point; ) {
            T value_to_insert = array[insert_index = start_index];

            if (value_to_insert < array[insert_index - 1]) {
                while (--insert_index >= range_start && value_to_insert < array[insert_index]) {
                    array[insert_index + 1] = array[insert_index];
                }
                array[insert_index + 1] = value_to_insert;
            }
        }
    } else {
        // Mixed strategy: pin insertion sort + pair insertion sort

        // Phase 1: Pin insertion sort on the initial part
        T pin_element = array[transition_point];  // Use pin element to separate small/large values

        for (int insert_index, large_element_index = end_index; ++start_index < transition_point; ) {
            T value_to_insert = array[insert_index = start_index];

            if (value_to_insert < array[insert_index - 1]) { // Small element - needs insertion
                // Insert small element into sorted part
                array[insert_index] = array[insert_index - 1];
                --insert_index;

                while (insert_index > range_start && value_to_insert < array[insert_index - 1]) {
                    array[insert_index] = array[insert_index - 1];
                    --insert_index;
                }
                array[insert_index] = value_to_insert;

            } else if (large_element_index > insert_index && value_to_insert > pin_element) { // Large element - move to end
                // Find position for large element
                while (array[--large_element_index] > pin_element);

                // Swap large element to proper position
                if (large_element_index > insert_index) {
                    value_to_insert = array[large_element_index];
                    array[large_element_index] = array[insert_index];
                }

                // Insert the swapped element (now small) into sorted part
                while (insert_index > range_start && value_to_insert < array[insert_index - 1]) {
                    array[insert_index] = array[insert_index - 1];
                    --insert_index;
                }
                array[insert_index] = value_to_insert;
            }
        }

        // Phase 2: Pair insertion sort on remaining part
        // Process two elements at a time for better cache efficiency
        for (int insert_index; start_index < end_index; ++start_index) {
            T first_element = array[insert_index = start_index], second_element = array[++start_index];

            // Insert pair of elements efficiently
            if (first_element > second_element) {
                // First element is larger - insert in reverse order
                while (insert_index > range_start && first_element < array[insert_index - 1]) {
                    array[insert_index + 2] = array[insert_index - 1];
                    --insert_index;
                }
                array[insert_index + 2] = first_element;

                while (insert_index > range_start && second_element < array[insert_index - 1]) {
                    array[insert_index + 1] = array[insert_index - 1];
                    --insert_index;
                }
                array[insert_index + 1] = second_element;

            } else if (first_element < array[insert_index - 1]) {
                // Both elements need insertion
                while (insert_index > range_start && second_element < array[insert_index - 1]) {
                    array[insert_index + 2] = array[insert_index - 1];
                    --insert_index;
                }
                array[insert_index + 2] = second_element;

                while (insert_index > range_start && first_element < array[insert_index - 1]) {
                    array[insert_index + 1] = array[insert_index - 1];
                    --insert_index;
                }
                array[insert_index + 1] = first_element;
            }
        }
    }
}

/**
 * @brief Specialized Insertion Sort for integers.
 *
 * Optimized implementation for int type with prefetching and branch prediction.
 *
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
static void insertionSort_int(int* array, int start_index, int end_index) {
    for (int insert_index, current_index = start_index; ++current_index < end_index; ) {
        int value_to_insert = array[insert_index = current_index];

        if (LIKELY(current_index + 1 < end_index)) {
            PREFETCH_READ(&array[current_index + 1]);
        }

        if (UNLIKELY(value_to_insert < array[insert_index - 1])) {
            while (--insert_index >= start_index && value_to_insert < array[insert_index]) {
                array[insert_index + 1] = array[insert_index];
            }
            array[insert_index + 1] = value_to_insert;
        }
    }
}

/**
 * @brief Specialized Insertion Sort for long integers.
 *
 * Optimized implementation for long type with prefetching and branch prediction.
 *
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
static void insertionSort_long(long* array, int start_index, int end_index) {
    for (int insert_index, current_index = start_index; ++current_index < end_index; ) {
        long value_to_insert = array[insert_index = current_index];

        if (LIKELY(current_index + 1 < end_index)) {
            PREFETCH_READ(&array[current_index + 1]);
        }

        if (UNLIKELY(value_to_insert < array[insert_index - 1])) {
            while (--insert_index >= start_index && value_to_insert < array[insert_index]) {
                array[insert_index + 1] = array[insert_index];
            }
            array[insert_index + 1] = value_to_insert;
        }
    }
}

/**
 * @brief Specialized Insertion Sort for floats.
 *
 * Optimized implementation for float type with prefetching and branch prediction.
 *
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
static void insertionSort_float(float* array, int start_index, int end_index) {
    for (int insert_index, current_index = start_index; ++current_index < end_index; ) {
        float value_to_insert = array[insert_index = current_index];

        if (LIKELY(current_index + 1 < end_index)) {
            PREFETCH_READ(&array[current_index + 1]);
        }

        if (UNLIKELY(value_to_insert < array[insert_index - 1])) {
            while (--insert_index >= start_index && value_to_insert < array[insert_index]) {
                array[insert_index + 1] = array[insert_index];
            }
            array[insert_index + 1] = value_to_insert;
        }
    }
}

/**
 * @brief Specialized Insertion Sort for doubles.
 *
 * Optimized implementation for double type with prefetching and branch prediction.
 *
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
static void insertionSort_double(double* array, int start_index, int end_index) {
    for (int insert_index, current_index = start_index; ++current_index < end_index; ) {
        double value_to_insert = array[insert_index = current_index];

        if (LIKELY(current_index + 1 < end_index)) {
            PREFETCH_READ(&array[current_index + 1]);
        }

        if (UNLIKELY(value_to_insert < array[insert_index - 1])) {
            while (--insert_index >= start_index && value_to_insert < array[insert_index]) {
                array[insert_index + 1] = array[insert_index];
            }
            array[insert_index + 1] = value_to_insert;
        }
    }
}

/**
 * @brief Sorts a range of the array using Counting Sort.
 *
 * This is optimized for char arrays.
 *
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
static void countingSort(char* array, int start_index, int end_index) {
    static constexpr int NUM_CHAR_VALUES = 1 << 16; // Full Unicode range
    std::vector<int> count(NUM_CHAR_VALUES, 0);

    // Direct unsigned access for characters (matching Java's approach)
    for (int i = end_index; i > start_index; ) {
        ++count[static_cast<unsigned char>(array[--i])];
    }

    // Optimized placement for character ranges
    int current_write_index = start_index;
    for (int i = 0; i < NUM_CHAR_VALUES; i++) {
        if (count[i] > 0) {
            int element_count = count[i];
            char value = static_cast<char>(i);
            while (element_count-- > 0) {
                array[current_write_index++] = value;
            }
        }
    }
}

} // namespace dual_pivot

#endif // DPQS_INSERTION_SORT_HPP
