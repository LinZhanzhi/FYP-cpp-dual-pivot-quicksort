#ifndef DPQS_FLOAT_SORT_HPP
#define DPQS_FLOAT_SORT_HPP

#include "dpqs/utils.hpp"
#include "dpqs/core_sort.hpp"
#include "dpqs/sequential_sorters.hpp"
#include <cmath>
#include <cstring>
#include <type_traits>

namespace dual_pivot {

/**
 * @brief Checks if a floating-point value is NaN (Not a Number).
 *
 * @tparam T The floating-point type.
 * @param value The value to check.
 * @return true if the value is NaN, false otherwise.
 */
template<typename T>
bool is_nan(T value) { return value != value; }

/**
 * @brief Checks if a floating-point value is negative zero (-0.0).
 *
 * @tparam T The floating-point type.
 * @param value The value to check.
 * @return true if the value is -0.0, false otherwise.
 */
template<typename T>
bool is_negative_zero(T value) {
    if (value != 0) return false;
    return std::signbit(value);
}

/**
 * @brief Checks if a floating-point value is positive zero (+0.0).
 *
 * @tparam T The floating-point type.
 * @param value The value to check.
 * @return true if the value is +0.0, false otherwise.
 */
template<typename T>
bool is_positive_zero(T value) {
    return value == 0 && !is_negative_zero(value);
}

/**
 * @brief Reinterprets the bits of an unsigned int as a float.
 *
 * Used to construct specific float values (like -0.0) directly from bit patterns.
 *
 * @param bits The bit pattern.
 * @return The float value corresponding to the bit pattern.
 */
inline float bits_to_float(unsigned int bits) {
    float f;
    std::memcpy(&f, &bits, sizeof(float));
    return f;
}

/**
 * @brief Reinterprets the bits of an unsigned long long as a double.
 *
 * Used to construct specific double values (like -0.0) directly from bit patterns.
 *
 * @param bits The bit pattern.
 * @return The double value corresponding to the bit pattern.
 */
inline double bits_to_double(unsigned long long bits) {
    double d;
    std::memcpy(&d, &bits, sizeof(double));
    return d;
}

/**
 * @brief Finds the insertion point for zeros in a sorted array containing negative numbers.
 *
 * Performs a binary search to find the first index where a non-negative value could exist.
 * This is used to place restored negative zeros (-0.0) correctly after the negative numbers.
 *
 * @tparam T The floating-point type.
 * @param array The sorted array.
 * @param start_index The inclusive start index of the range.
 * @param end_index The inclusive end index of the range.
 * @return The index where zeros should begin.
 */
template<typename T>
int find_zero_insertion_point(T* array, int start_index, int end_index) {
    int search_left = start_index;
    int search_right = end_index;
    while (search_left <= search_right) {
        int search_mid = (search_left + search_right) / 2;
        if (array[search_mid] < 0) {
            search_left = search_mid + 1;
        } else {
            search_right = search_mid - 1;
        }
    }
    return search_left;
}

/**
 * @brief Pre-processes and sorts a range of floating-point numbers.
 *
 * This function handles special floating-point cases:
 * 1. NaNs are moved to the end of the array and excluded from the sort range.
 * 2. Negative zeros (-0.0) are converted to positive zeros (+0.0) to ensure they
 *    are treated as equal during sorting.
 * 3. The array is sorted using the sequential sort implementation.
 * 4. Negative zeros are restored to their correct positions (immediately before positive zeros).
 *
 * @tparam T The floating-point type (float or double).
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
process_and_sort_floats(T* array, int start_index, int end_index) {
    int negative_zero_count = 0;
    int effective_end_index = end_index;

    // Phase 1: Pre-processing
    // Move NaNs to the end and normalize -0.0 to +0.0
    for (int k = effective_end_index - 1; k >= start_index; k--) {
        T current_value = array[k];

        if (is_nan(current_value)) {
            // Move NaN to the end of the active range
            array[k] = array[--effective_end_index];
            array[effective_end_index] = current_value;
        } else if (is_negative_zero(current_value)) {
            // Count -0.0 and convert to +0.0 for sorting
            negative_zero_count++;
            array[k] = T(0);
        }
    }

    // Phase 2: Sorting
    // Sort the range excluding NaNs
    if (effective_end_index > start_index) {
        if constexpr (std::is_same_v<T, float>) {
            sort_float_sequential(nullptr, array, 0, start_index, effective_end_index);
        } else if constexpr (std::is_same_v<T, double>) {
            sort_double_sequential(nullptr, array, 0, start_index, effective_end_index);
        }
    }

    // Phase 3: Post-processing
    // Restore -0.0 values. They should be placed before +0.0.
    if (negative_zero_count > 0) {
        // Find where the zeros begin (after the last negative number)
        int insertion_index = find_zero_insertion_point(array, start_index, effective_end_index - 1);

        // Replace the first 'negative_zero_count' zeros with -0.0
        for (int i = 0; i < negative_zero_count && insertion_index < effective_end_index; i++, insertion_index++) {
            if constexpr (std::is_same_v<T, float>) {
                if (is_positive_zero(array[insertion_index])) {
                    array[insertion_index] = bits_to_float(0x80000000U);
                }
            } else if constexpr (std::is_same_v<T, double>) {
                if (is_positive_zero(array[insertion_index])) {
                    array[insertion_index] = bits_to_double(0x8000000000000000ULL);
                }
            }
        }
    }
}

/**
 * @brief Entry point for sorting floating-point arrays.
 *
 * Delegates to the specialized floating-point processor.
 *
 * @tparam T The floating-point type.
 * @param array The array to sort.
 * @param start_index The inclusive start index.
 * @param end_index The exclusive end index.
 */
template<typename T>
#if __cplusplus >= 202002L
requires FloatingPoint<T>
void sort_specialized(T* array, int start_index, int end_index) {
#else
typename std::enable_if<std::is_floating_point<T>::value, void>::type
sort_specialized(T* array, int start_index, int end_index) {
#endif
    process_and_sort_floats(array, start_index, end_index);
}

} // namespace dual_pivot

#endif // DPQS_FLOAT_SORT_HPP
