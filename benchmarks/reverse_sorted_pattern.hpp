#pragma once
#include <vector>
#include <algorithm>

template<typename T>
std::vector<T> generate_reverse_sorted_pattern(size_t length) {
    std::vector<T> arr(length);
    
    if constexpr (std::is_integral_v<T>) {
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<T>(length - i);
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<T>(length - i);
        }
    } else if constexpr (std::is_same_v<T, bool>) {
        for (size_t i = 0; i < length; ++i) {
            arr[i] = (i % 2 == 0) ? true : false;
        }
    } else {
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<T>((length - i - 1) % 256);
        }
    }
    
    return arr;
}