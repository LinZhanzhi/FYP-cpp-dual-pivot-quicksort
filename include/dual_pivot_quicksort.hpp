#ifndef DUAL_PIVOT_QUICKSORT_HPP
#define DUAL_PIVOT_QUICKSORT_HPP

#include "dpqs/utils.hpp"
#include "dpqs/types.hpp"
#include "dpqs/core_sort.hpp"
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

// Forward declarations
template<typename RandomAccessIterator>
void dual_pivot_quicksort(RandomAccessIterator first, RandomAccessIterator last);

// -----------------------------------------------------------------------------
// Public API: Type-specific overloads
// -----------------------------------------------------------------------------

static void sort(int* a, int parallelism, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range");
    }

    if (checkEarlyTermination(a, low, high)) {
        return;
    }

    int size = high - low;
    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);
        std::vector<int> b(depth == 0 ? 0 : size);
        auto* sorter = new AdvancedSorter<int>(nullptr, a, depth == 0 ? nullptr : b.data(), low, size, low, depth);
        sorter->invoke();
        delete sorter;
    } else {
        sort_sequential<int>(nullptr, a, 0, low, high);
    }
}

static void sort(long* a, int parallelism, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range");
    }

    if (checkEarlyTermination(a, low, high)) {
        return;
    }

    int size = high - low;
    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);
        std::vector<long> b(depth == 0 ? 0 : size);
        auto* sorter = new AdvancedSorter<long>(nullptr, a, depth == 0 ? nullptr : b.data(), low, size, low, depth);
        sorter->invoke();
        delete sorter;
    } else {
        sort_sequential<long>(nullptr, a, 0, low, high);
    }
}

static void sort(float* a, int parallelism, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range");
    }

    if (checkEarlyTermination(a, low, high)) {
        return;
    }

    int size = high - low;
    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);
        std::vector<float> b(depth == 0 ? 0 : size);
        auto* sorter = new AdvancedSorter<float>(nullptr, a, depth == 0 ? nullptr : b.data(), low, size, low, depth);
        sorter->invoke();
        delete sorter;
    } else {
        sort_floats(a, low, high);
    }
}

static void sort(double* a, int parallelism, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range");
    }

    if (checkEarlyTermination(a, low, high)) {
        return;
    }

    int size = high - low;
    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);
        std::vector<double> b(depth == 0 ? 0 : size);
        auto* sorter = new AdvancedSorter<double>(nullptr, a, depth == 0 ? nullptr : b.data(), low, size, low, depth);
        sorter->invoke();
        delete sorter;
    } else {
        sort_floats(a, low, high);
    }
}

static void sort(signed char* a, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range");
    }

    if (checkEarlyTermination(a, low, high)) {
        return;
    }

    if (high - low >= MIN_BYTE_COUNTING_SORT_SIZE) {
        counting_sort(a, low, high);
    } else {
        insertion_sort(a, low, high);
    }
}

static void sort(char* a, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range");
    }

    if (checkEarlyTermination(a, low, high)) {
        return;
    }

    if (high - low >= MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE) {
        counting_sort(a, low, high);
    } else {
        sort_sequential<char>(nullptr, a, 0, low, high);
    }
}

static void sort(short* a, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range");
    }

    if (checkEarlyTermination(a, low, high)) {
        return;
    }

    if (high - low >= MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE) {
        counting_sort(a, low, high);
    } else {
        sort_sequential<short>(nullptr, a, 0, low, high);
    }
}

// -----------------------------------------------------------------------------
// Public API: Convenience wrappers
// -----------------------------------------------------------------------------

static void sort(int* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) throw std::invalid_argument("Array length cannot be negative");
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

static void sort(long* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) throw std::invalid_argument("Array length cannot be negative");
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

static void sort(float* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) throw std::invalid_argument("Array length cannot be negative");
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

static void sort(double* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) throw std::invalid_argument("Array length cannot be negative");
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

static void sort(signed char* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) throw std::invalid_argument("Array length cannot be negative");
    sort(a, 0, length);
}

static void sort(char* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) throw std::invalid_argument("Array length cannot be negative");
    sort(a, 0, length);
}

static void sort(short* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) throw std::invalid_argument("Array length cannot be negative");
    sort(a, 0, length);
}

// -----------------------------------------------------------------------------
// Public API: Generic Templates
// -----------------------------------------------------------------------------

template<typename Container>
void sort(Container& container) {
    using ValueType = typename Container::value_type;
    if constexpr (std::is_integral_v<ValueType> && sizeof(ValueType) <= 2) {
        if (container.size() >= MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE) {
            counting_sort(container.data(), 0, static_cast<int>(container.size()));
        } else {
            sort_sequential<ValueType>(nullptr, container.data(), 0, 0, static_cast<int>(container.size()));
        }
    } else if constexpr (std::is_floating_point_v<ValueType>) {
        sort_floats(container.data(), 0, static_cast<int>(container.size()));
    } else {
        dual_pivot_quicksort(container.begin(), container.end());
    }
}

template<typename Container>
void sort(Container& container, int parallelism) {
    using ValueType = typename Container::value_type;
    if (container.size() > MIN_PARALLEL_SORT_SIZE && parallelism > 1) {
        if constexpr (std::is_same_v<ValueType, int>) {
            sort(container.data(), parallelism, 0, static_cast<int>(container.size()));
        } else if constexpr (std::is_same_v<ValueType, long>) {
            sort(container.data(), parallelism, 0, static_cast<int>(container.size()));
        } else if constexpr (std::is_same_v<ValueType, float>) {
            sort(container.data(), parallelism, 0, static_cast<int>(container.size()));
        } else if constexpr (std::is_same_v<ValueType, double>) {
            sort(container.data(), parallelism, 0, static_cast<int>(container.size()));
        } else {
            sort(container); // Fallback to sequential
        }
    } else {
        sort(container);
    }
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

    using ValueType = typename std::iterator_traits<RandomAccessIterator>::value_type;
    if constexpr (std::is_integral_v<ValueType> && sizeof(ValueType) <= 2) {
        if (size >= MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE) {
            counting_sort(a, 0, size);
        } else {
            sort_sequential<ValueType>(nullptr, a, 0, 0, size);
        }
    } else if constexpr (std::is_floating_point_v<ValueType>) {
        sort_floats(a, 0, size);
    } else {
        // Use generic sequential sort
        sort_sequential<ValueType>(nullptr, a, 0, 0, size);
    }
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

    // For now, just delegate to type-specific parallel sort if possible
    using ValueType = typename std::iterator_traits<RandomAccessIterator>::value_type;
    if (size > MIN_PARALLEL_SORT_SIZE && parallelism > 1) {
        if constexpr (std::is_same_v<ValueType, int>) {
            sort(a, parallelism, 0, size);
        } else if constexpr (std::is_same_v<ValueType, long>) {
            sort(a, parallelism, 0, size);
        } else if constexpr (std::is_same_v<ValueType, float>) {
            sort(a, parallelism, 0, size);
        } else if constexpr (std::is_same_v<ValueType, double>) {
            sort(a, parallelism, 0, size);
        } else {
            dual_pivot_quicksort(first, last);
        }
    } else {
        dual_pivot_quicksort(first, last);
    }
}

} // namespace dual_pivot

#endif // DUAL_PIVOT_QUICKSORT_HPP
