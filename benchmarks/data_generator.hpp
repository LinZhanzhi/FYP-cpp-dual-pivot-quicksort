#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <cstddef>
#include <type_traits>

namespace benchmark_data {

enum class DataPattern {
    RANDOM,
    NEARLY_SORTED,
    REVERSE_SORTED,
    MANY_DUPLICATES_10,
    MANY_DUPLICATES_50,
    MANY_DUPLICATES_90,
    ORGAN_PIPE,
    SAWTOOTH
};

// Type traits for generating appropriate distributions
template<typename T>
struct TypeTraits {
    static constexpr bool is_integral = std::is_integral_v<T>;
    static constexpr bool is_floating_point = std::is_floating_point_v<T>;
    static constexpr bool is_signed = std::is_signed_v<T>;
    
    // Get appropriate range for the type
    static constexpr T min_value() {
        if constexpr (std::is_same_v<T, char>) return 0;
        else if constexpr (std::is_same_v<T, unsigned char>) return 0;
        else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) return 1;
        else if constexpr (std::is_integral_v<T> && !std::is_signed_v<T>) return 0;
        else if constexpr (std::is_floating_point_v<T>) return T(0.0);
        else return T(1);
    }
    
    static T max_value(size_t size) {
        if constexpr (std::is_same_v<T, char>) return static_cast<T>(std::min(size, 127ULL));
        else if constexpr (std::is_same_v<T, unsigned char>) return static_cast<T>(std::min(size, 255ULL));
        else if constexpr (std::is_same_v<T, short>) return static_cast<T>(std::min(size, 32767ULL));
        else if constexpr (std::is_same_v<T, unsigned short>) return static_cast<T>(std::min(size, 65535ULL));
        else if constexpr (std::is_integral_v<T>) return static_cast<T>(size);
        else if constexpr (std::is_floating_point_v<T>) return static_cast<T>(size);
        else return static_cast<T>(size);
    }
};

// Helper function to generate random values based on type
template<typename T>
T generate_random_value(std::mt19937& gen, T min_val, T max_val) {
    if constexpr (std::is_same_v<T, char> || std::is_same_v<T, unsigned char>) {
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

// Generate test data according to specified pattern
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
            for (size_t i = 0; i < size; ++i) {
                data.push_back(static_cast<T>(i));
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
            for (size_t i = 0; i < size; ++i) {
                data.push_back(static_cast<T>(size - i));
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
            for (size_t i = 0; i < mid; ++i) {
                data.push_back(static_cast<T>(i));
            }
            for (size_t i = mid; i < size; ++i) {
                data.push_back(static_cast<T>(size - i));
            }
            break;
        }
        
        case DataPattern::SAWTOOTH: {
            // Repeating ascending patterns
            size_t pattern_length = size / 10;  // 10 teeth
            if (pattern_length == 0) pattern_length = 1;
            
            for (size_t i = 0; i < size; ++i) {
                data.push_back(static_cast<T>(i % pattern_length));
            }
            break;
        }
    }
    
    return data;
}

// Get human-readable pattern name
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

// Standard test sizes from the plan
const std::vector<size_t> test_sizes = {
    10, 100, 1000,                                      // Small
    10000, 20000, 30000, 40000, 50000,                 // Medium
    60000, 70000, 80000, 90000, 100000                 // Large
};

// All test patterns
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