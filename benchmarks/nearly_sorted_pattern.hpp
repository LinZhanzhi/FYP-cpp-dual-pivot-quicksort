#pragma once
#include <vector>
#include <random>
#include <algorithm>

template<typename T>
std::vector<T> generate_nearly_sorted_pattern(size_t length) {
    std::vector<T> arr(length);
    
    // Generate sorted array first
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
    
    // Perform small number of random swaps (5% of array size or at least 1)
    std::random_device rd;
    std::mt19937 gen(rd());
    size_t num_swaps = std::max(static_cast<size_t>(1), length / 20);
    std::uniform_int_distribution<size_t> dis(0, length - 1);
    
    for (size_t i = 0; i < num_swaps; ++i) {
        size_t pos1 = dis(gen);
        size_t pos2 = dis(gen);
        if constexpr (std::is_same_v<T, bool>) {
            // Special handling for bool vector
            bool temp = arr[pos1];
            arr[pos1] = arr[pos2];
            arr[pos2] = temp;
        } else {
            std::swap(arr[pos1], arr[pos2]);
        }
    }
    
    return arr;
}