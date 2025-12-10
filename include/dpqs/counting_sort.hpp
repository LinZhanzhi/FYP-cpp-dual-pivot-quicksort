#ifndef DPQS_COUNTING_SORT_HPP
#define DPQS_COUNTING_SORT_HPP

#include "dpqs/utils.hpp"
#include "dpqs/core_sort.hpp"
#include <vector>
#include <type_traits>

namespace dual_pivot {

template<typename T>
#if __cplusplus >= 202002L
requires (Integral<T> && sizeof(T) == 1)
void countingSort(T* a, int low, int high) {
#else
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == 1, void>::type
countingSort(T* a, int low, int high) {
#endif
    static constexpr int NUM_VALUES = 1 << (8 * sizeof(T));
    static constexpr int OFFSET = std::is_signed<T>::value ? (1 << (8 * sizeof(T) - 1)) : 0;

    std::vector<int> count(NUM_VALUES, 0);

    for (int i = high; i > low; ) {
        count[static_cast<unsigned char>(a[--i]) + OFFSET]++;
    }

    int size = high - low;
    if (size > NUM_VALUES / 2) {
        int index = high;
        for (int i = NUM_VALUES; --i >= 0; ) {
            T value = static_cast<T>(i - OFFSET);
            int cnt = count[i];
            while (cnt-- > 0) {
                a[--index] = value;
            }
        }
    } else {
        int index = low;
        for (int i = 0; i < NUM_VALUES; i++) {
            if (count[i] > 0) {
                T value = static_cast<T>(i - OFFSET);
                int cnt = count[i];
                while (cnt-- > 0) {
                    a[index++] = value;
                }
            }
        }
    }
}

template<typename T>
#if __cplusplus >= 202002L
requires (Integral<T> && sizeof(T) == 2)
void countingSort(T* a, int low, int high) {
#else
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == 2, void>::type
countingSort(T* a, int low, int high) {
#endif
    static constexpr int NUM_SHORT_VALUES = 1 << 16;
    static constexpr int MAX_SHORT_INDEX = std::is_signed<T>::value ?
        (1 << 15) + NUM_SHORT_VALUES + 1 : NUM_SHORT_VALUES + 1;

    int size = high - low;

    if (size < NUM_SHORT_VALUES) {
        std::vector<int> count(NUM_SHORT_VALUES, 0);

        for (int i = high; i > low; ) {
            ++count[a[--i] & 0xFFFF];
        }

        int index = low;
        for (int i = 0; i < NUM_SHORT_VALUES; ) {
            while (i < NUM_SHORT_VALUES && count[i] == 0) ++i;
            if (i < NUM_SHORT_VALUES) {
                T value = static_cast<T>(i);
                if constexpr (std::is_signed<T>::value) {
                    value = static_cast<T>(static_cast<std::int16_t>(i));
                }
                int cnt = count[i];
                while (cnt-- > 0) {
                    a[index++] = value;
                }
                ++i;
            }
        }
    } else {
        std::vector<int> count(MAX_SHORT_INDEX, 0);

        for (int i = high; i > low; ) {
            T val = a[--i];
            int idx = static_cast<int>(val);
            if constexpr (std::is_signed<T>::value) {
                idx += (1 << 15);
            }
            ++count[idx];
        }

        int index = high;
        for (int i = MAX_SHORT_INDEX; --i >= 0; ) {
            if (count[i] > 0) {
                T value;
                if constexpr (std::is_signed<T>::value) {
                    value = static_cast<T>(i - (1 << 15));
                } else {
                    value = static_cast<T>(i);
                }
                int cnt = count[i];
                while (cnt-- > 0) {
                    a[--index] = value;
                }
            }
        }
    }
}

template<typename T>
#if __cplusplus >= 202002L
requires (!Integral<T> && !FloatingPoint<T>)
void sort_specialized(T* a, int low, int high) {
#else
typename std::enable_if<!std::is_integral<T>::value && !std::is_floating_point<T>::value, void>::type
sort_specialized(T* a, int low, int high) {
#endif
    sort(a, 0, low, high);
}

template<typename T>
#if __cplusplus >= 202002L
requires (Integral<T> && sizeof(T) > 2)
void sort_specialized(T* a, int low, int high) {
#else
typename std::enable_if<std::is_integral<T>::value && (sizeof(T) > 2), void>::type
sort_specialized(T* a, int low, int high) {
#endif
    sort(a, 0, low, high);
}

} // namespace dual_pivot

#endif // DPQS_COUNTING_SORT_HPP
