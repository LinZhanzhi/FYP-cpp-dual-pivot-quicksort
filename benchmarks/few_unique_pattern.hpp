#pragma once
#include <vector>
#include <random>
#include <algorithm>

template<typename T>
std::vector<T> generate_few_unique_pattern(size_t length) {
    std::vector<T> arr(length);
    
    // Use only 5-10 unique values regardless of array size
    size_t unique_count = std::min(static_cast<size_t>(10), std::max(static_cast<size_t>(3), length / 20));
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    if constexpr (std::is_integral_v<T>) {
        std::uniform_int_distribution<T> value_dis(1, static_cast<T>(unique_count * 5));
        std::uniform_int_distribution<size_t> index_dis(0, unique_count - 1);
        
        // Generate unique values
        std::vector<T> unique_values(unique_count);
        for (size_t i = 0; i < unique_count; ++i) {
            unique_values[i] = value_dis(gen);
        }
        
        // Fill array with repeated values
        for (size_t i = 0; i < length; ++i) {
            arr[i] = unique_values[index_dis(gen)];
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        std::uniform_real_distribution<T> value_dis(1.0, static_cast<T>(unique_count * 5));
        std::uniform_int_distribution<size_t> index_dis(0, unique_count - 1);
        
        std::vector<T> unique_values(unique_count);
        for (size_t i = 0; i < unique_count; ++i) {
            unique_values[i] = value_dis(gen);
        }
        
        for (size_t i = 0; i < length; ++i) {
            arr[i] = unique_values[index_dis(gen)];
        }
    } else if constexpr (std::is_same_v<T, bool>) {
        // For bool, only 2 unique values
        std::uniform_int_distribution<int> dis(0, 1);
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<bool>(dis(gen));
        }
    } else {
        std::uniform_int_distribution<int> value_dis(0, static_cast<int>(unique_count * 10));
        std::uniform_int_distribution<size_t> index_dis(0, unique_count - 1);
        
        std::vector<T> unique_values(unique_count);
        for (size_t i = 0; i < unique_count; ++i) {
            unique_values[i] = static_cast<T>(value_dis(gen) % 256);
        }
        
        for (size_t i = 0; i < length; ++i) {
            arr[i] = unique_values[index_dis(gen)];
        }
    }
    
    return arr;
}