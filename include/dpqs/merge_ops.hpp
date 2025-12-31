#ifndef DPQS_MERGE_OPS_HPP
#define DPQS_MERGE_OPS_HPP

#include <algorithm>
#include <utility>
#include <vector>
#include "dpqs/constants.hpp"
#include "dpqs/parallel/threadpool.hpp"

namespace dual_pivot {

/**
 * @brief Sequential merge operation for combining two sorted array segments
 *
 * This function merges two sorted array segments into a destination array.
 * It uses the standard two-pointer merge technique optimized for cache performance.
 * The implementation includes buffer management optimizations to handle cases
 * where the destination overlaps with source arrays.
 *
 * Algorithm: Standard merge with three phases:
 * 1. Merge while both arrays have elements (main merge loop)
 * 2. Copy remaining elements from first array if any
 * 3. Copy remaining elements from second array if any
 *
 * Buffer Management:
 * - Handles overlapping destination and source arrays safely
 * - Optimizes for cases where destination is the same as source
 * - Prevents unnecessary copying when arrays don't overlap
 *
 * Time Complexity: O(n + m) where n and m are the sizes of the two segments
 * Space Complexity: O(1) additional space
 *
 * @tparam T Element type (must support comparison and assignment)
 * @param dst Destination array for merged result
 * @param k Starting index in destination array
 * @param a1 First source array
 * @param lo1 Starting index of first segment (inclusive)
 * @param hi1 Ending index of first segment (exclusive)
 * @param a2 Second source array
 * @param lo2 Starting index of second segment (inclusive)
 * @param hi2 Ending index of second segment (exclusive)
 */
template<typename T, typename Compare>
void merge_parts(T* dst, std::ptrdiff_t k, T* a1, std::ptrdiff_t lo1, std::ptrdiff_t hi1, T* a2, std::ptrdiff_t lo2, std::ptrdiff_t hi2, Compare comp) {
    // Phase 1: Main merge loop - process both arrays while they have elements
    // Uses branch-free comparison for better performance on modern CPUs
    while (lo1 < hi1 && lo2 < hi2) {
        dst[k++] = comp(a1[lo1], a2[lo2]) ? a1[lo1++] : a2[lo2++];
    }

    // Phase 2: Copy remaining elements from first array
    // Buffer overlap check prevents unnecessary copying when dst == a1
    if (dst != a1 || k < lo1) {
        while (lo1 < hi1) {
            dst[k++] = a1[lo1++];
        }
    }

    // Phase 3: Copy remaining elements from second array
    // Buffer overlap check prevents unnecessary copying when dst == a2
    if (dst != a2 || k < lo2) {
        while (lo2 < hi2) {
            dst[k++] = a2[lo2++];
        }
    }
}

/**
 * @brief Parallel merge operation using divide-and-conquer with binary search
 *
 * This function implements a parallel merge algorithm that recursively divides
 * large merge operations into smaller subproblems that can be processed concurrently.
 * It uses binary search to find optimal split points that balance workload.
 *
 * Algorithm Strategy:
 * 1. Check if both segments are large enough for parallel processing
 * 2. Ensure the first array is the larger one (swap if necessary)
 * 3. Find median element of larger array and binary search position in smaller array
 * 4. Launch parallel tasks for the two resulting merge operations
 * 5. Fall back to sequential merge for small segments
 *
 * Parallel Subdivision:
 * - Uses binary search (std::lower_bound) to find split points
 * - Ensures balanced workload distribution between threads
 * - Maintains cache locality by processing related data together
 *
 * Load Balancing:
 * - Always makes the larger array the primary partitioning source
 * - Uses median split to ensure roughly equal work distribution
 * - Recursive subdivision continues until segments are too small for parallelism
 *
 * Time Complexity: O(log(n+m)) depth with O(n+m) total work
 * Space Complexity: O(log(n+m)) recursion stack space
 *
 * @tparam T Element type (must support comparison and assignment)
 * @tparam Compare Comparator type
 * @param dst Destination array for merged result
 * @param k Starting index in destination array
 * @param a1 First source array
 * @param lo1 Starting index of first segment (inclusive)
 * @param hi1 Ending index of first segment (exclusive)
 * @param a2 Second source array
 * @param lo2 Starting index of second segment (inclusive)
 * @param hi2 Ending index of second segment (exclusive)
 * @param comp Comparator instance
 */
template<typename T, typename Compare>
void parallel_merge_parts(T* dst, std::ptrdiff_t k, T* a1, std::ptrdiff_t lo1, std::ptrdiff_t hi1, T* a2, std::ptrdiff_t lo2, std::ptrdiff_t hi2, Compare comp) {
    // Parallel merge disabled for Work Stealing V3 compatibility
    merge_parts(dst, k, a1, lo1, hi1, a2, lo2, hi2, comp);
}

} // namespace dual_pivot

#endif // DPQS_MERGE_OPS_HPP
