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

namespace dual_pivot {

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
        sort_int_sequential(nullptr, a, 0, low, high);
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
        sort_long_sequential(nullptr, a, 0, low, high);
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
        sort_float_specialized(a, low, high);
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
        sort_float_specialized(a, low, high);
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
        countingSort(a, low, high);
    } else {
        insertionSort(a, low, high);
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
        countingSort(a, low, high);
    } else {
        sort(a, 0, low, high);
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
        countingSort(a, low, high);
    } else {
        sort(a, 0, low, high);
    }
}
static void sort(int* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative");
    }
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

static void sort(long* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative");
    }
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

static void sort(float* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative");
    }
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

static void sort(double* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative");
    }
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

static void sort(signed char* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative");
    }
    sort(a, 0, length);
}

static void sort(char* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative");
    }
    sort(a, 0, length);
}

static void sort(short* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative");
    }
    sort(a, 0, length);
}
template<typename Container>
void sort(Container& container) {
    using ValueType = typename Container::value_type;
    if constexpr (std::is_integral_v<ValueType> && sizeof(ValueType) <= 2) {
        sort_specialized(container.data(), 0, static_cast<int>(container.size()));
    } else if constexpr (std::is_floating_point_v<ValueType>) {
        sort_specialized(container.data(), 0, static_cast<int>(container.size()));
    } else {
        dual_pivot_quicksort(container.begin(), container.end());
    }
}

template<typename Container>
void sort(Container& container, int parallelism) {
    using ValueType = typename Container::value_type;
    if (container.size() > MIN_PARALLEL_SORT_SIZE && parallelism > 1) {
        parallelSort(container.data(), parallelism, 0, static_cast<int>(container.size()));
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
        sort_specialized(a, 0, size);
    } else if constexpr (std::is_floating_point_v<ValueType>) {
        sort_specialized(a, 0, size);
    } else {
        sort(a, 0, 0, size);
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

    using ValueType = typename std::iterator_traits<RandomAccessIterator>::value_type;
    if (size > MIN_PARALLEL_SORT_SIZE && parallelism > 1) {
        parallelSort(a, parallelism, 0, size);
    } else {
        dual_pivot_quicksort(first, last);
    }
}

} // namespace dual_pivot

#endif // DUAL_PIVOT_QUICKSORT_HPP
