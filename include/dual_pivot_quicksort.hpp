#ifndef DUAL_PIVOT_QUICKSORT_HPP
#define DUAL_PIVOT_QUICKSORT_HPP

#include "dpqs/utils.hpp"
#include "dpqs/types.hpp"
#include "dpqs/parallel/parallel_sort.hpp"
#include "dpqs/sequential_sorters.hpp"
#include "dpqs/counting_sort.hpp"
#include "dpqs/float_sort.hpp"
#include <stdexcept>
#include <string>
#include <thread>
#include <iterator>
#include <cmath>
#include <type_traits>

namespace dual_pivot {

// -----------------------------------------------------------------------------
// Core Generic Sort Implementation
// -----------------------------------------------------------------------------

/**
 * @brief Main entry point for Dual-Pivot Quicksort.
 *
 * This generic function handles all supported data types and execution modes (sequential/parallel).
 * It automatically dispatches to the most appropriate sorting strategy:
 * - Counting Sort for small integral types (char, short).
 * - Parallel Dual-Pivot Quicksort for large arrays of other types.
 * - Sequential Dual-Pivot Quicksort for smaller arrays or when parallelism is disabled.
 * - Specialized handling for floating-point types (NaNs, -0.0).
 *
 * @tparam T The element type.
 * @param a Pointer to the array.
 * @param parallelism Number of threads to use (0 or 1 for sequential).
 * @param low Starting index (inclusive).
 * @param high Ending index (exclusive).
 */
template<typename T>
void sort(T* a, int parallelism, int low, int high) {
    if (low >= high) return;
    checkNotNull(a, "array");
    if (low < 0 || high < 0) {
        throw std::out_of_range("Invalid range");
    }

    if (checkEarlyTermination(a, low, high)) {
        return;
    }

    int size = high - low;

    // Case 1: Small integral types (1 or 2 bytes) -> Counting Sort
    if constexpr (std::is_integral_v<T> && sizeof(T) <= 2) {
        // Use smaller threshold for 1-byte types (smaller frequency array overhead)
        int threshold = (sizeof(T) == 1) ? MIN_BYTE_COUNTING_SORT_SIZE : MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE;

        if (size >= threshold) {
            counting_sort(a, low, high);
        } else {
            // Fallback to sequential sort (which handles insertion sort for small arrays)
            sort_sequential<T>(nullptr, a, 0, low, high);
        }
        return;
    }

    // Case 2: Parallel Sort (for types > 2 bytes)
    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);
        std::vector<T> b(depth == 0 ? 0 : size);
        auto* sorter = new AdvancedSorter<T>(nullptr, a, depth == 0 ? nullptr : b.data(), low, size, low, depth);
        sorter->invoke();
        sorter->wait();
        delete sorter;
        return;
    }

    // Case 3: Sequential Sort (fallback)
    if constexpr (std::is_floating_point_v<T>) {
        sort_floats(a, low, high);
    } else {
        sort_sequential<T>(nullptr, a, 0, low, high);
    }
}

// -----------------------------------------------------------------------------
// Public API: Convenience wrappers
// -----------------------------------------------------------------------------

template<typename T>
void sort(T* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) throw std::invalid_argument("Array length cannot be negative");

    // Default to hardware concurrency
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

// -----------------------------------------------------------------------------
// Public API: Container & Iterator Adapters
// -----------------------------------------------------------------------------

template<typename Container>
void sort(Container& container) {
    sort(container.data(), std::thread::hardware_concurrency(), 0, static_cast<int>(container.size()));
}

template<typename Container>
void sort(Container& container, int parallelism) {
    sort(container.data(), parallelism, 0, static_cast<int>(container.size()));
}

template<typename RandomAccessIterator>
void dual_pivot_quicksort(RandomAccessIterator first, RandomAccessIterator last) {
    static_assert(std::is_same_v<typename std::iterator_traits<RandomAccessIterator>::iterator_category,
                                std::random_access_iterator_tag>,
                  "dual_pivot_quicksort requires random access iterators");

    if (first >= last) return;
    int size = last - first;
    if (size <= 1) return;

    auto* a = &(*first);
    // Use 0 parallelism for default sequential behavior of this specific function name
    sort(a, 0, 0, size);
}

template<typename RandomAccessIterator>
void dual_pivot_quicksort_parallel(RandomAccessIterator first, RandomAccessIterator last,
                                  int parallelism = std::thread::hardware_concurrency()) {
    static_assert(std::is_same_v<typename std::iterator_traits<RandomAccessIterator>::iterator_category,
                                std::random_access_iterator_tag>,
                  "dual_pivot_quicksort requires random access iterators");

    if (first >= last) return;
    int size = last - first;
    if (size <= 1) return;

    auto* a = &(*first);
    sort(a, parallelism, 0, size);
}

} // namespace dual_pivot

#endif // DUAL_PIVOT_QUICKSORT_HPP
