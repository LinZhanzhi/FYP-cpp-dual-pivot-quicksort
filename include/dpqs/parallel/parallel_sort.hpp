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
void parallelMergeSort(T* a, T* b, std::ptrdiff_t low, std::ptrdiff_t size, std::ptrdiff_t offset, int depth, Compare comp) {
    if (depth < 0) {
        std::ptrdiff_t half = size >> 1;

        auto& pool = getThreadPool();
        auto future1 = pool.enqueue([=] { parallelMergeSort(b, a, low, half, offset, depth + 1, comp); });
        auto future2 = pool.enqueue([=] { parallelMergeSort(b, a, low + half, size - half, offset, depth + 1, comp); });

        future1.get();
        future2.get();

        parallel_merge_parts(a, low, b, low, low + half, b, low + half, low + size, comp);
    } else {
        std::copy(a + low, a + low + size, b + low - offset);
        sort_sequential<T, Compare>(nullptr, b, depth, low - offset, low - offset + size, comp);
        std::copy(b + low - offset, b + low - offset + size, a + low);
    }
}

template<typename T, typename Compare>
void parallelQuickSort(T* a, int bits, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp) {
    std::ptrdiff_t size = high - low;

    if (size > MIN_PARALLEL_SORT_SIZE) {
        while (true) {
            std::ptrdiff_t end = high - 1;
            size = high - low;

            if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
                mixed_insertion_sort(a, low, high, comp);
                return;
            }

            if (size < MAX_INSERTION_SORT_SIZE) {
                insertion_sort(a, low, high, comp);
                return;
            }

            if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
                    && try_merge_runs(a, low, size, comp, true)) {
                return;
            }

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

                auto& pool = getThreadPool();
                auto future1 = pool.enqueue([=] { parallelQuickSort(a, bits | 1, lower + 1, upper, comp); });
                auto future2 = pool.enqueue([=] { parallelQuickSort(a, bits | 1, upper + 1, high, comp); });

                future1.get();
                future2.get();

            } else {
                auto pivotIndices = partition_single_pivot(a, low, high, e3, e3, comp);
                lower = pivotIndices.first;
                upper = pivotIndices.second;

                auto& pool = getThreadPool();
                auto future = pool.enqueue([=] { parallelQuickSort(a, bits | 1, upper, high, comp); });
                future.get();
            }

            high = lower;
        }
    } else {
        sort_sequential<T, Compare>(nullptr, a, bits, low, high, comp);
    }
}

template<typename T, typename Compare>
void parallelSort(T* a, int parallelism, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp) {
    std::ptrdiff_t size = high - low;

    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);

        if (depth < 0) {
            std::vector<T> b(size);
            parallelMergeSort(a, b.data(), low, size, low, depth, comp);
        } else {
            parallelQuickSort(a, depth, low, high, comp);
        }
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
