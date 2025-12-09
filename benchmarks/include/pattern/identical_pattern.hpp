#pragma once
#include <vector>
#include <algorithm>

template<typename T>
std::vector<T> generate_identical_pattern(size_t length) {
    std::vector<T> arr(length);
    
    if constexpr (std::is_integral_v<T>) {
        T value = static_cast<T>(42);
        std::fill(arr.begin(), arr.end(), value);
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