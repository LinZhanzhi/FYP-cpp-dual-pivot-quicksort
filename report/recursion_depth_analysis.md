# Performance Optimization: Recursion Depth Strategy

## 1. Issue Identification
During the constant tuning phase, we revisited the **Recursion Depth Limit** strategy.
The previous implementation used a static hard limit:
```cpp
constexpr int MAX_RECURSION_DEPTH = 64;
// Usage: if ((bits += 3) > 64) -> Switch to HeapSort
```
This resulted in an effective depth limit of **~21 levels** ($64 / 3$).

### The Problem
For an array of $10^7$ elements (10 MB), the theoretical ideal recursion depth is $\log_2(10^7) \approx 23.3$.
-   **Old Limit (21)** < **Required Depth (23)**.
-   **Consequence**: The algorithm was prematurely detecting "pathological behavior" on perfectly random data and switching to the slower **HeapSort** fallback. This degraded performance by interrupting the efficient Quicksort loops.

## 2. Solution: Aligning with Java's Design
We updated the constant to match the reference Java implementation (Dual-Pivot Quicksort by Vladimir Yaroslavskiy):

```cpp
// New Implementation in include/dpqs/constants.hpp
constexpr int MAX_RECURSION_DEPTH = 64 * DELTA; // 64 * 3 = 192
```

-   **New Limit**: 192.
-   **Effective Levels**: $192 / 3 = 64$ levels.
-   **Coverage**: $2^{64}$ elements. This covers all physically possible array sizes.

## 3. Performance Verification (10M Integers)
We ran a comparative benchmark on 10,000,000 random integers to verify the impact.

| Metric | Old Limit (~21 Levels) | New Limit (~64 Levels) |
| :--- | :--- | :--- |
| **Logic** | Premature HeapSort switch | Proper Quicksort execution |
| **Average Time** | **573 ms** | **524 ms** |
| **Improvement** | - | **~8.5% Speedup** |

## 4. Conclusion
The recursion depth cap was too aggressive. By relaxing it to **64 effective levels**, we eliminated unnecessary HeapSort fallbacks on large arrays. The safety mechanism now correctly triggers only for genuine $O(N^2)$ pathological cases (recursion bombs), rather than punishing large random datasets.
