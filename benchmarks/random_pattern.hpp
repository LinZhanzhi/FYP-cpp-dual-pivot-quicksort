#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <limits>

template<typename T>
std::vector<T> generate_random_pattern(size_t length) {
    std::vector<T> arr(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    
    if constexpr (std::is_integral_v<T>) {
        // Special handling for char types to avoid overflow
        if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
            std::uniform_int_distribution<int> dis(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
            for (size_t i = 0; i < length; ++i) {
                arr[i] = static_cast<T>(dis(gen));
            }
        } else {
            std::uniform_int_distribution<T> dis(1, static_cast<T>(std::min(static_cast<size_t>(std::numeric_limits<T>::max()), length * 10)));
            for (size_t i = 0; i < length; ++i) {
                arr[i] = dis(gen);
            }
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        std::uniform_real_distribution<T> dis(0.0, static_cast<T>(length * 10));
        for (size_t i = 0; i < length; ++i) {
            arr[i] = dis(gen);
        }
    } else if constexpr (std::is_same_v<T, bool>) {
        std::uniform_int_distribution<int> dis(0, 1);
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<bool>(dis(gen));
        }
    } else {
        std::uniform_int_distribution<int> dis(0, 255);
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<T>(dis(gen));
        }
    }
    
    return arr;
}