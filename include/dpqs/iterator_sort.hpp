#ifndef DPQS_ITERATOR_SORT_HPP
#define DPQS_ITERATOR_SORT_HPP

#include <iterator>
#include <algorithm>
#include <utility>
#include "dpqs/utils.hpp"

namespace dual_pivot {

template<typename RandomAccessIterator>
void insertion_sort_iterator(RandomAccessIterator first, RandomAccessIterator last) {
    if (first == last) return;
    for (RandomAccessIterator i = first + 1; i != last; ++i) {
        if (*i < *(i - 1)) {
            auto val = std::move(*i);
            RandomAccessIterator j = i;
            do {
                *j = std::move(*(j - 1));
                --j;
            } while (j != first && val < *(j - 1));
            *j = std::move(val);
        }
    }
}

template<typename RandomAccessIterator>
std::pair<RandomAccessIterator, RandomAccessIterator> partition_dual_pivot_iterator(RandomAccessIterator first, RandomAccessIterator last) {
    std::ptrdiff_t len = std::distance(first, last);
    if (len <= 1) return {first, first};

    RandomAccessIterator p1_iter = first;
    RandomAccessIterator p2_iter = last - 1;

    if (*p2_iter < *p1_iter) {
        std::iter_swap(p1_iter, p2_iter);
    }

    auto pivot1 = *p1_iter;
    auto pivot2 = *p2_iter;

    RandomAccessIterator lt = first + 1;
    RandomAccessIterator gt = last - 2;
    RandomAccessIterator i = lt;

    while (i <= gt) {
        if (*i < pivot1) {
            std::iter_swap(i, lt);
            ++lt;
            ++i;
        } else if (*i > pivot2) {
            std::iter_swap(i, gt);
            --gt;
        } else {
            ++i;
        }
    }

    --lt;
    ++gt;
    std::iter_swap(first, lt);
    std::iter_swap(last - 1, gt);

    return {lt, gt};
}

template<typename RandomAccessIterator>
void sort_iterator(RandomAccessIterator first, RandomAccessIterator last) {
    std::ptrdiff_t len = std::distance(first, last);
    
    if (len < 27) {
        insertion_sort_iterator(first, last);
        return;
    }

    auto pivots = partition_dual_pivot_iterator(first, last);
    
    sort_iterator(first, pivots.first);
    sort_iterator(pivots.first + 1, pivots.second);
    sort_iterator(pivots.second + 1, last);
}

} // namespace dual_pivot

#endif // DPQS_ITERATOR_SORT_HPP
