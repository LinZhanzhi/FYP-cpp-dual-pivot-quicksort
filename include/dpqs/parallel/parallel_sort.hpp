#ifndef DPQS_PARALLEL_PARALLEL_SORT_HPP
#define DPQS_PARALLEL_PARALLEL_SORT_HPP

#include "dpqs/core_sort.hpp"
#include "dpqs/sequential_sorters.hpp"
#include "dpqs/run_merger.hpp"
#include "dpqs/parallel/merger.hpp"
#include "dpqs/parallel/buffer_manager.hpp"
#include "dpqs/parallel/completer.hpp"
#include "dpqs/parallel/sorter.hpp"
#include "dpqs/utils.hpp"

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

        parallelMergeParts(a, low, b, low, low + half, b, low + half, low + size);
    } else {
        std::copy(a + low, a + low + size, b + low - offset);
        sort(b, depth, low - offset, low - offset + size);
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
            T a3 = a[e3];

            if (a[e5] < a[e2]) { T t = a[e5]; a[e5] = a[e2]; a[e2] = t; }
            if (a[e4] < a[e1]) { T t = a[e4]; a[e4] = a[e1]; a[e1] = t; }
            if (a[e5] < a[e4]) { T t = a[e5]; a[e5] = a[e4]; a[e4] = t; }
            if (a[e2] < a[e1]) { T t = a[e2]; a[e2] = a[e1]; a[e1] = t; }
            if (a[e4] < a[e2]) { T t = a[e4]; a[e4] = a[e2]; a[e2] = t; }

            if (a3 < a[e2]) {
                if (a3 < a[e1]) {
                    a[e3] = a[e2]; a[e2] = a[e1]; a[e1] = a3;
                } else {
                    a[e3] = a[e2]; a[e2] = a3;
                }
            } else if (a3 > a[e4]) {
                if (a3 > a[e5]) {
                    a[e3] = a[e4]; a[e4] = a[e5]; a[e5] = a3;
                } else {
                    a[e3] = a[e4]; a[e4] = a3;
                }
            }

            int lower, upper;

            if (a[e1] < a[e2] && a[e2] < a[e3] && a[e3] < a[e4] && a[e4] < a[e5]) {
                auto pivotIndices = partitionDualPivot(a, low, high, e1, e5);
                lower = pivotIndices.first;
                upper = pivotIndices.second;

                auto& pool = getThreadPool();
                auto future1 = pool.enqueue([=] { parallelQuickSort(a, bits | 1, lower + 1, upper); });
                auto future2 = pool.enqueue([=] { parallelQuickSort(a, bits | 1, upper + 1, high); });

                future1.get();
                future2.get();

            } else {
                auto pivotIndices = partitionSinglePivot(a, low, high, e3, e3);
                lower = pivotIndices.first;
                upper = pivotIndices.second;

                auto& pool = getThreadPool();
                auto future = pool.enqueue([=] { parallelQuickSort(a, bits | 1, upper, high); });
                future.get();
            }

            high = lower;
        }
    } else {
        sort(a, bits, low, high);
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
        sort(a, 0, low, high);
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
    bool owns_buffer;

public:
    AdvancedSorter(AdvancedSorter* parent, T* a, T* b, int low, int size, int offset, int depth)
        : Sorter<T>(parent, a, b, low, size, offset, depth), parent(parent), a(a), b(b),
          low(low), size(size), offset(offset), depth(depth), owns_buffer(false) {

        if (b == nullptr && depth >= 0) {
            this->b = BufferManager<T>::getBuffer(size, this->offset);
            owns_buffer = true;
        }
    }

    ~AdvancedSorter() {
        if (owns_buffer && b != nullptr) {
            BufferManager<T>::returnBuffer(b, size, offset);
        }
    }

    void compute() override {
        if (depth < 0) {
            this->setPendingCount(2);
            int half = size >> 1;

            auto* left = new AdvancedSorter(this, b, a, low, half, offset, depth + 1);
            auto* right = new AdvancedSorter(this, b, a, low + half, size - half, offset, depth + 1);

            left->fork();
            right->compute();
        } else {
            if constexpr (std::is_same_v<T, int>) {
                sort_int_sequential(this, a, depth, low, low + size);
            } else if constexpr (std::is_same_v<T, long>) {
                sort_long_sequential(this, a, depth, low, low + size);
            } else if constexpr (std::is_same_v<T, float>) {
                sort_float_sequential(this, a, depth, low, low + size);
            } else if constexpr (std::is_same_v<T, double>) {
                sort_double_sequential(this, a, depth, low, low + size);
            } else {
                parallelQuickSort(a, depth, low, low + size);
            }
        }
        this->tryComplete();
    }

    void onCompletion(CountedCompleter<T>* caller) override {
        if (depth < 0) {
            int mi = low + (size >> 1);
            bool src = (depth & 1) == 0;

            T* dst = src ? a : b;
            int k = src ? (low - offset) : low;

            auto* merger = new Merger<T>(nullptr,
                dst, k,
                b, src ? (low - offset) : low, src ? (mi - offset) : mi,
                b, src ? (mi - offset) : mi, src ? (low + size - offset) : (low + size)
            );
            merger->invoke();
            delete merger;
        }
    }

    void forkSorter(int depth, int low, int high) {
        this->addToPendingCount(1);
        T* localA = this->a;
        auto* child = new AdvancedSorter(this, localA, b, low, high - low, offset, depth);
        child->fork();
    }

    void setPendingCount(int count) {
        this->pending.store(count);
    }

    T* getBuffer() { return b; }
    int getOffset() { return offset; }
};

} // namespace dual_pivot

#endif // DPQS_PARALLEL_PARALLEL_SORT_HPP
