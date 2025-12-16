#ifndef DPQS_INSERTION_SORT_HPP
#define DPQS_INSERTION_SORT_HPP

#include "utils.hpp"

namespace dual_pivot {

template<typename T>
FORCE_INLINE void insertionSort(T* a, int low, int high) {
    // Phase 6: Cache-friendly insertion sort with prefetching
    for (int i, k = low; ++k < high; ) {
        T ai = a[i = k];

        // Prefetch next elements to improve cache performance
        // This is crucial for the "memory wall" - CPU-memory speed gap
        if (LIKELY(k + 1 < high)) {
            PREFETCH_READ(&a[k + 1]);
        }

        // Use branch prediction hints for the common case (already sorted)
        if (UNLIKELY(ai < a[i - 1])) {
            // Element is out of place - shift elements to make room
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

template<typename T>
void mixedInsertionSort(T* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);  // Calculate transition point

    if (end == high) {
        // Tiny array: use simple insertion sort
        for (int i; ++low < end; ) {
            T ai = a[i = low];

            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Mixed strategy: pin insertion sort + pair insertion sort

        // Phase 1: Pin insertion sort on the initial part
        T pin = a[end];  // Use pin element to separate small/large values

        for (int i, p = high; ++low < end; ) {
            T ai = a[i = low];

            if (ai < a[i - 1]) { // Small element - needs insertion
                // Insert small element into sorted part
                a[i] = a[i - 1];
                --i;

                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;

            } else if (p > i && ai > pin) { // Large element - move to end
                // Find position for large element
                while (a[--p] > pin);

                // Swap large element to proper position
                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }

                // Insert the swapped element (now small) into sorted part
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }

        // Phase 2: Pair insertion sort on remaining part
        // Process two elements at a time for better cache efficiency
        for (int i; low < high; ++low) {
            T a1 = a[i = low], a2 = a[++low];

            // Insert pair of elements efficiently
            if (a1 > a2) {
                // First element is larger - insert in reverse order
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;

                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;

            } else if (a1 < a[i - 1]) {
                // Both elements need insertion
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

static void insertionSort_int(int* a, int low, int high) {
    for (int i, k = low; ++k < high; ) {
        int ai = a[i = k];

        if (LIKELY(k + 1 < high)) {
            PREFETCH_READ(&a[k + 1]);
        }

        if (UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

static void insertionSort_long(long* a, int low, int high) {
    for (int i, k = low; ++k < high; ) {
        long ai = a[i = k];

        if (LIKELY(k + 1 < high)) {
            PREFETCH_READ(&a[k + 1]);
        }

        if (UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

static void insertionSort_float(float* a, int low, int high) {
    for (int i, k = low; ++k < high; ) {
        float ai = a[i = k];

        if (LIKELY(k + 1 < high)) {
            PREFETCH_READ(&a[k + 1]);
        }

        if (UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

static void insertionSort_double(double* a, int low, int high) {
    for (int i, k = low; ++k < high; ) {
        double ai = a[i = k];

        if (LIKELY(k + 1 < high)) {
            PREFETCH_READ(&a[k + 1]);
        }

        if (UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

static void countingSort(char* a, int low, int high) {
    static constexpr int NUM_CHAR_VALUES = 1 << 16; // Full Unicode range
    std::vector<int> count(NUM_CHAR_VALUES, 0);

    // Direct unsigned access for characters (matching Java's approach)
    for (int i = high; i > low; ) {
        ++count[static_cast<unsigned char>(a[--i])];
    }

    // Optimized placement for character ranges
    int index = low;
    for (int i = 0; i < NUM_CHAR_VALUES; i++) {
        if (count[i] > 0) {
            int cnt = count[i];
            char value = static_cast<char>(i);
            while (cnt-- > 0) {
                a[index++] = value;
            }
        }
    }
}

} // namespace dual_pivot

#endif // DPQS_INSERTION_SORT_HPP
