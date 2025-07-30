#pragma once
#include <vector>
#include <random>
#include <algorithm>

template<typename T>
std::vector<T> generate_range_pattern(size_t length) {
    std::vector<T> arr(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Randomly choose between small range and large range
    std::uniform_int_distribution<int> choice_dis(0, 1);
    bool use_small_range = choice_dis(gen);
    
    if constexpr (std::is_integral_v<T>) {
        if (use_small_range) {
            // Small range: values between 1-10
            std::uniform_int_distribution<T> dis(1, 10);
            for (size_t i = 0; i < length; ++i) {
                arr[i] = dis(gen);
            }
        } else {
            // Large range: values between 1 and length*1000
            std::uniform_int_distribution<T> dis(1, static_cast<T>(std::min(static_cast<size_t>(std::numeric_limits<T>::max()), length * 1000)));
            for (size_t i = 0; i < length; ++i) {
                arr[i] = dis(gen);
            }
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        if (use_small_range) {
            // Small range: values between 1.0-10.0
            std::uniform_real_distribution<T> dis(1.0, 10.0);
            for (size_t i = 0; i < length; ++i) {
                arr[i] = dis(gen);
            }
        } else {
            // Large range: values between 1.0 and length*1000.0
            std::uniform_real_distribution<T> dis(1.0, static_cast<T>(length * 1000));
            for (size_t i = 0; i < length; ++i) {
                arr[i] = dis(gen);
            }
        }
    } else if constexpr (std::is_same_v<T, bool>) {
        // For bool, range is always small (0 or 1)
        std::uniform_int_distribution<int> dis(0, 1);
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<bool>(dis(gen));
        }
    } else {
        if (use_small_range) {
            // Small range: character values between 'A'-'J'
            std::uniform_int_distribution<int> dis(65, 74); // ASCII A-J
            for (size_t i = 0; i < length; ++i) {
                arr[i] = static_cast<T>(dis(gen));
            }
        } else {
            // Large range: full character range
            std::uniform_int_distribution<int> dis(0, 255);
            for (size_t i = 0; i < length; ++i) {
                arr[i] = static_cast<T>(dis(gen));
            }
        }
    }
    
    return arr;
}