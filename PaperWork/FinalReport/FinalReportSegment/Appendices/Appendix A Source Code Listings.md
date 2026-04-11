# Appendix A: Source Code Listings

This appendix presents the key algorithm implementations from the dual-pivot quicksort library. The complete source code is available in the `include/` directory.

---

## A.1 Dual-Pivot Partitioning (`dpqs/partition.hpp`)

The heart of Yaroslavskiy's dual-pivot quicksort algorithm. Partitions an array around two pivots P1 and P2 into three regions: elements < P1, P1 ≤ elements ≤ P2, and elements > P2.

```cpp
template<typename T, typename Compare>
DPQS_FORCE_INLINE std::pair<std::ptrdiff_t, std::ptrdiff_t>
partition_dual_pivot(T* a, std::ptrdiff_t low, std::ptrdiff_t high,
                     std::ptrdiff_t pivotIndex1, std::ptrdiff_t pivotIndex2,
                     Compare comp) {
    using std::swap;
    // Move pivots to ends
    swap(a[low], a[pivotIndex1]);
    swap(a[high - 1], a[pivotIndex2]);

    T pivot1 = a[low];
    T pivot2 = a[high - 1];

    std::ptrdiff_t lt = low + 1;
    std::ptrdiff_t gt = high - 2;
    std::ptrdiff_t k = lt;

    while (k <= gt) {
        if (comp(a[k], pivot1)) {
            swap(a[k], a[lt]);
            lt++;
            k++;
        } else if (comp(pivot2, a[k])) {
            while (k < gt && comp(pivot2, a[gt])) {
                gt--;
            }
            swap(a[k], a[gt]);
            gt--;
            if (comp(a[k], pivot1)) {
                swap(a[k], a[lt]);
                lt++;
            }
            k++;
        } else {
            k++;
        }
    }

    --lt;
    ++gt;
    swap(a[low], a[lt]);
    swap(a[high - 1], a[gt]);

    return std::make_pair(lt, gt);
}
```

---

## A.2 Dutch National Flag Partitioning (`dpqs/partition.hpp`)

Single-pivot 3-way partitioning for arrays with many duplicates. Creates three regions: < pivot, == pivot, and > pivot.

```cpp
template<typename T, typename Compare>
std::pair<std::ptrdiff_t, std::ptrdiff_t>
partition_single_pivot(T* a, std::ptrdiff_t low, std::ptrdiff_t high,
                       std::ptrdiff_t pivotIndex1, std::ptrdiff_t, Compare comp) {
    using std::swap;
    std::ptrdiff_t lt = low;
    std::ptrdiff_t gt = high;
    T pivot = a[pivotIndex1];

    swap(a[low], a[pivotIndex1]);

    std::ptrdiff_t i = low + 1;
    while (i < gt) {
        if (comp(a[i], pivot)) {
            swap(a[lt++], a[i++]);
        } else if (comp(pivot, a[i])) {
            gt--;
            while (i < gt && comp(pivot, a[gt])) {
                gt--;
            }
            swap(a[i], a[gt]);
        } else {
            i++;
        }
    }

    return std::make_pair(lt, gt - 1);
}
```

---

## A.3 Work-Stealing Thread Pool (`dpqs/parallel/threadpool.hpp`)

Distributed queue architecture with cache-line aligned queues to prevent false sharing.

```cpp
class ThreadPool {
private:
    // Cache-line aligned to prevent false sharing
    struct alignas(64) WorkStealingQueue {
        std::deque<std::function<void()>> q;
        std::mutex mtx;

        void push(std::function<void()> task) {
            std::lock_guard<std::mutex> lock(mtx);
            q.push_back(std::move(task));
        }

        // Pop from bottom (Owner only) - LIFO for cache locality
        bool try_pop(std::function<void()>& task) {
            std::lock_guard<std::mutex> lock(mtx);
            if (q.empty()) return false;
            task = std::move(q.back());
            q.pop_back();
            return true;
        }

        // Steal from top (Thieves only) - FIFO for large tasks
        bool try_steal(std::function<void()>& task) {
            // Non-blocking: use try_lock to avoid spinning
            std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
            if (!lock || q.empty()) return false;
            task = std::move(q.front());
            q.pop_front();
            return true;
        }
    };

    std::vector<std::unique_ptr<WorkStealingQueue>> queues;
    std::vector<std::thread> workers;
    alignas(64) std::atomic<bool> stop{false};
    alignas(64) std::atomic<long> incomplete_tasks{0};

public:
    ThreadPool(size_t num_threads) {
        queues.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            queues.push_back(std::make_unique<WorkStealingQueue>());
        }

        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this, i, num_threads] {
                thread_index = static_cast<int>(i);
                // Sticky victim strategy for spatial locality
                size_t last_victim = (i + 1) % num_threads;

                while (!stop) {
                    std::function<void()> task;
                    bool found = false;

                    // 1. Try Local Pop (LIFO)
                    if (queues[i]->try_pop(task)) {
                        found = true;
                    }
                    // 2. Try Steal (FIFO) - scan from last successful victim
                    else {
                        for (size_t k = 0; k < num_threads; ++k) {
                            size_t victim = (last_victim + k) % num_threads;
                            if (victim == i) continue;
                            if (queues[victim]->try_steal(task)) {
                                found = true;
                                last_victim = victim;  // Remember for next time
                                break;
                            }
                        }
                    }

                    if (found) {
                        task();
                        if (--incomplete_tasks == 0) {
                            wait_cv.notify_all();
                        }
                    } else {
                        std::this_thread::yield();
                    }
                }
            });
        }
    }

    template<typename F>
    void submit(F&& f) {
        incomplete_tasks++;
        int idx = (thread_index == -1) ? 0 : thread_index;
        queues[idx]->push(std::forward<F>(f));
    }
};
```

---

## A.4 Run Detection and Merging (`dpqs/run_merger.hpp`)

Identifies naturally occurring sorted subsequences (runs) and merges them efficiently.

```cpp
template<typename T, typename Compare>
bool try_merge_runs(T* a, std::ptrdiff_t low, std::ptrdiff_t size,
                    Compare comp, bool parallel = false) {
    std::vector<std::ptrdiff_t> run;
    std::ptrdiff_t high = low + size;
    std::ptrdiff_t count = 1, last = low;

    // Identify all runs
    for (std::ptrdiff_t k = low + 1; k < high; ) {
        if (comp(a[k - 1], a[k])) {
            // Ascending run
            while (++k < high && !comp(a[k], a[k - 1]));
        } else if (comp(a[k], a[k - 1])) {
            // Descending run - reverse it
            while (++k < high && !comp(a[k - 1], a[k]));
            for (int i = last - 1, j = k; ++i < --j && comp(a[j], a[i]); ) {
                std::swap(a[i], a[j]);
            }
        } else {
            // Constant sequence
            T ak = a[k];
            while (++k < high && !comp(ak, a[k]) && !comp(a[k], ak));
            if (k < high) continue;
        }

        // Quality heuristics
        if (run.empty()) {
            if (k == high) return true;  // Already sorted
            if (k - low < MIN_FIRST_RUN_SIZE) return false;  // Too small
            run.push_back(low);
            run.push_back(last = k);
        } else if (comp(a[last], a[last - 1])) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) return false;
            if (++count == MAX_RUN_CAPACITY) return false;
            run.push_back(last = k);
        } else {
            run.back() = last = k;
        }
    }

    // Merge runs
    if (count > 1) {
        std::vector<T> b(size);
        merge_runs(a, b.data(), low, 1, run, 0, count, comp);
    }
    return true;
}
```

---

## A.5 Counting Sort for Small Integers (`dpqs/counting_sort.hpp`)

O(n) sorting for 1-byte and 2-byte integral types using bucket counting.

```cpp
template<typename T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == 1, void>::type
counting_sort(T* array, std::ptrdiff_t start_index, std::ptrdiff_t end_index) {
    static constexpr int NUM_VALUES = 256;
    static constexpr int OFFSET = std::is_signed<T>::value ? 128 : 0;

    std::vector<int> frequency_count(NUM_VALUES, 0);

    // Count frequencies
    for (std::ptrdiff_t i = end_index; i > start_index; ) {
        int idx = static_cast<int>(array[--i]) + OFFSET;
        frequency_count[idx]++;
    }

    std::ptrdiff_t size = end_index - start_index;

    if (size > NUM_VALUES / 2) {
        // Dense: iterate backwards, fill from end
        std::ptrdiff_t write_index = end_index;
        for (int i = NUM_VALUES; --i >= 0; ) {
            T value = static_cast<T>(i - OFFSET);
            int count = frequency_count[i];
            while (count-- > 0) {
                array[--write_index] = value;
            }
        }
    } else {
        // Sparse: iterate forwards, fill from start
        std::ptrdiff_t write_index = start_index;
        for (int i = 0; i < NUM_VALUES; i++) {
            if (frequency_count[i] > 0) {
                T value = static_cast<T>(i - OFFSET);
                int count = frequency_count[i];
                while (count-- > 0) {
                    array[write_index++] = value;
                }
            }
        }
    }
}
```

---

## A.6 Constants (`dpqs/constants.hpp`)

Tuned threshold constants for algorithm switching.

```cpp
namespace dual_pivot {

// Insertion sort threshold (tuned: 60)
constexpr int MAX_INSERTION_SORT_SIZE = 60;

// Parallel task granularity (tuned: 8192)
constexpr int MIN_PARALLEL_SORT_SIZE = 8192;

// Run merger thresholds
constexpr int MIN_FIRST_RUN_SIZE = 16;
constexpr int MIN_FIRST_RUNS_FACTOR = 6;  // Tuned from Java's 7
constexpr int MAX_RUN_CAPACITY = 500;
constexpr int MIN_RUN_COUNT = 5;

// Counting sort thresholds
constexpr int MIN_BYTE_COUNTING_SORT_SIZE = 64;
constexpr int MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE = 1750;

// Introsort depth multiplier
constexpr int DELTA = 3;

} // namespace dual_pivot
```

---

## A.7 Cache-Friendly Insertion Sort (`dpqs/insertion_sort.hpp`)

Optimized insertion sort with prefetching for small array base case.

```cpp
template<typename T, typename Compare>
DPQS_FORCE_INLINE void insertion_sort(T* a, std::ptrdiff_t low,
                                       std::ptrdiff_t high, Compare comp) {
    for (std::ptrdiff_t i, k = low; ++k < high; ) {
        T ai = a[i = k];

        // Prefetch next element to improve cache performance
        if (DPQS_LIKELY(k + 1 < high)) {
            DPQS_PREFETCH_READ(&a[k + 1]);
        }

        // Branch prediction hint for common case (already sorted)
        if (DPQS_UNLIKELY(comp(ai, a[i - 1]))) {
            while (--i >= low && comp(ai, a[i])) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}
```

---

## A.8 Public API (`dual_pivot_quicksort.hpp`)

STL-compatible interface for easy integration.

```cpp
namespace dual_pivot {

// Sort array with automatic parallelism
template<typename T>
void sort(T* a, std::ptrdiff_t length);

// Sort with custom comparator
template<typename T, typename Compare>
void sort(T* a, std::ptrdiff_t length, Compare comp);

// Sort container (std::vector, std::array, etc.)
template<typename Container>
void sort(Container& container);

// Sort with explicit thread count
template<typename Container>
void sort(Container& container, int parallelism);

// Iterator interface (STL-style)
template<typename RandomAccessIterator>
void dual_pivot_quicksort(RandomAccessIterator first, RandomAccessIterator last);

// Parallel iterator interface
template<typename RandomAccessIterator>
void dual_pivot_quicksort_parallel(RandomAccessIterator first,
                                   RandomAccessIterator last,
                                   int parallelism = std::thread::hardware_concurrency());

} // namespace dual_pivot
```

---

## Source File Directory Structure

```
include/
├── dual_pivot_quicksort.hpp     # Main public header
└── dpqs/
    ├── constants.hpp            # Tuned threshold constants
    ├── counting_sort.hpp        # O(n) sort for small integers
    ├── float_sort.hpp           # IEEE-754 handling (NaN, -0.0)
    ├── heap_sort.hpp            # Fallback for Introsort
    ├── insertion_sort.hpp       # Base case for small arrays
    ├── iterator_sort.hpp        # Non-contiguous iterator support
    ├── merge_ops.hpp            # Two-pointer merge operations
    ├── partition.hpp            # Dual-pivot & DNF partitioning
    ├── run_merger.hpp           # Run detection and merging
    ├── sequential_sorters.hpp   # Sequential sort dispatch
    ├── types.hpp                # Type traits and concepts
    ├── utils.hpp                # Utility macros (prefetch, likely)
    └── parallel/
        ├── buffer_manager.hpp   # Merge buffer pooling
        ├── completer.hpp        # CountedCompleter pattern
        ├── merger.hpp           # Parallel run merging
        ├── parallel_sort.hpp    # Parallel sort orchestration
        ├── sorter.hpp           # Parallel recursive sorter
        └── threadpool.hpp       # Work-stealing thread pool
```
