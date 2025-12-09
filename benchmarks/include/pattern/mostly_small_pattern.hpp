#pragma once
#include <vector>
#include <random>
#include <algorithm>

template<typename T>
std::vector<T> generate_mostly_small_pattern(size_t length) {
    std::vector<T> arr(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // 90% small values, 10% large values
    size_t large_count = std::max(1UL, length / 10);
    
    if constexpr (std::is_integral_v<T>) {
        T small_max = 10;
        T large_min = static_cast<T>(length * 100);
        T large_max = static_cast<T>(std::min(static_cast<size_t>(std::numeric_limits<T>::max()), length * 1000));
        
        std::uniform_int_distribution<T> small_dis(1, small_max);
        std::uniform_int_distribution<T> large_dis(large_min, large_max);
        
        // Fill with small values first
        for (size_t i = 0; i < length; ++i) {
            arr[i] = small_dis(gen);
        }
        
        // Replace some with large values
        std::uniform_int_distribution<size_t> pos_dis(0, length - 1);
        for (size_t i = 0; i < large_count; ++i) {
            size_t pos = pos_dis(gen);
            arr[pos] = large_dis(gen);
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        T small_max = 10.0;
        T large_min = static_cast<T>(length * 100);
        T large_max = static_cast<T>(length * 1000);
        
        std::uniform_real_distribution<T> small_dis(1.0, small_max);
        std::uniform_real_distribution<T> large_dis(large_min, large_max);
        
        for (size_t i = 0; i < length; ++i) {
            arr[i] = small_dis(gen);
        }
        
        std::uniform_int_distribution<size_t> pos_dis(0, length - 1);
        for (size_t i = 0; i < large_count; ++i) {
            size_t pos = pos_dis(gen);
            arr[pos] = large_dis(gen);
        }
    } else if constexpr (std::is_same_v<T, bool>) {
        // For bool, mostly false (small), few true (large)
        std::fill(arr.begin(), arr.end(), false);
        std::uniform_int_distribution<size_t> pos_dis(0, length - 1);
        for (size_t i = 0; i < large_count; ++i) {
            size_t pos = pos_dis(gen);
            arr[pos] = true;
        }
    } else {
        // For character types, mostly printable ASCII (32-126), few high values (128-255)
        std::uniform_int_distribution<int> small_dis(32, 126);
        std::uniform_int_distribution<int> large_dis(128, 255);
        
        for (size_t i = 0; i < length; ++i) {
            arr[i] = static_cast<T>(small_dis(gen));
        }
        
        std::uniform_int_distribution<size_t> pos_dis(0, length - 1);
        for (size_t i = 0; i < large_count; ++i) {
            size_t pos = pos_dis(gen);
            arr[pos] = static_cast<T>(large_dis(gen));
        }
    }
    
    return arr;
}