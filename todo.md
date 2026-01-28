
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

## Upcoming Tanning Experiments
- [x] **Cleanup**: Unify duplicate constants `COUNTING_SORT_THRESHOLD_BYTE` / `MIN_BYTE_COUNTING_SORT_SIZE` and `COUNTING_SORT_THRESHOLD_SHORT` / `MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE`.
- [x] Tune `MIN_FIRST_RUN_SIZE`: **Verified 16 is optimal.**
    - Benchmarks on "Runs of length 20" showed Merging (39ms) was faster/equal to Quicksort (39-43ms).
    - Benchmarks on "Runs of length 32" showed Merging (37ms) was faster than Quicksort (39ms).
    - Random data performance was invariant (110ms) across values 4-64.
    - Conclusion: 16 safely enables optimization for runs >16 without penalizing random data.
- [x] Tune `MIN_FIRST_RUNS_FACTOR`: **Updated to 6** (was 7).
    - Benchmarks on 10M integers showed the performance crossover point is between run lengths of 32 and 64.
    - At Run Length 64: Merge (75ms) > Quicksort (77ms).
    - At Run Length 32: Quicksort (78ms) > Merge (82ms).
    - Random Data (Run Length 2): Unaffected (remains on Quicksort path).
    - Updating from 7 (limit 128) to 6 (limit 64) captures the optimization for run lengths 64-128.
- [ ] Tune `MIN_PARALLEL_MERGE_PARTS_SIZE` (currently 4096). Controls granularity of parallel merging.

