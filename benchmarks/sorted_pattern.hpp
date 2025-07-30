#pragma once
#include <vector>
#include <algorithm>

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