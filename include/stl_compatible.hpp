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