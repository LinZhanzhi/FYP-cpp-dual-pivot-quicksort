# Feature Report: Introsort Strategy (Worst-Case Guarantee)

## 1. Overview
While Dual-Pivot Quicksort is generally faster than standard Quicksort, it still shares the same theoretical worst-case time complexity of $O(N^2)$. This occurs when the pivot selection strategy consistently picks poor pivots (e.g., min/max elements), causing the recursion depth to become linear rather than logarithmic.

To prevent this and guarantee $O(N \log N)$ performance in all cases, our library implements an **Introsort (Introspective Sort)** strategy.

## 2. Mechanism
The `sort_sequential` function tracks the recursion depth using a `bits` parameter.

### Recursion Depth Limit
The maximum allowed recursion depth is controlled by `MAX_RECURSION_DEPTH` (64) and a decrement constant `DELTA` (3).
```cpp
constexpr std::ptrdiff_t MAX_RECURSION_DEPTH = 64;
constexpr std::ptrdiff_t DELTA = 3;
```
This limits the effective maximum depth to approximately $64 / 3 \approx 21$ levels on the stack before fallback.

### Fallback Trigger
In each recursive call, the depth tracker `bits` is incremented. If it exceeds the limit:
```cpp
if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
    heap_sort(a, low, high, comp);
    return;
}
```

### Heap Sort Fallback
When the limit is reached, the algorithm switches to **Heap Sort** for the current subarray.
- **Heap Sort** has a guaranteed worst-case time complexity of $O(N \log N)$.
- It does not rely on pivot selection, making it immune to "killer sequences" that degrade Quicksort.

## 3. Small Array Optimization
To allow the recursion to bottom out efficiently, the algorithm switches to **Insertion Sort** for small subarrays.
- **Thresholds**:
    - **32 elements** (`INSERTION_SORT_THRESHOLD`) for the leftmost subarray.
    - **48 elements** (`MIXED_INSERTION_SORT_THRESHOLD`) for internal subarrays.
- **Rationale**: For tiny arrays, the overhead of recursive calls and partitioning outweighs the $O(N^2)$ cost of insertion sort. Mixed Insertion Sort handles subarrays that may already be partially sorted relative to pivots, providing further optimization.

## 4. Benefits
- **Security**: Prevents "algorithmic complexity attacks" where an attacker feeds specific data to cause a Denial of Service (DoS) by triggering $O(N^2)$ behavior.
- **Stability**: Ensures predictable performance even on pathological datasets.
- **Performance**: In normal cases, the limit is never reached, so the average-case performance remains that of Dual-Pivot Quicksort.

## 4. Implementation Details
- **File**: `include/dpqs/sequential_sorters.hpp`
- **Helper**: `include/dpqs/heap_sort.hpp` implements a standard binary heap sort.
