#ifndef DPQS_PARTITION_HPP
#define DPQS_PARTITION_HPP

#include <utility>
#include <immintrin.h>
#include "dpqs/utils.hpp"

namespace dual_pivot {

/**
 * @brief Optimized dual-pivot partitioning with cache-aware memory access
 *
 * This is the heart of Yaroslavskiy's dual-pivot quicksort algorithm. It partitions
 * an array around two pivot elements P1 and P2 (where P1 ≤ P2) into three regions:
 *
 * Array Layout After Partitioning:
 * [elements < P1] [P1 ≤ elements ≤ P2] [elements > P2]
 *                 ↑                    ↑
 *              lower                upper
 *
 * Key Algorithm Innovations:
 * 1. Three-way partitioning reduces average comparisons vs. traditional quicksort
 * 2. Backward scanning minimizes cache misses (matching Java's optimization)
 * 3. Prefetching hints improve memory access patterns
 * 4. Branch prediction hints optimize for common cases
 *
 * Performance Benefits:
 * - 20% fewer swaps compared to single-pivot quicksort (0.8×n×ln(n) vs 1.0×n×ln(n))
 * - Better cache locality due to sequential access patterns
 * - Fewer "scanned elements" - crucial for modern CPU-memory performance gaps
 *
 * The algorithm processes elements from right to left, which improves cache
 * performance by accessing memory in a more predictable pattern.
 *
 * @tparam T Element type (must support comparison and assignment)
 * @param a Pointer to the array to partition
 * @param low Starting index of the region to partition
 * @param high Ending index of the region to partition (exclusive)
 * @param pivotIndex1 Index of the first pivot element (P1)
 * @param pivotIndex2 Index of the second pivot element (P2)
 * @return std::pair<int, int> containing (lower, upper) partition boundaries
 */
template<typename T>
FORCE_INLINE std::pair<int, int> partition_dual_pivot(T* a, int low, int high, int pivotIndex1, int pivotIndex2) {
    // Phase 6: Optimized dual pivot partitioning with prefetching
    int end = high - 1;
    int lower = low;
    int upper = end;

    // Extract pivot values
    int e1 = pivotIndex1;
    int e5 = pivotIndex2;
    T pivot1 = a[e1];  // P1 - smaller pivot
    T pivot2 = a[e5];  // P2 - larger pivot (P1 ≤ P2)

    // Move pivots to the boundaries for partitioning
    // The first and last elements are moved to positions formerly occupied by pivots
    a[e1] = a[lower];
    a[e5] = a[upper];

    // Skip elements that are already in correct positions
    // This optimization reduces unnecessary work for partially sorted data
    // Added bounds checks for safety, though sentinels should prevent OOB
    while (lower < end && a[++lower] < pivot1);
    while (upper > low && a[--upper] > pivot2);

    // Backward 3-interval partitioning with cache optimization
    // Process from right to left for better cache utilization
    (void)--lower; // Mark lower as used (avoid compiler warning)
    for (int k = ++upper; --k > lower; ) {

        // SIMD Optimization: Skip elements that belong in the middle partition
        // We look for a block of 8 elements where pivot1 <= x <= pivot2
        // This significantly speeds up processing of random data where most elements
        // fall into the middle partition.
#ifdef __AVX2__
        if constexpr (std::is_same_v<T, int>) {
            // Ensure we have at least 8 elements to process (k down to k-7)
            // and we don't cross the lower boundary
            while (k > lower + 8) {
                // Load 8 elements ending at k (indices k-7 to k)
                // Note: _mm256_loadu_si256 loads 256 bits (8 ints)
                // We load from &a[k-7] to get elements a[k-7]...a[k]
                __m256i v = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&a[k - 7]));

                // Broadcast pivots
                __m256i p1 = _mm256_set1_epi32(pivot1);
                __m256i p2 = _mm256_set1_epi32(pivot2);

                // Check if any element is < pivot1
                // _mm256_cmpgt_epi32(a, b) returns 0xFFFFFFFF if a > b
                // We want v < p1, which is p1 > v
                __m256i lt_p1 = _mm256_cmpgt_epi32(p1, v);

                // Check if any element is > pivot2
                // We want v > p2
                __m256i gt_p2 = _mm256_cmpgt_epi32(v, p2);

                // Combine conditions: if any element is outside [pivot1, pivot2]
                __m256i outside = _mm256_or_si256(lt_p1, gt_p2);

                // If all bits are zero, then all elements are in range [pivot1, pivot2]
                if (_mm256_testz_si256(outside, outside)) {
                    // All 8 elements are in the middle partition, skip them!
                    k -= 8;
                } else {
                    // Block contains elements that need moving, process one by one
                    break;
                }
            }
        }
#endif

        T ak = a[k];

        // Prefetch elements ahead for better cache utilization
        // This addresses the "memory wall" problem where CPU speed >> memory speed
        if (LIKELY(k > lower + 1)) {
            PREFETCH_READ(&a[k - 1]);
        }

        if (UNLIKELY(ak < pivot1)) {
            // Element belongs in left partition (< P1)
            while (lower < k) {
                if (LIKELY(a[++lower] >= pivot1)) {
                    if (UNLIKELY(a[lower] > pivot2)) {
                        // Found element > P2, move it to right partition
                        a[k] = a[--upper];
                        a[upper] = a[lower];
                    } else {
                        // Found element in middle partition
                        a[k] = a[lower];
                    }
                    a[lower] = ak;
                    break;
                }
            }
        } else if (UNLIKELY(ak > pivot2)) {
            // Element belongs in right partition (> P2)
            a[k] = a[--upper];
            a[upper] = ak;
        }
        // Elements with P1 ≤ ak ≤ P2 stay in place (middle partition)
    }

    // Swap the pivots into their final positions
    a[low] = a[lower];
    a[lower] = pivot1;  // P1 to its final position
    a[end] = a[upper];
    a[upper] = pivot2;  // P2 to its final position

    return std::make_pair(lower, upper);
}

template<typename T>
std::pair<int, int> partition_single_pivot(T* a, int low, int high, int pivotIndex1, int) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    int e3 = pivotIndex1;
    T pivot = a[e3];

    // The first element to be sorted is moved to the location formerly occupied by the pivot
    a[e3] = a[lower];

    // Traditional 3-way (Dutch National Flag) partitioning
    for (int k = ++upper; --k > lower; ) {
        T ak = a[k];

        if (ak != pivot) {
            a[k] = pivot;

            if (ak < pivot) { // Move a[k] to the left side
                while (lower < end && a[++lower] < pivot); // Added bounds check

                if (a[lower] > pivot) {
                    a[--upper] = a[lower];
                }
                a[lower] = ak;
            } else { // ak > pivot - Move a[k] to the right side
                a[--upper] = ak;
            }
        }
    }

    // The first element is moved to the pivot position
    a[low] = a[lower];
    a[lower] = pivot;

    return std::make_pair(lower, upper);
}

} // namespace dual_pivot

#endif // DPQS_PARTITION_HPP
