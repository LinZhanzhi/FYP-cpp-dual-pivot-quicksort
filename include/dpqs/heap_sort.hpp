#ifndef DPQS_HEAP_SORT_HPP
#define DPQS_HEAP_SORT_HPP

#include "utils.hpp"

namespace dual_pivot {

static void pushDown_int(int* a, int p, int value, int low, int high);
static void pushDown_long(long* a, int p, long value, int low, int high);
static void pushDown_float(float* a, int p, float value, int low, int high);
static void pushDown_double(double* a, int p, double value, int low, int high);
// static void pushDown_byte(signed char* a, int p, signed char value, int low, int high);
// static void pushDown_char(char* a, int p, char value, int low, int high);
// static void pushDown_short(short* a, int p, short value, int low, int high);

template<typename T>
void pushDown(T* a, int p, T value, int low, int high) {
    for (int k;;) {
        k = (p << 1) - low + 2; // Index of the right child

        if (k > high) {
            break;
        }
        if (k == high || a[k] < a[k - 1]) {
            --k;
        }
        if (a[k] <= value) {
            break;
        }
        a[p] = a[k];
        p = k;
    }
    a[p] = value;
}

template<typename T>
void heapSort(T* a, int low, int high) {
    for (int k = (low + high) >> 1; k > low; ) {
        --k;
        pushDown(a, k, a[k], low, high);
    }
    while (--high > low) {
        T max = a[low];
        pushDown(a, low, a[high], low, high);
        a[high] = max;
    }
}

static void heapSort_int(int* a, int low, int high) {
    for (int k = (low + high) >> 1; k > low; ) {
        --k;
        pushDown_int(a, k, a[k], low, high);
    }
    while (--high > low) {
        int max = a[low];
        pushDown_int(a, low, a[high], low, high);
        a[high] = max;
    }
}

static void pushDown_int(int* a, int p, int value, int low, int high) {
    for (int k;;) {
        k = (p << 1) - low + 2;

        if (k > high) {
            break;
        }
        if (k == high || a[k] < a[k - 1]) {
            --k;
        }
        if (a[k] <= value) {
            break;
        }
        a[p] = a[k];
        p = k;
    }
    a[p] = value;
}

static void heapSort_long(long* a, int low, int high) {
    for (int k = (low + high) >> 1; k > low; ) {
        --k;
        pushDown_long(a, k, a[k], low, high);
    }
    while (--high > low) {
        long max = a[low];
        pushDown_long(a, low, a[high], low, high);
        a[high] = max;
    }
}

static void pushDown_long(long* a, int p, long value, int low, int high) {
    for (int k;;) {
        k = (p << 1) - low + 2;

        if (k > high) {
            break;
        }
        if (k == high || a[k] < a[k - 1]) {
            --k;
        }
        if (a[k] <= value) {
            break;
        }
        a[p] = a[k];
        p = k;
    }
    a[p] = value;
}

static void heapSort_float(float* a, int low, int high) {
    for (int k = (low + high) >> 1; k > low; ) {
        --k;
        pushDown_float(a, k, a[k], low, high);
    }
    while (--high > low) {
        float max = a[low];
        pushDown_float(a, low, a[high], low, high);
        a[high] = max;
    }
}

static void pushDown_float(float* a, int p, float value, int low, int high) {
    for (int k;;) {
        k = (p << 1) - low + 2;

        if (k > high) {
            break;
        }
        if (k == high || a[k] < a[k - 1]) {
            --k;
        }
        if (a[k] <= value) {
            break;
        }
        a[p] = a[k];
        p = k;
    }
    a[p] = value;
}

static void heapSort_double(double* a, int low, int high) {
    for (int k = (low + high) >> 1; k > low; ) {
        --k;
        pushDown_double(a, k, a[k], low, high);
    }
    while (--high > low) {
        double max = a[low];
        pushDown_double(a, low, a[high], low, high);
        a[high] = max;
    }
}

static void pushDown_double(double* a, int p, double value, int low, int high) {
    for (int k;;) {
        k = (p << 1) - low + 2;

        if (k > high) {
            break;
        }
        if (k == high || a[k] < a[k - 1]) {
            --k;
        }
        if (a[k] <= value) {
            break;
        }
        a[p] = a[k];
        p = k;
    }
    a[p] = value;
}

} // namespace dual_pivot

#endif // DPQS_HEAP_SORT_HPP
