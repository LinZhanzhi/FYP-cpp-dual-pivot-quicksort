#ifndef SORT_HPP
#define SORT_HPP

#include <vector>
#include <algorithm>
#include <iterator>

// Standard Insertion Sort
template <typename RandomIt>
void insertion_sort(RandomIt first, RandomIt last) {
    for (auto it = first + 1; it != last; ++it) {
        auto key = std::move(*it);
        auto j = it;
        while (j != first && key < *(j - 1)) {
            *j = std::move(*(j - 1));
            --j;
        }
        *j = std::move(key);
    }
}

// Partition function (Hoare partition scheme is often more efficient)
template <typename RandomIt>
RandomIt partition(RandomIt first, RandomIt last) {
    auto pivot = *first;
    auto i = first - 1;
    auto j = last;

    while (true) {
        do {
            ++i;
        } while (*i < pivot);

        do {
            --j;
        } while (pivot < *j);

        if (i >= j) return j;

        std::iter_swap(i, j);
    }
}

// Hybrid Quicksort with Threshold
template <typename RandomIt>
void hybrid_quicksort(RandomIt first, RandomIt last, int threshold) {
    auto len = std::distance(first, last);

    if (len <= 1) return;

    // If the subarray is small enough, use Insertion Sort
    if (len <= threshold) {
        insertion_sort(first, last);
        return;
    }    // Otherwise, partition and recurse
    auto p = partition(first, last);
    hybrid_quicksort(first, p + 1, threshold);
    hybrid_quicksort(p + 1, last, threshold);
}

#endif // SORT_HPP
