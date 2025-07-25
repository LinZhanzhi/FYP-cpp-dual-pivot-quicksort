#pragma once

#include <iterator>
#include <algorithm>

namespace dual_pivot {

// Insertion sort for small arrays (< 17 elements)
template<typename RandomIt, typename Compare>
void insertion_sort(RandomIt first, RandomIt last, Compare comp) {
    if (first == last) return;
    
    for (auto i = first + 1; i != last; ++i) {
        auto key = *i;
        auto j = i;
        
        while (j != first && comp(key, *(j - 1))) {
            *j = *(j - 1);
            --j;
        }
        *j = key;
    }
}

// 5-element sorting network based on DualPivotQuicksort.md:500-523
template<typename RandomIt, typename Compare>
void sort_5_network(RandomIt first, Compare comp) {
    auto& a0 = *first;
    auto& a1 = *(first + 1);
    auto& a2 = *(first + 2);
    auto& a3 = *(first + 3);
    auto& a4 = *(first + 4);
    
    // 5-element sorting network implementation
    if (comp(a1, a0)) std::swap(a0, a1);
    if (comp(a4, a3)) std::swap(a3, a4);
    if (comp(a2, a0)) std::swap(a0, a2);
    if (comp(a2, a1)) std::swap(a1, a2);
    if (comp(a3, a0)) std::swap(a0, a3);
    if (comp(a3, a2)) std::swap(a2, a3);
    if (comp(a4, a1)) std::swap(a1, a4);
    if (comp(a2, a1)) std::swap(a1, a2);
    if (comp(a4, a3)) std::swap(a3, a4);
}

// Core dual-pivot partitioning algorithm
template<typename RandomIt, typename Compare>
void dual_pivot_quicksort_impl(RandomIt first, RandomIt last, Compare comp) {
    const auto len = std::distance(first, last);
    
    // Use insertion sort for small arrays
    if (static_cast<size_t>(len) < INSERTION_SORT_THRESHOLD) {
        insertion_sort(first, last, comp);
        return;
    }
    
    // Calculate median positions for pivot selection (median-of-5 approach)
    auto sixth = len / 6;
    auto m1 = first + sixth;
    auto m2 = m1 + sixth;
    auto m3 = m2 + sixth;
    auto m4 = m3 + sixth;
    auto m5 = m4 + sixth;
    
    // Sort the 5 median elements using sorting network
    sort_5_network(m1, comp);
    
    // Choose pivots: m2 and m4 after sorting
    auto pivot1 = *m2;
    auto pivot2 = *m4;
    
    // Ensure pivot1 <= pivot2
    if (comp(pivot2, pivot1)) {
        std::swap(pivot1, pivot2);
    }
    
    bool different_pivots = !comp(pivot1, pivot2) && !comp(pivot2, pivot1) ? false : true;
    
    // Place pivots at the ends temporarily
    *m2 = *first;
    *m4 = *(last - 1);
    
    // Initialize pointers for three-way partitioning
    auto less = first + 1;      // pointer for elements < pivot1
    auto great = last - 2;      // pointer for elements > pivot2
    auto k = less;              // scanning pointer
    
    // Main partitioning loop
    if (different_pivots) {
        while (k <= great) {
            if (comp(*k, pivot1)) {
                // Element < pivot1: move to left part
                std::iter_swap(k, less);
                ++less;
            }
            else if (!comp(*k, pivot2)) {
                // Element >= pivot2: move to right part
                while (!comp(*great, pivot2) && k < great) {
                    --great;
                }
                std::iter_swap(k, great);
                --great;
                
                // Check if swapped element should go to left part
                if (comp(*k, pivot1)) {
                    std::iter_swap(k, less);
                    ++less;
                }
            }
            // Elements between pivot1 and pivot2 stay in middle
            ++k;
        }
    }
    else {
        // Special case: pivot1 == pivot2
        while (k <= great) {
            if (comp(*k, pivot1)) {
                std::iter_swap(k, less);
                ++less;
            }
            else if (comp(pivot1, *k)) {
                while (comp(pivot1, *great) && k < great) {
                    --great;
                }
                std::iter_swap(k, great);
                --great;
                
                if (comp(*k, pivot1)) {
                    std::iter_swap(k, less);
                    ++less;
                }
            }
            ++k;
        }
    }
    
    // Place pivots in their final positions
    --less;
    ++great;
    std::iter_swap(first, less);
    *less = pivot1;
    std::iter_swap(last - 1, great);
    *great = pivot2;
    
    // Recursively sort the three parts
    dual_pivot_quicksort_impl(first, less, comp);
    dual_pivot_quicksort_impl(great + 1, last, comp);
    
    // Handle middle part for equal elements if needed
    if (different_pivots && (static_cast<size_t>(great - less) > static_cast<size_t>(len) - DIST_SIZE)) {
        // Sort middle part, handling equal elements
        for (auto i = less + 1; i < great; ++i) {
            if (!comp(*i, pivot1) && !comp(pivot1, *i)) {
                // Element equals pivot1
                std::iter_swap(i, less + 1);
                ++less;
            }
            else if (!comp(*i, pivot2) && !comp(pivot2, *i)) {
                // Element equals pivot2
                std::iter_swap(i, great - 1);
                --great;
                --i; // Re-examine swapped element
                
                if (!comp(*i, pivot1) && !comp(pivot1, *i)) {
                    std::iter_swap(i, less + 1);
                    ++less;
                }
            }
        }
    }
    
    // Sort middle part if pivots are different
    if (different_pivots) {
        dual_pivot_quicksort_impl(less + 1, great, comp);
    }
}

// Main interface function
template<typename RandomIt, typename Compare>
void dual_pivot_quicksort(RandomIt first, RandomIt last, Compare comp) {
    static_assert(std::is_base_of_v<std::random_access_iterator_tag, 
                  typename std::iterator_traits<RandomIt>::iterator_category>,
                  "dual_pivot_quicksort requires random access iterators");
    
    if (std::distance(first, last) <= 1) return;
    dual_pivot_quicksort_impl(first, last, comp);
}

// Vector convenience function
template<typename T>
void dual_pivot_quicksort(std::vector<T>& arr, size_t left, size_t right) {
    if (right == SIZE_MAX) right = arr.size();
    if (left >= right || right > arr.size()) return;
    
    dual_pivot_quicksort(arr.begin() + left, arr.begin() + right, std::less<T>{});
}

// Instrumented version for performance analysis
template<typename RandomIt, typename Compare>
void dual_pivot_quicksort_instrumented(RandomIt first, RandomIt last, 
                                     Compare comp, SortMetrics<RandomIt, Compare>& metrics) {
    // This version counts operations for analysis
    // Implementation would mirror the main algorithm but increment counters
    // For now, delegate to main implementation
    dual_pivot_quicksort_impl(first, last, comp);
}

} // namespace dual_pivot