#pragma once

#include <vector>
#include <iterator>
#include <functional>
#include <algorithm>
#include <cstddef>

namespace dual_pivot {

// Constants from the reference implementation
constexpr size_t INSERTION_SORT_THRESHOLD = 17;
constexpr size_t DIST_SIZE = 13;

// Forward declarations
template<typename RandomIt, typename Compare>
void dual_pivot_quicksort_impl(RandomIt first, RandomIt last, Compare comp);

template<typename RandomIt, typename Compare>
void insertion_sort(RandomIt first, RandomIt last, Compare comp);

template<typename RandomIt, typename Compare>
void sort_5_network(RandomIt first, Compare comp);

// Main interface functions
template<typename RandomIt, typename Compare = std::less<>>
void dual_pivot_quicksort(RandomIt first, RandomIt last, Compare comp = Compare{});

template<typename T>
void dual_pivot_quicksort(std::vector<T>& arr, size_t left = 0, size_t right = SIZE_MAX);

// Instrumented versions for performance analysis
template<typename RandomIt, typename Compare>
struct SortMetrics {
    size_t comparisons = 0;
    size_t swaps = 0;
    size_t scanned_elements = 0;
};

template<typename RandomIt, typename Compare>
void dual_pivot_quicksort_instrumented(RandomIt first, RandomIt last, 
                                     Compare comp, SortMetrics<RandomIt, Compare>& metrics);

} // namespace dual_pivot

#include "dual_pivot_quicksort_impl.hpp"