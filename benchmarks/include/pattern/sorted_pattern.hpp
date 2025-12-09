/**
 * @file sorted_pattern.hpp
 * @brief Pre-sorted data pattern generator for best-case sorting algorithm analysis
 * 
 * This file implements generation of already-sorted data patterns that test
 * the best-case performance characteristics of sorting algorithms. Pre-sorted
 * data is crucial for evaluating algorithm efficiency on structured input and
 * validating adaptive behavior in sorting implementations.
 * 
 * Key Testing Scenarios:
 * - Best-case performance validation
 * - Adaptive algorithm behavior testing
 * - Early termination optimization verification
 * - Cache efficiency on sequential access patterns
 * 
 * Well-designed sorting algorithms should recognize pre-sorted data and
 * perform minimal work, often achieving O(n) performance instead of O(n log n).
 * This pattern helps validate such optimizations.
 * 
 * @author Dual-Pivot Quicksort Research Project
 * @version 1.0
 * @date 2024
 */

#pragma once
#include <vector>
#include <algorithm>

/**
 * @brief Generate pre-sorted ascending data pattern for best-case analysis
 * 
 * Creates an array of values arranged in strictly ascending order, representing
 * the ideal input for most sorting algorithms. This pattern is essential for
 * testing best-case performance and validating adaptive optimizations.
 * 
 * Type-Specific Generation:
 * - **Integral types**: Sequential values starting from 1 (1, 2, 3, ...)
 * - **Floating-point types**: Sequential values starting from 1.0 (1.0, 2.0, 3.0, ...)
 * - **Boolean type**: Alternating pattern (false, true, false, true, ...)
 * - **Other types**: Modular sequence using values 0-255 with wrapping
 * 
 * The generated patterns provide perfect ascending order while maintaining
 * reasonable value distributions for each data type. This ensures that
 * performance measurements reflect algorithmic behavior rather than
 * type-specific artifacts.
 * 
 * Performance Expectations:
 * - Adaptive algorithms should achieve near-linear performance
 * - Non-adaptive algorithms should still benefit from good cache locality
 * - Memory access patterns should be highly predictable and efficient
 * 
 * @tparam T Element type for the generated array
 * @param length Number of elements to generate
 * @return std::vector<T> containing values in strictly ascending order
 * 
 * @note This pattern represents the theoretical best case for comparison-based
 *       sorting algorithms and helps establish performance baselines.
 * 
 * Example:
 * @code
 * auto sorted_ints = generate_sorted_pattern<int>(1000);
 * // Result: [1, 2, 3, 4, ..., 1000]
 * 
 * auto sorted_doubles = generate_sorted_pattern<double>(500);
 * // Result: [1.0, 2.0, 3.0, 4.0, ..., 500.0]
 * @endcode
 */
template<typename T>
std::vector<T> generate_sorted_pattern(size_t length) {
    std::vector<T> arr(length);
    
    if constexpr (std::is_integral_v<T>) {
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<T>(i + 1);
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<T>(i + 1.0);
        }
    } else if constexpr (std::is_same_v<T, bool>) {
        for (size_t i = 0; i < length; ++i) {
            arr[i] = (i % 2 == 0) ? false : true;
        }
    } else {
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<T>(i % 256);
        }
    }
    
    return arr;
}