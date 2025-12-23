# Dual-Pivot Quicksort Implementation Report

## 1. Overview

This report details the implementation of the Dual-Pivot Quicksort library located in `include/`. The library is designed as a high-performance, generic C++ sorting framework that mimics the behavior of Java's `Arrays.sort()` and `Arrays.parallelSort()`. It features a hybrid architecture that combines:

1.  **Dual-Pivot Quicksort**: For sequential sorting of primitive types (int, double, etc.).
2.  **Parallel Merge Sort**: For parallel sorting of large arrays, using the Dual-Pivot algorithm for leaf tasks.
3.  **Counting Sort**: Optimized handling for small integral types (byte, short).
4.  **CountedCompleter Framework**: A custom C++ implementation of Java's ForkJoin task scheduling model.

## 2. File Structure & Responsibilities

The implementation is header-only to support generic templating.

### Core Interface
- **`include/dual_pivot_quicksort.hpp`**: The main entry point. It contains the `sort()` function which acts as a dispatcher. It decides which algorithm to use based on:
    - **Type**: Integral vs. Floating Point.
    - **Size**: Small vs. Large.
    - **Parallelism**: Enabled vs. Disabled.

### Sequential Algorithms (`include/dpqs/`)
- **`partition.hpp`**: Implements Yaroslavskiy's dual-pivot partitioning logic. This is the core mathematical engine of the library.
- **`sequential_sorters.hpp`**: Contains the recursive `sort_sequential` function. It manages the recursion stack and switches to insertion sort for small subarrays.
- **`insertion_sort.hpp`**: A highly optimized insertion sort for small arrays (typically < 47 elements).
- **`counting_sort.hpp`**: Specialized O(n) sorter for `int8_t`, `uint8_t`, `int16_t`, and `uint16_t`.
- **`float_sort.hpp`**: Handles floating-point specific edge cases like `NaN` (moved to end) and `-0.0` vs `0.0`.

### Parallel Framework (`include/dpqs/parallel/`)
- **`completer.hpp`**: Defines `CountedCompleter`, a base class for tasks. It implements a lock-free(ish) pending count mechanism to manage task dependencies without blocking threads.
- **`sorter.hpp`**: Defines `Sorter` (and `AdvancedSorter`), the concrete task that performs sorting. It splits the array and launches child tasks.
- **`merger.hpp`**: Defines `RunMerger`, a task responsible for merging two sorted subarrays.
- **`threadpool.hpp`**: A simple thread pool to execute the tasks.

## 3. Implementation Workflow

### Step 1: Dispatching (`dual_pivot_quicksort.hpp`)
When a user calls `dual_pivot::sort(arr, size)`, the following happens:
1.  **Type Check**:
    - If `T` is `char` or `short` (1-2 bytes) and size > threshold -> Call `counting_sort`.
    - If `T` is `float` or `double` -> Call `sort_floats` (which eventually calls sequential sort).
2.  **Parallelism Check**:
    - If `parallelism > 1` and `size > MIN_PARALLEL_SORT_SIZE` (e.g., 8192) -> Initialize Parallel Sort.
3.  **Fallback**:
    - Otherwise -> Call `sort_sequential`.

### Step 2: Sequential Dual-Pivot Sort
Used for small arrays or as the base case for parallel sort.
1.  **Check Size**: If size < 47 (INSERTION_SORT_THRESHOLD), run `insertion_sort`.
2.  **Partition**: Call `partition_dual_pivot` (in `partition.hpp`).
    - Selects two pivots (P1, P2).
    - Reorders array into: `[ < P1 | P1 <= x <= P2 | > P2 ]`.
3.  **Recurse**: Recursively call `sort_sequential` on the three regions.

### Step 3: Parallel Sort (Hybrid Merge/Quicksort)
Used for large arrays. This is actually a **Parallel Merge Sort**.
1.  **Setup**: A temporary buffer `b` of the same size as `a` is allocated.
2.  **Root Task**: An `AdvancedSorter` task is created covering the whole array.
3.  **Task Execution (`Sorter::compute`)**:
    - **Base Case**: If the subarray size is small (e.g., fits in L2 cache), it calls `sort_sequential` (Dual-Pivot) and returns.
    - **Recursive Step**:
        - Split the range into 4 quarters.
        - Create 4 child `Sorter` tasks.
        - Call `fork()` to submit them to the thread pool.
        - Increment `pending` count by 4.
4.  **Completion (`tryComplete`)**:
    - When children finish, they decrement the parent's `pending` count.
    - When `pending` reaches 0, the parent's `onCompletion` method is triggered.
5.  **Merging (`RunMerger`)**:
    - `onCompletion` launches `RunMerger` tasks.
    - These tasks merge the 4 sorted quarters into a sorted whole using the buffer `b`.

## 4. Detailed Design Decisions (Q&A Prep)

**Q: Why use Dual-Pivot Quicksort instead of standard Quicksort?**
**A:** Dual-pivot quicksort (Yaroslavskiy's algorithm) reduces the total number of swaps required. By partitioning into three regions instead of two, it reduces the recursion depth and moves elements closer to their final positions faster. It is particularly effective on modern CPUs because it scans memory linearly (cache-friendly) and handles duplicate elements more efficiently than single-pivot schemes.

**Q: Why is the parallel implementation a Merge Sort?**
**A:** While Quicksort is great for in-place sorting, parallelizing the *partitioning* step of Quicksort is complex and hard to load-balance perfectly. Parallel Merge Sort is easier to implement robustly: you simply split the array arbitrarily (perfect load balancing), sort the parts in parallel, and then merge. We use Dual-Pivot Quicksort for the "leaf" tasks because it's the fastest sequential sorter, giving us the best of both worlds.

**Q: How does `CountedCompleter` work?**
**A:** It's a mechanism to avoid blocking threads. Instead of a thread calling `future.get()` and sleeping (wasting a thread), tasks use a "continuation" style. A task finishes its work and says "I'm done". If it was the last child to finish, it executes the parent's completion logic (merging) immediately on the *current* thread. This keeps all threads busy processing data rather than waiting on locks.

**Q: How do you handle memory management in the parallel sort?**
**A:** We allocate a single auxiliary buffer `b` at the start. The `Sorter` tasks pass pointers to specific regions of this buffer down the tree. This avoids repeated `malloc/free` calls during recursion. We also recently fixed memory leaks by ensuring that `Sorter` tasks properly delete their child objects in their destructors.

**Q: What happens with floating-point numbers?**
**A:** Standard comparison (`<`) fails for `NaN`. Our `sort_floats` wrapper moves all `NaN` values to the end of the array first, treating them as "larger than everything". It also normalizes `-0.0` to `0.0` to ensure stable-like behavior where negative zero doesn't disrupt the order.

## 5. Workflow Diagram

```mermaid
graph TD
    A[User calls sort()] --> B{Type?}
    B -- Byte/Short --> C[Counting Sort]
    B -- Float/Double --> D[Float Pre-processing] --> E[Sequential Sort]
    B -- Int/Long --> F{Size > Threshold?}
    F -- No --> E[Sequential Dual-Pivot Sort]
    F -- Yes --> G[Parallel Sort Setup]

    subgraph Parallel Sort
    G --> H[Create Root Sorter]
    H --> I{Size < Granularity?}
    I -- Yes --> J[Sequential Dual-Pivot Sort]
    I -- No --> K[Split into 4]
    K --> L[Fork 4 Child Sorters]
    L --> M[Wait for Children]
    M --> N[Merge Sorted Parts]
    end
```
