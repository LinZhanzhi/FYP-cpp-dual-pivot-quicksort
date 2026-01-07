<img src="https://r2cdn.perplexity.ai/pplx-full-logo-primary-dark%402x.png" style="height:64px;margin-right:32px"/>

# read this and output its intro sort algorithm . output in pseudo code.

Intrsort uses quicksort-style partitioning with a depth limit; when the depth limit is exceeded, it switches to heapsort on the current subarray.[^1]

Here is the introsort algorithm from the paper in pseudocode form:

```pseudo
procedure introsort(A, f, b)
    // A[f..b-1] is the range to sort
    depth_limit ← floor(log2(b - f))        // Floor lg(b − f)
    introsort_loop(A, f, b, depth_limit)
    insertion_sort(A, f, b)
end procedure

procedure introsort_loop(A, f, b, depth_limit)
    // Sort A[f..b-1] except for small segments (handled later by insertion_sort)
    while (b - f) > size_threshold do
        if depth_limit = 0 then
            heapsort(A, f, b)
            return
        end if

        depth_limit ← depth_limit - 1

        // Choose pivot as median of first, middle, last
        pivot_index ← median_of(A, f, (f + b) div 2, b - 1)

        // Partition around pivot; p is first index in right partition
        p ← partition(A, f, b, pivot_index)

        // Recurse on right side, iterate on left (as in the STL version)
        introsort_loop(A, p, b, depth_limit)
        b ← p
    end while
end procedure
```

Notes:

- `size_threshold` is a small constant; subarrays of size ≤ `size_threshold` are left for a final `insertion_sort` pass.[^1]
- `heapsort`, `insertion_sort`, `median_of`, and `partition` are standard implementations with worst-case $O(n \log n)$ heapsort and in-place partitioning as assumed in the paper.[^1]

<div align="center">⁂</div>

[^1]: Introspective-Sorting-and-Selection-Algorithms-David-R.pdf

