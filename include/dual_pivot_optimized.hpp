#pragma once

#include "dual_pivot_quicksort.hpp"
#include <algorithm>
#include <cmath>
#include <iterator>

namespace dual_pivot_optimized {

// Forward declaration
template <typename RandomIt, typename Compare>
bool is_nearly_sorted(RandomIt first, RandomIt last, Compare comp);

// Heapsort for fallback when recursion gets too deep
template <typename RandomIt, typename Compare>
void heapsort(RandomIt first, RandomIt last, Compare comp) {
  std::make_heap(first, last, comp);
  std::sort_heap(first, last, comp);
}

// Calculate maximum recursion depth (2 * log2(n))
template <typename RandomIt>
constexpr int max_depth(RandomIt first, RandomIt last) {
  auto n = std::distance(first, last);
  return n <= 1 ? 0 : 2 * static_cast<int>(std::log2(n));
}

// Enhanced dual-pivot quicksort with introsort-style depth limiting
template <typename RandomIt, typename Compare>
void dual_pivot_introsort_impl(RandomIt first, RandomIt last, Compare comp,
                               int depth_limit) {
  const auto len = std::distance(first, last);

  // Use insertion sort for small arrays
  if (static_cast<size_t>(len) < dual_pivot::INSERTION_SORT_THRESHOLD) {
    dual_pivot::insertion_sort(first, last, comp);
    return;
  }

  // Fall back to heapsort if recursion too deep
  if (depth_limit == 0) {
    heapsort(first, last, comp);
    return;
  }

  // Enhanced pivot selection with more samples for larger arrays
  auto sixth = len / 6;

  // For very large arrays, use more sophisticated sampling
  if (len > 1000) {
    // Sample more elements for better pivot selection
    auto step = len / 40; // Sample every 40th element
    std::vector<typename std::iterator_traits<RandomIt>::value_type> samples;

    for (auto i = first; i < last; i += step) {
      samples.push_back(*i);
    }

    if (samples.size() >= 5) {
      std::sort(samples.begin(), samples.end(), comp);
      auto mid = samples.size() / 2;
      auto pivot1 = samples[mid - 1];
      auto pivot2 = samples[mid + 1];

      // Find positions of these pivots in original array
      auto p1_pos = std::find(first, last, pivot1);
      auto p2_pos = std::find(first, last, pivot2);

      if (p1_pos != last && p2_pos != last) {
        // Use sampled pivots
        *first = pivot1;
        *(last - 1) = pivot2;
      }
    }
  }

  // Adaptive algorithm selection based on array characteristics
  bool nearly_sorted = is_nearly_sorted(first, last, comp);
  if (nearly_sorted && len > 100) {
    // Use Timsort-like approach for nearly sorted data
    std::stable_sort(first, last, comp);
    return;
  }

  // Standard dual-pivot partitioning with optimizations
  auto m1 = first + sixth;
  auto m2 = m1 + sixth;
  auto m3 = m2 + sixth;
  auto m4 = m3 + sixth;
  auto m5 = m4 + sixth;
  (void)m5; // Suppress unused variable warning

  // Enhanced 5-element sorting network
  dual_pivot::sort_5_network(m1, comp);

  auto pivot1 = *m2;
  auto pivot2 = *m4;

  if (comp(pivot2, pivot1)) {
    std::swap(pivot1, pivot2);
  }

  bool different_pivots = comp(pivot1, pivot2);

  *m2 = *first;
  *m4 = *(last - 1);

  auto less = first + 1;
  auto great = last - 2;
  auto k = less;

  // Optimized partitioning loop with prefetching hints
  if (different_pivots) {
    while (k <= great) {
      // Prefetch next elements for better cache performance
      if (k + 8 <= great) {
        __builtin_prefetch(&*(k + 8), 0, 3);
      }

      if (comp(*k, pivot1)) {
        std::iter_swap(k, less);
        ++less;
      } else if (!comp(*k, pivot2)) {
        while (!comp(*great, pivot2) && k < great) {
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
  } else {
    // Optimized path for equal pivots
    while (k <= great) {
      if (comp(*k, pivot1)) {
        std::iter_swap(k, less);
        ++less;
      } else if (comp(pivot1, *k)) {
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

  // Place pivots in final positions
  --less;
  ++great;
  std::iter_swap(first, less);
  *less = pivot1;
  std::iter_swap(last - 1, great);
  *great = pivot2;

  // Recursive calls with decremented depth
  dual_pivot_introsort_impl(first, less, comp, depth_limit - 1);
  dual_pivot_introsort_impl(great + 1, last, comp, depth_limit - 1);

  // Handle middle section
  if (different_pivots) {
    // Check if middle section needs special handling
    auto middle_size = std::distance(less + 1, great);
    if (middle_size > len / 8) { // If middle section is large
      // Use different strategy for middle section
      dual_pivot_introsort_impl(less + 1, great, comp, depth_limit - 1);
    }
  }
}

// Helper function to detect nearly sorted arrays
template <typename RandomIt, typename Compare>
bool is_nearly_sorted(RandomIt first, RandomIt last, Compare comp) {
  const auto len = std::distance(first, last);
  if (len < 10)
    return true;

  int inversions = 0;
  const int max_inversions = len / 10; // Allow 10% inversions

  for (auto it = first; it != last - 1 && inversions <= max_inversions; ++it) {
    if (comp(*(it + 1), *it)) {
      ++inversions;
    }
  }

  return inversions <= max_inversions;
}

// Main optimized interface
template <typename RandomIt, typename Compare = std::less<>>
void dual_pivot_introsort(RandomIt first, RandomIt last,
                          Compare comp = Compare{}) {
  static_assert(std::is_base_of_v<
                    std::random_access_iterator_tag,
                    typename std::iterator_traits<RandomIt>::iterator_category>,
                "dual_pivot_introsort requires random access iterators");

  if (std::distance(first, last) <= 1)
    return;

  int depth_limit = max_depth(first, last);
  dual_pivot_introsort_impl(first, last, comp, depth_limit);
}

// Quickselect variant using dual-pivot partitioning
template <typename RandomIt, typename Compare = std::less<>>
void dual_pivot_nth_element(RandomIt first, RandomIt nth, RandomIt last,
                            Compare comp = Compare{}) {
  while (std::distance(first, last) > 1) {
    // Simplified dual-pivot partitioning for selection
    auto len = std::distance(first, last);

    if (static_cast<size_t>(len) < dual_pivot::INSERTION_SORT_THRESHOLD) {
      dual_pivot::insertion_sort(first, last, comp);
      return;
    }

    // Use middle elements as pivots for selection
    auto mid1 = first + len / 3;
    auto mid2 = first + 2 * len / 3;

    auto pivot1 = *mid1;
    auto pivot2 = *mid2;

    if (comp(pivot2, pivot1)) {
      std::swap(pivot1, pivot2);
    }

    // Partition similar to main algorithm
    auto less = first;
    auto great = last - 1;

    for (auto k = first; k <= great;) {
      if (comp(*k, pivot1)) {
        std::iter_swap(k, less);
        ++less;
        ++k;
      } else if (!comp(*k, pivot2)) {
        std::iter_swap(k, great);
        --great;
      } else {
        ++k;
      }
    }

    // Recurse on appropriate partition
    if (nth < less) {
      last = less;
    } else if (nth > great) {
      first = great + 1;
    } else {
      return; // nth is in the middle section
    }
  }
}

} // namespace dual_pivot_optimized