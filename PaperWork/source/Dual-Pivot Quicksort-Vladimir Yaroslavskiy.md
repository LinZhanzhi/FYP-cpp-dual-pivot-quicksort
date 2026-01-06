The paper’s core algorithm is a dual‑pivot quicksort with 3-way partitioning and a special pivot sampling; below is clean pseudocode capturing that algorithmic structure (not Java-specific).

***

## Top-level sort

```pseudo
procedure sort(A):
    dualPivotQuicksort(A, 0, length(A) - 1)
```

***

## Dual-pivot quicksort

```pseudo
const TINYSIZE  := 17
const DISTSIZE  := 13

procedure dualPivotQuicksort(A, left, right):
    len := right - left
    if len < TINYSIZE:
        insertionSort(A, left, right)
        return

    # --- choose pivots using 5-sample from the middle ---
    sixth := len / 6
    m1 := left  + sixth
    m2 := m1    + sixth
    m3 := m2    + sixth
    m4 := m3    + sixth
    m5 := m4    + sixth

    # 5-element sorting network on A[m1..m5]
    sortFive(A, m1, m2, m3, m4, m5)

    pivot1 := A[m2]
    pivot2 := A[m4]
    diffPivots := (pivot1 != pivot2)

    # move pivots to ends
    swap(A[m2], A[left])
    swap(A[m4], A[right])

    less  := left  + 1    # boundary of < pivot1
    great := right - 1    # boundary of > pivot2

    # --- main partitioning loop ---
    if diffPivots:
        k := less
        while k <= great:
            x := A[k]
            if x < pivot1:
                swap(A[k], A[less])
                less := less + 1
                k    := k + 1
            else if x > pivot2:
                while A[great] > pivot2 and k < great:
                    great := great - 1
                swap(A[k], A[great])
                great := great - 1
                x := A[k]
                if x < pivot1:
                    swap(A[k], A[less])
                    less := less + 1
                # k moves on after handling x
                k := k + 1
            else:
                # pivot1 <= x <= pivot2, stays in middle
                k := k + 1
    else:
        # pivots equal: two-way partition wrt pivot1
        k := less
        while k <= great:
            x := A[k]
            if x < pivot1:
                swap(A[k], A[less])
                less := less + 1
                k    := k + 1
            else if x > pivot1:
                while A[great] > pivot1 and k < great:
                    great := great - 1
                swap(A[k], A[great])
                great := great - 1
                x := A[k]
                if x < pivot1:
                    swap(A[k], A[less])
                    less := less + 1
                k := k + 1
            else:
                # x == pivot1
                k := k + 1

    # --- move pivots into final positions ---
    less  := less  - 1
    great := great + 1
    swap(A[left],  A[less])   # place pivot1
    swap(A[right], A[great])  # place pivot2

    # --- recursive calls on left and right parts ---
    dualPivotQuicksort(A, left,  less  - 1)
    dualPivotQuicksort(A, great + 1, right)

    # --- handle equal elements around pivots ---
    if diffPivots and (great - less > len - DISTSIZE):
        k := less + 1
        while k < great:
            x := A[k]
            if x == pivot1:
                swap(A[k], A[less])
                less := less + 1
            else if x == pivot2:
                swap(A[k], A[great])
                great := great - 1
                x := A[k]
                if x == pivot1:
                    swap(A[k], A[less])
                    less := less + 1
            k := k + 1

    # --- recursive call on center part (between pivots) ---
    if diffPivots:
        dualPivotQuicksort(A, less + 1, great - 1)
```

***

## Insertion sort (for small segments)

```pseudo
procedure insertionSort(A, left, right):
    for i from left + 1 to right:
        x := A[i]
        j := i
        while j > left and A[j - 1] > x:
            A[j] := A[j - 1]
            j := j - 1
        A[j] := x
```

***

## 5-element sort helper

```pseudo
procedure sortFive(A, i1, i2, i3, i4, i5):
    # apply fixed sequence of compare–swap operations
    cmpSwap(A, i1, i2)
    cmpSwap(A, i4, i5)
    cmpSwap(A, i1, i3)
    cmpSwap(A, i2, i3)
    cmpSwap(A, i1, i4)
    cmpSwap(A, i3, i4)
    cmpSwap(A, i2, i5)
    cmpSwap(A, i2, i3)
    cmpSwap(A, i4, i5)

procedure cmpSwap(A, i, j):
    if A[i] > A[j]:
        swap(A[i], A[j])
```

This pseudocode reflects the algorithm as described and implemented in the paper, including the pivot sampling, three-way partitioning, small‑array insertion sort, and special handling of equal elements.


For both classic and dual‑pivot Quicksort, the paper derives a recurrence for the expected cost and then solves it asymptotically to get $A n \ln n$, determining the constant $A$ by comparing leading terms.

***

## Classic Quicksort – comparisons

1. Model and recurrence
   - Input is a random permutation of $n$ distinct keys.
   - In one step: choose one pivot uniformly, compare it with the other $n-1$ elements, then recursively sort left and right subarrays.
   - Let $C_n$ be the expected number of comparisons for size $n$. Then
     $$
     C_n = (n-1) + \frac{1}{n} \sum_{k=0}^{n-1} \bigl(C_k + C_{n-k-1}\bigr) \tag{1}
     $$
     because each pivot position $k$ has probability $1/n$.
   - Using symmetry of $k$ and $n-k-1$:
     $$
     C_n = (n-1) + \frac{2}{n} \sum_{k=0}^{n-1} C_k. \tag{2}
     $$

2. Linearization
   - Write the same for $n+1$:
     $$
     C_{n+1} = n + \frac{2}{n+1} \sum_{k=0}^n C_k. \tag{3}
     $$
   - Multiply (2) by $n$ and (3) by $n+1$, subtract:
     $$
     (n+1)C_{n+1} - n C_n = 2n + 2 C_n. \tag{4}
     $$

3. Asymptotic form and constant
   - Assume $C_n \approx A n \ln n$ for large $n$ (from information‑theoretic lower bound and Stirling’s formula).
- Substituting $C_n = A n \ln n$ into (4), we get
  $$
  A(n+1)^2 \ln(n+1) - A n^2 \ln n = 2n + 2 A n \ln n. \tag{5}
  $$
- Rearranging to group terms by $A$:
  $$
  A \Bigl[ (n+1)^2 \ln(n+1) - (n^2 + 2n) \ln n \Bigr] = 2n. \tag{6}
  $$
- Divide by $n A$ and use the identity $\ln(n+1) - \ln n = \ln(1 + \frac{1}{n})$ to get
  $$
  (n+2) \ln\left(1 + \frac{1}{n}\right) + \frac{1}{n}\ln(n+1) = \frac{2}{A}. \tag{7}
  $$
- Expanding the term $(n+2)$ yields the explicit form:
    $$
    n \ln\!\left(1 + \frac{1}{n}\right) + 2 \ln\!\left(1 + \frac{1}{n}\right) + \frac{1}{n}\ln(n+1) = \frac{2}{A}. \tag{8}
    $$
- To evaluate the limit as $n \to \infty$, consider the substitution $x = \frac{1}{n}$. As $n \to \infty$, $x \to 0$.
- We use the standard limit $\lim_{x \to 0} \frac{\ln(1+x)}{x} = 1$. Applying this to the terms:
    1. **First term**: $n \ln(1 + \frac{1}{n}) = \frac{1}{x} \ln(1+x) = \frac{\ln(1+x)}{x}$, which approaches **1**.
    2. **Second term**: $2 \ln(1 + \frac{1}{n}) \to 2 \ln(1) = 0$.
    3. **Third term**: $\frac{\ln(n+1)}{n} \to 0$ (since $n$ grows faster than $\ln n$).
- Summing the limits gives $1 + 0 + 0 = \frac{2}{A}$, leading to $A = 2$.
- Therefore:
    $$
    C_n \sim 2 n \ln n. \tag{9}
    $$

***

## Classic Quicksort – swaps

1. Recurrence
   - Let $S_n$ be the expected number of swaps.
   - In partitioning, on average half of the $n-1$ non‑pivot elements are swapped, so expected swaps per partition step is $\tfrac{1}{2}(n-1)$.
   - The recurrence is
     $$
     S_n = \tfrac{1}{2}(n-1) + \frac{2}{n} \sum_{k=0}^{n-1} S_k. \tag{10}
     $$

2. Linearization
    - Multiply (10) by $n$:
      $$
      n S_n = \frac{n(n-1)}{2} + 2 \sum_{k=0}^{n-1} S_k. \tag{11}
      $$
    - Write for $n+1$:
      $$
      (n+1) S_{n+1} = \frac{(n+1)n}{2} + 2 \sum_{k=0}^{n} S_k. \tag{12}
      $$
    - Subtract (11) from (12):
      $$
      (n+1) S_{n+1} - n S_n = n + 2 S_n. \tag{13}
      $$

3. Asymptotic form and constant
    - Assume $S_n \approx A n \ln n$ for large $n$.
    - Substitute $S_n = A n \ln n$ into (13), yields:
      $$
      A(n+1)^2 \ln(n+1) - A n^2 \ln n = n + 2 A n \ln n. \tag{14}
      $$
    - Rearranging to group terms by $A$:
      $$
      A \Bigl[ (n+1)^2 \ln(n+1) - (n^2 + 2n) \ln n \Bigr] = n. \tag{15}
      $$
    - Divide by $n A$ and use the identity $\ln n = \ln(n+1) - \ln(1 + \frac{1}{n})$. The term in brackets becomes:
      $$
      (n+1)^2 \ln(n+1) - (n^2 + 2n) \left[ \ln(n+1) - \ln\left(1 + \frac{1}{n}\right) \right]
      $$
    - simplifying the coefficients of $\ln(n+1)$:
      $$
      \bigl[ (n+1)^2 - (n^2 + 2n) \bigr] \ln(n+1) + (n^2 + 2n) \ln\left(1 + \frac{1}{n}\right)
      $$
      $$
      = \ln(n+1) + (n^2 + 2n) \ln\left(1 + \frac{1}{n}\right).
      $$
    - Now dividing the whole equation by $nA$:
      $$
      \frac{1}{n}\ln(n+1) + (n+2) \ln\left(1 + \frac{1}{n}\right) = \frac{1}{A}. \tag{16}
      $$
    - Expanding the term $(n+2)$ yields the explicit form:
      $$
      \frac{\ln(n+1)}{n} + n \ln\left(1 + \frac{1}{n}\right) + 2 \ln\left(1 + \frac{1}{n}\right) = \frac{1}{A}. \tag{17}
      $$
    - To evaluate the limit as $n \to \infty$:
      1. **First term**: $\frac{\ln(n+1)}{n} \to 0$.
      2. **Second term**: Let $x = 1/n$. Then $n \ln(1+1/n) = \frac{\ln(1+x)}{x}$. As $x \to 0$, this limit is **1**.
      3. **Third term**: $2 \ln(1+1/n) \to 2 \ln(1) = 0$.
    - Summing the limits gives $0 + 1 + 0 = \frac{1}{A}$, leading to $A = 1$.
    - Therefore:
      $$
      S_n \sim 1 \cdot n \ln n. \tag{18}
      $$

***

## Dual‑pivot Quicksort – comparisons

1. Model and full recurrence
   - Algorithm: choose two random pivots $p_1 \le p_2$, partition into three parts $(\le p_1), (p_1 \le \cdot \le p_2), (\ge p_2)$, then recursively sort the three parts.
   - Let $C_n$ be expected number of comparisons. For given split sizes $(i, j-i-1, n-j-1)$ determined by the two pivot positions $i < j$, cost is:
    - Recursive costs: $C_i + C_{j-i-1} + C_{n-j-1}$.
        - Comparisons during partitioning:
          - Every element $x$ is first compared to $p_1$.
          - If $x < p_1$, it goes to the left part (1 comparison total).
          - If $x \ge p_1$, it is then compared to $p_2$.
          - If $p_1 \le x \le p_2$, it goes to the middle part (2 comparisons total).
          - If $x > p_2$, it goes to the right part (2 comparisons total).
        - Additionally, there is 1 initial comparison to ensure the pivots satisfy $p_1 \le p_2$.
       - Taking expectation over all ordered pivot pairs $(i,j)$—where $i$ is the number of elements in the left partition and $j$ is the index of the second pivot—each pair has probability $2/(n(n-1))$. This gives:
        $$
        \begin{aligned}
        C_n &= 1 + \frac{2}{n(n-1)} \sum_{i=0}^{n-2} \sum_{j=i+1}^{n-1} \Bigl( C_i + i \\
            &\quad + C_{j-i-1} + 2(j-i-1) + C_{n-j-1} + 2(n-j-1)\Bigr). \tag{1}
        \end{aligned}
        $$

2. Expected partition comparisons term
   - The non‑recursive term inside the double sum is
     $$
     i + 2(j-i-1) + 2(n-j-1).
     $$
- **Detailed derivation**:
  - First, simplify the cost term $c(i,j)$ inside the sum:
     $$
     \begin{aligned}
     c(i,j) &= i + 2(j - i - 1) + 2(n - j - 1) \\
              &= i + 2j - 2i - 2 + 2n - 2j - 2 \\
              &= 2n - i - 4.
     \end{aligned}
     $$
     Note that this term depends only on $i$, not on $j$.
  - The inner sum runs for $j$ from $i+1$ to $n-1$. The number of terms is $(n-1) - (i+1) + 1 = n - i - 1$.
  - Thus the double sum reduces to a single sum:
     $$
     S = \sum_{i=0}^{n-2} (n - i - 1)(2n - i - 4).
     $$
  - Change variables closer to the array bounds: Let $k = n - 1 - i$. As $i$ ranges from $0$ to $n-2$, $k$ ranges from $n-1$ down to $1$. Substituting $i = n - 1 - k$:
     $$
     \begin{aligned}
     S &= \sum_{k=1}^{n-1} k \bigl(2n - (n - 1 - k) - 4\bigr) \\
        &= \sum_{k=1}^{n-1} k (n + k - 3) \\
        &= (n-3) \sum_{k=1}^{n-1} k + \sum_{k=1}^{n-1} k^2.
     \end{aligned}
     $$
  - Using standard summation identify $\sum_{1}^{m} k^2 = \frac{m(m+1)(2m+1)}{6}$ with $m = n-1$:
     $$
     \begin{aligned}
     S &= (n-3) \frac{(n-1)n}{2} + \frac{(n-1)n(2n-1)}{6} \\
        &= \frac{n(n-1)}{6} \Bigl[ 3(n-3) + (2n-1) \Bigr] \\
        &= \frac{n(n-1)}{6} (3n - 9 + 2n - 1) \\
        &= \frac{n(n-1)}{6} (5n - 10) \\
        &= \frac{5}{6} n(n-1)(n-2). \tag{*}
     \end{aligned}
     $$
   - Hence the average per partition is
     $$
     \frac{2}{n(n-1)} \cdot \frac{5}{6} n(n-1)(n-2) = \frac{5}{3}(n-2).
     $$
   - Substituting back:
     $$
     C_n = 1 + \frac{5}{3}(n-2) + \frac{2}{n(n-1)}
           \sum_{i=0}^{n-2} \sum_{j=i+1}^{n-1}
           \bigl(C_i + C_{j-i-1} + C_{n-j-1}\bigr). \tag{2}
     $$
    3. Reducing the double sum
        - We need to simplify the triple term inside the double sum:
          $$
          \Sigma_{triple} = \sum_{i=0}^{n-2} \sum_{j=i+1}^{n-1} \bigl(C_i + C_{j-i-1} + C_{n-j-1}\bigr).
          $$
        - **Detailed re-indexing**:
          - **Term $C_i$**:
             - The index $i$ is fixed in the inner sum.
             - For a fixed $i$, the term $C_i$ is repeated $(n-1) - (i+1) + 1 = n - i - 1$ times.
             - Sum over all $i$: $\sum_{i=0}^{n-2} (n - i - 1) C_i$.
             - Let $k=i$: this contributes $\sum_{k=0}^{n-2} (n - k - 1) C_k$.
          - **Term $C_{j-i-1}$**:
             - Let $k = j - i - 1$.
             - As $j$ ranges from $i+1$ to $n-1$, $k$ ranges from $0$ to $n - i - 2$.
             - We sum this over all $i$: $\sum_{i=0}^{n-2} \sum_{k=0}^{n-i-2} C_k$.
             - Swap the order of summation:
                - $k$ can range from $0$ to $n-2$ (when $i=0$).
                - For a fixed $k$, $i$ ranges from $0$ up to $n - k - 2$.
                - The number of such $i$'s is $(n - k - 2) - 0 + 1 = n - k - 1$.
             - Thus, this term contributes $\sum_{k=0}^{n-2} (n - k - 1) C_k$.
          - **Term $C_{n-j-1}$**:
             - Let $k = n - j - 1$.
             - As $j$ ranges from $i+1$ to $n-1$, $k$ ranges from $n - i - 2$ down to $0$.
             - The summation becomes $\sum_{i=0}^{n-2} \sum_{k=0}^{n-i-2} C_k$.
             - This is identical to the structure of the second term.
             - By symmetry, this also contributes $\sum_{k=0}^{n-2} (n - k - 1) C_k$.
        - **Conclusion**:
          - All three terms contribute the exact same sum.
          - Adding them up:
             $$
             \Sigma_{triple} = 3 \sum_{k=0}^{n-2} (n - k - 1) C_k.
             $$
   - Thus
     $$
     C_n = 1 + \frac{5}{3}(n-2) + \frac{2}{n(n-1)} \sum_{k=0}^{n-2} 3 (n-k-1) C_k. \tag{3}
     $$

4. Linearization step
   - Let $f_n = 1 + \frac{5}{3}(n-2)$. Multiply (3) by $n(n-1)$:
     $$
     n(n-1) C_n = n(n-1) f_n + 6 \sum_{k=0}^{n-2} (n-k-1) C_k. \tag{4}
     $$
   - Write for $n+1$:
     $$
     n(n+1) C_{n+1} = n(n+1) f_n + 6 \sum_{k=0}^{n-1} (n-k) C_k. \tag{5}
     $$
   - Subtract (4) from (5), define
     $$
     X_n = n(n+1) C_{n+1} - n(n-1) C_n,\quad
     F_n = n(n+1) f_n - n(n-1) f_n,
     $$
     to get
     $$
     X_n = F_n + 6 \sum_{k=0}^{n-2} C_k + 6 C_{n-1}. \tag{7}
     $$
   - Write for $n+1$:
     $$
     X_{n+1} = F_{n+1} + 6 \sum_{k=0}^{n-1} C_k + 6 C_n. \tag{8}
     $$
   - Subtract (7) from (8):
     $$
     X_{n+1} - X_n = F_{n+1} - F_n + 6 C_n. \tag{9}
     $$
   - The paper simplifies $F_{n+1} - F_n$ to $2 + 10 n$, giving
    > **Detailed derivation of $(10n + 2)$**:
    > 1. The linear cost is $f_n = 1 + \frac{5}{3}(n-2) = \frac{5n - 7}{3}$.
    > 2. The driving term $F_n$ arises from the difference of the scaled partial costs, correctly indexed as $n(n+1)f_{n+1} - n(n-1)f_n$.
    >    - $n(n+1)f_{n+1} = n(n+1)\frac{5(n+1)-7}{3} = \frac{5n^3 + 3n^2 - 2n}{3}$.
    >    - $n(n-1)f_n = n(n-1)\frac{5n-7}{3} = \frac{5n^3 - 12n^2 + 7n}{3}$.
    >    - Subtracting these yields $F_n = \frac{15n^2 - 9n}{3} = 5n^2 - 3n$.
    > 3. Then $F_{n+1} - F_n$ is:
    >    - $F_{n+1} = 5(n+1)^2 - 3(n+1) = 5n^2 + 7n + 2$.
    >    - $F_{n+1} - F_n = (5n^2 + 7n + 2) - (5n^2 - 3n) = 10n + 2$.

     $$
     X_{n+1} - X_n = 2 + 10 n + 6 C_n. \tag{10}
     $$
   - Expanding $X_n$ in terms of $C_n$ leads to the three‑term recurrence
     $$
     (n+1)(n+2) C_{n+2}
       - 2 n(n+1) C_{n+1}
       + (n(n-1) - 6) C_n
       = 2 + 10 n. \tag{11}
     $$

5. Asymptotic form and constant
   - Assume $C_n \approx A n \ln n$.
   - Substitute into (11); after expanding and simplifying logs, obtain
     $$
     (n^3 + 5n^2 + 8n + 4) \ln(n+2)
      - (2n^3 + 4n^2 + 2n) \ln(n+1)
      + (n^3 - n^2 - 6n) \ln n
        = \frac{10n + 2}{A}. \tag{13}
     $$
   - Rearranged:
     $$
     \begin{aligned}
     &n^3(\ln(n+2) - 2\ln(n+1) + \ln n) \\
     &\quad + n^2(5\ln(n+2) - 4\ln(n+1) - \ln n) \\
     &\quad + n(8\ln(n+2) - 2\ln(n+1) - 6\ln n)
     + 4\ln(n+2) = \frac{10n+2}{A}. \tag{14}
     \end{aligned}
     $$
- To find the constant $A$, we analyze the asymptotic behavior of equation (14) as $n \to \infty$. We compare the coefficients of linear term $n$ on both sides. On the RHS, the coefficient is $10/A$. On the LHS, we expand the logarithmic terms using the Taylor series approximation $\ln(1+x) \approx x - \frac{x^2}{2}$ for small $x$.

    1.  **The $n^3$ term**:
         $$
         n^3 \left[ \ln\left(1+\frac{2}{n}\right) - 2\ln\left(1+\frac{1}{n}\right) \right]
         $$
         $$
         \approx n^3 \left[ \left(\frac{2}{n} - \frac{2}{n^2}\right) - 2\left(\frac{1}{n} - \frac{1}{2n^2}\right) \right]
         $$
         $$
         = n^3 \left[ \frac{2}{n} - \frac{2}{n^2} - \frac{2}{n} + \frac{1}{n^2} \right] = n^3 \left[ -\frac{1}{n^2} \right] = -n.
         $$
         Contribution to coefficient: **-1**.

    2.  **The $n^2$ term**:
         $$
         n^2 \left[ 5\ln\left(1+\frac{2}{n}\right) - 4\ln\left(1+\frac{1}{n}\right) \right]
         $$
         $$
         \approx n^2 \left[ 5\left(\frac{2}{n}\right) - 4\left(\frac{1}{n}\right) \right] = n^2 \left[ \frac{10}{n} - \frac{4}{n} \right] = 6n.
         $$
         Contribution to coefficient: **6**.

    3.  **The $n$ term**:
         $$
         n \left[ 8\ln\left(1+\frac{2}{n}\right) - 2\ln\left(1+\frac{1}{n}\right) \right] \approx n \left[ \frac{16}{n} - \frac{2}{n} \right] = 14.
         $$
         This is a constant term (scales as $n^0$), so the contribution to the linear coefficient is **0**.

    - Summing the coefficients of $n$:
      $$
      -1 + 6 + 0 = \frac{10}{A}. \tag{15}
      $$
   - Thus $5/A = 5$, giving $A = 2$.
   - Therefore
     $$
     C_n \sim 2 n \ln n. \tag{9}
     $$

***

## Dual‑pivot Quicksort – swaps

1. Recurrence
   - Let $S_n$ be the expected number of swaps.
   - In one partition step, the average number of swaps on the $n-2$ non‑pivot elements is modeled as $\tfrac{2}{3}(n-2)$ (about one third of elements swapped).
- **Rigorous derivation of swap probability**:
    - Unlike the simplified geometric argument, we derive the exact probability algebraically.
    - Consider an array of $n$ distinct elements. We choose two pivots $p_1$ and $p_2$ uniformly at random from the $\binom{n}{2}$ possible pairs. Let the sorted order of the array be $z_1 < z_2 < \dots < z_n$.
    - Any pair of indices $\{i, j\}$ with $1 \le i < j \le n$ is equally likely to be selected as the pivots' positions in the sorted sequence.
    - An arbitrary non-pivot element $x$ has rank $k$ in the sorted array ($x = z_k$). We need to determine the probability that $x < p_1$ (requires swap to left), $p_1 < x < p_2$ (stays), or $x > p_2$ (requires swap to right).
    - There are total $\binom{n}{2}$ ways to pick indices $i, j$ for pivots.
    - **Case 1: $x < p_1$**. This occurs if the rank $k$ of element $x$ is strictly less than the rank $i$ of the first pivot $p_1$. The indices must satisfy $k < i < j$. The number of ways to choose $i, j$ such that they are both greater than $k$ is $\binom{n-k}{2}$.
    - **Case 2: $x > p_2$**. This occurs if the rank $k$ is strictly greater than the rank $j$ of the second pivot $p_2$. The indices must satisfy $i < j < k$. The number of ways to choose $i, j$ such that they are both less than $k$ is $\binom{k-1}{2}$.
    - **Case 3: $p_1 < x < p_2$**. This occurs if $i < k < j$. The number of ways to choose one index less than $k$ and one index greater than $k$ is $(k-1)(n-k)$.
    - We sum these probabilities over all possible ranks $k$ for the element $x$. Since $x$ is a random non-pivot element, any rank $k \in \{1, \dots, n\}$ is equally likely, excluding the 2 pivot spots. For large $n$, we can approximate by summing over all $k$.
    - **Total Swap Probability**:
      The probability an arbitrary element $x$ needs moving is $P(x < p_1) + P(x > p_2)$.
      $$
      P(\text{swap}) \approx \frac{1}{n} \sum_{k=1}^{n} \frac{\binom{n-k}{2} + \binom{k-1}{2}}{\binom{n}{2}}
      $$
      Using limiting arguments as $n \to \infty$, let $t = k/n$ where $t \in [0, 1]$. The probability becomes:
      $$
      \int_0^1 \frac{(1-t)^2 + t^2}{1} dt = \left[ -\frac{(1-t)^3}{3} + \frac{t^3}{3} \right]_0^1 = \left(0 + \frac{1}{3}\right) - \left(-\frac{1}{3} + 0\right) = \frac{2}{3}.
      $$
      Similarly, the probability of staying in the middle is $\int_0^1 2t(1-t) dt = 1/3$.
    - Thus, mathematically, the expected number of swaps per non-pivot element converges to precisely $2/3$.
    - Total expected swaps for $n-2$ elements is $\frac{2}{3}(n-2)$.
- **Total non-recursive swaps**:
  - **Pivots to ends**: 2 swaps (`swap(A[m2], A[left])`, `swap(A[m4], A[right])`).
  - **Pivots to final**: 2 swaps (`swap(A[left], A[less])`, `swap(A[right], A[great])`).
  - **Partitioning**: $\frac{2}{3}(n-2)$ swaps (derived above).
  - Sum: $4 + \frac{2}{3}(n-2)$.

- **Recursive calls**:
  - Just like comparisons, we sum the expected swaps of the three subarrays over all possible pivot pairs.
  - By the symmetry argument derived in the comparisons section (Section 3), the triple sum contributions of the left, middle, and right subarrays are identical.
  - The average recursive cost is thus $3 \times \frac{2}{n(n-1)} \sum_{k=0}^{n-2} (n-k-1) S_k$.

- **Final Recurrence**:
  Combining these terms gives the full recurrence relation:
  $$
  S_n = 4 + \frac{2}{3}(n-2)
          + \frac{6}{n(n-1)} \sum_{k=0}^{n-2} (n-k-1) S_k. \tag{10}
  $$
    2. Linearization step
        - Let $g_n = 4 + \frac{2}{3}(n-2) = \frac{2n + 8}{3}$.
        - Apply the same transformation as in the comparisons section. Multiply the recurrence by $n(n-1)$, write for $n+1$, and subtract to find the driving term $G_{n+1} - G_n$ derived from $g_n$.
        - Calculate $G_n = n(n+1)g_{n+1} - n(n-1)g_n$:
          $$
          \begin{aligned}
          n(n+1) g_{n+1} &= n(n+1) \frac{2(n+1)+8}{3} = \frac{2n^3 + 12n^2 + 10n}{3} \\
          n(n-1) g_n     &= n(n-1) \frac{2n+8}{3}     = \frac{2n^3 + 6n^2 - 8n}{3} \\
          G_n            &= \frac{6n^2 + 18n}{3}      = 2n^2 + 6n.
          \end{aligned}
          $$
        - Calculate the difference $G_{n+1} - G_n$:
          $$
          \begin{aligned}
          G_{n+1} &= 2(n+1)^2 + 6(n+1) = 2n^2 + 10n + 8 \\
          G_{n+1} - G_n &= (2n^2 + 10n + 8) - (2n^2 + 6n) = 4n + 8.
          \end{aligned}
          $$
        - The recursive part (summation) cancels out exactly as it did for comparisons, leaving the differential equation structure:
          $$
          (n+1)(n+2) S_{n+2}
             - 2 n(n+1) S_{n+1}
             + (n(n-1) - 6) S_n
             = 4n + 8. \tag{12}
          $$

    3. Asymptotic form and constant
        - Assume $S_n \approx A n \ln n$.
        - We compare the coefficients of linear term $n$ on both sides, as done in eq(15).
        - **LHS**: The structural coefficients determined by the recursive calls are identical to the comparisons case (since the recurrence relation is the same on the left side). Thus, the coefficient of $n$ on the LHS expands to **$5A$**.
        - **RHS**: The linear term is **$4n$**.
        - Equating the coefficients:
          $$
          5A = 4 \implies A = \frac{4}{5} = 0.8.
          $$
   - Therefore
     $$
     S_n \sim 0.8\, n \ln n. \tag{11}
     $$

***

## Final asymptotic results

- **Classic Quicksort**:
  - Average comparisons: $C_n \sim 2 n \ln n$.
  - Average swaps: $S_n \sim 1 \cdot n \ln n$.

- **Dual‑pivot Quicksort**:
  - Average comparisons: $C_n \sim 2 n \ln n$ (same leading coefficient).
  - Average swaps: $S_n \sim 0.8 n \ln n$ (20% fewer swaps in the leading term).