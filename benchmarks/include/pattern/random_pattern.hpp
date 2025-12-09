/**
 * @file random_pattern.hpp
 * @brief Random data pattern generator for sorting algorithm benchmarks
 * 
 * This file implements random data pattern generation that provides uniformly
 * distributed random values for testing average-case sorting performance.
 * Random patterns are essential for establishing baseline performance metrics
 * and validating that algorithms perform well on unstructured data.
 * 
 * The random pattern generator is designed to:
 * - Create truly random, unbiased test data
 * - Support multiple data types with appropriate distributions
 * - Generate values in ranges suitable for performance testing
 * - Provide reproducible results when seeded appropriately
 * 
 * @author Dual-Pivot Quicksort Research Project
 * @version 1.0
 * @date 2024
 */

#pragma once
#include <vector>
#include <random>
#include <algorithm>

/**
 * @brief Generate uniformly random data pattern for sorting algorithm testing
 * 
 * Creates an array of uniformly distributed random values appropriate for
 * testing average-case sorting performance. The function uses high-quality
 * random number generation (MT19937) and type-appropriate distributions
 * to ensure statistical validity of benchmark results.
 * 
 * Type-Specific Behavior:
 * - **Integral types**: Uniform integer distribution from 1 to (length * 10)
 * - **Floating-point types**: Uniform real distribution from 0.0 to (length * 10)
 * - **Boolean type**: Uniform distribution between true/false
 * - **Other types**: Fallback using values 0-255 with appropriate casting
 * 
 * The value ranges are chosen to provide good distribution characteristics
 * while avoiding extremes that might introduce artifacts into performance
 * measurements.
 * 
 * @tparam T Element type for the generated array
 * @param length Number of elements to generate
 * @return std::vector<T> containing uniformly distributed random values
 * 
 * @note Uses std::random_device for seeding to ensure different random
 *       sequences on each call, suitable for independent test runs.
 * 
 * Example:
 * @code
 * auto random_ints = generate_random_pattern<int>(1000);
 * auto random_doubles = generate_random_pattern<double>(500);
 * @endcode
 */
template<typename T>
std::vector<T> generate_random_pattern(size_t length) {
    std::vector<T> arr(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    
    if constexpr (std::is_integral_v<T>) {
        std::uniform_int_distribution<T> dis(1, static_cast<T>(length * 10));
        for (size_t i = 0; i < length; ++i) {
            arr[i] = dis(gen);
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        std::uniform_real_distribution<T> dis(0.0, static_cast<T>(length * 10));
        for (size_t i = 0; i < length; ++i) {
            arr[i] = dis(gen);
        }
    } else if constexpr (std::is_same_v<T, bool>) {
        std::uniform_int_distribution<int> dis(0, 1);
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<bool>(dis(gen));
        }
    } else {
        std::uniform_int_distribution<int> dis(0, 255);
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<T>(dis(gen));
        }
    }
    
    return arr;
}