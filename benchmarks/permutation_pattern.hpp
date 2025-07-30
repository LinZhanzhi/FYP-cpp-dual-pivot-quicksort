#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <limits>
#include <cmath>

template<typename T>
std::vector<T> generate_permutation_pattern(size_t length) {
    std::vector<T> arr(length);
    
    // Create a sorted array first
    if constexpr (std::is_integral_v<T>) {
        if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
            // For char types, use modulo arithmetic to cycle through valid range
            const int range = static_cast<int>(std::numeric_limits<T>::max()) - static_cast<int>(std::numeric_limits<T>::min()) + 1;
            for (size_t i = 0; i < length; ++i) {
                arr[i] = static_cast<T>((i % range) + std::numeric_limits<T>::min());
            }
        } else {
            for (size_t i = 0; i < length; ++i) {
                arr[i] = static_cast<T>(i + 1);
            }
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
            arr[i] = static_cast<T>((i % 256));
        }
    }
    
    // Apply a small number of specific swaps to create a permutation
    // that requires only a few swaps to reach sorted order
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Perform sqrt(length) swaps, but at least 1 and at most 10
    size_t num_swaps = std::max(static_cast<size_t>(1), std::min(static_cast<size_t>(10), static_cast<size_t>(std::sqrt(length))));
    
    // Use specific patterns of swaps that create interesting permutations
    std::uniform_int_distribution<size_t> dis(0, length - 1);
    
    for (size_t i = 0; i < num_swaps; ++i) {
        size_t pos1 = dis(gen);
        size_t pos2 = dis(gen);
        
        // Ensure we don't swap the same position
        if (pos1 != pos2) {
            if constexpr (std::is_same_v<T, bool>) {
                // Special handling for bool vector
                bool temp = arr[pos1];
                arr[pos1] = arr[pos2];
                arr[pos2] = temp;
            } else {
                std::swap(arr[pos1], arr[pos2]);
            }
        }
    }
    
    // Additionally, create some cyclic permutations for more interesting patterns
    if (length > 3) {
        size_t cycle_length = std::min(length, static_cast<size_t>(5));
        std::uniform_int_distribution<size_t> start_dis(0, length - cycle_length);
        size_t start = start_dis(gen);
        
        // Create a small cycle
        T temp = arr[start];
        for (size_t i = 0; i < cycle_length - 1; ++i) {
            arr[start + i] = arr[start + i + 1];
        }
        arr[start + cycle_length - 1] = temp;
    }
    
    return arr;
}