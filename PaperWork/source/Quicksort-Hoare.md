<img src="https://r2cdn.perplexity.ai/pplx-full-logo-primary-dark%402x.png" style="height:64px;margin-right:32px"/>

# i find this scanned pdf of the paper. and i am reading it. and i don't understand how the paper cope with the situation when the selected bound value is minimum of maximum value of the array . can you read and explain to em

In the awkward case where the bound is the minimum or maximum key, Hoare adds a “fix‑up” step that guarantees at least one element is placed in its final position each time partition is applied.[^1]

## Where the problem arises

Hoare explicitly notes the issue you are asking about in the “Partition” section:[^1]

- If the bound is the smallest, largest, or all keys are equal, the two pointers can cross **outside** the segment being partitioned, so the “dividing line” would lie outside the segment.
- Then the whole segment would have to be partitioned again, and this can lead to an infinite cycle if nothing changes between attempts.[^1]

So your scenario “bound is minimum or maximum value of the array” is exactly this “awkward situation.”[^1]

## The paper’s fix for min/max bound

Immediately after describing that danger, Hoare gives a rule that forces progress:[^1]

> “This may be prevented by the use of a method which ensures that at least one item is placed in its correct position as a result of each application of the partitioning process.”[^1]

The method:

- Suppose the bound value came from some particular item in the segment (call that item B).
- After partition finishes, B ends up either in the lower segment or in the upper segment.[^1]

Now:

- **If B ends up in the lower segment**:
    - Then B’s key is **≥ all keys in that lower segment**, because everything in the lower segment is ≤ bound, and the bound is B’s key.[^1]
    - So B is **the maximum** of that lower segment.
    - Therefore B is already in its **final sorted position** relative to that lower segment.
    - Hoare then says: exchange B with the item at the **highest address of the lower segment** and **shrink** that lower segment size by one (so the segment no longer includes B).[^1]
- **If B ends up in the upper segment**:
    - Symmetrically, B’s key is **≤ all keys in that upper segment**, so B is the **minimum** of that upper segment.
    - Again, B is in its **final position** relative to that upper segment.
    - Exchange B with the **lowest-addressed item of the upper segment**, then shrink that upper segment size by one.[^1]

In either case, the item that gave the bound is removed from further recursive partitioning, because it is in a position that is correct within the global sorted order once the two resulting segments are sorted.[^1]

## Why this solves the min/max problem

Consider the extreme cases you asked about:

- **Bound is the minimum of the whole array**:
    - After partition, all items are ≥ bound.
    - B (the minimum) must end up in the **lower segment**, and all other items there have keys ≥ bound.
    - So B is the smallest of the whole array, hence its final sorted position is at the **very beginning**.
    - By Hoare’s rule, B is fixed in that position and removed from the remaining segment, so the total size of segments decreases by at least 1.[^1]
- **Bound is the maximum of the whole array**:
    - Symmetrically, B ends up in the **upper segment** and is the largest of all keys.
    - It is fixed at the **end** of the array and removed from further work.[^1]

Because at least one element (the bound’s item) is fixed and removed each time, the sum of sizes of the two subsegments after partition is **always one less** than the original segment size. Hoare states this explicitly:[^1]

> “Thus the sum of the numbers of items in the two segments, resulting from the partitioning process, is always one less than the number of items in the original segment, so that it is certain that the stage will be reached, by repeated partitioning, when each segment will contain one or no items.”[^1]

So even when the chosen bound is the minimum, maximum, or all keys are equal, the algorithm still strictly reduces the problem size, and no infinite loop occurs.[^1]

If you like, next step can be to walk through a concrete small example (e.g. array  with bound 1 or 4) using Hoare’s pointer and segment-shrinking rules.[^2][^3][^4][^5]

<div align="center">⁂</div>

[^1]: Quicksort.pdf

[^2]: https://academic.oup.com/comjnl/article-abstract/5/1/10/395338

[^3]: https://en.wikipedia.org/wiki/Quicksort

[^4]: https://academic.oup.com/comjnl/pages/65th-anniversary-perspective-ym2

[^5]: https://www.youtube.com/watch?v=brrSizH7q1o

<img src="https://r2cdn.perplexity.ai/pplx-full-logo-primary-dark%402x.png" style="height:64px;margin-right:32px"/>

# also in this paper . i am not understanding the idea of a nest and that information are stored in pointer and so on. and it said it need to know the maximum number of locations in the nest. and it is sufficient to always postpone the larger segment . can you explain

Hoare’s **nest** is exactly what a modern programmer would call an explicit stack used to remember subarrays that still need sorting.[^1]

## What the nest is

- The nest is a **contiguous block of memory** plus a pointer into it.[^1]
- The pointer always marks the **next free slot** in that block (lowest-addressed location whose contents may be overwritten).
- To **push** information: write it at the pointer, then increment the pointer one slot up.[^1]
- To **pop** information: decrement the pointer, then read what is stored at that position.[^1]

So the nest is just a LIFO stack implemented as an array + index; “information is read out in the reverse order to that in which it is written.”[^1]

## What gets stored in the nest

When partitioning a segment $[L, R]$ into two segments, Hoare wants to sort one segment now and **postpone** the other.[^1]

- For each postponed segment, the algorithm stores in the nest:
    - The address of the **first item** in the segment.
    - The address of the **last item** in the segment.
- Later, when the current segment is finished (size 0 or 1, or sorted by a small routine), a postponed segment is taken from the nest by popping its $[L, R]$ pair and processing it next.[^1]

So the nest content is just “to-do list” entries: (start, end) for subarrays.

## Why it behaves like a stack

- Every time the algorithm postpones a segment, it **pushes** its details onto the nest.[^1]
- When there is nothing more to do in the current segment, it **pops** the most recently postponed segment and continues with that one.[^1]
- Because postponed segments are resumed in reverse order, and reading them also frees their memory slots, the data structure has stack behavior and automatically reuses nest space.[^1]

This is equivalent to a recursive quicksort using the call stack, just written iteratively with an explicit stack.

## Why “maximum number of locations” matters

The nest is a finite array, so Hoare wants an upper bound on how many postponed segments can exist **at the same time**.[^1]

- Each postponed segment uses a fixed number of locations (e.g. 2 words: L and R).
- If the algorithm could generate $O(N)$ postponed segments at once, the nest would need $O(N)$ space, which is undesirable.
- He proves it is enough to size the nest so that the number of postponed segments never exceeds $\log_2 N$.[^1]

So “maximum number of locations used by the nest” = worst-case stack depth × words per segment.

## Why postponing the larger segment gives log₂ N bound

Key idea: always **continue with the smaller segment, postpone the larger one**.[^1]

- After partition of a segment of size $n$, you get segments of sizes $a$ and $b$ with $a + b \leq n - 1$ (because the pivot element is fixed).[^1]
- The rule is:
    - Process the **smaller** segment immediately (no extra stack depth).
    - **Push** only the **larger** segment on the nest.

Intuition for the $\log_2 N$ bound:

- Each time you push a segment on the nest, that segment is **at least half as large** as the segment you just split (because it is the larger of the two).
- If you track the sizes of segments on the stack at any moment:
    - The first postponed segment is of size ≤ $N$.
    - The next postponed segment (created while processing something smaller) is ≤ $N/2$.
    - The next is ≤ $N/4$, etc.
- This descending-by-at-least-half sequence can have length at most $\log_2 N$ before reaching size 1.

Therefore, the number of simultaneously postponed segments is at most $\lfloor \log_2 N \rfloor$, so a nest sized for about $\log_2 N$ entries is sufficient.[^1]

If you want, next step can be mapping this directly to a C/Java-style iterative quicksort using an array as the “nest” and seeing the stack-depth bound in code.
<span style="display:none">[^2]</span>

<div align="center">⁂</div>

[^1]: Quicksort.pdf

[^2]: https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/30537770/3e7ab5e0-3e7d-440c-9eba-5e49c4716f6a/Quicksort.pdf

<img src="https://r2cdn.perplexity.ai/pplx-full-logo-primary-dark%402x.png" style="height:64px;margin-right:32px"/>

# read the paper and show me how it does the calculation about the number of comparisons and swaps.

Hoare does two levels of calculation: first the **expected exchanges in one partition**, then the **expected comparisons over the whole sort**.[^1]

## Expected exchanges in one partition

Setup in the paper:[^1]

- Segment size: $N$ items.
- Assume the bound (pivot value) is the **$r$-th smallest** key among these $N$ keys.
- After partition is finished and the final exchange is done:
    - The item with the bound’s key ends up in position $r$.
    - The lower segment contains the $r - 1$ items with keys less than the bound.
    - The upper segment contains the $N - r$ items with keys greater than the bound.[^1]

He then reasons about **how many items moved out of the lower segment** during partition:

1. Before partition, the $r - 1$ positions that will become the lower segment contained some random $r - 1$ items from the whole segment.[^1]
2. Each of those $r - 1$ initial occupants has probability

$$
\Pr(\text{key} > \text{bound}) = \frac{N - r}{N}
$$

because there are $N - r$ keys greater than the bound out of $N$ total.[^1]
3. Each such “too large for lower segment” item will be **exchanged out** during the partition process.[^1]

So, **conditional on pivot being the $r$-th order statistic**, the expected number of exchanges is

$$
E[\text{exchanges} \mid r] = (r - 1) \cdot \frac{N - r}{N} = \frac{(r - 1)(N - r)}{N}.[file:43]
$$

Finally, Hoare wants the **unconditional expectation** over all possible $r$:

- Under the assumption that the bound item is chosen uniformly at random from the $N$ items, $r = 1, \dots, N$ are all equally likely with probability $1/N$.[^1]
- So he sums over $r$ and divides by $N$, then adds **1** for the final exchange that puts the pivot item into position $r$:[^1]

$$
E[\text{exchanges}] = \frac{1}{N} \sum_{r=1}^{N} \frac{(r - 1)(N - r)}{N} + 1.[file:43]
$$

He simplifies that sum algebraically and gets

$$
E[\text{exchanges}] = \frac{N + 1}{6} + \frac{5}{6N}.[file:43]
$$

He then notes you can subtract $1/N$ if you skip the final exchange when the pivot item is already in its correct place, but concludes that testing for this usually is not worth the overhead.[^1]

## Cost of one partition: comparisons vs exchanges

Hoare models the **time to partition $N$ items** as

$$
aN + b + \frac{c}{N},[file:43]
$$

where:

- $a$ corresponds to the cost per key comparison (inner loop).
- $b$ and $c/N$ capture constant and small overhead terms.[^1]

He states, without full derivation, that **the number of key comparisons in one partition is of the form** $N + k$, where $k \in \{-2, -1, 0, 1\}$ depending on implementation details like sentinel use and exact loop termination tests. That justifies taking the main term as linear in $N$.[^1]

## Recurrence for expected sorting time

Let $T_N$ be the expected time to sort a segment of size $N$, and suppose segments smaller than $M$ are handled by some other (given) method.[^1]

Condition on the pivot being the $r$-th smallest value:

- Partition cost: $aN + b + c/N$.
- Then recursively sort lower segment size $r - 1$ and upper segment size $N - r$.[^1]

So, **conditional on r**:

$$
T_N \mid r = T_{r - 1} + T_{N - r} + aN + b + \frac{c}{N}.[file:43]
$$

All $r = 1, \dots, N$ are equally likely, so take the expectation over $r$:

$$
T_N = \frac{1}{N} \sum_{r=1}^{N} \bigl( T_{r - 1} + T_{N - r} \bigr) + aN + b + \frac{c}{N}, \quad N \ge M.[file:43]
$$

Reindexing the sum, Hoare writes this as

$$
T_N = \frac{2}{N} \sum_{j=M}^{N-1} T_j \;+\; aN + b + \frac{c}{N}, \quad N \ge M,[file:43]
$$

with the convention that smaller $T_j$ (for $j < M$) come from the base-case method.[^1]

He then solves this recurrence (with some summation identities he sketches) and obtains an explicit formula for $T_N$.[^1]

## Extracting the average number of comparisons

To get just the **average number of comparisons**, he plugs in:

- $a = 1$ (one unit of time per comparison).
- $b = 0, c = 0$.
- $T_1 = 0$.
- A small cutoff $M = 2$.[^1]

Then he takes $N$ very large and keeps only the leading term. The result is:

$$
\text{Expected number of comparisons} \approx 2(N + 1) \sum_{j=2}^{N} \frac{1}{j + 1} \sim 2 N \log_2 N.[file:43]
$$

He states this in words as:

> “The figure obtained for the expected number of comparisons is $\frac{1}{2N} \sum \sim 2 N \log_2 N$.”[^1]

Finally he compares this with the information-theoretic lower bound $N \log_2 N$ comparisons, concluding that Quicksort’s average number of comparisons is worse by about a factor of $2 \log_e 2 \approx 1.4$.[^1]

If you want, next step can be to explicitly go through the algebra of the exchange expectation sum or the recurrence solution with substitutions so you can see each simplification step.
<span style="display:none">[^2][^3]</span>

<div align="center">⁂</div>

[^1]: Quicksort.pdf

[^2]: https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/30537770/3e7ab5e0-3e7d-440c-9eba-5e49c4716f6a/Quicksort.pdf

[^3]: https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/30537770/a221be01-1f26-4b07-8303-544240c9f8ae/Quicksort.pdf

