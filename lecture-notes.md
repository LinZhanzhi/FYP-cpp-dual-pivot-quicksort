# Lecture Notes
Date: December 9, 2025

## Dual-Pivot Quicksort Algorithm

### Core Concept
Standard Quicksort uses **one pivot** to split the array into two parts (elements < pivot and elements > pivot).
**Dual-Pivot Quicksort** uses **two pivots** ($P1$ and $P2$) to split the array into **three parts**:
1.  **Part I:** Elements $< P1$
2.  **Part II:** Elements between $P1$ and $P2$ ($P1 \le x \le P2$)
3.  **Part III:** Elements $> P2$

This approach is generally faster for many datasets (used in Java's `Arrays.sort` for primitives) because it reduces the recursion depth.

### Variables and Pointers (Figure 1)
*   **Pivots:**
    *   $P1$: Initially `a[left]`
    *   $P2$: Initially `a[right]`
    *   *Constraint:* We must ensure $P1 < P2$ (swap them if necessary).
*   **Indices:**
    *   `left`, `right`: Boundaries of the current subarray.
    *   `L`: Pointer to the first element of Part II (just after Part I).
    *   `K`: The "cursor" or iterator. It points to the next element to be examined.
    *   `G`: Pointer to the start of Part III (just after the unexamined region).

### The Partitioning Process
The array is divided into 4 regions during the sort:
1.  `left+1` to `L-1`: **Part I** ($< P1$)
2.  `L` to `K-1`: **Part II** ($P1 \le x \le P2$)
3.  `K` to `G`: **Part IV** (Unexamined / Unknown elements)
4.  `G+1` to `right-1`: **Part III** ($> P2$)

### Algorithm Steps
1.  **Base Case:** If array length $< 17$, use **Insertion Sort** (faster for small arrays due to lower overhead).
2.  **Init:** Pick $P1$ and $P2$. Ensure $P1 < P2$.
3.  **Iterate:** While $K \le G$:
    *   Compare `a[K]` with $P1$ and $P2$.
    *   If `a[K] < P1`: Move to Part I (swap with `a[L]`, increment `L` and `K`).
    *   If `a[K] > P2`: Move to Part III (swap with `a[G]`, decrement `G`). *Note: Check the new swapped element again.*
    *   Otherwise ($P1 \le a[K] \le P2$): Leave in Part II (increment `K`).
4.  **Finalize:**
    *   Swap $P1$ into its final position (swap `a[left]` with `a[L-1]`).
    *   Swap $P2$ into its final position (swap `a[right]` with `a[G+1]`).
5.  **Recurse:** Recursively sort Part I, Part II, and Part III.
