# Header File Organization

The project's header files are organized to separate the public API from internal implementation details, ensuring modularity and maintainability. The root `include` directory contains the public interface, while the `dpqs` subdirectory houses the implementation logic.

## Directory Structure

```
include/
├── dual_pivot_quicksort.hpp       # Public API Entry Point
└── dpqs/                          # Internal Implementation Details
    ├── constants.hpp              # Tuning constants & thresholds
    ├── types.hpp                  # Type definitions & SFINAE helpers
    ├── utils.hpp                  # Common utility functions
    ├── partition.hpp              # Partitioning logic
    ├── sequential_sorters.hpp     # Sequential sort implementations
    ├── insertion_sort.hpp         # Small array fallback
    ├── heap_sort.hpp              # Introsort fallback
    ├── counting_sort.hpp          # Optimization for byte/short/char
    ├── float_sort.hpp             # Floating-point handling (NaNs, -0.0)
    ├── iterator_sort.hpp          # Iterator-based interface support
    ├── merge_ops.hpp              # Run merging utilities
    ├── run_merger.hpp             # Adaptive run merging
    └── parallel/                  # Parallel Implementation
        ├── parallel_sort.hpp      # Parallel sort entry point
        ├── threadpool.hpp         # Thread management
        ├── sorter.hpp             # Sorting tasks
        ├── merger.hpp             # Merging tasks
        ├── completer.hpp          # Task completion handling
        └── buffer_manager.hpp     # Memory management for tasks
```

## Public Interface

### `dual_pivot_quicksort.hpp`
This is the **only file users need to include**. It exposes the `dual_pivot::sort` function template and handles dispatching to the appropriate implementation based on the input type and size.

*   **Role**: Facade / Entry Point.
*   **Key Responsibilities**:
    *   Provides user-facing API (`sort()`).
    *   Includes necessary internal headers.
    *   Handles input validation.
    *   Dispatches to parallel or sequential implementations.

## Internal Implementation (`dpqs/`)

The `dpqs` directory contains the internal building blocks of the sorting library. These files are not intended to be included directly by end-users.

### Core Logic
*   **`sequential_sorters.hpp`**: Implements the main sequential Dual-Pivot Quicksort algorithm. It coordinates the recursive calls and strategy switching.
*   **`partition.hpp`**: Contains the logic for partitioning arrays around two pivots. This is the heart of the Dual-Pivot algorithm.
*   **`insertion_sort.hpp`**: Highly optimized insertion sort used for small subarrays (leaf nodes of the recursion).
*   **`heap_sort.hpp`**: Used as a fallback when recursion depth becomes too great (Introsort strategy) to guarantee $O(n \log n)$ worst-case performance.

### Specialized Sorters
*   **`counting_sort.hpp`**: Specialized implementation for `byte`, `short`, and `char` types using counting sort when the range is small.
*   **`float_sort.hpp`**: Pre-processes floating-point arrays to handle special cases like `NaN` and negative zero (`-0.0`) before sorting.
*   **`iterator_sort.hpp`**: Adapters to allow the library to sort generic STL containers using iterators, converting them to pointer-based calls where possible.

### Utilities and Config
*   **`types.hpp`**: Defines internal types, traits, and concepts used throughout the library for type safety and metaprogramming.
*   **`constants.hpp`**: Central location for tuning parameters (e.g., `INSERTION_SORT_THRESHOLD`, `PARALLEL_THRESHOLD`).
*   **`utils.hpp`**: Generic helper functions for swapping, range checking, and other common low-level operations.

### Parallel Submodule (`dpqs/parallel/`)
Contains all logic related to multi-threaded sorting.

*   **`parallel_sort.hpp`**: Main driver for the parallel sorting algorithm.
*   **`threadpool.hpp`**: A custom thread pool implementation to manage worker threads efficiently.
*   **`sorter.hpp`**: Defines the "Sorter" task, which handles sorting chunks of the array in parallel.
*   **`merger.hpp`**: Defines the "Merger" task, which merges sorted runs together.
*   **`buffer_manager.hpp`**: Manages temporary memory buffers required for merging operations.

## Design Philosophy

1.  **Header-Only**: The library is header-only to simplify integration while utilizing templates for generic type support.
2.  **Logic Separation**: Splitting the large sorting logic into smaller, focused files (`partition.hpp`, `insertion_sort.hpp`) improves readability and makes testing specific components easier.
3.  **Parallel Isolation**: All threading logic is isolated in the `parallel` subdirectory, making it clear which parts of the codebase deal with concurrency.
