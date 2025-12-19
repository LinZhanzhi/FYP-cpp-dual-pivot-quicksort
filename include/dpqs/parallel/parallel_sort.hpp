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
template<typename T> void parallelQuickSort(T* a, int bits, int low, int high);

template<typename T>
void parallelMergeSort(T* a, T* b, int low, int size, int offset, int depth) {
    if (depth < 0) {
        int half = size >> 1;

        auto& pool = getThreadPool();
        auto future1 = pool.enqueue([=] { parallelMergeSort(b, a, low, half, offset, depth + 1); });
        auto future2 = pool.enqueue([=] { parallelMergeSort(b, a, low + half, size - half, offset, depth + 1); });

        future1.get();
        future2.get();

        parallel_merge_parts(a, low, b, low, low + half, b, low + half, low + size);
    } else {
        std::copy(a + low, a + low + size, b + low - offset);
        sort_sequential<T>(nullptr, b, depth, low - offset, low - offset + size);
        std::copy(b + low - offset, b + low - offset + size, a + low);
    }
}

template<typename T>
void parallelQuickSort(T* a, int bits, int low, int high) {
    int size = high - low;

    if (size > MIN_PARALLEL_SORT_SIZE) {
        while (true) {
            int end = high - 1;
            size = high - low;

            if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
                mixed_insertion_sort(a, low, high);
                return;
            }

            if (size < MAX_INSERTION_SORT_SIZE) {
                insertion_sort(a, low, high);
                return;
            }

            if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
                    && try_merge_runs(a, low, size, true)) {
                return;
            }

            if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
                heap_sort(a, low, high);
                return;
            }

            int step = (size >> 3) * 3 + 3;
            int e1 = low + step;
            int e5 = end - step;
            int e3 = (e1 + e5) >> 1;
            int e2 = (e1 + e3) >> 1;
            int e4 = (e3 + e5) >> 1;

            // Sort 5-element sample
            sort5_network(a, e1, e2, e3, e4, e5);

            int lower, upper;

            if (a[e1] < a[e2] && a[e2] < a[e3] && a[e3] < a[e4] && a[e4] < a[e5]) {
                auto pivotIndices = partition_dual_pivot(a, low, high, e1, e5);
                lower = pivotIndices.first;
                upper = pivotIndices.second;

                auto& pool = getThreadPool();
                auto future1 = pool.enqueue([=] { parallelQuickSort(a, bits | 1, lower + 1, upper); });
                auto future2 = pool.enqueue([=] { parallelQuickSort(a, bits | 1, upper + 1, high); });

                future1.get();
                future2.get();

            } else {
                auto pivotIndices = partition_single_pivot(a, low, high, e3, e3);
                lower = pivotIndices.first;
                upper = pivotIndices.second;

                auto& pool = getThreadPool();
                auto future = pool.enqueue([=] { parallelQuickSort(a, bits | 1, upper, high); });
                future.get();
            }

            high = lower;
        }
    } else {
        sort_sequential<T>(nullptr, a, bits, low, high);
    }
}

template<typename T>
void parallelSort(T* a, int parallelism, int low, int high) {
    int size = high - low;

    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);

        if (depth < 0) {
            std::vector<T> b(size);
            parallelMergeSort(a, b.data(), low, size, low, depth);
        } else {
            parallelQuickSort(a, depth, low, high);
        }
    } else {
        sort_sequential<T>(nullptr, a, 0, low, high);
    }
}

template<typename T>
class AdvancedSorter : public Sorter<T> {
private:
    AdvancedSorter* parent;
    T* a;
    T* b;
    int low;
    int size;
    int offset;
    int depth;

public:
    AdvancedSorter(AdvancedSorter* parent, T* a, T* b, int low, int size, int offset, int depth)
        : Sorter<T>(parent, a, b, low, size, offset, depth),
          parent(parent), a(a), b(b), low(low), size(size), offset(offset), depth(depth) {}

    void compute() override {
        Sorter<T>::compute();
    }
};

} // namespace dual_pivot

#endif // DPQS_PARALLEL_PARALLEL_SORT_HPP
