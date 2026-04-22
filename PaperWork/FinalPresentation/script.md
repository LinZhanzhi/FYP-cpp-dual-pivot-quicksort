# Presentation Script — DPQS Final Presentation

Per-slide speaker notes with anticipated supervisor questions and prepared answers.
Slides numbered by Beamer order (title page = slide 1).

Legend: **[SAY]** = spoken script, **[Q]** = likely question, **[A]** = prepared answer.

---

## Slide 1 — Title

**[SAY]**
"Good afternoon. My project is a high-performance C++ dual-pivot quicksort, designed as a drop-in replacement for `std::sort`. I'm Lin Zhanzhi, and this is the final presentation of my FYP."

---

## Slide 2 — Agenda

**[SAY]**
"I'll cover seven things: the product I built, the research gap that motivated it, a sequential walkthrough, the parallel path, a VTune root-cause analysis of the scaling curve, engineering and limitations, and then Q&A. About ten minutes of content plus questions."

---

## Slide 3 — Section 1 divider

**[SAY]**
"First, what the product actually is."

---

## Slide 4 — The Product

**[SAY]**
"`dpqs::sort` is a header-only C++17 library. One `#include`, zero dependencies, and it matches `std::sort`'s iterator-plus-comparator contract — so it's a literal drop-in replacement.
On random 32-bit `int` we hit parity with `std::sort`, on reverse-sorted inputs we're about ten times faster with near-linear time, and with work-stealing parallelism we peak at 4.72× on eight threads. The right-hand column shows all five call forms we support — range, iterator pair, pointer+length, custom comparator, and a forced-sequential mode."

**[Q]** Why header-only rather than a compiled library?
**[A]** Templates for generic `T` and `Compare` have to be visible at the call site anyway, so shipping headers keeps the interface clean, lets the compiler inline comparators (which is where a lot of our speed comes from), and avoids ABI versioning.

**[Q]** Can I use it in C++20/23 projects?
**[A]** Yes. C++17 is the minimum; nothing prevents newer standards.

---

## Slide 5 — Section 2 divider

**[SAY]**
"Why build another sort at all."

---

## Slide 6 — Research Gap

**[SAY]**
"There's a fifteen-year discrepancy. Java adopted Yaroslavskiy's dual-pivot quicksort in JDK 7 back in 2011 for all primitive types. Mainstream C++ standard libraries — libstdc++, libc++, MSVC — still ship single-pivot introsort.
The open question is whether Java's win translates. In Java, comparisons are expensive virtual calls, so dual-pivot's 'more comparisons, fewer memory accesses' trade is clearly profitable. In C++, templates inline comparators, so comparisons are almost free. My project engineers a production-grade dual-pivot sort and measures whether the memory-traffic reduction survives the translation."

**[Q]** Isn't there already a C++ dual-pivot implementation?
**[A]** Demo code exists, but nothing generic, STL-compliant, or production-grade. Existing ports are pre-C++11, fixed-type, or research snippets. This is the first header-only, templated, STL-compatible one I've found.

**[Q]** Why not just use `pdqsort` or `ips4o`?
**[A]** Those are excellent but solve different problems — `pdqsort` is pattern-defeating quicksort, still single-pivot; `ips4o` is in-place super-scalar multi-pivot for HPC. This project specifically investigates whether Java's DPQS design ports cleanly to C++, which neither of those answers.

---

## Slide 7 — Methodology

**[SAY]**
"All numbers come from one machine — an Intel i5-12600KF, Alder Lake hybrid, six P-cores with SMT plus four E-cores, so ten physical and sixteen logical threads, 20 MB shared L3, 32 GB DDR5. Compiler is GCC 13.3 with `-O2 -march=native -DNDEBUG -pthread`.
The workload is 7,872 configurations — six algorithms, four data types, eight input patterns, forty-one log-spaced sizes from a thousand up to ten million. For randomness I use ten seeds, three warmups and ten timed iterations per seed, and the representative number is the median of per-seed minima — a hundred samples per cell. Structured patterns run thirty iterations with min-time. This is the harness behind every result today."

**[Q]** Why median of minima rather than median of medians?
**[A]** Minima within a seed absorb scheduling jitter; taking the median across seeds then absorbs one unlucky permutation. It's closer to the "best repeatable" signal than mean-of-means, which skews with outliers.

**[Q]** Is "E-cluster" a typo on the cache row?
**[A]** No — on Alder Lake the four E-cores come in two clusters of two, each cluster sharing 2 MB of L2. P-cores have a private 1.25 MB L2 each. That shared L2 is one of the effects we see on 16 threads.

**[Q]** Why only ten seeds?
**[A]** Error margin plateaued after about six seeds in preliminary runs; ten gives a comfortable confidence margin without doubling the benchmark runtime.

---

## Slide 8 — Section 3 divider

**[SAY]**
"Now the sequential walkthrough."

---

## Slide 9 — Execution Flow, Steps 0–2

**[SAY]**
"This is everything that happens when you call `dpqs::sort`. Step 0 normalises the public overloads into one canonical signature. Step 1 runs cheap guards — null check, range validation, and a single linear scan that short-circuits if the input is already sorted. Step 2 is a compile-time type dispatch using `if constexpr` — one-byte or two-byte integers go to counting sort, floats route through a NaN-and-negative-zero-aware wrapper that still uses the dual-pivot core, and anything parallel-eligible goes to the work-stealing path. Everything else falls through to sequential dual-pivot."

**[Q]** Is `sort_floats` a different algorithm?
**[A]** No — it's dual-pivot with IEEE-754 preprocessing. Three phases: move NaNs to the end, convert negative zeros to positive zeros, run sequential DPQS on the cleaned range, then restore negative zeros to their correct positions before positive zeros.

**[Q]** Is `parallelQuickSort` a different algorithm?
**[A]** No — same dual-pivot partitioning, just with a work-stealing thread pool fanning out the recursion. The name is historical.

**[Q]** Why compile-time dispatch rather than runtime?
**[A]** The type is known at compile time, so `if constexpr` eliminates the wrong branches entirely. Zero runtime cost, no code bloat for unused paths.

---

## Slide 10 — Execution Flow, Steps 3–5

**[SAY]**
"Step 3 computes a depth bound — `2 log₂ n · DELTA` — which is our introsort safety net. If recursion ever goes deeper than this we abort to heapsort, guaranteeing O(n log n) worst case. Step 4 is the recursive core with seven numbered guards; I'll walk them next. Step 5 returns."

---

## Slide 11 — Step 1 Early Termination

**[SAY]**
"The cheapest guard wins the easiest case. Before any sorting work, one linear scan detects already-sorted input. On ten-million fully-sorted integers `std::sort` still partitions and spends real time; we return in microseconds.
Sorted or append-heavy input is more common in production than people think — log ingestion, time-series, prefix tables — so an O(n) free win at the top of the call is worth it."

**[Q]** Isn't `std::is_sorted` then `std::sort` a bit slower because of two passes?
**[A]** Ours is one pass fused with validation, but yes — the cost is one linear scan on every call. It pays off whenever the scan lands early on the first inversion, so even partially sorted inputs amortise it well.

---

## Slide 12 — Step 2 Type Dispatch

**[SAY]**
"For `int8_t` and `int16_t` we dispatch to counting sort — O(n+k) with k bounded to 256 or 65 536. The two plots show the speedup: about 8–10× on `int8_t` and 5–7× on `int16_t`. Floats route through the NaN-aware wrapper; parallel-eligible goes to §4; the default for larger integer and non-primitive types is sequential DPQS."

**[Q]** Why not use counting sort for `int32_t` too?
**[A]** The frequency array would be 4 GB. There's a threshold — `MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE` — that falls back to sequential DPQS when counting sort isn't profitable, e.g. tiny 1-byte arrays.

**[Q]** What about `uint8_t`, `char`?
**[A]** Handled through `std::is_integral_v<T> && sizeof(T) <= 2`, so all one- and two-byte integer types get counting sort.

---

## Slide 13 — Step 4 Seven Guards

**[SAY]**
"This is the execution order for every recursive call. Size below 65 in a non-leftmost partition goes to mixed insertion — 4.1. Below 45 in a leftmost partition goes to plain insertion — 4.2. Above a larger threshold we try run-merging — 4.3. Depth overrun goes to heapsort — 4.4. Otherwise we pick five samples with a 9-comparator network — 4.5 — and branch: if all five samples are strictly ordered we go dual-pivot — 4.6a; if any equality appears we go Dutch National Flag — 4.6b. Finally 4.7 recurses on the two smaller sub-ranges and loops on the largest to keep stack depth bounded."

**[Q]** Why is 4.1 numbered before 4.2 even though 4.2 covers a smaller size?
**[A]** The numbers follow the check order in the source, which matches the Java reference. The non-leftmost case is tried first because a parent pivot at `lo-1` acts as a sentinel for insertion sort's inner loop — fewer branches per iteration.

**[Q]** What's "leftmost"?
**[A]** A partition is leftmost if it contains `a[0]`. When we split, the left child inherits the flag; middle and right children never are — there's a pivot to their left. So leftmost forms a single root-to-leaf spine in the recursion tree.

**[Q]** Why does a sentinel matter?
**[A]** In plain insertion sort the inner loop has two conditions — `j > lo` and `a[j-1] > key`. A parent pivot at `lo-1` is guaranteed ≤ every element in the partition, so the inner loop stops at the pivot naturally and we drop the `j > lo` check. One less branch per iteration, better pipelining. Plain insertion keeps the bounds check and is therefore capped at a smaller size (45 vs 65) to limit its cost.

---

## Slide 14 — Insertion Leaves

**[SAY]**
"This is where every recursion bottoms out. Short sub-arrays are faster with insertion sort than with any partitioning scheme because the branch cost dominates at small sizes. On 10⁷ random `int` we land at about 1.1× over `std::sort`, which is a result in itself — parity with a heavily tuned mature library is hard to earn."

**[Q]** Why 65 and 45 specifically?
**[A]** Tuned empirically. There's a separate tuning report — those two thresholds were swept and the values that minimise total time across the workload matrix were picked. Different CPUs could prefer slightly different values.

---

## Slide 15 — Run Merging

**[SAY]**
"Guard 4.3 is the adaptive shortcut. One forward scan detects runs — ascending, descending-reversed-in-place, or constant. If the first run is shorter than `MIN_FIRST_RUN_SIZE`, the heuristic bails out fast; otherwise it merges the runs in O(n log r), where r is the number of runs. No partitioning, no recursion, no pivots.
On reverse-sorted input r equals 1 after the one in-place reversal, so the whole sort finishes in O(n) — the chart shows about 10× over `std::sort`, essentially linear.
Nearly-sorted is where we actually lose — about 0.74× — because our bail-out triggers but `std::sort`'s insertion-sort tail handles the near-sorted case very efficiently. We win decisively when runs are long and clearly defined; on borderline inputs `std::sort` edges us out."

**[Q]** How do you get to `O(n log r)`?
**[A]** The merge tree has log₂ r levels, and at every level the active runs partition the original array, so their sizes sum to at most n. Therefore total work is bounded by n per level times log r levels.

**[Q]** Aren't there only r−1 merges, not r log r?
**[A]** Right. r−1 merges total in any merge order; the log r comes from the *depth* a single element could be copied — not the count of merge operations.

**[Q]** What does `r` actually count for the "organ-pipe" pattern?
**[A]** Two runs — one ascending half, one descending half. The descending half gets reversed in place by the detector, giving r = 2 effectively.

**[Q]** Is the merging truly balanced?
**[A]** Yes. In `merge_runs`, we split by **element-index midpoint**, not by run count — `(run[lo] + run[hi]) >> 1`. That keeps each tree level doing roughly n/2 + n/2 = n work even when run sizes are uneven.

**[Q]** Why does `sort_sequential` have `size > MIN_TRY_MERGE_SIZE` as a precondition?
**[A]** Below a few thousand elements, the one-pass run scan costs more than the merge could possibly save. The threshold ensures run-merging only runs where it pays.

**[Q]** Does run-merging use extra memory?
**[A]** Yes, O(n) scratch buffer. We ping-pong between the input array and this buffer across merge levels — one final copy if the result ends up in the wrong buffer. Total extra copy cost is bounded by O(n), not O(n log r).

**[Q]** Does it spawn recursion when runs are detected?
**[A]** No — run-merge is a terminal branch. When `try_merge_runs` returns true, the partition is fully sorted and the caller returns.

**[Q]** Why do we pay a penalty on nearly-sorted?
**[A]** `MIN_FIRST_RUN_SIZE` is tuned to protect random-data performance. On *borderline* nearly-sorted input the run detector scans the prefix, decides the first run isn't long enough, and falls through to DPQS. `std::sort`'s insertion-sort tail happens to handle that case cheaply, so it edges us out. Deliberate tradeoff — we gain on the structured cases and pay a small, bounded scan cost on borderline ones.

---

## Slide 16 — Organ-Pipe & Sawtooth

**[SAY]**
"Same mechanism, two more patterns. Organ-pipe has exactly two runs after in-place reversal; sawtooth has a small fixed count. Both charts flatten versus `std::sort`. The message: once a pattern has identifiable runs, our sort collapses to linearithmic work in r rather than n log n."

---

## Slide 17 — Pivots & DNF

**[SAY]**
"Pivot picking. We sample five indices anchored to the two ends — `e1` at roughly 3/8 of the range, `e5` at 5/8, `e3` the midpoint of those, and `e2`, `e4` as midpoints of the halves. All five sit in the central quarter of the array, which gives more balanced pivots than picking from the endpoints. Then we sort those five with a hardcoded 9-comparator network — branch-free `min/max`, nine comparators total.
Then the duplicate oracle: if all four inequalities are strict, we use `a[e1]` and `a[e5]` as dual pivots and partition into three bands. If *any* inequality is non-strict — any equality among the five samples — that signals a duplicate-heavy input, and we switch to Dutch National Flag with `a[e3]` as the single pivot, producing `[<P] [=P] [>P]` and excluding the middle band from recursion."

**[Q]** Why the weird index formula — won't `e1 - 2·step` go negative?
**[A]** It would, which is why the real formula anchors `e1` at `low + step` and `e5` at `high - step`, not at the midpoint minus 2·step. Both are computed from opposite ends; no negatives possible.

**[Q]** Why 9 comparators?
**[A]** Proven optimal — 9 is the minimum number of comparators that sorts 5 elements. Depth of 6, so it pipelines well on modern out-of-order CPUs.

**[Q]** How is the duplicate oracle "free"?
**[A]** The five samples are sorted anyway for pivot selection — we just look at whether any of the four adjacent comparisons ended equal. Zero extra comparisons.

**[Q]** 4.6a spawns three sub-problems; 4.6b spawns two?
**[A]** Yes. 4.6a recurses on `[<P1]`, `[P1..P2]`, `[>P2]` — three regions. 4.6b recurses on `[<P]` and `[>P]` only — the `[=P]` band is final; every element in it equals the pivot and is already in place.

---

## Slide 18 — Duplicates Route to DNF

**[SAY]**
"Three duplicate profiles. Caveat on naming first: the number is the *unique* fraction, not the duplicate fraction — `MANY_DUPLICATES_90` means 90% unique values, which is *light* duplication. `MANY_DUPLICATES_10` is 90% duplicate. That's the code's definition.
Against `std::sort`, we win by 9% to 18% across the whole duplicate range, widest at light duplication, not heavy. Three reasons we don't beat our own random baseline bigger: the input is still randomly permuted so run-merging bails, the 5-sample oracle only detects duplicates probabilistically, and each DNF call absorbs only one pivot-value's band — typically 10–20% of the sub-problem.
We still beat `std::sort` because it's 2-way introsort with no DNF at all. At heavy duplication its pivots accidentally centre; at moderate duplication pivot quality degrades and our DNF keeps a steady edge."

**[Q]** So does "MANY_DUPLICATES_90" have 10% or 90% duplicates?
**[A]** 10%. The naming convention is unique-fraction. Chart captions are labelled by unique fraction to remove the ambiguity.

**[Q]** Could the oracle be stronger?
**[A]** Yes — sample more than 5, require a specific pair (e.g. `e3==e4`) to trigger, or add a dedicated "all-equal" early-exit at the top of each recursion. Every such change costs extra comparisons on random input, so the current 5-sample scheme is a deliberate compromise.

**[Q]** What if duplicates were clustered rather than random?
**[A]** Then run-merging would fire — clusters of equal elements are constant runs, valid run-detector inputs. We'd see the reverse-sorted-style near-linear performance instead.

---

## Slide 19 — Sequential Summary

**[SAY]**
"One table, every pattern. Already-sorted is near-instant from early termination. `int8_t` and `int16_t` random see 6–8× from counting-sort dispatch. Random `int` is 1.1× parity from tuned insertion-sort leaves. Nearly-sorted is 0.74× — a known regression where `try_merge_runs` bails but `std::sort`'s insertion tail wins. Reverse-sorted is 10× near-linear from one in-place reversal plus a single merge pass. Duplicate mixes win 9–18% from DNF. Every number: same hardware, same 10⁷ elements, same methodology."

---

## Slide 20 — Section 4 divider

**[SAY]**
"Now the parallel path."

---

## Slide 21 — Work-Stealing Thread Pool

**[SAY]**
"Each worker thread owns a local deque. When a partition splits in 4.7, the worker pushes the two larger sub-ranges onto its own deque and keeps the smallest in the current loop. Idle workers steal from another worker's deque *tail* — sticky-victim selection reduces contention. `MIN_PARALLEL_SORT_SIZE` is a cutoff that prevents micro-tasks with worse-than-overhead granularity.
Start-up is the subtle phase. At launch there's one task — the root — on worker zero's deque, and N−1 idle workers. Each idle worker polls a global done-flag; while it's unset, the worker spins on a steal loop — pick a random victim deque, try to pop its tail, retry if empty. The tree fans out one level per successful steal, so after log₂ N steals everyone has work. The flag flips only when the root task completes, at which point every spinner exits.
See references 7 and 8 for the theoretical basis. Scaling on the right: near-linear through four threads, 4.72× peak at eight, then regression at sixteen."

**[Q]** Why push two and keep the third instead of pushing all three?
**[A]** Depth-balancing. Keeping the smallest on the loop bounds stack depth to O(log n); pushing it would let one chain of small tasks monopolise a worker. Pushing the two larger makes them available for stealing, maximising parallelism.

**[Q]** Why steal from the tail instead of the head?
**[A]** Owner pushes and pops at the head — classic deque discipline. Thieves take from the tail — the *oldest* task, which is the largest partition on average since we push smaller ones later. Minimises cache-line contention with the owner and biases thieves toward big sub-problems.

**[Q]** What's "sticky-victim"?
**[A]** Once a thief successfully steals from victim V, it tries V again next time before picking a new random victim. Reduces the expected number of failed steal attempts.

**[Q]** Why is there mutex contention at all?
**[A]** The current deque is guarded by a per-worker `std::mutex` rather than a lock-free Chase–Lev deque. Lock-free is future work.

**[Q]** What happens to the idle workers when the tree is nearly drained?
**[A]** They cycle through random victims finding empty deques. Slow-Pause on the VTune summary is that drain phase — 0.8 percentage points at 16T.

**[Q]** Why 4.72× peak instead of 8×?
**[A]** VTune slide covers it — four stacked hardware ceilings: L3 pressure, hybrid-core dilution onto E-cores, SMT cache sharing on the 16T case, and residual mutex contention.

---

## Slide 22 — Scaling Chart

**[SAY]**
"The shape of the curve. One to four threads is near-linear — 1×, 1.30×, 3.21×. Eight threads peaks at 4.72×. Sixteen threads *regresses* to 2.10× — 2.25× *slower* than eight. That regression is not noise. Next slide shows the four stacked ceilings that cause it."

---

## Slide 23 — Section 5 divider

**[SAY]**
"VTune tells us exactly why."

---

## Slide 24 — VTune Root-Cause

**[SAY]**
"I ran Intel VTune `uarch-exploration` on both 8T and 16T and diffed the counters. Four effects compound.
One — L3-Bound jumps from 14.5% to 25.1%. The working set is 40 MB on 10⁷ `int`; doubling threads doubles simultaneous active footprint, which spills past the 20 MB L3.
Two — E-core clockticks go up 2.7×. The extra eight logical threads land on four slower E-cores whose CPI is 20% worse than P-cores, so every instruction costs more real time.
Three — SMT sibling contention: machine clears quadruple from 1.8% to 7.5%, L2-bound and L1D-bound creep up. Hyperthreading means two logical threads fight for one L1D and one L2 — a classic false-sharing and memory-ordering hazard.
Four — slow-pause spin-wait picks up 0.8 points, residual mutex contention in the work-stealing deques.
The decisive line is DRAM-Bound at 0.6% and 0.5%: memory bandwidth is *not* the bottleneck. What's saturated is cache latency, core heterogeneity, SMT sharing, and thread-pool locks — in that order.
The one-sentence summary: the silicon is the ceiling, not our algorithm."

**[Q]** Why does P-core CPI increase by 36% at 16T when the P-cores are doing the same work?
**[A]** Because SMT siblings now share each P-core's L1D and L2. Cache misses that were zero at 8T (one thread per core, full cache) become non-zero at 16T (two threads per core, half cache each). Every missed load stalls the core longer.

**[Q]** Why don't machine clears matter at 8T?
**[A]** At 8T we pin to the 8 SMT-free threads — one per P-core plus all E-cores. No two threads share a core's memory ordering. At 16T every P-core has both siblings active, so the cross-sibling memory-order nukes spike.

**[Q]** Is there anything the software can do?
**[A]** Three things, in order of impact: (a) cache-aware partitioning — stop recursion before working sets blow past 20 MB, process blocks, go top-down again; (b) lock-free Chase–Lev deque to remove the 0.8% slow-pause; (c) explicit core affinity to keep work off E-cores for latency-sensitive workloads. (a) is the high-leverage future work.

**[Q]** Why is this not our algorithm's fault?
**[A]** At 1T through 4T we achieve ~80% efficiency, close to the theoretical work-stealing bound. The 16T regression is hardware — DRAM-bandwidth-free, so not our memory-access pattern; E-core drag; SMT contention. A different algorithm with the same cache footprint on the same silicon would see similar effects.

---

## Slide 25 — Section 6 divider

**[SAY]**
"Engineering and honest limitations."

---

## Slide 26 — Engineering

**[SAY]**
"How we used C++ itself. Header-only, zero dependencies — one `#include` and any C++17 build works. Templates are fully generic on `T`, iterator category, and `Compare`. An iterator adapter gives contiguous iterators a zero-cost raw-pointer fast path; non-contiguous iterators get a temporary buffer. `if constexpr` makes type dispatch a compile-time decision with zero runtime cost. And `__builtin_prefetch` wraps the partition hot loop to hide L1 misses.
Headers are split into `dpqs/sequential_sorters.hpp`, `run_merger.hpp`, `counting_sort.hpp`, `thread_pool.hpp`, `utils.hpp` under a single `dual_pivot_quicksort.hpp` façade."

**[Q]** Why prefetch only the partition hot loop?
**[A]** Profiling showed that's where the memory misses cluster — the backward scan over the partition has predictable stride. Merge-phase access is already sequential with hardware prefetch, so explicit prefetch there didn't help.

**[Q]** Does the iterator adapter allocate?
**[A]** Only for non-contiguous iterators, where correctness requires materialising into contiguous storage anyway. Contiguous iterators — `std::vector`, `std::array`, raw pointers — skip straight to the pointer path, zero overhead.

---

## Slide 27 — Limitations, Reflection & Conclusion

**[SAY]**
"Honest limitations. Like `std::sort`, we're not stable — equal elements may be reordered; use `std::stable_sort` if that matters. Move-only types aren't supported by the run-merger; it needs a copy path. And there's the nearly-sorted regression at some sizes — threshold tuning WIP.
Reflection: the biggest engineering lesson wasn't the algorithm — most of the algorithm is in the Java reference. The biggest lesson was that building a 7,800-configuration benchmark harness with multi-seed methodology is what makes every tuning decision defensible. You can't tune what you can't measure repeatably.
Delivered: header-only STL-compatible DPQS for C++17, adaptive with counting sort, run-merger, DNF, and introsort fallback, work-stealing parallelism at 4.72× peak, and VTune-validated root-cause for every anomaly in the scaling curve."

**[Q]** What would you do differently?
**[A]** Start with the benchmark harness. I built it after the first implementation, which meant re-running hundreds of configurations once I trusted the numbers. Measure first, tune second.

---

## Slide 28 — Personal Reflection

**[SAY]**
"Before I take questions, one slide on what this project actually taught me.

First, tuning a program for a *real machine* is as much an exercise in silicon literacy as in algorithms. You can't separate the algorithm from the caches, the SMT sharing, and the hybrid-core scheduler that actually runs it. Every optimisation is a trade-off — the 10× win on reverse-sorted costs us 0.74× on borderline nearly-sorted, and that's a bargain I'm willing to defend with data.

Second, the more specific the target scenario, the easier tuning becomes — and the easier trade-offs are to accept. Universal 'best' sorts don't exist; you pick the shape of the curve you want.

And third, honest disclosure: the generality of this implementation isn't fully proven yet. Everything is tuned on one CPU, one compiler, one workload matrix. Cross-platform, cross-compiler, cross-architecture validation is the real next milestone. So there's still a long way to go — but this repository gave me my first taste of real performance engineering, and the 'measure first, tune second' discipline is something I'll carry forward into every future project."

**[Q]** If generality isn't proven, why claim "drop-in replacement for `std::sort`"?
**[A]** Correctness is proven — the sort returns a correctly sorted range for every type and pattern in the test matrix. What's not proven is that the *performance* wins generalise to other CPUs. On a different machine the thresholds might need re-tuning, though the shape of the adaptive strategy should transfer.

**[Q]** What's the single most important lesson?
**[A]** That measurement infrastructure is the real engineering. Without a defensible harness you can't tell an optimisation from a lucky run, and you end up tuning noise.

---

## Slide 29 — Section 7 divider

**[SAY]**
"Happy to take questions."

---

## Slide 30 — Q & A

**[SAY]**
"Questions?"

---

## Slides 31+ — References and Backups

Backup slides available if the questioner wants detail on:
- `sort5_network` comparator ordering
- Multi-seed protocol pseudo-code
- Full hardware CPU-Z spec

---

# Master Anticipated-Question Bank

Grouped by theme.

## Methodology / benchmarking

**[Q]** How do you know this isn't noise?
**[A]** 100 samples per cell (10 seeds × 10 timed iters + 3 warmups). Median-of-minima robustly rejects both lucky-seed and unlucky-jitter outliers.

**[Q]** Does `-march=native` give unfair advantage?
**[A]** Used for both `std::sort` and `dpqs` — same flags both sides. Comparison is fair; absolute numbers drop on `-O2` without `-march=native`, but *relative* ordering stays.

## Algorithm

**[Q]** What happens at the base of recursion on tiny arrays?
**[A]** Insertion sort via 4.1 or 4.2. Below those thresholds there's no further recursive call.

**[Q]** Is DPQS stable?
**[A]** No. Like `std::sort`. Three-way splits don't preserve equal-element order.

**[Q]** Worst-case complexity?
**[A]** O(n log n) guaranteed by the introsort safety net at guard 4.4 — depth bound `2 log₂ n · DELTA` triggers heapsort fallback before quadratic behaviour can happen.

## Parallelism

**[Q]** Why not OpenMP or TBB?
**[A]** Dependency-free mandate — header-only, no external libraries. A custom work-stealing pool is ~400 lines and lets us control cutoff, deque discipline, and stealing policy exactly.

**[Q]** NUMA support?
**[A]** None currently — the test machine is single-socket. Multi-socket NUMA would require per-node thread pools and is future work.

**[Q]** Is the current deque lock-free?
**[A]** No — per-deque `std::mutex`. Chase–Lev lock-free deque is listed as future work; VTune shows residual cost is only 0.8% of 16T runtime.

## C++ specifics

**[Q]** Why C++17 and not C++20?
**[A]** Widest compiler compatibility. Nothing needs concepts, ranges, or C++20 contracts.

**[Q]** How do you handle custom comparators?
**[A]** Passed as template parameter; inlined by the compiler. Internal partition loop is a fully specialised instantiation, zero indirection overhead.

**[Q]** Does it compile on MSVC/libc++?
**[A]** Tested on GCC 13.3 MinGW on Windows. No MSVC-specific intrinsics; `__builtin_prefetch` is wrapped in `DPQS_PREFETCH_READ` with a fallback.

## Future work

**[Q]** What's next?
**[A]** Three things: (a) cache-aware top-down partitioning to avoid the L3 cliff, (b) lock-free Chase–Lev deque for the 16T case, (c) SIMD partitioning à la BlockQuicksort for the core hot loop. Each independently scoped.