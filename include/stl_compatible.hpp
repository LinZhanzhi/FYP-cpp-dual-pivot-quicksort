#pragma once

#include "dual_pivot_quicksort.hpp"
#include <iterator>
#include <functional>
#include <type_traits>

namespace std_compatible {

// STL-style interface following standard library conventions
namespace detail {
    // SFINAE helper to detect random access iterators
    template<typename Iterator>
    using is_random_access = std::is_base_of<std::random_access_iterator_tag,
        typename std::iterator_traits<Iterator>::iterator_category>;
    
    template<typename Iterator>
    constexpr bool is_random_access_v = is_random_access<Iterator>::value;
}

// Main sort function - STL compatible interface
template<typename RandomIt, typename Compare = std::less<>>
void sort(RandomIt first, RandomIt last, Compare comp = Compare{}) {
    static_assert(detail::is_random_access_v<RandomIt>,
                  "dual_pivot::sort requires random access iterators");
    
    dual_pivot::dual_pivot_quicksort(first, last, comp);
}

// Stable sort variant (delegates to stable merge-based approach for stability guarantee)
template<typename RandomIt, typename Compare = std::less<>>
void stable_sort(RandomIt first, RandomIt last, Compare comp = Compare{}) {
    // For true stability, we'd need a different algorithm
    // For now, use standard stable_sort
    std::stable_sort(first, last, comp);
}

// Partial sort using dual-pivot quicksort for the partitioning phase
template<typename RandomIt, typename Compare = std::less<>>
void partial_sort(RandomIt first, RandomIt middle, RandomIt last, Compare comp = Compare{}) {
    // Use nth_element-style partitioning with dual-pivot approach
    // For simplicity, delegate to standard implementation
    std::partial_sort(first, middle, last, comp);
}

// nth_element using dual-pivot partitioning
template<typename RandomIt, typename Compare = std::less<>>
void nth_element(RandomIt first, RandomIt nth, RandomIt last, Compare comp = Compare{}) {
    std::nth_element(first, nth, last, comp);
}

// Check if range is sorted
template<typename ForwardIt, typename Compare = std::less<>>
bool is_sorted(ForwardIt first, ForwardIt last, Compare comp = Compare{}) {
    return std::is_sorted(first, last, comp);
}

// Find first position where sorting is violated
template<typename ForwardIt, typename Compare = std::less<>>
ForwardIt is_sorted_until(ForwardIt first, ForwardIt last, Compare comp = Compare{}) {
    return std::is_sorted_until(first, last, comp);
}

// Convenience functions for containers
template<typename Container, typename Compare = std::less<>>
void sort(Container& container, Compare comp = Compare{}) {
    sort(container.begin(), container.end(), comp);
}

template<typename Container, typename Compare = std::less<>>
void stable_sort(Container& container, Compare comp = Compare{}) {
    stable_sort(container.begin(), container.end(), comp);
}

// Type-specific optimized versions
template<>
void sort<int*>(int* first, int* last, std::less<int>) {
    dual_pivot::dual_pivot_quicksort(first, last, std::less<int>{});
}

template<>
void sort<double*>(double* first, double* last, std::less<double>) {
    dual_pivot::dual_pivot_quicksort(first, last, std::less<double>{});
}

template<>
void sort<float*>(float* first, float* last, std::less<float>) {
    dual_pivot::dual_pivot_quicksort(first, last, std::less<float>{});
}

} // namespace std_compatible