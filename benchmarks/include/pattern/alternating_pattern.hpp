#pragma once
#include <vector>
#include <algorithm>

template<typename T>
std::vector<T> generate_alternating_pattern(size_t length) {
    std::vector<T> arr(length);
    
    if constexpr (std::is_integral_v<T>) {
        T low_val = 1;
        T high_val = static_cast<T>(length * 100);
        
        for (size_t i = 0; i < length; ++i) {
            if (i % 2 == 0) {
                arr[i] = low_val + static_cast<T>(i / 2);
            } else {
                arr[i] = high_val - static_cast<T>(i / 2);
            }
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        T low_val = 1.0;
        T high_val = static_cast<T>(length * 100);
        
        for (size_t i = 0; i < length; ++i) {
            if (i % 2 == 0) {
                arr[i] = low_val + static_cast<T>(i / 2);
            } else {
                arr[i] = high_val - static_cast<T>(i / 2);
            }
        }
    } else if constexpr (std::is_same_v<T, bool>) {
        // For bool, alternate between false and true
        for (size_t i = 0; i < length; ++i) {
            arr[i] = (i % 2 == 1);
        }
    } else {
        // For character types, alternate between low and high ASCII values
        T low_val = static_cast<T>(33);  // '!'
        T high_val = static_cast<T>(126); // '~'
        
        for (size_t i = 0; i < length; ++i) {
            if (i % 2 == 0) {
                arr[i] = static_cast<T>(low_val + (i / 2) % (high_val - low_val));
            } else {
                arr[i] = static_cast<T>(high_val - (i / 2) % (high_val - low_val));
            }
        }
    }
    
    return arr;
}