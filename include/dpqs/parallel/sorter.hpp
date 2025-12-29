#ifndef DPQS_PARALLEL_SORTER_HPP
#define DPQS_PARALLEL_SORTER_HPP

#include "dpqs/parallel/completer.hpp"
#include "dpqs/parallel/merger.hpp"
#include "dpqs/types.hpp"
#include "dpqs/utils.hpp"
#include "dpqs/parallel/threadpool.hpp"
#include <vector>

namespace dual_pivot {

// Forward declarations
template<typename T> class Sorter;
template<typename T>
void sort_sequential(Sorter<T>* sorter, T* a, int bits, std::ptrdiff_t low, std::ptrdiff_t high);

/**
 * @brief Generic sorter for type-erased array operations
 *
 * This class provides a generic interface for sorting arrays of different primitive
 * types using a single implementation. It mimics Java's Object-based array handling
 * while maintaining C++ type safety through the ArrayPointer variant system.
 *
 * Type Erasure Strategy:
 * - Uses ArrayPointer variant to store typed array pointers safely
 * - Runtime type dispatch to appropriate type-specific sorting functions
 * - Maintains Java compatibility for algorithm behavior
 * - Enables generic parallel coordination without template instantiation issues
 *
 * Buffer Management:
 * - Coordinates primary array (a) and auxiliary buffer (b) usage
 * - Tracks offset for complex buffer reuse patterns
 * - Manages buffer allocation and deallocation automatically
 *
 * Parallel Coordination:
 * - Integrates with CountedCompleter framework for work distribution
 * - Supports fork-join pattern for recursive parallel sorting
 * - Handles completion propagation and error handling
 *
 * Note: This is a simplified implementation to resolve forward declaration
 * dependencies. Full runtime type dispatch will be implemented after all
 * type-specific functions are properly declared.
 */
class GenericSorter : public CountedCompleter<void> {
private:
    GenericSorter* parent;       ///< Parent task for completion propagation
    ArrayPointer a;              ///< Primary array (equivalent to Java's Object a)
    ArrayPointer b;              ///< Buffer array (equivalent to Java's Object b)
    std::ptrdiff_t low;                     ///< Starting index of range to sort
    std::ptrdiff_t size;                    ///< Number of elements to sort
    std::ptrdiff_t offset;                  ///< Buffer offset for reuse optimization
    int depth;                   ///< Recursion depth for algorithm selection
    std::vector<GenericSorter*> children;

public:
    /**
     * @brief Construct a generic sorter task
     * @param parent Parent task for completion coordination
     * @param a Primary array to sort
     * @param b Auxiliary buffer for merge operations
     * @param low Starting index of sort range
     * @param size Number of elements to sort
     * @param offset Buffer offset for reuse patterns
     * @param depth Recursion depth for algorithm selection
     */
    GenericSorter(GenericSorter* parent, ArrayPointer a, ArrayPointer b, std::ptrdiff_t low, std::ptrdiff_t size, std::ptrdiff_t offset, int depth)
        : CountedCompleter<void>(parent), parent(parent), a(a), b(b),
          low(low), size(size), offset(offset), depth(depth) {}

    ~GenericSorter() {
        for (auto* child : children) {
            delete child;
        }
    }

    /**
     * @brief Main computation method - performs type dispatch and sorting
     *
     * This method will perform runtime type dispatch to call the appropriate
     * type-specific sorting function based on the ArrayPointer variant type.
     * Currently simplified to avoid forward declaration circular dependencies.
     */
    void compute() override {
        this->addToPendingCount(1);
        // TODO: Implement full runtime type dispatch here
        // Will dispatch to appropriate type-specific sorter based on a.data type
        // For now, just complete the task to avoid compilation issues
        this->addToPendingCount(-1);
        tryComplete();
    }

    void onCompletion(CountedCompleter<void>* caller) override {
        // Simplified completion - full merge logic will be implemented later
    }

        /**
     * @brief Fork a child sorter task with array range
     *
     * Creates and launches a child sorting task for the specified range.
     * Uses Java-style forkSorter pattern with local variable optimization
     * to improve performance through better register allocation.
     *
     * @param depth Recursion depth for the child task
     * @param low Starting index of range for child task
     * @param high Ending index of range for child task (exclusive)
     */
    void forkSorter(int depth, int low, int high) {
        // addToPendingCount(1); // Removed: Constructor already increments pending count
        // Local variable optimization (matching Java pattern)
        // Helps compiler with register allocation and reduces memory accesses
        ArrayPointer localA = this->a;
        auto* child = new GenericSorter(this, localA, b, low, high - low, offset, depth);
        children.push_back(child);
        child->fork();
    }

    // Getter methods for buffer access
    ArrayPointer getArrayA() const { return a; }
    ArrayPointer getArrayB() const { return b; }
    int getOffset() const { return offset; }
};

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
    std::vector<Sorter*> children; ///< Children tasks to manage memory

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

    ~Sorter() {
        for (auto* child : children) {
            delete child;
        }
    }

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
        this->addToPendingCount(1); // Hold pending count while running
        if (depth < 0) {
            // Use parallel merge sort for highly parallel scenarios
            // this->setPendingCount(2); // Removed to avoid deadlock
            int half = size >> 1;

            // Adjust b pointer so that b[low] maps to b_buffer[low - offset]
            // This is necessary because sort_sequential expects the array to be indexable by 'low'
            T* b_adjusted = b - offset;
            auto* left = new Sorter(this, b_adjusted, a, low, half, offset, depth + 1);
            auto* right = new Sorter(this, b_adjusted, a, low + half, size - half, offset, depth + 1);

            children.push_back(left);
            children.push_back(right);

            left->fork();
            right->compute();
        } else {
            // Use type-specific parallel quicksort via the generic sequential sorter
            // which handles forking if a sorter is provided
            sort_sequential(this, a, depth, low, low + size);
        }
        this->addToPendingCount(-1); // Release hold
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
        // this->addToPendingCount(1); // Removed: Constructor already increments pending count
        auto* child = new Sorter(this, a, b, low, high - low, offset, depth);
        children.push_back(child);
        child->fork();
    }

    // Helper method to set pending count (matching Java API)
    void setPendingCount(int count) {
        this->pending.store(count);
    }
};


}

#endif // DPQS_PARALLEL_SORTER_HPP
