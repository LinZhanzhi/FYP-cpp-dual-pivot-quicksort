#pragma once

#include <iterator>
#include <algorithm>
#include <functional>

namespace classic_quicksort {

template<typename RandomIt, typename Compare>
RandomIt classic_partition(RandomIt first, RandomIt last, Compare comp) {
    auto pivot = *(last - 1);  // Choose last element as pivot
    auto i = first;
    
    for (auto j = first; j != last - 1; ++j) {
        if (comp(*j, pivot)) {
            std::iter_swap(i, j);
            ++i;
        }
    }
    std::iter_swap(i, last - 1);
    return i;
}

template<typename RandomIt, typename Compare>
void quicksort_impl(RandomIt first, RandomIt last, Compare comp) {
    if (std::distance(first, last) <= 1) return;
    
    auto pivot_pos = classic_partition(first, last, comp);
    quicksort_impl(first, pivot_pos, comp);
    quicksort_impl(pivot_pos + 1, last, comp);
}

template<typename RandomIt, typename Compare = std::less<>>
void quicksort(RandomIt first, RandomIt last, Compare comp = Compare{}) {
    static_assert(std::is_base_of_v<std::random_access_iterator_tag, 
                  typename std::iterator_traits<RandomIt>::iterator_category>,
                  "quicksort requires random access iterators");
    
    quicksort_impl(first, last, comp);
}

} // namespace classic_quicksort