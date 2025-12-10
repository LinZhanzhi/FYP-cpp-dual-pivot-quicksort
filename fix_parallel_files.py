import os

# Merger code from .bak
merger_code = """
/**
 * @brief Parallel merger for combining sorted array segments
 *
 * This class implements parallel merging of two sorted array segments using
 * recursive decomposition. It automatically switches between parallel and
 * sequential merging based on segment sizes to optimize performance.
 *
 * Parallel Merge Strategy:
 * - Large segments: Use binary search partitioning for parallel subdivision
 * - Small segments: Use sequential merge to avoid thread overhead
 * - Load balancing: Ensure work is distributed evenly across threads
 *
 * Binary Search Partitioning:
 * - Find split points using std::lower_bound for balanced workload
 * - Recursive subdivision until segments are too small for parallelism
 * - Cache-aware processing to minimize memory access overhead
 *
 * @tparam T Element type being merged
 */
template<typename T>
class Merger : public CountedCompleter<T> {
private:
    T* dst;                      ///< Destination array for merged result
    int k;                       ///< Starting index in destination
    T* a1;                       ///< First source array
    int lo1, hi1;                ///< Range of first segment [lo1, hi1)
    T* a2;                       ///< Second source array
    int lo2, hi2;                ///< Range of second segment [lo2, hi2)

public:
    /**
     * @brief Construct a parallel merger task
     * @param parent Parent task for completion coordination
     * @param dst Destination array for merged output
     * @param k Starting index in destination array
     * @param a1 First source array
     * @param lo1 Start of first segment (inclusive)
     * @param hi1 End of first segment (exclusive)
     * @param a2 Second source array
     * @param lo2 Start of second segment (inclusive)
     * @param hi2 End of second segment (exclusive)
     */
    Merger(CountedCompleter<T>* parent, T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2)
        : CountedCompleter<T>(parent), dst(dst), k(k), a1(a1), lo1(lo1), hi1(hi1), a2(a2), lo2(lo2), hi2(hi2) {}

    /**
     * @brief Main computation method for parallel merging
     *
     * Implements the core merge logic with automatic parallelization for
     * large segments. Uses sophisticated load balancing to ensure optimal
     * thread utilization while maintaining cache efficiency.
     *
     * Merge Strategy:
     * - Check segment sizes against parallelization threshold
     * - Use parallel subdivision for large segments
     * - Fall back to sequential merge for small segments
     * - Maintain cache locality through careful work distribution
     */
    void compute() override {
        // Use parallel merge with subdivision for large parts
        if (hi1 - lo1 >= MIN_PARALLEL_MERGE_PARTS_SIZE && hi2 - lo2 >= MIN_PARALLEL_MERGE_PARTS_SIZE) {
            // Parallel merge with binary search partitioning
            parallelMergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
        } else {
            // Sequential merge for small parts
            mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
        }
    }
};
"""

# Sorter code from merger.hpp (I'll just paste it here)
sorter_code = """
/**
 * @brief Parallel sorter using work-stealing and recursive decomposition
 *
 * This class implements the core parallel sorting logic using the CountedCompleter
 * framework. It automatically decomposes large sorting tasks into smaller parallel
 * subtasks while maintaining cache efficiency and load balance.
 *
 * Parallel Strategy:
 * - Large arrays: Recursive subdivision using dual-pivot partitioning
 * - Medium arrays: Parallel merge sort with buffer management
 * - Small arrays: Sequential sorting with optimized algorithms
 *
 * Work Distribution:
 * - Uses forkSorter() to create parallel subtasks
 * - Automatic load balancing through work stealing
 * - Cache-aware task sizes to minimize memory contention
 *
 * Buffer Management:
 * - Coordinates primary array (a) and auxiliary buffer (b)
 * - Tracks buffer offsets for efficient reuse
 * - Handles buffer allocation and deallocation automatically
 *
 * Algorithm Selection:
 * - Depth-based algorithm switching (quicksort vs merge sort)
 * - Type-specific optimizations through static dispatch
 * - Automatic fallback to sequential algorithms for small tasks
 *
 * @tparam T Element type being sorted
 */
template<typename T>
class Sorter : public CountedCompleter<T> {
private:
    Sorter* parent;              ///< Parent task for completion propagation
    T* a;                        ///< Primary array to sort
    T* b;                        ///< Auxiliary array for merge operations
    int low;                     ///< Starting index of range to sort
    int size;                    ///< Number of elements to sort
    int offset;                  ///< Buffer offset for reuse optimization
    int depth;                   ///< Recursion depth for algorithm selection

public:
    /**
     * @brief Construct a parallel sorter task
     * @param parent Parent task for completion coordination
     * @param a Primary array to sort
     * @param b Auxiliary buffer for merge operations
     * @param low Starting index of sort range
     * @param size Number of elements to sort
     * @param offset Buffer offset for reuse patterns
     * @param depth Recursion depth for algorithm selection
     */
    Sorter(Sorter* parent, T* a, T* b, int low, int size, int offset, int depth)
        : CountedCompleter<T>(parent), parent(parent), a(a), b(b),
          low(low), size(size), offset(offset), depth(depth) {}

    /**
     * @brief Main computation method for parallel sorting
     *
     * This method implements the core sorting logic with automatic algorithm
     * selection based on recursion depth. It coordinates between parallel
     * quicksort and parallel merge sort depending on the parallelism requirements.
     *
     * Algorithm Selection:
     * - Negative depth: Use parallel merge sort for maximum parallelism
     * - Positive depth: Use type-specific parallel quicksort
     *
     * Type Dispatch:
     * - Compile-time selection of optimized sorting algorithms
     * - Fallback to generic implementation for unsupported types
     * - Integration with type-specific Sorter coordination
     */
    void compute() override {
        if (depth < 0) {
            // Use parallel merge sort for highly parallel scenarios
            this->setPendingCount(2);
            int half = size >> 1;

            auto* left = new Sorter(this, b, a, low, half, offset, depth + 1);
            auto* right = new Sorter(this, b, a, low + half, size - half, offset, depth + 1);

            left->fork();
            right->compute();
        } else {
            // Use type-specific parallel quicksort
            if constexpr (std::is_same_v<T, int>) {
                sort_int_sequential(this, a, depth, low, low + size);
            } else if constexpr (std::is_same_v<T, long>) {
                sort_long_sequential(this, a, depth, low, low + size);
            } else if constexpr (std::is_same_v<T, float>) {
                sort_float_sequential(this, a, depth, low, low + size);
            } else if constexpr (std::is_same_v<T, double>) {
                sort_double_sequential(this, a, depth, low, low + size);
            } else {
                // Fallback to generic implementation
                parallelQuickSort(a, depth, low, low + size);
            }
        }
        this->tryComplete();
    }

    /**
     * @brief Completion handler for merge operations
     *
     * Called when child tasks complete to perform necessary merge operations.
     * Handles the complex buffer management required for parallel merge sort
     * and coordinates the final merge of sorted segments.
     *
     * Buffer Coordination:
     * - Determines source and destination buffers based on depth
     * - Calculates proper offset values for buffer reuse
     * - Creates Merger tasks for combining sorted segments
     *
     * @param caller The child task that completed
     */
    void onCompletion(CountedCompleter<T>* caller) override {
        // Handle merge operations for negative depth (merge sort mode)
        if (depth < 0) {
            int mi = low + (size >> 1);
            bool src = (depth & 1) == 0;

            // Create merger for the two halves
            auto* merger = new Merger<T>(nullptr,
                a,                          // dst
                src ? low : low - offset,   // k
                b,                          // a1
                src ? low - offset : low,   // lo1
                src ? mi - offset : mi,     // hi1
                b,                          // a2
                src ? mi - offset : mi,     // lo2
                src ? low + size - offset : low + size  // hi2
            );
            merger->invoke();
            delete merger;
        }
    }

    // Factory method for creating child sorters (matching Java's forkSorter pattern)
    void forkSorter(int depth, int low, int high) {
        this->addToPendingCount(1);
        auto* child = new Sorter(this, a, b, low, high - low, offset, depth);
        child->fork();
    }

    // Helper method to set pending count (matching Java API)
    void setPendingCount(int count) {
        this->pending.store(count);
    }
};
"""

# Read GenericMerger from merger.hpp
with open('include/dpqs/parallel/merger.hpp', 'r') as f:
    merger_content = f.read()
    # Extract GenericMerger (assuming it's correct there)
    # It starts after namespace dual_pivot {
    start = merger_content.find('class GenericMerger')
    end = merger_content.find('template<typename T> class Merger;')
    if start != -1 and end != -1:
        generic_merger = merger_content[start:end]
    else:
        # Fallback if extraction fails, assume it's the first class
        # Try to find end of class GenericMerger
        end = merger_content.find('};', start)
        if start != -1 and end != -1:
            generic_merger = merger_content[start:end+2]
        else:
            generic_merger = ""

# Read GenericSorter from sorter.hpp
with open('include/dpqs/parallel/sorter.hpp', 'r') as f:
    sorter_content = f.read()
    # Extract GenericSorter
    start = sorter_content.find('class GenericSorter')
    # It ends before template<typename T> class Sorter (forward decl) or just before Sorter definition if it was there
    # But sorter.hpp currently has forward decl
    end = sorter_content.find('template<typename T> class Sorter')
    if start != -1 and end != -1:
        generic_sorter = sorter_content[start:end]
    else:
        # Try to find end of class GenericSorter
        end = sorter_content.find('};', start)
        if start != -1 and end != -1:
            generic_sorter = sorter_content[start:end+2]
        else:
            generic_sorter = ""

# Write merger.hpp
new_merger_content = f"""#ifndef DPQS_PARALLEL_MERGER_HPP
#define DPQS_PARALLEL_MERGER_HPP

#include "dpqs/parallel/completer.hpp"
#include "dpqs/types.hpp"
#include "dpqs/utils.hpp"

namespace dual_pivot {{

{generic_merger}

{merger_code}

}}

#endif // DPQS_PARALLEL_MERGER_HPP
"""

with open('include/dpqs/parallel/merger.hpp', 'w') as f:
    f.write(new_merger_content)

# Write sorter.hpp
new_sorter_content = f"""#ifndef DPQS_PARALLEL_SORTER_HPP
#define DPQS_PARALLEL_SORTER_HPP

#include "dpqs/parallel/completer.hpp"
#include "dpqs/parallel/merger.hpp"
#include "dpqs/types.hpp"
#include "dpqs/utils.hpp"

namespace dual_pivot {{

// Forward declarations
template<typename T> class Sorter;
static void sort_int_sequential(Sorter<int>* sorter, int* a, int bits, int low, int high);
static void sort_long_sequential(Sorter<long>* sorter, long* a, int bits, int low, int high);
static void sort_float_sequential(Sorter<float>* sorter, float* a, int bits, int low, int high);
static void sort_double_sequential(Sorter<double>* sorter, double* a, int bits, int low, int high);
template<typename T> void parallelQuickSort(T* a, int depth, int low, int high);

{generic_sorter}

{sorter_code}

}}

#endif // DPQS_PARALLEL_SORTER_HPP
"""

with open('include/dpqs/parallel/sorter.hpp', 'w') as f:
    f.write(new_sorter_content)
