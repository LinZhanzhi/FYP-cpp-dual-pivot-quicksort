/**
 * @file data_generator.hpp
 * @brief Comprehensive test data generation framework for sorting algorithm benchmarks
 *
 * This file provides a sophisticated data generation framework specifically designed
 * for testing and benchmarking sorting algorithms. It implements various data patterns
 * that are crucial for understanding algorithmic performance characteristics.
 *
 * The framework addresses key testing scenarios for sorting algorithms:
 * - Random data for average-case performance analysis
 * - Pathological cases that can cause quadratic behavior in poor implementations
 * - Real-world patterns that occur in practical applications
 * - Edge cases that test algorithm robustness and correctness
 *
 * Data Pattern Categories:
 * 1. **Random Patterns**: For average-case performance analysis
 * 2. **Ordered Patterns**: Nearly sorted, reverse sorted, organ pipe
 * 3. **Duplicate Patterns**: Various levels of duplicate elements
 * 4. **Pathological Patterns**: Sawtooth and other challenging cases
 *
 * The framework is particularly important for validating the performance claims
 * of dual-pivot quicksort, which promises improvements over traditional quicksort
 * especially for arrays with many duplicate elements.
 *
 * Type Safety Features:
 * - Template-based generation for all standard numeric types
 * - Automatic range handling for different data types
 * - Proper handling of signed/unsigned integer boundaries
 * - Floating-point specific pattern generation
 *
 * @author Dual-Pivot Quicksort Research Project
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <limits>

/**
 * @brief Namespace containing comprehensive benchmark data generation utilities
 *
 * This namespace provides all the tools necessary for generating test data
 * patterns that thoroughly exercise sorting algorithms. The patterns are chosen
 * based on algorithmic analysis and real-world data characteristics.
 */
namespace benchmark_data {

/**
 * @brief Enumeration of test data patterns for comprehensive sorting algorithm analysis
 *
 * This enumeration defines the standard test patterns used in sorting algorithm
 * benchmarks. Each pattern is designed to expose specific performance characteristics
 * and potential weaknesses in sorting implementations.
 *
 * Pattern Categories and Their Purpose:
 *
 * **RANDOM**: Tests average-case performance with no inherent structure.
 * Essential for comparing general-purpose sorting performance.
 *
 * **NEARLY_SORTED**: Tests performance on partially ordered data, which is
 * common in real-world applications. Good algorithms should perform very well here.
 *
 * **REVERSE_SORTED**: Tests worst-case scenarios for naive quicksort implementations.
 * Dual-pivot quicksort should handle this efficiently.
 *
 * **MANY_DUPLICATES_***: Tests handling of duplicate elements, where dual-pivot
 * quicksort claims significant advantages over traditional quicksort.
 * - 10% unique: Heavy duplication (stress test)
 * - 50% unique: Moderate duplication (realistic scenario)
 * - 90% unique: Light duplication (edge case)
 *
 * **ORGAN_PIPE**: Ascending then descending pattern that can challenge
 * partitioning algorithms with poor pivot selection.
 *
 * **SAWTOOTH**: Repeating patterns that can expose inefficiencies in
 * algorithms that don't handle regular structures well.
 */
enum class DataPattern {
    RANDOM,                ///< Uniformly random distribution
    NEARLY_SORTED,         ///< Mostly sorted with ~10% random swaps
    REVERSE_SORTED,        ///< Strictly decreasing order
    MANY_DUPLICATES_10,    ///< Only 10% unique values (heavy duplication)
    MANY_DUPLICATES_50,    ///< Only 50% unique values (moderate duplication)
    MANY_DUPLICATES_90,    ///< Only 90% unique values (light duplication)
    ORGAN_PIPE,            ///< Ascending then descending (∩ shape)
    SAWTOOTH               ///< Repeating ascending patterns
};

/**
 * @brief Template-based type traits for intelligent data generation
 *
 * This traits class provides compile-time information about different data types
 * to enable appropriate value generation for each type. It handles the complexities
 * of different numeric types while ensuring generated data is meaningful for
 * sorting performance analysis.
 *
 * Key Features:
 * - Automatic range determination based on type characteristics
 * - Special handling for character types (full range utilization)
 * - Size-appropriate ranges for smaller integer types
 * - Floating-point specific considerations
 *
 * The traits system ensures that generated data patterns are meaningful across
 * all supported types while avoiding integer overflow or other type-specific issues.
 *
 * @tparam T The data type for which traits are being determined
 */
template<typename T>
struct TypeTraits {
    static constexpr bool is_integral = std::is_integral_v<T>;        ///< True for integer types
    static constexpr bool is_floating_point = std::is_floating_point_v<T>;  ///< True for float/double
    static constexpr bool is_signed = std::is_signed_v<T>;            ///< True for signed types

    /**
     * @brief Determine minimum value for data generation
     *
     * Provides appropriate minimum values for different types:
     * - Character types: Use full type range for comprehensive testing
     * - Signed integers: Start from 1 to avoid special zero cases
     * - Unsigned integers: Start from 0 (natural minimum)
     * - Floating-point: Start from 0.0 for simplicity
     *
     * @return Minimum value appropriate for the type
     */
    static constexpr T min_value() {
        if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
            return std::numeric_limits<T>::min();
        }
        else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) return 1;
        else if constexpr (std::is_integral_v<T> && !std::is_signed_v<T>) return 0;
        else if constexpr (std::is_floating_point_v<T>) return T(0.0);
        else return T(1);
    }

    /**
     * @brief Determine maximum value for data generation based on array size
     *
     * Provides appropriate maximum values that scale with array size for
     * meaningful test data generation:
     * - Character types: Use full type range regardless of array size
     * - Short types: Respect type limits while using available range
     * - Other types: Scale maximum with array size for good distribution
     *
     * @param size Size of the array being generated
     * @return Maximum value appropriate for the type and array size
     */
    static T max_value(size_t size) {
        if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
            return std::numeric_limits<T>::max();
        }
        else if constexpr (std::is_same_v<T, short>) return static_cast<T>(std::min<size_t>(size, 32767));
        else if constexpr (std::is_same_v<T, unsigned short>) return static_cast<T>(std::min<size_t>(size, 65535));
        else if constexpr (std::is_integral_v<T>) return static_cast<T>(size);
        else if constexpr (std::is_floating_point_v<T>) return static_cast<T>(size);
        else return static_cast<T>(size);
    }
};

/**
 * @brief Generate random values with type-appropriate distribution
 *
 * This template function generates random values within specified ranges while
 * handling the complexities of different numeric types. It ensures that the
 * generated values are appropriate for the target type and follow the expected
 * distribution characteristics.
 *
 * Type-Specific Handling:
 * - **Character types**: Use integer distribution with casting to avoid
 *   distribution issues with narrow character types
 * - **Integer types**: Use appropriate integer distribution for the type
 * - **Floating-point types**: Use real distribution for continuous values
 * - **Other types**: Fallback to minimum value (for custom types)
 *
 * The function ensures that all generated values are within the valid range
 * for the target type and follow uniform distribution principles.
 *
 * @tparam T Target data type for value generation
 * @param gen Random number generator (mutable reference)
 * @param min_val Minimum value for the distribution (inclusive)
 * @param max_val Maximum value for the distribution (inclusive)
 * @return Random value of type T within the specified range
 *
 * @note Character types use integer distribution internally to avoid
 *       issues with narrow type distributions in some standard library implementations.
 */
template<typename T>
T generate_random_value(std::mt19937& gen, T min_val, T max_val) {
    if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
        std::uniform_int_distribution<int> dis(static_cast<int>(min_val), static_cast<int>(max_val));
        return static_cast<T>(dis(gen));
    } else if constexpr (std::is_integral_v<T>) {
        std::uniform_int_distribution<T> dis(min_val, max_val);
        return dis(gen);
    } else if constexpr (std::is_floating_point_v<T>) {
        std::uniform_real_distribution<T> dis(min_val, max_val);
        return dis(gen);
    } else {
        return static_cast<T>(min_val);
    }
}

/**
 * @brief Comprehensive test data generator with configurable patterns
 *
 * This is the main data generation function that creates test arrays according
 * to specified patterns. It's designed to provide comprehensive test coverage
 * for sorting algorithms by generating data that exercises different algorithmic
 * paths and performance characteristics.
 *
 * Pattern Implementation Details:
 *
 * **RANDOM**: Uses cryptographically secure seeding with MT19937 for
 * reproducible yet high-quality random data that doesn't favor any particular
 * algorithm.
 *
 * **NEARLY_SORTED**: Creates sorted data then performs controlled random swaps
 * (10% of array size) to simulate real-world partially ordered data.
 *
 * **REVERSE_SORTED**: Generates strictly decreasing sequences that challenge
 * algorithms with poor pivot selection strategies.
 *
 * **MANY_DUPLICATES_***: Controls duplication levels to test how algorithms
 * handle equal elements - a key strength of dual-pivot quicksort.
 *
 * **ORGAN_PIPE**: Creates ∩-shaped distribution (ascending then descending)
 * that can challenge partitioning strategies.
 *
 * **SAWTOOTH**: Generates repeating ascending patterns that can expose
 * inefficiencies in algorithms that don't detect structure.
 *
 * Type-Specific Optimizations:
 * - Character types use modular arithmetic to handle range limitations
 * - Integer types scale appropriately with array size
 * - Floating-point types maintain precision and avoid special values
 *
 * @tparam T Element type for the generated array (default: int)
 * @param size Number of elements to generate
 * @param pattern Data pattern to generate (see DataPattern enum)
 * @param seed Random seed for reproducible results (default: 42)
 * @return std::vector<T> containing the generated test data
 *
 * @note The function is designed to be thread-safe when called with different
 *       seeds, enabling parallel test data generation.
 *
 * Example Usage:
 * @code
 * // Generate random integers for general testing
 * auto random_data = generate_data<int>(1000, DataPattern::RANDOM);
 *
 * // Generate challenging duplicate pattern for specialized testing
 * auto dup_data = generate_data<int>(1000, DataPattern::MANY_DUPLICATES_10);
 *
 * // Generate floating-point test data
 * auto float_data = generate_data<double>(500, DataPattern::NEARLY_SORTED);
 * @endcode
 */
template<typename T = int>
std::vector<T> generate_data(size_t size, DataPattern pattern, unsigned seed = 42) {
    std::vector<T> data;
    data.reserve(size);

    std::mt19937 gen(seed);

    const T min_val = TypeTraits<T>::min_value();
    const T max_val = TypeTraits<T>::max_value(size);

    switch (pattern) {
        case DataPattern::RANDOM: {
            for (size_t i = 0; i < size; ++i) {
                data.push_back(generate_random_value(gen, min_val, max_val));
            }
            break;
        }

        case DataPattern::NEARLY_SORTED: {
            // Generate sorted array, then randomly swap 10% of elements
            if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
                // For char types, use modulo arithmetic to cycle through valid range
                const T range = std::numeric_limits<T>::max() - std::numeric_limits<T>::min() + 1;
                for (size_t i = 0; i < size; ++i) {
                    data.push_back(static_cast<T>((i % range) + std::numeric_limits<T>::min()));
                }
            } else {
                for (size_t i = 0; i < size; ++i) {
                    data.push_back(static_cast<T>(i));
                }
            }

            std::uniform_int_distribution<size_t> dis(0, size - 1);
            size_t swaps = size / 10;  // 10% swaps

            for (size_t i = 0; i < swaps; ++i) {
                size_t pos1 = dis(gen);
                size_t pos2 = dis(gen);
                std::swap(data[pos1], data[pos2]);
            }
            break;
        }

        case DataPattern::REVERSE_SORTED: {
            if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
                // For char types, use modulo arithmetic to cycle through valid range
                const T range = std::numeric_limits<T>::max() - std::numeric_limits<T>::min() + 1;
                for (size_t i = 0; i < size; ++i) {
                    data.push_back(static_cast<T>(std::numeric_limits<T>::max() - (i % range)));
                }
            } else {
                for (size_t i = 0; i < size; ++i) {
                    data.push_back(static_cast<T>(size - i));
                }
            }
            break;
        }

        case DataPattern::MANY_DUPLICATES_10: {
            // 10% unique values
            size_t unique_count = std::max(size_t(1), size / 10);
            T max_unique = static_cast<T>(std::min(static_cast<size_t>(max_val), unique_count));
            for (size_t i = 0; i < size; ++i) {
                data.push_back(generate_random_value(gen, min_val, max_unique));
            }
            break;
        }

        case DataPattern::MANY_DUPLICATES_50: {
            // 50% unique values
            size_t unique_count = std::max(size_t(1), size / 2);
            T max_unique = static_cast<T>(std::min(static_cast<size_t>(max_val), unique_count));
            for (size_t i = 0; i < size; ++i) {
                data.push_back(generate_random_value(gen, min_val, max_unique));
            }
            break;
        }

        case DataPattern::MANY_DUPLICATES_90: {
            // 90% unique values
            size_t unique_count = std::max(size_t(1), size * 9 / 10);
            T max_unique = static_cast<T>(std::min(static_cast<size_t>(max_val), unique_count));
            for (size_t i = 0; i < size; ++i) {
                data.push_back(generate_random_value(gen, min_val, max_unique));
            }
            break;
        }

        case DataPattern::ORGAN_PIPE: {
            // Ascending then descending
            size_t mid = size / 2;
            if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
                const T range = std::numeric_limits<T>::max() - std::numeric_limits<T>::min() + 1;
                for (size_t i = 0; i < mid; ++i) {
                    data.push_back(static_cast<T>((i % range) + std::numeric_limits<T>::min()));
                }
                for (size_t i = mid; i < size; ++i) {
                    data.push_back(static_cast<T>(std::numeric_limits<T>::max() - ((i - mid) % range)));
                }
            } else {
                for (size_t i = 0; i < mid; ++i) {
                    data.push_back(static_cast<T>(i));
                }
                for (size_t i = mid; i < size; ++i) {
                    data.push_back(static_cast<T>(size - i));
                }
            }
            break;
        }

        case DataPattern::SAWTOOTH: {
            // Repeating ascending patterns
            if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
                const T range = std::numeric_limits<T>::max() - std::numeric_limits<T>::min() + 1;
                size_t pattern_length = std::min(static_cast<size_t>(range), size / 10);
                if (pattern_length == 0) pattern_length = 1;

                for (size_t i = 0; i < size; ++i) {
                    data.push_back(static_cast<T>((i % pattern_length) + std::numeric_limits<T>::min()));
                }
            } else {
                size_t pattern_length = size / 10;  // 10 teeth
                if (pattern_length == 0) pattern_length = 1;

                for (size_t i = 0; i < size; ++i) {
                    data.push_back(static_cast<T>(i % pattern_length));
                }
            }
            break;
        }
    }

    return data;
}

/**
 * @brief Convert data pattern enum to human-readable string
 *
 * This utility function provides user-friendly names for data patterns,
 * essential for generating readable benchmark reports and test output.
 * The names are designed to be concise yet descriptive for technical
 * documentation and performance analysis reports.
 *
 * @param pattern The DataPattern enum value to convert
 * @return Const character string with human-readable pattern name
 *
 * @note The returned strings are designed for use in benchmark reports
 *       and should remain stable across versions for consistency.
 */
const char* pattern_name(DataPattern pattern) {
    switch (pattern) {
        case DataPattern::RANDOM: return "Random";
        case DataPattern::NEARLY_SORTED: return "Nearly Sorted";
        case DataPattern::REVERSE_SORTED: return "Reverse Sorted";
        case DataPattern::MANY_DUPLICATES_10: return "10% Unique";
        case DataPattern::MANY_DUPLICATES_50: return "50% Unique";
        case DataPattern::MANY_DUPLICATES_90: return "90% Unique";
        case DataPattern::ORGAN_PIPE: return "Organ Pipe";
        case DataPattern::SAWTOOTH: return "Sawtooth";
        default: return "Unknown";
    }
}

/**
 * @brief Standard test sizes for comprehensive performance analysis
 *
 * This predefined array contains carefully chosen test sizes that provide
 * comprehensive coverage of sorting algorithm performance characteristics:
 *
 * **Small sizes (10-1000)**: Test overhead and base-case optimizations
 * - Critical for validating small-array optimizations like insertion sort
 * - Expose constant factors and algorithm overhead
 *
 * **Medium sizes (10K-50K)**: Test main algorithm performance
 * - Primary range for comparing algorithmic efficiency
 * - Large enough to amortize startup costs
 *
 * **Large sizes (60K-100K)**: Test scalability and memory effects
 * - Expose cache effects and memory bandwidth limitations
 * - Test behavior as data exceeds CPU cache sizes
 *
 * The progression is designed to capture performance transitions and
 * provide statistically meaningful sample points for analysis.
 */
const std::vector<size_t> test_sizes = {
    10, 100, 1000,                                      // Small arrays: overhead analysis
    10000, 20000, 30000, 40000, 50000,                 // Medium arrays: core performance
    60000, 70000, 80000, 90000, 100000                 // Large arrays: scalability testing
};

/**
 * @brief Complete set of test patterns for comprehensive algorithm evaluation
 *
 * This array contains all implemented data patterns and serves as the canonical
 * list for comprehensive sorting algorithm testing. Each pattern is designed to
 * expose specific performance characteristics:
 *
 * - **RANDOM**: Baseline average-case performance
 * - **NEARLY_SORTED**: Real-world performance on partially ordered data
 * - **REVERSE_SORTED**: Worst-case scenarios for naive implementations
 * - **MANY_DUPLICATES_***: Duplicate handling efficiency (key dual-pivot advantage)
 * - **ORGAN_PIPE**: Challenging partitioning scenarios
 * - **SAWTOOTH**: Regular pattern detection and handling
 *
 * This comprehensive pattern set ensures that benchmark results provide
 * complete coverage of algorithmic behavior across all expected use cases.
 */
const std::vector<DataPattern> all_patterns = {
    DataPattern::RANDOM,
    DataPattern::NEARLY_SORTED,
    DataPattern::REVERSE_SORTED,
    DataPattern::MANY_DUPLICATES_10,
    DataPattern::MANY_DUPLICATES_50,
    DataPattern::MANY_DUPLICATES_90,
    DataPattern::ORGAN_PIPE,
    DataPattern::SAWTOOTH
};

} // namespace benchmark_data