#ifndef DPQS_COUNTING_SORT_HPP
#define DPQS_COUNTING_SORT_HPP

#include "dpqs/utils.hpp"
#include "dpqs/core_sort.hpp"
#include <vector>
#include <type_traits>

namespace dual_pivot {

/**
 * @brief Sorts a range of the array using Counting Sort for byte-sized elements.
 *
 * This implementation is optimized for 1-byte types (char, unsigned char, int8_t, uint8_t).
 * It uses a fixed-size frequency array (256 entries) to count occurrences of each value.
 *
 * @tparam T The type of elements in the array (must be 1 byte).
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
template<typename T>
#if __cplusplus >= 202002L
requires (Integral<T> && sizeof(T) == 1)
void countingSort(T* array, int start_index, int end_index) {
#else
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == 1, void>::type
countingSort(T* array, int start_index, int end_index) {
#endif
    // Calculate total number of possible values (e.g., 2^8 = 256 for 1-byte types)
    static constexpr int NUM_VALUES = 1 << (8 * sizeof(T));

    // Calculate offset to map signed values to positive array indices.
    // Example for signed char (1 byte): Range is [-128, 127].
    // We need to map -128 to index 0.
    // OFFSET = 1 << (8*1 - 1) = 1 << 7 = 128.
    // Mapping: -128 + 128 = 0, 0 + 128 = 128, 127 + 128 = 255.
    static constexpr int OFFSET = std::is_signed<T>::value ? (1 << (8 * sizeof(T) - 1)) : 0;

    std::vector<int> frequency_count(NUM_VALUES, 0);

    // Calculate frequencies
    for (int i = end_index; i > start_index; ) {
        // Use integer cast and offset to map values to [0, 255] range
        // For signed char: -128 + 128 = 0, 127 + 128 = 255
        int val = static_cast<int>(array[--i]);
        int idx = val + OFFSET;
        frequency_count[idx]++;
    }

    int size = end_index - start_index;
    // Optimization: Choose iteration direction based on array density.
    // If size > 128 (half of 256), the frequency array is likely "dense" (most buckets have values).
    // If size <= 128, the frequency array is "sparse" (many buckets are 0).
    if (size > NUM_VALUES / 2) {
        // Case 1: Dense Array.
        // We iterate BACKWARDS (255 -> 0) and fill from the END of the array.
        // Why?
        // 1. Loop overhead: Decrementing to 0 (`--i >= 0`) is often faster in assembly (ZF flag).
        // 2. No explicit `if (count > 0)` check: Since it's dense, we assume most counts > 0.
        //    The `while` loop handles the 0 case naturally, avoiding branch misprediction penalties.
        int write_index = end_index;
        for (int i = NUM_VALUES; --i >= 0; ) {
            T value = static_cast<T>(i - OFFSET);
            int element_count = frequency_count[i];
            while (element_count-- > 0) {
                array[--write_index] = value;
            }
        }
    } else {
        // Case 2: Sparse Array.
        // We iterate FORWARDS (0 -> 255) and fill from the START.
        // Why?
        // 1. Explicit `if (count > 0)` check: Since it's sparse, many buckets are 0.
        //    The branch predictor will learn to skip the body, making it very fast.
        int write_index = start_index;
        for (int i = 0; i < NUM_VALUES; i++) {
            if (frequency_count[i] > 0) {
                T value = static_cast<T>(i - OFFSET);
                int element_count = frequency_count[i];
                while (element_count-- > 0) {
                    array[write_index++] = value;
                }
            }
        }
    }
}

/**
 * @brief Sorts a range of the array using Counting Sort for 2-byte elements.
 *
 * This implementation is optimized for 2-byte types (short, unsigned short, char16_t).
 * It handles larger ranges and signed/unsigned distinctions.
 *
 * @tparam T The type of elements in the array (must be 2 bytes).
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
template<typename T>
#if __cplusplus >= 202002L
requires (Integral<T> && sizeof(T) == 2)
void countingSort(T* array, int start_index, int end_index) {
#else
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == 2, void>::type
countingSort(T* array, int start_index, int end_index) {
#endif
    // Total number of unique values for a 2-byte type (2^16 = 65536).
    static constexpr int NUM_VALUES = 1 << 16;

    // Calculate offset to map signed values to positive array indices.
    // For signed short (2 bytes): Range is [-32768, 32767].
    // OFFSET = 1 << 15 = 32768.
    static constexpr int OFFSET = std::is_signed<T>::value ? (1 << 15) : 0;

    std::vector<int> frequency_count(NUM_VALUES, 0);

    // Calculate frequencies
    for (int i = end_index; i > start_index; ) {
        int val = static_cast<int>(array[--i]);
        int idx = val + OFFSET;
        frequency_count[idx]++;
    }

    int size = end_index - start_index;
    // Optimization: Choose iteration direction based on array density.
    if (size > NUM_VALUES / 2) {
        // Dense: iterate backwards and fill from end
        int write_index = end_index;
        for (int i = NUM_VALUES; --i >= 0; ) {
            T value = static_cast<T>(i - OFFSET);
            int element_count = frequency_count[i];
            while (element_count-- > 0) {
                array[--write_index] = value;
            }
        }
    } else {
        // Sparse: iterate forwards and fill from start
        int write_index = start_index;
        for (int i = 0; i < NUM_VALUES; i++) {
            if (frequency_count[i] > 0) {
                T value = static_cast<T>(i - OFFSET);
                int element_count = frequency_count[i];
                while (element_count-- > 0) {
                    array[write_index++] = value;
                }
            }
        }
    }
}

/**
 * @brief Fallback sort for types that are not 1 or 2 bytes (e.g., custom types).
 *
 * Delegates to the main Dual-Pivot Quicksort implementation.
 *
 * @tparam T The type of elements.
 * @param array The array to sort.
 * @param start_index The inclusive start index.
 * @param end_index The exclusive end index.
 */
template<typename T>
#if __cplusplus >= 202002L
requires (!Integral<T> && !FloatingPoint<T>)
void sort_specialized(T* array, int start_index, int end_index) {
#else
typename std::enable_if<!std::is_integral<T>::value && !std::is_floating_point<T>::value, void>::type
sort_specialized(T* array, int start_index, int end_index) {
#endif
    sort(array, 0, start_index, end_index);
}

/**
 * @brief Fallback sort for larger integral types (e.g., int, long).
 *
 * Delegates to the main Dual-Pivot Quicksort implementation.
 *
 * @tparam T The type of elements.
 * @param array The array to sort.
 * @param start_index The inclusive start index.
 * @param end_index The exclusive end index.
 */
template<typename T>
#if __cplusplus >= 202002L
requires (Integral<T> && sizeof(T) > 2)
void sort_specialized(T* array, int start_index, int end_index) {
#else
typename std::enable_if<std::is_integral<T>::value && (sizeof(T) > 2), void>::type
sort_specialized(T* array, int start_index, int end_index) {
#endif
    sort(array, 0, start_index, end_index);
}

} // namespace dual_pivot

#endif // DPQS_COUNTING_SORT_HPP
