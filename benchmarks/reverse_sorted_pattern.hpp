#pragma once
#include <vector>
#include <algorithm>
#include <limits>

template<typename T>
std::vector<T> generate_reverse_sorted_pattern(size_t length) {
    std::vector<T> arr(length);
    
    if constexpr (std::is_integral_v<T>) {
        if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
            // For char types, use modulo arithmetic to stay within valid range
            const int range = static_cast<int>(std::numeric_limits<T>::max()) - static_cast<int>(std::numeric_limits<T>::min()) + 1;
            for (size_t i = 0; i < length; ++i) {
                arr[i] = static_cast<T>(std::numeric_limits<T>::max() - (i % range));
            }
        } else {
            for (size_t i = 0; i < length; ++i) {
                arr[i] = static_cast<T>(length - i);
            }
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
        // For character types, use modulo to stay within ASCII range
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<T>((length - i - 1) % 256);
        }
    }
    
    return arr;
}