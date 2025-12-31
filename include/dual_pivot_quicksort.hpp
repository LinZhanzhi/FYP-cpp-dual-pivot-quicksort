#ifndef DUAL_PIVOT_QUICKSORT_HPP
#define DUAL_PIVOT_QUICKSORT_HPP

#include "dpqs/utils.hpp"
#include "dpqs/types.hpp"
#include "dpqs/parallel/parallel_sort.hpp"
#include "dpqs/sequential_sorters.hpp"
#include "dpqs/counting_sort.hpp"
#include "dpqs/float_sort.hpp"
#include "dpqs/iterator_sort.hpp"
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
 * @brief Main entry point for Dual-Pivot Quicksort with custom comparator.
 *
 * This generic function handles all supported data types and execution modes (sequential/parallel).
 * It automatically dispatches to the most appropriate sorting strategy:
 * - Parallel Dual-Pivot Quicksort for large arrays.
 * - Sequential Dual-Pivot Quicksort for smaller arrays or when parallelism is disabled.
 *
 * @tparam T The element type.
 * @tparam Compare The comparator type.
 * @param a Pointer to the array.
 * @param parallelism Number of threads to use (0 or 1 for sequential).
 * @param low Starting index (inclusive).
 * @param high Ending index (exclusive).
 * @param comp The comparator to use.
 */
template<typename T, typename Compare>
void sort(T* a, int parallelism, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp) {
    if (low >= high) return;
    checkNotNull(a, "array");
    if (low < 0 || high < 0) {
        throw std::out_of_range("Invalid range");
    }

    if (checkEarlyTermination(a, low, high, comp)) {
        return;
    }

    std::ptrdiff_t size = high - low;

    // Case 2: Parallel Sort (for types > 2 bytes)
    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);
        // Use V3 Parallel QuickSort directly (Work Stealing)
        parallelQuickSort(a, depth, low, high, comp, parallelism);
        return;
    }

    // Case 3: Sequential Sort (fallback)
    sort_sequential<T, Compare>(nullptr, a, 0, low, high, comp);
}

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
void sort(T* a, int parallelism, std::ptrdiff_t low, std::ptrdiff_t high) {
    if (low >= high) return;
    checkNotNull(a, "array");
    if (low < 0 || high < 0) {
        throw std::out_of_range("Invalid range");
    }

    if (checkEarlyTermination(a, low, high)) {
        return;
    }

    std::ptrdiff_t size = high - low;

    // Case 1: Small integral types (1 or 2 bytes) -> Counting Sort
    if constexpr (std::is_integral_v<T> && sizeof(T) <= 2) {
        // Use smaller threshold for 1-byte types (smaller frequency array overhead)
        std::ptrdiff_t threshold = (sizeof(T) == 1) ? MIN_BYTE_COUNTING_SORT_SIZE : MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE;

        if (size >= threshold) {
            counting_sort(a, low, high);
        } else {
            // Fallback to sequential sort (which handles insertion sort for small arrays)
            sort_sequential<T, std::less<T>>(nullptr, a, 0, low, high, std::less<T>());
        }
        return;
    }

    // Case 2: Parallel Sort (for types > 2 bytes)
    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        sort(a, parallelism, low, high, std::less<T>());
        return;
    }

    // Case 3: Sequential Sort (fallback)
    if constexpr (std::is_floating_point_v<T>) {
        sort_floats(a, low, high);
    } else {
        sort_sequential<T, std::less<T>>(nullptr, a, 0, low, high, std::less<T>());
    }
}

// -----------------------------------------------------------------------------
// Public API: Convenience wrappers
// -----------------------------------------------------------------------------

template<typename T>
void sort(T* a, std::ptrdiff_t length) {
    checkNotNull(a, "array");
    if (length < 0) throw std::invalid_argument("Array length cannot be negative");

    // Default to hardware concurrency
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

template<typename T, typename Compare>
void sort(T* a, std::ptrdiff_t length, Compare comp) {
    checkNotNull(a, "array");
    if (length < 0) throw std::invalid_argument("Array length cannot be negative");

    // Default to hardware concurrency
    sort(a, std::thread::hardware_concurrency(), 0, length, comp);
}

// -----------------------------------------------------------------------------
// Public API: Container & Iterator Adapters
// -----------------------------------------------------------------------------

template<typename Container>
void sort(Container& container) {
    sort(container.data(), std::thread::hardware_concurrency(), 0, static_cast<std::ptrdiff_t>(container.size()));
}

template<typename Container, typename Compare>
void sort(Container& container, Compare comp) {
    sort(container.data(), std::thread::hardware_concurrency(), 0, static_cast<std::ptrdiff_t>(container.size()), comp);
}

template<typename Container>
void sort(Container& container, int parallelism) {
    sort(container.data(), parallelism, 0, static_cast<std::ptrdiff_t>(container.size()));
}

template<typename Container, typename Compare>
void sort(Container& container, int parallelism, Compare comp) {
    sort(container.data(), parallelism, 0, static_cast<std::ptrdiff_t>(container.size()), comp);
}

template<typename RandomAccessIterator>
void dual_pivot_quicksort(RandomAccessIterator first, RandomAccessIterator last) {
    static_assert(std::is_same_v<typename std::iterator_traits<RandomAccessIterator>::iterator_category,
                                std::random_access_iterator_tag>,
                  "dual_pivot_quicksort requires random access iterators");

    if (first >= last) return;
    std::ptrdiff_t size = last - first;
    if (size <= 1) return;

    if constexpr (is_contiguous_iterator_v<RandomAccessIterator>) {
        auto* a = &(*first);
        // Use 0 parallelism for default sequential behavior of this specific function name
        sort(a, 0, 0, size);
    } else {
        sort_iterator(first, last);
    }
}

template<typename RandomAccessIterator, typename Compare>
void dual_pivot_quicksort(RandomAccessIterator first, RandomAccessIterator last, Compare comp) {
    static_assert(std::is_same_v<typename std::iterator_traits<RandomAccessIterator>::iterator_category,
                                std::random_access_iterator_tag>,
                  "dual_pivot_quicksort requires random access iterators");

    if (first >= last) return;
    std::ptrdiff_t size = last - first;
    if (size <= 1) return;

    if constexpr (is_contiguous_iterator_v<RandomAccessIterator>) {
        auto* a = &(*first);
        // Use 0 parallelism for default sequential behavior of this specific function name
        sort(a, 0, 0, size, comp);
    } else {
        // Fallback to vector copy for non-contiguous iterators with custom comparator
        // until sort_iterator supports custom comparators
        using ValueType = typename std::iterator_traits<RandomAccessIterator>::value_type;
        std::vector<ValueType> temp(first, last);
        sort(temp.data(), 0, 0, size, comp);
        std::copy(temp.begin(), temp.end(), first);
    }
}

template<typename RandomAccessIterator>
void dual_pivot_quicksort_parallel(RandomAccessIterator first, RandomAccessIterator last,
                                  int parallelism = std::thread::hardware_concurrency()) {
    static_assert(std::is_same_v<typename std::iterator_traits<RandomAccessIterator>::iterator_category,
                                std::random_access_iterator_tag>,
                  "dual_pivot_quicksort requires random access iterators");

    if (first >= last) return;
    std::ptrdiff_t size = last - first;
    if (size <= 1) return;

    if constexpr (is_contiguous_iterator_v<RandomAccessIterator>) {
        auto* a = &(*first);
        sort(a, parallelism, 0, size);
    } else {
        using ValueType = typename std::iterator_traits<RandomAccessIterator>::value_type;
        std::vector<ValueType> temp(first, last);
        sort(temp.data(), parallelism, 0, size);
        std::copy(temp.begin(), temp.end(), first);
    }
}

template<typename RandomAccessIterator, typename Compare>
void dual_pivot_quicksort_parallel(RandomAccessIterator first, RandomAccessIterator last, Compare comp,
                                  int parallelism = std::thread::hardware_concurrency()) {
    static_assert(std::is_same_v<typename std::iterator_traits<RandomAccessIterator>::iterator_category,
                                std::random_access_iterator_tag>,
                  "dual_pivot_quicksort requires random access iterators");

    if (first >= last) return;
    std::ptrdiff_t size = last - first;
    if (size <= 1) return;

    if constexpr (is_contiguous_iterator_v<RandomAccessIterator>) {
        auto* a = &(*first);
        sort(a, parallelism, 0, size, comp);
    } else {
        using ValueType = typename std::iterator_traits<RandomAccessIterator>::value_type;
        std::vector<ValueType> temp(first, last);
        sort(temp.data(), parallelism, 0, size, comp);
        std::copy(temp.begin(), temp.end(), first);
    }
}

} // namespace dual_pivot

#endif // DUAL_PIVOT_QUICKSORT_HPP
