#ifndef DPQS_CORE_SORT_HPP
#define DPQS_CORE_SORT_HPP

#include "dpqs/utils.hpp"
#include "dpqs/sequential_sorters.hpp"
#include "dpqs/partition.hpp"
#include "dpqs/run_merger.hpp"
#include "dpqs/insertion_sort.hpp"
#include "dpqs/heap_sort.hpp"

namespace dual_pivot {

template<typename T>
void sort(T* a, int bits, int low, int high) {
    while (true) {
        int end = high - 1;
        int size = high - low;

        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            mixed_insertion_sort(a, low, high);
            return;
        }

        if (size < MAX_INSERTION_SORT_SIZE) {
            insertion_sort(a, low, high);
            return;
        }

        if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
                && tryMergeRuns(a, low, size)) {
            return;
        }

        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            heapSort(a, low, high);
            return;
        }

        int step = (size >> 3) * 3 + 3;
        int e1 = low + step;
        int e5 = end - step;
        int e3 = (e1 + e5) >> 1;
        int e2 = (e1 + e3) >> 1;
        int e4 = (e3 + e5) >> 1;

        sort5Network(a, e1, e2, e3, e4, e5);

        int lower;
        int upper;

        if (a[e1] < a[e2] && a[e2] < a[e3] && a[e3] < a[e4] && a[e4] < a[e5]) {
            auto pivotIndices = partition_dual_pivot(a, low, high, e1, e5);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            sort(a, bits | 1, lower + 1, upper);
            sort(a, bits | 1, upper + 1, high);

        } else {
            auto pivotIndices = partitionSinglePivot(a, low, high, e3, e3);
            lower = pivotIndices.first;
            upper = pivotIndices.second;

            sort(a, bits | 1, upper, high);
        }

        high = lower;
    }
}

} // namespace dual_pivot

#endif // DPQS_CORE_SORT_HPP
