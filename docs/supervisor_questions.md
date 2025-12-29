# Questions for Supervisor Meeting

## 1. Validation & Benchmarking (The "Proof")
*   **Success Criteria**: What is the standard for "success"? Should I compare against `std::sort` (Introsort), `std::stable_sort` (Merge Sort), or parallel libraries like `TBB` or `OpenMP`?
*   **Datasets**: Are there specific datasets or distributions to target? (e.g., "organ pipe" distribution, real-world weather/financial data, or just random/sorted/duplicate cases?)
*   **Analysis Depth**: How deep should the performance analysis go? Is "Time vs. Size" sufficient, or do we need CPU counter analysis (cache misses, branch mispredictions) to prove *why* optimizations like prefetching work?

## 2. Scope & Missing Features (The "Next Steps")
*   **Custom Comparators**: Is support for custom comparators (e.g., `[](int a, int b) { return a > b; }`) a requirement? Currently, the library hardcodes `<`. Adding this would make it truly generic but requires significant templating changes.
*   **Standard Compliance**: Should I aim for C++17 Parallel Algorithms compliance (`std::execution::par`), or is the current custom API (`dual_pivot::sort`) sufficient?
*   **Out-of-Core Sorting**: We added 64-bit support for large RAM arrays. Is disk-based (external) sorting for datasets larger than RAM in scope?

## 3. Academic & Report Value (The "Thesis")
*   **Granularity**: I have written technical reports on specific optimizations (Introsort, Timsort-heuristics, Memory Layout). Is this the right level of detail for the final thesis, or should I focus more on mathematical proofs vs. empirical data?
*   **Java vs. C++**: Since this is an adaptation of Yaroslavskiy's Java algorithm, should the thesis focus on the *translation challenges* (JVM vs. C++ hardware access) or primarily on the *final performance*?

## 4. Code Quality & Architecture
*   **Header-Only Design**: I've used a header-only design for templates. Do you have concerns regarding compile times or binary size?
*   **Production Standards**: I've implemented "Namespace Safety" and "64-bit Addressing". Are there other standards (like Fuzz Testing or Address Sanitizers) you recommend implementing?
