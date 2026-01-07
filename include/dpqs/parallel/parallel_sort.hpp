#ifndef DPQS_PARALLEL_PARALLEL_SORT_HPP
#define DPQS_PARALLEL_PARALLEL_SORT_HPP

#include "dpqs/sequential_sorters.hpp"
#include "dpqs/run_merger.hpp"
#include "dpqs/parallel/merger.hpp"
#include "dpqs/parallel/buffer_manager.hpp"
#include "dpqs/parallel/completer.hpp"
#include "dpqs/parallel/sorter.hpp"
#include "dpqs/utils.hpp"
#include "dpqs/partition.hpp"
#include "dpqs/insertion_sort.hpp"
#include "dpqs/heap_sort.hpp"

namespace dual_pivot {

// Forward declarations
template<typename T, typename Compare> void parallelQuickSort(T* a, int bits, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp);

template<typename T, typename Compare>
/**
 * @brief Core Parallel Sort Task.
 *
 * This function implements the recursive step of the parallel sort algorithm.
 * It employs a hybrid strategy combining Introsort, Dual-Pivot Quicksort, and Insertion Sort.
 * It manages parallelism by offloading larger partitions to a thread pool while processing
 * smaller partitions iteratively in the current thread to minimize overhead and stack depth.
 *
 * @tparam T Type of elements in the array.
 * @tparam Compare Type of the comparison function object.
 *
 * @param a Pointer to the array to sort.
 * @param bits An integer acting as both a recursion depth counter and a state flag.
 *             It is incremented by `DELTA` to track depth for Introsort fallback.
 *             The least significant bit (bit 0) is used as a heuristic flag for
 *             Mixed Insertion Sort optimizations (indicating specific recursion branches).
 * @param low The starting index (inclusive) of the current segment to sort.
 * @param high The ending index (exclusive) of the current segment to sort.
 * @param comp The comparator function object used to determine the order of elements.
 *             Returns `true` if the first argument is strictly less than the second.
 */
/**
 * @brief Orchestrates a recursive, parallel dual-pivot quicksort task.
 *
 * This function forms the core work unit for the parallel sort algorithm. It processes a
 * contiguous range of an array, deciding whether to perform sorting immediately (for small
 * ranges), switch to a fallback algorithm (Introsort/Heapsort), or partition the range and
 * offload sub-tasks to a thread pool.
 *
 * @tparam T The type of elements in the array.
 * @tparam Compare The type of the comparison function object.
 *
 * @param a Pointer to the first element of the array being sorted.
 * @param bits A bitmask integer tracking recursion depth and partition origin.
 *             Incremented by `DELTA` to detect excessive recursion. Bit 0 indicates
 *             if the task is a "right-most" or derived part, affecting insertion sort heuristics.
 * @param low The starting index (inclusive) of the current range.
 * @param high The ending index (exclusive) of the current range.
 * @param comp The binary comparison function.
 *
 * @details
 * **Algorithm Strategy:**
 * 1. **Base Cases:**
 *    - If the range is small (depends on recursion depth), uses Mixed Insertion Sort.
 *    - If the range is very small, uses standard Insertion Sort.
 *    - If recursion depth is too high (Introsort), switches to Heap Sort to guarantee O(N log N).
 *
 * 2. **Pivot Selection Calculation:**
 *    The algorithm selects 5 equidistant points to sample the array for dual pivots.
 *    Given `size = high - low`, the step size is calculated to distribute points across the range.
 *
 *    Calculation Logic:
 *    - `step = (size / 8) * 3 + 3`: This approximates `3/8 * size`.
 *    - `e1 = low + step`          (Approx 3/8 point)
 *    - `e5 = (high - 1) - step`   (Approx 5/8 point)
 *    - `e3 = (e1 + e5) / 2`       (Midpoint/Median)
 *    - `e2 = (e1 + e3) / 2`       (Midpoint of first half)
 *    - `e4 = (e3 + e5) / 2`       (Midpoint of second half)
 *
 *    This ensures that `e1, e2, e3, e4, e5` are symmetrically distributed mostly within the center
 *    quartiles of the array to avoid edge cases in pre-sorted data.
 *
 * 3. **Partitioning & Parallelism:**
 *    - **Dual-Pivot:** If the 5 samples are strictly ordered, the array is partitioned into three
 *      segments (Left, Middle, Right) using `e1` and `e5` as pivots.
 *    - **Single-Pivot:** If samples contain duplicates, it falls back to single-pivot partitioning
 *      using the median `e3`.
 *    - **Task Shedding:** The resulting sub-ranges are sorted by size. The largest partitions
 *      are submitted to the thread pool to maximize load balancing, while the current thread
 *      immediately processes the smallest partition iteratively (Tail Call Optimization) to
 *      minimize stack usage.
 */
void parallel_sort_task(T* a, int bits, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp) {
    // std::cout << "Task: " << low << "-" << high << std::endl;

    // Core Loop: Continue iteratively as long as the segment is large enough specific parallel handling.
    // Ideally, we process the smallest segment in this loop (Tail Call Elimination equivalent)
    // while pushing larger segments to the thread pool.
    while (high - low > MIN_PARALLEL_SORT_SIZE) {
        std::ptrdiff_t size = high - low; // Size of the current range
        std::ptrdiff_t end = high - 1;    // Inclusive index of the last element

        // OPTIMIZATION: Mixed Insertion Sort
        // If the recursion depth is deep (bits are high) or bit 0 is set (indicating right-most or derived part),
        // and the size is small, use Mixed Insertion Sort. This is faster for nearly-sorted data.
        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            mixed_insertion_sort(a, low, high, comp);
            return;
        }

        // OPTIMIZATION: Standard Insertion Sort
        // For very small subarrays, the overhead of partitioning is high.
        // Simple insertion sort is faster here.
        if (size < MAX_INSERTION_SORT_SIZE) {
            insertion_sort(a, low, high, comp);
            return;
        }

        // Introsort Fallback:
        // If recursion depth exceeds the limit (MAX_RECURSION_DEPTH), switch to HeapSort.
        // This guarantees O(N log N) worst-case performance, preventing recursion bombs.
        // 'bits' acts as the depth counter here.
        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            heap_sort(a, low, high, comp);
            return;
        }

        // Pivot Selection Strategy (5 points):
        // Calculate 5 equidistant points (e1, e2, e3, e4, e5) to sample the array.
        //
        std::ptrdiff_t step = (size >> 3) * 3 + 3;
        std::ptrdiff_t e1 = low + step;
        std::ptrdiff_t e5 = end - step;
        std::ptrdiff_t e3 = (e1 + e5) >> 1;
        std::ptrdiff_t e2 = (e1 + e3) >> 1;
        std::ptrdiff_t e4 = (e3 + e5) >> 1;

        // Sort the 5 distinct sample elements.
        sort5_network(a, e1, e2, e3, e4, e5, comp);

        std::ptrdiff_t lower, upper; // Output partition boundaries

        // Dual-Pivot Condition:
        // Ensure the selected pivots are distinct enough.
        // e1 and e5 are chosen as Left (P1) and Right (P2) pivots.
        // We need P1 < P2. AND strict ordering between the samples helps guarantee good partitioning.
        if (comp(a[e1], a[e2]) && comp(a[e2], a[e3]) && comp(a[e3], a[e4]) && comp(a[e4], a[e5])) {
            // Perform Dual-Pivot Partitioning.
            // Rearranges array into [ < P1 | P1 <= .. <= P2 | > P2 ]
            auto pivotIndices = partition_dual_pivot(a, low, high, e1, e5, comp);
            lower = pivotIndices.first;   // End of Left part
            upper = pivotIndices.second;  // Start of Right part

            // Define the 3 resulting sub-ranges
            struct Range { std::ptrdiff_t l, h; std::ptrdiff_t sz; };
            Range ranges[3] = {
                {low, lower, lower - low},               // Range 0: Left
                {lower + 1, upper, upper - (lower + 1)}, // Range 1: Middle
                {upper + 1, high, high - (upper + 1)}    // Range 2: Right
            };

            // Sort ranges by size DESCENDING.
            // Why? We want to push the LARGEST tasks to the thread pool (for other threads to steal)
            // and keep the SMALLEST task for the current thread to process immediately.
            if (ranges[0].sz < ranges[1].sz) std::swap(ranges[0], ranges[1]);
            if (ranges[1].sz < ranges[2].sz) std::swap(ranges[1], ranges[2]);
            if (ranges[0].sz < ranges[1].sz) std::swap(ranges[0], ranges[1]);

            // Submit largest 2 ranges to pool
            auto& pool = getThreadPool();

            // Capture values explicitly to avoid array lifetime issues or reference decay
            std::ptrdiff_t r0_l = ranges[0].l, r0_h = ranges[0].h;
            std::ptrdiff_t r1_l = ranges[1].l, r1_h = ranges[1].h;

            // Enqueue largest tasks
            pool.submit([=]{ parallel_sort_task(a, bits | 1, r0_l, r0_h, comp); });
            pool.submit([=]{ parallel_sort_task(a, bits | 1, r1_l, r1_h, comp); });

            // LOOP OPTIMIZATION (Recursion depth capping):
            // The current thread ITERATES on the smallest range (ranges[2]).
            // This replaces a recursive call with a loop, keeping stack usage logarithmic.
            low = ranges[2].l;
            high = ranges[2].h;

        } else {
            // Fallback: Single-Pivot Partitioning
            // If the 5 samples were not strictly distinct, Dual-Pivot might not be efficient.
            // Use e3 (median of samples) as single pivot.
            auto pivotIndices = partition_single_pivot(a, low, high, e3, e3, comp);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            // 2 ranges: Left [low, lower) and Right [upper + 1, high)
            std::ptrdiff_t left_size = lower - low;
            std::ptrdiff_t right_size = high - (upper + 1);

            auto& pool = getThreadPool();

            // "Push Larger, Iterate Smaller" Strategy for Single Pivot case
            if (left_size > right_size) {
                // Left is bigger -> Push to pool
                pool.submit([=]{ parallel_sort_task(a, bits | 1, low, lower, comp); });
                // Iterate on Right (smaller)
                low = upper + 1;
                // high remains high
            } else {
                // Right is bigger -> Push to pool
                pool.submit([=]{ parallel_sort_task(a, bits | 1, upper + 1, high, comp); });
                // Iterate on Left (smaller)
                high = lower;
                // low remains low
            }
        }
    }

    // Process remainder sequentially.
    // Once the segment size drops below MIN_PARALLEL_SORT_SIZE, we stop parallelizing
    // and just run standard Sequential Dual-Pivot Quicksort.
    sort_sequential<T, Compare>(nullptr, a, bits, low, high, comp);
}

template<typename T, typename Compare>
void parallelQuickSort(T* a, int bits, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp, int parallelism = 0) {
    auto& pool = getThreadPool(parallelism);
    // Initial task submission: The entire array is one task.
    pool.submit([=]{ parallel_sort_task(a, bits, low, high, comp); });
    // Wait for all tasks to complete (barrier).
    pool.wait_for_completion();
}

/**
 * @brief Public entrance for Parallel Sort.
 *
 * @param parallelism Number of threads to use. If 0 or 1, might fallback or use default.
 */
template<typename T, typename Compare>
void parallelSort(T* a, int parallelism, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp) {
    std::ptrdiff_t size = high - low;

    // Only parallelize if we have >1 thread and the array is large enough.
    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);
        // Always use QuickSort for now as MergeSort is not adapted to V3 pool
        parallelQuickSort(a, depth, low, high, comp, parallelism);
    } else {
        // Fallback for single-thread or small arrays
        sort_sequential<T, Compare>(nullptr, a, 0, low, high, comp);
    }
}

template<typename T, typename Compare>
class AdvancedSorter : public Sorter<T, Compare> {
private:
    AdvancedSorter* parent;
    T* a;
    T* b;
    std::ptrdiff_t low;
    std::ptrdiff_t size;
    std::ptrdiff_t offset;
    int depth;
    Compare comp;

public:
    AdvancedSorter(AdvancedSorter* parent, T* a, T* b, std::ptrdiff_t low, std::ptrdiff_t size, std::ptrdiff_t offset, int depth, Compare comp)
        : Sorter<T, Compare>(parent, a, b, low, size, offset, depth, comp),
          parent(parent), a(a), b(b), low(low), size(size), offset(offset), depth(depth), comp(comp) {}

    void compute() override {
        Sorter<T, Compare>::compute();
    }
};

} // namespace dual_pivot

#endif // DPQS_PARALLEL_PARALLEL_SORT_HPP
