
report pivot selection

report runtime comparison on 3 machine. desktop. asus laptop. apple laptop.
report operations of the algorithm. making a new counting implementation that sacrifice performance. to the count is portable.

## Untuned Constants Experiments
- [x] Tune `MIN_TRY_MERGE_SIZE`: **Updated to 286** (Java's value).
    - Benchmarks show **~13.5% speedup** (126ms -> 109ms) on large random data by reducing recursive overhead.
    - Tested higher values (up to 1000). While 600 was slightly faster for random data (105ms), it caused a **20% regression** on small sorted arrays (size 500) by skipping the O(N) run merger optimization.
    - Conclusion: 286 is the optimal safe balance.
- [x] Cleanup dead/legacy constants: Removed `QUICKSORT_THRESHOLD` (286), `MAX_RUN_COUNT` (67), `MAX_RUN_LENGTH` (33) as they were unused in the C++ implementation.
- [x] Tune `MAX_MIXED_INSERTION_SORT_SIZE`: **Updated to 60** (Matches `MAX_INSERTION_SORT_SIZE`).
    - Benchmarks (20 runs x 10M ints) showed a slight improvement (110ms -> 109ms).
    - Unifies the insertion sort entry threshold across the algorithm.
    - Higher values (65+) caused regression (113ms).
- [x] Tune `MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE`: **Retained 1750**.
    - Experiments confirmed that for `short` arrays of size 2000, Counting Sort (185ms) is ~32% faster than Quicksort (245ms).
    - Threshold 1750 correctly enables this optimization while protecting very small arrays from the 256KB table initialization cost.
    - Also validated `MIN_BYTE_COUNTING_SORT_SIZE` (64) as optimal.

