#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <limits>

template<typename T>
std::vector<T> generate_scalability_pattern(size_t length) {
    std::vector<T> arr(length);
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // This pattern adapts based on array length to test scalability
    // For short arrays: simple random pattern
    // For long arrays: pattern that stresses cache and memory access
    
    if (length <= 100) {
        // Short array: simple random values
        if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
                std::uniform_int_distribution<int> dis(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
                for (size_t i = 0; i < length; ++i) {
                    arr[i] = static_cast<T>(dis(gen));
                }
            } else {
                std::uniform_int_distribution<T> dis(1, static_cast<T>(length));
                for (size_t i = 0; i < length; ++i) {
                    arr[i] = dis(gen);
                }
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dis(1.0, static_cast<T>(length));
            for (size_t i = 0; i < length; ++i) {
                arr[i] = dis(gen);
            }
        } else if constexpr (std::is_same_v<T, bool>) {
            std::uniform_int_distribution<int> dis(0, 1);
            for (size_t i = 0; i < length; ++i) {
                arr[i] = static_cast<bool>(dis(gen));
            }
        } else {
            if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
                std::uniform_int_distribution<int> dis(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
                for (size_t i = 0; i < length; ++i) {
                    arr[i] = static_cast<T>(dis(gen));
                }
            } else {
                std::uniform_int_distribution<int> dis(65, 90); // A-Z
                for (size_t i = 0; i < length; ++i) {
                    arr[i] = static_cast<T>(dis(gen));
                }
            }
        }
    } else {
        // Long array: cache-unfriendly pattern with wide value ranges
        if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
                std::uniform_int_distribution<int> dis(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
                
                // Create pattern with poor cache locality
                for (size_t i = 0; i < length; ++i) {
                    // Use stride pattern to reduce cache efficiency
                    size_t index = (i * 7919) % length; // Prime number for pseudo-random access
                    if (arr[index] == T{}) { // Only set if not already set
                        arr[index] = static_cast<T>(dis(gen));
                    }
                }
                
                // Fill any remaining zeros
                for (size_t i = 0; i < length; ++i) {
                    if (arr[i] == T{}) {
                        arr[i] = static_cast<T>(dis(gen));
                    }
                }
            } else {
                std::uniform_int_distribution<T> dis(1, static_cast<T>(std::min(static_cast<size_t>(std::numeric_limits<T>::max()), length * 10)));
                
                // Create pattern with poor cache locality
                for (size_t i = 0; i < length; ++i) {
                    // Use stride pattern to reduce cache efficiency
                    size_t index = (i * 7919) % length; // Prime number for pseudo-random access
                    if (arr[index] == T{}) { // Only set if not already set
                        arr[index] = dis(gen);
                    }
                }
                
                // Fill any remaining zeros
                for (size_t i = 0; i < length; ++i) {
                    if (arr[i] == T{}) {
                        arr[i] = dis(gen);
                    }
                }
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dis(1.0, static_cast<T>(length * 10));
            
            for (size_t i = 0; i < length; ++i) {
                size_t index = (i * 7919) % length;
                if (arr[index] == T{}) {
                    arr[index] = dis(gen);
                }
            }
            
            for (size_t i = 0; i < length; ++i) {
                if (arr[i] == T{}) {
                    arr[i] = dis(gen);
                }
            }
        } else if constexpr (std::is_same_v<T, bool>) {
            std::uniform_int_distribution<int> dis(0, 1);
            for (size_t i = 0; i < length; ++i) {
                arr[i] = static_cast<bool>(dis(gen));
            }
        } else {
            if constexpr (std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>) {
                std::uniform_int_distribution<int> dis(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
                
                for (size_t i = 0; i < length; ++i) {
                    size_t index = (i * 7919) % length;
                    if (arr[index] == T{}) {
                        arr[index] = static_cast<T>(dis(gen));
                    }
                }
                
                for (size_t i = 0; i < length; ++i) {
                    if (arr[i] == T{}) {
                        arr[i] = static_cast<T>(dis(gen));
                    }
                }
            } else {
                std::uniform_int_distribution<int> dis(0, 255);
                
                for (size_t i = 0; i < length; ++i) {
                    size_t index = (i * 7919) % length;
                    if (arr[index] == T{}) {
                        arr[index] = static_cast<T>(dis(gen));
                    }
                }
                
                for (size_t i = 0; i < length; ++i) {
                    if (arr[i] == T{}) {
                        arr[i] = static_cast<T>(dis(gen));
                    }
                }
            }
        }
    }
    
    return arr;
}