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
template<typename T>
void merge_parts(T* dst, std::ptrdiff_t k, T* a1, std::ptrdiff_t lo1, std::ptrdiff_t hi1, T* a2, std::ptrdiff_t lo2, std::ptrdiff_t hi2) {
    // Phase 1: Main merge loop - process both arrays while they have elements
    // Uses branch-free comparison for better performance on modern CPUs
    while (lo1 < hi1 && lo2 < hi2) {
        dst[k++] = (a1[lo1] < a2[lo2]) ? a1[lo1++] : a2[lo2++];
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
 * @param dst Destination array for merged result
 * @param k Starting index in destination array
 * @param a1 First source array
 * @param lo1 Starting index of first segment (inclusive)
 * @param hi1 Ending index of first segment (exclusive)
 * @param a2 Second source array
 * @param lo2 Starting index of second segment (inclusive)
 * @param hi2 Ending index of second segment (exclusive)
 */
template<typename T>
void parallel_merge_parts(T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2) {
    // Check if both segments are large enough for parallel processing
    if (hi1 - lo1 >= MIN_PARALLEL_MERGE_PARTS_SIZE && hi2 - lo2 >= MIN_PARALLEL_MERGE_PARTS_SIZE) {
        // Ensure first array is larger for optimal partitioning
        // This load balancing step ensures the binary search is performed on the smaller array
        if (hi1 - lo1 < hi2 - lo2) {
            std::swap(lo1, lo2);
            std::swap(hi1, hi2);
            std::swap(a1, a2);
        }

        // Find median of larger array for balanced workload distribution
        int mi1 = (lo1 + hi1) >> 1;
        T key = a1[mi1];

        // Binary search to find split point in smaller array
        // This ensures elements < key go to left merge, elements >= key go to right merge
        int mi2 = std::lower_bound(a2 + lo2, a2 + hi2, key) - a2;

        // Calculate destination offset for right merge operation
        int d = mi2 - lo2 + mi1 - lo1;

        // Launch parallel task for right partition
        auto& pool = getThreadPool();
        auto future = pool.enqueue([=] {
            parallel_merge_parts(dst, k + d, a1, mi1, hi1, a2, mi2, hi2);
        });

        // Process left partition in current thread
        parallel_merge_parts(dst, k, a1, lo1, mi1, a2, lo2, mi2);

        // Wait for right partition to complete
        future.get();
    } else {
        // Fall back to sequential merge for small segments
        // This avoids thread creation overhead for small workloads
        merge_parts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    }
}

} // namespace dual_pivot

#endif // DPQS_MERGE_OPS_HPP
