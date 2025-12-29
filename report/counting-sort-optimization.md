# Feature Report: Optimized Counting Sort (Dense vs. Sparse)

## 1. Overview
For small integral types (1-byte `char/uint8_t` and 2-byte `short/uint16_t`), Comparison Sorts ($O(N \log N)$) are inefficient compared to Counting Sort ($O(N + K)$), where $K$ is the range of values.

Our library automatically dispatches these types to a specialized Counting Sort implementation. However, we go a step further by optimizing the memory access pattern based on the **density** of the data.

## 2. The Problem
Counting Sort involves two passes:
1.  **Count Pass**: Iterate over input, increment frequency array.
2.  **Write Pass**: Iterate over frequency array, write values back to input.

In the Write Pass, if the input array is small but the range $K$ is large (e.g., sorting 50 `short`s where $K=65536$), the algorithm wastes time iterating over thousands of empty zeros in the frequency array. This is the "Sparse" case.

Conversely, if the input array is large (e.g., 1,000,000 elements), the frequency array is likely full. This is the "Dense" case.

## 3. Optimization Strategy
The `counting_sort` function (in `include/dpqs/counting_sort.hpp`) dynamically selects the iteration strategy for the Write Pass based on the input size.

### Case 1: Dense Array (Size > Range / 2)
- **Assumption**: Most buckets in the frequency array are non-zero.
- **Strategy**: Iterate **backwards** from the maximum value to the minimum.
- **Action**: Fill the input array from the **end** to the **start**.
- **Benefit**: This matches the cache prefetching patterns often optimized for sequential access, and avoids checking for zeros in a sparse array (since it's not sparse).

### Case 2: Sparse Array (Size <= Range / 2)
- **Assumption**: Many buckets are zero.
- **Strategy**: Iterate **forwards** from minimum to maximum.
- **Action**: Fill the input array from the **start** to the **end**.
- **Benefit**: While it still has to skip zeros, the forward iteration is generally friendlier to the CPU's hardware prefetcher when skipping through memory.

## 4. Implementation Details
- **File**: `include/dpqs/counting_sort.hpp`
- **Threshold**: The switch happens when `size > NUM_VALUES / 2`.
- **Support**:
    - 1-byte types: `NUM_VALUES = 256`
    - 2-byte types: `NUM_VALUES = 65536`

This micro-optimization ensures that Counting Sort remains efficient even for very small arrays where the $O(K)$ initialization cost usually dominates.
