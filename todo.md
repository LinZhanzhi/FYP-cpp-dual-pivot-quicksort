benchmark add average number of swaps.
graph show C n ln n , show the C
swap graph show C n ln n, show the C
run test in my pc  .

space optimization


report pivot selection

report runtime comparison on 3 machine. desktop. asus laptop. apple laptop.
report operations of the algorithm. making a new counting implementation that sacrifice performance. to the count is portable.


recursion depth cap still 2log2 n  ?

use data to find boundary for small array to use insertion sort ?

why can merge on reverse sorted , cannot merge on nearly sorted ?


Uniform random integers with a fixed PRNG seed s

Existing prepared datasets
There are a few datasets or frameworks you can reuse, though none is an official standard like ImageNet is for vision:

A Kaggle “Benchmark Dataset for Sorting Algorithms” provides numeric arrays drawn from a normal distribution specifically to compare sort implementations.
​

The cpp-sort project’s benchmark suite uses a variety of synthetic data patterns (random, sorted, nearly sorted, etc.) and sizes as a de‑facto reference for C++ sorting algorithm comparisons.
​

Some GitHub projects publish their array generators and benchmark harnesses (e.g., random integer arrays for timing classic sorting algorithms), which you can clone and adapt instead of starting from scratch.
​

These are useful as examples or starting points, but not community-wide standards.
​

If you want “standardized” inputs
To make your results comparable and reproducible, a practical approach is:

Fix your own “suite” of input generators:

uniform random, Gaussian-like, almost-sorted, reverse-sorted, few-distinct, etc.
​

Fix PRNG seed(s) and array sizes (e.g., 1e3, 1e4, 1e5, 1e6) and publish the generator code (or the arrays themselves) with your results.
​

Optionally, take inspiration from the patterns and distributions used in existing benchmarks like the Kaggle dataset or cpp-sort benchmarks.
​

This gives you something “standard” within your project or paper, and others can reproduce or extend your benchmarks by reusing your generator.
