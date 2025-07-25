#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <cstddef>

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

// Generate test data according to specified pattern
template<typename T = int>
std::vector<T> generate_data(size_t size, DataPattern pattern, unsigned seed = 42) {
    std::vector<T> data;
    data.reserve(size);
    
    std::mt19937 gen(seed);
    
    switch (pattern) {
        case DataPattern::RANDOM: {
            std::uniform_int_distribution<T> dis(1, static_cast<T>(size));
            for (size_t i = 0; i < size; ++i) {
                data.push_back(dis(gen));
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
            std::uniform_int_distribution<T> dis(1, static_cast<T>(unique_count));
            for (size_t i = 0; i < size; ++i) {
                data.push_back(dis(gen));
            }
            break;
        }
        
        case DataPattern::MANY_DUPLICATES_50: {
            // 50% unique values
            size_t unique_count = std::max(size_t(1), size / 2);
            std::uniform_int_distribution<T> dis(1, static_cast<T>(unique_count));
            for (size_t i = 0; i < size; ++i) {
                data.push_back(dis(gen));
            }
            break;
        }
        
        case DataPattern::MANY_DUPLICATES_90: {
            // 90% unique values
            size_t unique_count = std::max(size_t(1), size * 9 / 10);
            std::uniform_int_distribution<T> dis(1, static_cast<T>(unique_count));
            for (size_t i = 0; i < size; ++i) {
                data.push_back(dis(gen));
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
    10, 100, 1000,           // Small
    10000, 100000,           // Medium  
    1000000, 10000000        // Large
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