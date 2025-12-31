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
void parallel_sort_task(T* a, int bits, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp) {
    // std::cout << "Task: " << low << "-" << high << std::endl;
    while (high - low > MIN_PARALLEL_SORT_SIZE) {
        std::ptrdiff_t size = high - low;
        std::ptrdiff_t end = high - 1;

        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            mixed_insertion_sort(a, low, high, comp);
            return;
        }

        if (size < MAX_INSERTION_SORT_SIZE) {
            insertion_sort(a, low, high, comp);
            return;
        }

        // if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
        //         && try_merge_runs(a, low, size, comp, true)) {
        //     return;
        // }

        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            heap_sort(a, low, high, comp);
            return;
        }

        std::ptrdiff_t step = (size >> 3) * 3 + 3;
        std::ptrdiff_t e1 = low + step;
        std::ptrdiff_t e5 = end - step;
        std::ptrdiff_t e3 = (e1 + e5) >> 1;
        std::ptrdiff_t e2 = (e1 + e3) >> 1;
        std::ptrdiff_t e4 = (e3 + e5) >> 1;

        // Sort 5-element sample
        sort5_network(a, e1, e2, e3, e4, e5, comp);

        std::ptrdiff_t lower, upper;

        if (comp(a[e1], a[e2]) && comp(a[e2], a[e3]) && comp(a[e3], a[e4]) && comp(a[e4], a[e5])) {
            auto pivotIndices = partition_dual_pivot(a, low, high, e1, e5, comp);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            // 3 ranges: [low, lower), [lower+1, upper), [upper+1, high)
            struct Range { std::ptrdiff_t l, h; std::ptrdiff_t sz; };
            Range ranges[3] = {
                {low, lower, lower - low},
                {lower + 1, upper, upper - (lower + 1)},
                {upper + 1, high, high - (upper + 1)}
            };

            // Sort ranges by size descending (Bubble sort for 3 elements is fine)
            if (ranges[0].sz < ranges[1].sz) std::swap(ranges[0], ranges[1]);
            if (ranges[1].sz < ranges[2].sz) std::swap(ranges[1], ranges[2]);
            if (ranges[0].sz < ranges[1].sz) std::swap(ranges[0], ranges[1]);

            // Submit largest 2 to pool
            auto& pool = getThreadPool();

            // Capture values explicitly to avoid array lifetime issues
            std::ptrdiff_t r0_l = ranges[0].l, r0_h = ranges[0].h;
            std::ptrdiff_t r1_l = ranges[1].l, r1_h = ranges[1].h;

            pool.submit([=]{ parallel_sort_task(a, bits | 1, r0_l, r0_h, comp); });
            pool.submit([=]{ parallel_sort_task(a, bits | 1, r1_l, r1_h, comp); });

            // Iterate on smallest
            low = ranges[2].l;
            high = ranges[2].h;

        } else {
            auto pivotIndices = partition_single_pivot(a, low, high, e3, e3, comp);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            // 2 ranges: [low, lower), [upper + 1, high)
            std::ptrdiff_t left_size = lower - low;
            std::ptrdiff_t right_size = high - (upper + 1);

            auto& pool = getThreadPool();
            if (left_size > right_size) {
                pool.submit([=]{ parallel_sort_task(a, bits | 1, low, lower, comp); });
                low = upper + 1;
                // high remains high
            } else {
                pool.submit([=]{ parallel_sort_task(a, bits | 1, upper + 1, high, comp); });
                high = lower;
                // low remains low
            }
        }
    }

    sort_sequential<T, Compare>(nullptr, a, bits, low, high, comp);
}

template<typename T, typename Compare>
void parallelQuickSort(T* a, int bits, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp, int parallelism = 0) {
    auto& pool = getThreadPool(parallelism);
    pool.submit([=]{ parallel_sort_task(a, bits, low, high, comp); });
    pool.wait_for_completion();
}

template<typename T, typename Compare>
void parallelSort(T* a, int parallelism, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp) {
    std::ptrdiff_t size = high - low;

    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);
        // Always use QuickSort for now as MergeSort is not adapted to V3 pool
        parallelQuickSort(a, depth, low, high, comp, parallelism);
    } else {
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
