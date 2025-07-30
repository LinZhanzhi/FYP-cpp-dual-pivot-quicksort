#pragma once
#include <vector>
#include <random>
#include <algorithm>

template<typename T>
std::vector<T> generate_random_pattern(size_t length) {
    std::vector<T> arr(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    
    if constexpr (std::is_integral_v<T>) {
        std::uniform_int_distribution<T> dis(1, static_cast<T>(length * 10));
        for (size_t i = 0; i < length; ++i) {
            arr[i] = dis(gen);
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