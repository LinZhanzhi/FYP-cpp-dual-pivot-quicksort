#ifndef DPQS_FLOAT_SORT_HPP
#define DPQS_FLOAT_SORT_HPP

#include "dpqs/utils.hpp"
#include "dpqs/core_sort.hpp"
#include "dpqs/sequential_sorters.hpp"
#include <cmath>
#include <cstring>
#include <type_traits>

namespace dual_pivot {

template<typename T>
bool isNaN(T x) { return x != x; }

template<typename T>
bool isNegativeZero(T x) {
    if (x != 0) return false;
    if constexpr (std::is_same_v<T, float>) {
        return std::signbit(x);
    } else {
        return std::signbit(x);
    }
}

template<typename T>
bool isPositiveZero(T x) {
    return x == 0 && !isNegativeZero(x);
}

inline float intBitsToFloat(unsigned int x) {
    float f;
    std::memcpy(&f, &x, sizeof(float));
    return f;
}

inline double longBitsToDouble(unsigned long long x) {
    double d;
    std::memcpy(&d, &x, sizeof(double));
    return d;
}

template<typename T>
int findZeroPosition(T* a, int low, int high) {
    int left = low;
    int right = high;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (a[mid] < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return left;
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
sort_float_specialized(T* a, int low, int hi) {
    int numNegativeZero = 0;
    int actualHigh = hi;

    for (int k = actualHigh - 1; k >= low; k--) {
        T ak = a[k];

        if (isNaN(ak)) {
            a[k] = a[--actualHigh];
            a[actualHigh] = ak;
        } else if (isNegativeZero(ak)) {
            numNegativeZero++;
            a[k] = T(0);
        }
    }

    if (actualHigh > low) {
        if constexpr (std::is_same_v<T, float>) {
            sort_float_sequential(nullptr, a, 0, low, actualHigh);
        } else if constexpr (std::is_same_v<T, double>) {
            sort_double_sequential(nullptr, a, 0, low, actualHigh);
        }
    }

    if (numNegativeZero > 0) {
        int left = findZeroPosition(a, low, actualHigh - 1);

        for (int i = 0; i < numNegativeZero && left < actualHigh; i++, left++) {
            if constexpr (std::is_same_v<T, float>) {
                if (isPositiveZero(a[left])) {
                    a[left] = intBitsToFloat(0x80000000U);
                }
            } else if constexpr (std::is_same_v<T, double>) {
                if (isPositiveZero(a[left])) {
                    a[left] = longBitsToDouble(0x8000000000000000ULL);
                }
            }
        }
    }
}

template<typename T>
#if __cplusplus >= 202002L
requires FloatingPoint<T>
void sort_specialized(T* a, int low, int high) {
#else
typename std::enable_if<std::is_floating_point<T>::value, void>::type
sort_specialized(T* a, int low, int high) {
#endif
    sort_float_specialized(a, low, high);
}

} // namespace dual_pivot

#endif // DPQS_FLOAT_SORT_HPP
