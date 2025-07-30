#pragma once
#include <vector>
#include <algorithm>
#include <limits>

template<typename T>
std::vector<T> generate_identical_pattern(size_t length) {
    std::vector<T> arr(length);
    
    if constexpr (std::is_integral_v<T>) {
        if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
            // For char types, use a safe value within the valid range
            T value = static_cast<T>(std::max(static_cast<int>(std::numeric_limits<T>::min()), std::min(42, static_cast<int>(std::numeric_limits<T>::max()))));
            std::fill(arr.begin(), arr.end(), value);
        } else {
            T value = static_cast<T>(42);
            std::fill(arr.begin(), arr.end(), value);
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        T value = static_cast<T>(42.5);
        std::fill(arr.begin(), arr.end(), value);
    } else if constexpr (std::is_same_v<T, bool>) {
        std::fill(arr.begin(), arr.end(), true);
    } else {
        T value = static_cast<T>('A');
        std::fill(arr.begin(), arr.end(), value);
    }
    
    return arr;
}