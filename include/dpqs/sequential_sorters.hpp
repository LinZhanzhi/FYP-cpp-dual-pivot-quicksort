#ifndef DPQS_SEQUENTIAL_SORTERS_HPP
#define DPQS_SEQUENTIAL_SORTERS_HPP

#include "dpqs/utils.hpp"
#include "dpqs/parallel/sorter.hpp"
#include "dpqs/partition.hpp"
#include "dpqs/insertion_sort.hpp"
#include "dpqs/heap_sort.hpp"
#include <vector>
#include <utility>

namespace dual_pivot {

// Intrinsic helpers
template<typename T, typename F>
FORCE_INLINE void sort_intrinsic(T* a, int low, int high, F sorter) {
    sorter(a, low, high);
}

template<typename T, typename F>
FORCE_INLINE std::pair<int, int> partition_intrinsic(T* a, int low, int high, int p1, int p2, F partitioner) {
    return partitioner(a, low, high, p1, p2);
}

// Helper for sorting 5 elements
template<typename T>
FORCE_INLINE void sort5Network(T* a, int e1, int e2, int e3, int e4, int e5) {
    if (a[e1] > a[e2]) std::swap(a[e1], a[e2]);
    if (a[e4] > a[e5]) std::swap(a[e4], a[e5]);
    if (a[e1] > a[e3]) std::swap(a[e1], a[e3]);
    if (a[e2] > a[e3]) std::swap(a[e2], a[e3]);
    if (a[e1] > a[e4]) std::swap(a[e1], a[e4]);
    if (a[e3] > a[e4]) std::swap(a[e3], a[e4]);
    if (a[e2] > a[e5]) std::swap(a[e2], a[e5]);
    if (a[e2] > a[e3]) std::swap(a[e2], a[e3]);
    if (a[e4] > a[e5]) std::swap(a[e4], a[e5]);
}

// Forward declarations
void mixed_insertion_sort_int(int* a, int low, int high);
std::pair<int, int> partition_dual_pivot_int(int* a, int low, int high, int pivotIndex1, int pivotIndex2);
std::pair<int, int> partition_single_pivot_int(int* a, int low, int high, int pivotIndex1, int);
void mixed_insertion_sort_long(long* a, int low, int high);
void mixed_insertion_sort_float(float* a, int low, int high);
std::pair<int, int> partition_dual_pivot_float(float* a, int low, int high, int pivotIndex1, int pivotIndex2);
std::pair<int, int> partition_single_pivot_float(float* a, int low, int high, int pivotIndex1, int);
void mixed_insertion_sort_double(double* a, int low, int high);
std::pair<int, int> partition_dual_pivot_double(double* a, int low, int high, int pivotIndex1, int pivotIndex2);
std::pair<int, int> partition_single_pivot_double(double* a, int low, int high, int pivotIndex1, int);
void mixed_insertion_sort_byte(signed char* a, int low, int high);
void mixed_insertion_sort_char(char* a, int low, int high);
void mixed_insertion_sort_short(short* a, int low, int high);
std::pair<int, int> partition_dual_pivot_long(long* a, int low, int high, int pivotIndex1, int pivotIndex2);
std::pair<int, int> partition_single_pivot_long(long* a, int low, int high, int pivotIndex1, int);
bool tryMergeRuns_int(Sorter<int>* sorter, int* a, int low, int size);
bool tryMergeRuns_long(Sorter<long>* sorter, long* a, int low, int size);
bool tryMergeRuns_float(Sorter<float>* sorter, float* a, int low, int size);
bool tryMergeRuns_double(Sorter<double>* sorter, double* a, int low, int size);
int* mergeRuns_int(int* a, int* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);
long* mergeRuns_long(long* a, long* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);
float* mergeRuns_float(float* a, float* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);
double* mergeRuns_double(double* a, double* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);
bool tryMergeRuns_byte(signed char* a, int low, int size);
bool tryMergeRuns_char(char* a, int low, int size);
bool tryMergeRuns_short(short* a, int low, int size);
signed char* mergeRuns_byte(signed char* a, signed char* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);
char* mergeRuns_char(char* a, char* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);
short* mergeRuns_short(short* a, short* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);
void sort_int_sequential(Sorter<int>* sorter, int* a, int bits, int low, int high);
void sort_long_sequential(Sorter<long>* sorter, long* a, int bits, int low, int high);
void sort_float_sequential(Sorter<float>* sorter, float* a, int bits, int low, int high);
void sort_double_sequential(Sorter<double>* sorter, double* a, int bits, int low, int high);

// Definitions

inline void mixed_insertion_sort_int(int* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);

    if (end == high) {
        // Simple insertion sort for tiny arrays
        for (int i; ++low < end; ) {
            int ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Pin insertion sort + pair insertion sort
        int pin = a[end];

        for (int i, p = high; ++low < end; ) {
            int ai = a[i = low];

            if (ai < a[i - 1]) { // Small element
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;

            } else if (p > i && ai > pin) { // Large element
                while (a[--p] > pin);

                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }

                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }

        // Pair insertion sort on remaining part
        for (int i; low < high; ++low) {
            int a1 = a[i = low], a2 = a[++low];

            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;

                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;

            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;

                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

inline std::pair<int, int> partition_dual_pivot_int(int* a, int low, int high, int pivotIndex1, int pivotIndex2) {
    int end = high - 1;
    int lower = low;
    int upper = end;

    int e1 = pivotIndex1;
    int e5 = pivotIndex2;
    int pivot1 = a[e1];
    int pivot2 = a[e5];

    a[e1] = a[lower];
    a[e5] = a[upper];

    while (a[++lower] < pivot1);
    while (a[--upper] > pivot2);

    (void)--lower;
    for (int k = ++upper; --k > lower; ) {
        int ak = a[k];

        if (LIKELY(k > lower + 1)) {
            PREFETCH_READ(&a[k - 1]);
        }

        if (UNLIKELY(ak < pivot1)) {
            while (lower < k) {
                if (LIKELY(a[++lower] >= pivot1)) {
                    if (UNLIKELY(a[lower] > pivot2)) {
                        a[k] = a[--upper];
                        a[upper] = a[lower];
                    } else {
                        a[k] = a[lower];
                    }
                    a[lower] = ak;
                    break;
                }
            }
        } else if (UNLIKELY(ak > pivot2)) {
            a[k] = a[--upper];
            a[upper] = ak;
        }
    }

    a[low] = a[lower];
    a[lower] = pivot1;
    a[end] = a[upper];
    a[upper] = pivot2;

    return std::make_pair(lower, upper);
}

inline std::pair<int, int> partition_single_pivot_int(int* a, int low, int high, int pivotIndex1, int) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    int e3 = pivotIndex1;
    int pivot = a[e3];

    a[e3] = a[lower];

    for (int k = ++upper; --k > lower; ) {
        int ak = a[k];

        if (ak != pivot) {
            a[k] = pivot;

            if (ak < pivot) {
                while (a[++lower] < pivot);

                if (a[lower] > pivot) {
                    a[--upper] = a[lower];
                }
                a[lower] = ak;
            } else {
                a[--upper] = ak;
            }
        }
    }

    a[low] = a[lower];
    a[lower] = pivot;

    return std::make_pair(lower, upper);
}

inline void mixed_insertion_sort_long(long* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);

    if (end == high) {
        for (int i; ++low < end; ) {
            long ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        long pin = a[end];

        for (int i, p = high; ++low < end; ) {
            long ai = a[i = low];

            if (ai < a[i - 1]) {
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;

            } else if (p > i && ai > pin) {
                while (a[--p] > pin);

                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }

                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }

        for (int i; low < high; ++low) {
            long a1 = a[i = low], a2 = a[++low];

            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;

                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;

            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;

                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

inline void mixed_insertion_sort_float(float* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);

    if (end == high) {
        // Simple insertion sort for tiny arrays
        for (int i; ++low < end; ) {
            float ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Pin insertion sort + pair insertion sort
        float pin = a[end];

        for (int i, p = high; ++low < end; ) {
            float ai = a[i = low];

            if (ai < a[i - 1]) { // Small element
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;

            } else if (p > i && ai > pin) { // Large element
                while (a[--p] > pin);

                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }

                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }

        // Pair insertion sort on remaining part
        for (int i; low < high; ++low) {
            float a1 = a[i = low], a2 = a[++low];

            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;

                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;

            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;

                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

inline std::pair<int, int> partition_dual_pivot_float(float* a, int low, int high, int pivotIndex1, int pivotIndex2) {
    int end = high - 1;
    int lower = low;
    int upper = end;

    int e1 = pivotIndex1;
    int e5 = pivotIndex2;
    float pivot1 = a[e1];
    float pivot2 = a[e5];

    a[e1] = a[lower];
    a[e5] = a[upper];

    while (a[++lower] < pivot1);
    while (a[--upper] > pivot2);

    (void)--lower;
    for (int k = ++upper; --k > lower; ) {
        float ak = a[k];

        if (LIKELY(k > lower + 1)) {
            PREFETCH_READ(&a[k - 1]);
        }

        if (UNLIKELY(ak < pivot1)) {
            while (lower < k) {
                if (LIKELY(a[++lower] >= pivot1)) {
                    if (UNLIKELY(a[lower] > pivot2)) {
                        a[k] = a[--upper];
                        a[upper] = a[lower];
                    } else {
                        a[k] = a[lower];
                    }
                    a[lower] = ak;
                    break;
                }
            }
        } else if (UNLIKELY(ak > pivot2)) {
            a[k] = a[--upper];
            a[upper] = ak;
        }
    }

    a[low] = a[lower];
    a[lower] = pivot1;
    a[end] = a[upper];
    a[upper] = pivot2;

    return std::make_pair(lower, upper);
}

inline std::pair<int, int> partition_single_pivot_float(float* a, int low, int high, int pivotIndex1, int) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    int e3 = pivotIndex1;
    float pivot = a[e3];

    a[e3] = a[lower];

    for (int k = ++upper; --k > lower; ) {
        float ak = a[k];

        if (ak != pivot) {
            a[k] = pivot;

            if (ak < pivot) {
                while (a[++lower] < pivot);

                if (a[lower] > pivot) {
                    a[--upper] = a[lower];
                }
                a[lower] = ak;
            } else {
                a[--upper] = ak;
            }
        }
    }

    a[low] = a[lower];
    a[lower] = pivot;

    return std::make_pair(lower, upper);
}

inline void mixed_insertion_sort_double(double* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);

    if (end == high) {
        for (int i; ++low < end; ) {
            double ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        double pin = a[end];

        for (int i, p = high; ++low < end; ) {
            double ai = a[i = low];

            if (ai < a[i - 1]) {
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;

            } else if (p > i && ai > pin) {
                while (a[--p] > pin);

                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }

                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }

        for (int i; low < high; ++low) {
            double a1 = a[i = low], a2 = a[++low];

            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;

                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;

            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;

                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

inline std::pair<int, int> partition_dual_pivot_double(double* a, int low, int high, int pivotIndex1, int pivotIndex2) {
    int end = high - 1;
    int lower = low;
    int upper = end;

    int e1 = pivotIndex1;
    int e5 = pivotIndex2;
    double pivot1 = a[e1];
    double pivot2 = a[e5];

    a[e1] = a[lower];
    a[e5] = a[upper];

    while (a[++lower] < pivot1);
    while (a[--upper] > pivot2);

    (void)--lower;
    for (int k = ++upper; --k > lower; ) {
        double ak = a[k];

        if (LIKELY(k > lower + 1)) {
            PREFETCH_READ(&a[k - 1]);
        }

        if (UNLIKELY(ak < pivot1)) {
            while (lower < k) {
                if (LIKELY(a[++lower] >= pivot1)) {
                    if (UNLIKELY(a[lower] > pivot2)) {
                        a[k] = a[--upper];
                        a[upper] = a[lower];
                    } else {
                        a[k] = a[lower];
                    }
                    a[lower] = ak;
                    break;
                }
            }
        } else if (UNLIKELY(ak > pivot2)) {
            a[k] = a[--upper];
            a[upper] = ak;
        }
    }

    a[low] = a[lower];
    a[lower] = pivot1;
    a[end] = a[upper];
    a[upper] = pivot2;

    return std::make_pair(lower, upper);
}

inline std::pair<int, int> partition_single_pivot_double(double* a, int low, int high, int pivotIndex1, int) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    int e3 = pivotIndex1;
    double pivot = a[e3];

    a[e3] = a[lower];

    for (int k = ++upper; --k > lower; ) {
        double ak = a[k];

        if (ak != pivot) {
            a[k] = pivot;

            if (ak < pivot) {
                while (a[++lower] < pivot);

                if (a[lower] > pivot) {
                    a[--upper] = a[lower];
                }
                a[lower] = ak;
            } else {
                a[--upper] = ak;
            }
        }
    }

    a[low] = a[lower];
    a[lower] = pivot;

    return std::make_pair(lower, upper);
}

inline void mixed_insertion_sort_byte(signed char* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);

    if (end == high) {
        // Simple insertion sort for tiny byte arrays
        for (int i; ++low < end; ) {
            signed char ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Pin insertion sort optimized for byte values
        signed char pin = a[end];

        for (int i, p = high; ++low < end; ) {
            signed char ai = a[i = low];

            if (ai < a[i - 1]) { // Small element
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;

            } else if (p > i && ai > pin) { // Large element
                while (a[--p] > pin);

                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }

                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }

        // Pair insertion sort optimized for byte values
        for (int i; low < high; ++low) {
            signed char a1 = a[i = low], a2 = a[++low];

            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;

                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;

            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;

                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

inline void mixed_insertion_sort_char(char* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);

    if (end == high) {
        // Simple insertion sort for tiny char arrays
        for (int i; ++low < end; ) {
            char ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Pin insertion sort optimized for char values
        char pin = a[end];

        for (int i, p = high; ++low < end; ) {
            char ai = a[i = low];

            if (ai < a[i - 1]) { // Small element
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;

            } else if (p > i && ai > pin) { // Large element
                while (a[--p] > pin);

                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }

                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }

        // Pair insertion sort optimized for char values
        for (int i; low < high; ++low) {
            char a1 = a[i = low], a2 = a[++low];

            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;

                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;

            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;

                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

inline void mixed_insertion_sort_short(short* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);

    if (end == high) {
        // Simple insertion sort for tiny short arrays
        for (int i; ++low < end; ) {
            short ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Pin insertion sort optimized for short values
        short pin = a[end];

        for (int i, p = high; ++low < end; ) {
            short ai = a[i = low];

            if (ai < a[i - 1]) { // Small element
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;

            } else if (p > i && ai > pin) { // Large element
                while (a[--p] > pin);

                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }

                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }

        // Pair insertion sort optimized for short values
        for (int i; low < high; ++low) {
            short a1 = a[i = low], a2 = a[++low];

            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;

                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;

            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;

                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

inline std::pair<int, int> partition_dual_pivot_long(long* a, int low, int high, int pivotIndex1, int pivotIndex2) {
    int end = high - 1;
    int lower = low;
    int upper = end;

    int e1 = pivotIndex1;
    int e5 = pivotIndex2;
    long pivot1 = a[e1];
    long pivot2 = a[e5];

    a[e1] = a[lower];
    a[e5] = a[upper];

    while (a[++lower] < pivot1);
    while (a[--upper] > pivot2);

    (void)--lower;
    for (int k = ++upper; --k > lower; ) {
        long ak = a[k];

        if (LIKELY(k > lower + 1)) {
            PREFETCH_READ(&a[k - 1]);
        }

        if (UNLIKELY(ak < pivot1)) {
            while (lower < k) {
                if (LIKELY(a[++lower] >= pivot1)) {
                    if (UNLIKELY(a[lower] > pivot2)) {
                        a[k] = a[--upper];
                        a[upper] = a[lower];
                    } else {
                        a[k] = a[lower];
                    }
                    a[lower] = ak;
                    break;
                }
            }
        } else if (UNLIKELY(ak > pivot2)) {
            a[k] = a[--upper];
            a[upper] = ak;
        }
    }

    a[low] = a[lower];
    a[lower] = pivot1;
    a[end] = a[upper];
    a[upper] = pivot2;

    return std::make_pair(lower, upper);
}

inline std::pair<int, int> partition_single_pivot_long(long* a, int low, int high, int pivotIndex1, int) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    int e3 = pivotIndex1;
    long pivot = a[e3];

    a[e3] = a[lower];

    for (int k = ++upper; --k > lower; ) {
        long ak = a[k];

        if (ak != pivot) {
            a[k] = pivot;

            if (ak < pivot) {
                while (a[++lower] < pivot);

                if (a[lower] > pivot) {
                    a[--upper] = a[lower];
                }
                a[lower] = ak;
            } else {
                a[--upper] = ak;
            }
        }
    }

    a[low] = a[lower];
    a[lower] = pivot;

    return std::make_pair(lower, upper);
}

inline bool tryMergeRuns_int(Sorter<int>* sorter, int* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;

    // Run detection logic (same as generic version but type-specific)
    for (int k = low + 1; k < high; ) {

        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);

            // Reverse into ascending order
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                int temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        } else {
            int ak = a[k];
            while (++k < high && ak == a[k]);

            if (k < high) {
                continue;
            }
        }

        if (run.empty()) {
            if (k == high) {
                return true;
            }

            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }

            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);

        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }

            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }

    // Merge runs with advanced parallel coordination (matching Java's approach)
    if (count > 1) {
        std::vector<int> b(size);

        // Advanced parallel run merging logic with MIN_RUN_COUNT threshold
        if (count >= MIN_RUN_COUNT && sorter != nullptr) {
            // Use Java-style forkMe/getDestination pattern for parallel merging
            RunMerger<int> merger(a, b.data(), low, 1, run, 0, count);
            RunMerger<int>& forked = merger.forkMe();
            int* result = forked.getDestination();

            // Copy back to main array if needed (matching Java's buffer management)
            if (result != a) {
                std::copy(result + low, result + low + size, a + low);
            }
        } else {
            // Sequential merging with local variable optimization
            int* localA = a; // Local variable optimization (matching Java pattern)
            mergeRuns_int(localA, b.data(), low, 1, false, run, 0, count);
        }
    }
    return true;
}

inline bool tryMergeRuns_long(Sorter<long>* sorter, long* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;

    for (int k = low + 1; k < high; ) {

        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);

            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                long temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        } else {
            long ak = a[k];
            while (++k < high && ak == a[k]);

            if (k < high) {
                continue;
            }
        }

        if (run.empty()) {
            if (k == high) {
                return true;
            }

            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }

            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);

        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }

            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }

    if (count > 1) {
        std::vector<long> b(size);
        mergeRuns_long(a, b.data(), low, 1, count >= MIN_RUN_COUNT && sorter != nullptr, run, 0, count);
    }
    return true;
}

inline bool tryMergeRuns_float(Sorter<float>* sorter, float* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;

    for (int k = low + 1; k < high; ) {

        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);

            // Reverse into ascending order
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                float temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        } else {
            float ak = a[k];
            while (++k < high && ak == a[k]);

            if (k < high) {
                continue;
            }
        }

        if (run.empty()) {
            if (k == high) {
                return true;
            }

            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }

            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);

        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }

            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }

    if (count > 1) {
        std::vector<float> b(size);
        mergeRuns_float(a, b.data(), low, 1, count >= MIN_RUN_COUNT && sorter != nullptr, run, 0, count);
    }
    return true;
}

inline bool tryMergeRuns_double(Sorter<double>* sorter, double* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;

    for (int k = low + 1; k < high; ) {

        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);

            // Reverse into ascending order
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                double temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        } else {
            double ak = a[k];
            while (++k < high && ak == a[k]);

            if (k < high) {
                continue;
            }
        }

        if (run.empty()) {
            if (k == high) {
                return true;
            }

            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }

            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);

        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }

            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }

    if (count > 1) {
        std::vector<double> b(size);
        mergeRuns_double(a, b.data(), low, 1, count >= MIN_RUN_COUNT && sorter != nullptr, run, 0, count);
    }
    return true;
}

inline int* mergeRuns_int(int* a, int* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }

    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);

    int* a1 = mergeRuns_int(a, b, offset, -aim, parallel, run, lo, mi);
    int* a2 = mergeRuns_int(a, b, offset,    0, parallel, run, mi, hi);

    int* dst = (a1 == a) ? b : a;

    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];

    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

inline long* mergeRuns_long(long* a, long* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }

    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);

    long* a1 = mergeRuns_long(a, b, offset, -aim, parallel, run, lo, mi);
    long* a2 = mergeRuns_long(a, b, offset,    0, parallel, run, mi, hi);

    long* dst = (a1 == a) ? b : a;

    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];

    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

inline float* mergeRuns_float(float* a, float* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }

    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);

    float* a1 = mergeRuns_float(a, b, offset, -aim, parallel, run, lo, mi);
    float* a2 = mergeRuns_float(a, b, offset,    0, parallel, run, mi, hi);

    float* dst = (a1 == a) ? b : a;

    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];

    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

inline double* mergeRuns_double(double* a, double* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }

    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);

    double* a1 = mergeRuns_double(a, b, offset, -aim, parallel, run, lo, mi);
    double* a2 = mergeRuns_double(a, b, offset,    0, parallel, run, mi, hi);

    double* dst = (a1 == a) ? b : a;

    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];

    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

inline bool tryMergeRuns_byte(signed char* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;

    // Run detection logic (matching Java's approach for byte arrays)
    for (int k = low + 1; k < high; ) {

        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);

            // Reverse into ascending order
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                signed char temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        } else {
            signed char ak = a[k];
            while (++k < high && ak == a[k]);

            if (k < high) {
                continue;
            }
        }

        if (run.empty()) {
            if (k == high) {
                return true;
            }

            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }

            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);

        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }

            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }

    // Merge runs for byte arrays (simplified, no parallel coordination)
    if (count > 1) {
        std::vector<signed char> b(size);
        mergeRuns_byte(a, b.data(), low, 1, false, run, 0, count);
    }
    return true;
}

inline bool tryMergeRuns_char(char* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;

    for (int k = low + 1; k < high; ) {

        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);

            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                char temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        } else {
            char ak = a[k];
            while (++k < high && ak == a[k]);

            if (k < high) {
                continue;
            }
        }

        if (run.empty()) {
            if (k == high) {
                return true;
            }

            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }

            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);

        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }

            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }

    if (count > 1) {
        std::vector<char> b(size);
        mergeRuns_char(a, b.data(), low, 1, false, run, 0, count);
    }
    return true;
}

inline bool tryMergeRuns_short(short* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;

    for (int k = low + 1; k < high; ) {

        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);

            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                short temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        } else {
            short ak = a[k];
            while (++k < high && ak == a[k]);

            if (k < high) {
                continue;
            }
        }

        if (run.empty()) {
            if (k == high) {
                return true;
            }

            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }

            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);

        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }

            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }

    if (count > 1) {
        std::vector<short> b(size);
        mergeRuns_short(a, b.data(), low, 1, false, run, 0, count);
    }
    return true;
}

inline signed char* mergeRuns_byte(signed char* a, signed char* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }

    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);

    signed char* a1 = mergeRuns_byte(a, b, offset, -aim, parallel, run, lo, mi);
    signed char* a2 = mergeRuns_byte(a, b, offset,    0, parallel, run, mi, hi);

    signed char* dst = (a1 == a) ? b : a;

    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];

    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

inline char* mergeRuns_char(char* a, char* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }

    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);

    char* a1 = mergeRuns_char(a, b, offset, -aim, parallel, run, lo, mi);
    char* a2 = mergeRuns_char(a, b, offset,    0, parallel, run, mi, hi);

    char* dst = (a1 == a) ? b : a;

    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];

    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

inline short* mergeRuns_short(short* a, short* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }

    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);

    short* a1 = mergeRuns_short(a, b, offset, -aim, parallel, run, lo, mi);
    short* a2 = mergeRuns_short(a, b, offset,    0, parallel, run, mi, hi);

    short* dst = (a1 == a) ? b : a;

    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];

    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

inline void sort_int_sequential(Sorter<int>* sorter, int* a, int bits, int low, int high) {
    while (true) {
        int end = high - 1;
        int size = high - low;

        // Use mixed insertion sort on small non-leftmost parts
        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            sort_intrinsic(a, low, high, mixed_insertion_sort_int);
            return;
        }

        // Use insertion sort on small leftmost parts
        if (size < MAX_INSERTION_SORT_SIZE) {
            sort_intrinsic(a, low, high, insertionSort_int);
            return;
        }

        // Try merge runs for nearly sorted data
        if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
                && tryMergeRuns_int(sorter, a, low, size)) {
            return;
        }

        // Switch to heap sort if execution time is becoming quadratic
        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            sort_intrinsic(a, low, high, heapSort_int);
            return;
        }

        // Five-element pivot selection
        int step = (size >> 3) * 3 + 3;
        int e1 = low + step;
        int e5 = end - step;
        int e3 = (e1 + e5) >> 1;
        int e2 = (e1 + e3) >> 1;
        int e4 = (e3 + e5) >> 1;

        // Sort 5-element sample
        sort5Network(a, e1, e2, e3, e4, e5);

        int lower, upper;

        // Dual-pivot partitioning
        if (a[e1] < a[e2] && a[e2] < a[e3] && a[e3] < a[e4] && a[e4] < a[e5]) {
            auto pivotIndices = partition_intrinsic(a, low, high, e1, e5, partition_dual_pivot_int);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            // Fork parallel tasks if sorter available
            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, lower + 1, upper);
                sorter->forkSorter(bits | 1, upper + 1, high);
            } else {
                sort_int_sequential(sorter, a, bits | 1, lower + 1, upper);
                sort_int_sequential(sorter, a, bits | 1, upper + 1, high);
            }
        } else {
            // Single-pivot partitioning
            auto pivotIndices = partition_intrinsic(a, low, high, e3, e3, partition_single_pivot_int);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, upper, high);
            } else {
                sort_int_sequential(sorter, a, bits | 1, upper, high);
            }
        }

        high = lower; // Continue with left part
    }
}

inline void sort_long_sequential(Sorter<long>* sorter, long* a, int bits, int low, int high) {
    while (true) {
        int end = high - 1;
        int size = high - low;

        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            sort_intrinsic(a, low, high, mixed_insertion_sort_long);
            return;
        }

        if (size < MAX_INSERTION_SORT_SIZE) {
            sort_intrinsic(a, low, high, insertionSort_long);
            return;
        }

        if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
                && tryMergeRuns_long(sorter, a, low, size)) {
            return;
        }

        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            sort_intrinsic(a, low, high, heapSort_long);
            return;
        }

        int step = (size >> 3) * 3 + 3;
        int e1 = low + step;
        int e5 = end - step;
        int e3 = (e1 + e5) >> 1;
        int e2 = (e1 + e3) >> 1;
        int e4 = (e3 + e5) >> 1;

        sort5Network(a, e1, e2, e3, e4, e5);

        int lower, upper;

        if (a[e1] < a[e2] && a[e2] < a[e3] && a[e3] < a[e4] && a[e4] < a[e5]) {
            auto pivotIndices = partition_dual_pivot_long(a, low, high, e1, e5);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, lower + 1, upper);
                sorter->forkSorter(bits | 1, upper + 1, high);
            } else {
                sort_long_sequential(sorter, a, bits | 1, lower + 1, upper);
                sort_long_sequential(sorter, a, bits | 1, upper + 1, high);
            }
        } else {
            auto pivotIndices = partition_single_pivot_long(a, low, high, e3, e3);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, upper, high);
            } else {
                sort_long_sequential(sorter, a, bits | 1, upper, high);
            }
        }

        high = lower;
    }
}

inline void sort_float_sequential(Sorter<float>* sorter, float* a, int bits, int low, int high) {
    while (true) {
        int end = high - 1;
        int size = high - low;

        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            sort_intrinsic(a, low, high, mixed_insertion_sort_float);
            return;
        }

        if (size < MAX_INSERTION_SORT_SIZE) {
            sort_intrinsic(a, low, high, insertionSort_float);
            return;
        }

        if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
                && tryMergeRuns_float(sorter, a, low, size)) {
            return;
        }

        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            sort_intrinsic(a, low, high, heapSort_float);
            return;
        }

        int step = (size >> 3) * 3 + 3;
        int e1 = low + step;
        int e5 = end - step;
        int e3 = (e1 + e5) >> 1;
        int e2 = (e1 + e3) >> 1;
        int e4 = (e3 + e5) >> 1;

        sort5Network(a, e1, e2, e3, e4, e5);

        int lower, upper;

        if (a[e1] < a[e2] && a[e2] < a[e3] && a[e3] < a[e4] && a[e4] < a[e5]) {
            auto pivotIndices = partition_dual_pivot_float(a, low, high, e1, e5);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, lower + 1, upper);
                sorter->forkSorter(bits | 1, upper + 1, high);
            } else {
                sort_float_sequential(sorter, a, bits | 1, lower + 1, upper);
                sort_float_sequential(sorter, a, bits | 1, upper + 1, high);
            }
        } else {
            auto pivotIndices = partition_single_pivot_float(a, low, high, e3, e3);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, upper, high);
            } else {
                sort_float_sequential(sorter, a, bits | 1, upper, high);
            }
        }

        high = lower;
    }
}

inline void sort_double_sequential(Sorter<double>* sorter, double* a, int bits, int low, int high) {
    while (true) {
        int end = high - 1;
        int size = high - low;

        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            sort_intrinsic(a, low, high, mixed_insertion_sort_double);
            return;
        }

        if (size < MAX_INSERTION_SORT_SIZE) {
            sort_intrinsic(a, low, high, insertionSort_double);
            return;
        }

        if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
                && tryMergeRuns_double(sorter, a, low, size)) {
            return;
        }

        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            sort_intrinsic(a, low, high, heapSort_double);
            return;
        }

        int step = (size >> 3) * 3 + 3;
        int e1 = low + step;
        int e5 = end - step;
        int e3 = (e1 + e5) >> 1;
        int e2 = (e1 + e3) >> 1;
        int e4 = (e3 + e5) >> 1;

        sort5Network(a, e1, e2, e3, e4, e5);

        int lower, upper;

        if (a[e1] < a[e2] && a[e2] < a[e3] && a[e3] < a[e4] && a[e4] < a[e5]) {
            auto pivotIndices = partition_dual_pivot_double(a, low, high, e1, e5);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, lower + 1, upper);
                sorter->forkSorter(bits | 1, upper + 1, high);
            } else {
                sort_double_sequential(sorter, a, bits | 1, lower + 1, upper);
                sort_double_sequential(sorter, a, bits | 1, upper + 1, high);
            }
        } else {
            auto pivotIndices = partition_single_pivot_double(a, low, high, e3, e3);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, upper, high);
            } else {
                sort_double_sequential(sorter, a, bits | 1, upper, high);
            }
        }

        high = lower;
    }
}


} // namespace dual_pivot

#endif // DPQS_SEQUENTIAL_SORTERS_HPP
