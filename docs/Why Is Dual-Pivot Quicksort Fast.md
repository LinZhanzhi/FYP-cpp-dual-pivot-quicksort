# Why Is Dual-Pivot Quicksort Fast?

#### Sebastian Wild [(][A][)]

(A) Fachbereich Informatik, TU Kaiserslautern

wild@cs.uni-kl.de


**Abstract**


I discuss the new dual-pivot Quicksort that is nowadays used to sort arrays of primitive
types in Java. I sketch theoretical analyses of this algorithm that offer a possible, and in my
opinion plausible, explanation why (a) dual-pivot Quicksort is faster than the previously
used (classic) Quicksort and (b) why this improvement was not already found much earlier.

### **1. Introduction**



Quicksort became popular shortly after its original presentation by Hoare [7] and many authors
contributed variations of and implementation tricks for the basic algorithm. From a practical
point of view, the most notable improvements appear in [17, 15, 2, 14].
After the improvements to Quicksort

D UAL P IVOT Q UICKSORT (A, _left_, _right_ ) // sort A[ _left_ .. _right_ ]

in the 1990’s, almost all programming li- 1 **if** _right_ − _left_ ≥ 1
braries used almost identical versions of

// Take outermost elements as pivots (replace by sampling)

the algorithm: the classic Quicksort im- 2 p := min {A[ _left_ ],A[ _right_ ]}
plementation had reached calm waters. 3 q := max{A[ _left_ ],A[ _right_ ]}
It was not until 2009, over a decade 4 ℓ := _left_ + 1; g := _right_ − 1; k := ℓ
later, that previously unknown Russian de- 5 **while** k ≤ g

6 **if** A[k] < p

veloper Vladimir Yaroslavskiy caused a 7 Swap A[k] and A[ℓ]; ℓ := ℓ + 1
sea change upon releasing the outcome of 8 **else if** A[k] ≥ q
his free-time experiments to the public: a 9 **while** A[g] > q and k < g
dual-pivot Quicksort implementation that 10 g := g − 1

11 **end while**

clearly outperforms the classic Quicksort 12 Swap A[k] and A[g]; g := g − 1
in Oracle’s Java 6. The core innovation 13 **if** A[k] < p
is the arguably natural ternary partitioning 14 Swap A[k] and A[ℓ]; ℓ := ℓ + 1
algorithm given to the right. 15 **end if**
Yaroslavskiy’s finding was so surpris- 1617 **end if** k := k + 1
ing that people were initially reluctant to 18 **end while**
believe him, but his Quicksort has finally 19 ℓ := ℓ − 1; g := g + 1
been deployed to millions of devices with 20 A[ _left_ ] := A[ℓ]; A[ℓ] := p // p to final position
the release of Java 7 in 2011. 21 A[ _right_ ] := A[g]; A[g] := q // q to final position

22 D UAL P IVOT Q UICKSORT (A, _left_, ℓ − 1)

How could this substantial improve
23 D UAL P IVOT Q UICKSORT (A, ℓ + 1,g − 1)

ment to the well-studied Quicksort al- 24 D UAL P IVOT Q UICKSORT (A,g + 1, _right_ )
gorithm escape the eyes of researchers 25 **end if**
around the world for nearly 50 years?



D UAL P IVOT Q UICKSORT (A, _left_, _right_ ) // sort A[ _left_ .. _right_ ]



1 **if** _right_ − _left_ ≥ 1



// Take outermost elements as pivots (replace by sampling)



2 p := min {A[ _left_ ],A[ _right_ ]}
3 q := max{A[ _left_ ],A[ _right_ ]}



4 ℓ := _left_ + 1; g := _right_ − 1; k := ℓ



5 **while** k ≤ g
6 **if** A[k] < p



7 Swap A[k] and A[ℓ]; ℓ := ℓ + 1
8 **else if** A[k] ≥ q



9 **while** A[g] > q and k < g



10 g := g − 1
11 **end while**



12 Swap A[k] and A[g]; g := g − 1



13 **if** A[k] < p
14 Swap A[k] and A[ℓ]; ℓ := ℓ + 1



15 **end if**

16 **end if**


17 k := k + 1



18 **end while**

19 ℓ := ℓ − 1; g := g + 1



20 A[ _left_ ] := A[ℓ]; A[ℓ] := p // p to final position

21 A[ _right_ ] := A[g]; A[g] := q // q to final position
22 D UAL P IVOT Q UICKSORT (A, _left_, ℓ − 1)



23 D UAL P IVOT Q UICKSORT (A, ℓ + 1,g − 1)
24 D UAL P IVOT Q UICKSORT (A,g + 1, _right_ )



25 **end if**


2 Sebastian Wild


For programs as heavily used as library sorting methods, it is advisable to back up experimental data with mathematically proven properties. The latter consider however only a _model_
of reality, which may or may not reflect accurately enough the behavior of actual machines.
The answer to above question is in part a tale of the pitfalls of theoretical models, so we
start with a summary of the mathematical analysis of Quicksort and the underlying model in
Section 2. We then briefly discuss the “memory wall” metaphor and its implications for Quicksort in Section 3, and finally propose an alternative model in Section 4 that offers an explanation
for the superiority of Yaroslavskiy’s Quicksort.

### **2. Analysis of Quicksort**


The classical model for the analysis of sorting algorithm considers the average number of key
comparisons on random permutations. Quicksort has been extensively studied under this model,
including variations like choosing the pivot as median of a sample [7, 4, 15, 10, 3]: Let c n denote
the expected number of comparisons used by classic Quicksort (as given in [16]), when each
pivot is chosen as median of a sample of 2t + 1 random elements. c n fulfills the recurrence


### c n = n − 1 + ∑

0≤j 1,j 2 ≤n−1
j 1 +j 2 =n−1



� jt 1 �� jt 2 �
n (c j 1 + c j 2 ) (1)
� 2t+1 �



since n −
1 comparisons are needed in the first partitioning step, and we have two recursive
calls of random sizes, where the probability to have sizes j 1 and j 2 is given by the fraction of
binomials (see [9] for details). This recurrence can be solved asymptotically [4, 15] to


1
c n ∼                - n ln n,
H 2(t+1) − H t+1


where H n = ∑ [n] i=1 [1][/i][ is the][ n][th harmonic number and][ f] [(][n][)][ ∼] [g][(][n][)][ means lim] [n][→∞] [f] [(][n][)][/g][(][n][) =]
1. The mathematical details are beyond the scope of this abstract, but a rather elementary
derivation is possible [10]. Large values of t are impractical; a good compromise in practice is
given by the “ninther”, the median of three medians, each chosen from three elements [2]. This
scheme can be analyzed similarly to the above [3].
The author generalized Equation (1) to Yaroslavskiy’s Quicksort [19, 18, 9]. Note that
unlike for classic Quicksort, the comparison count of Yaroslavskiy’s partitioning depends on
pivot values, so its expected value has to be computed over the choices for the pivots. We
obtain for tertiles-of-(3t + 2)



5 1
c n = � 3 [−] 9t + 12


### · n + O(1) + ∑

� 0≤j 1,j 2,j 3 ≤n−2
j 1 +j 2 +j 3 =n−2



� jt 1 �� jnt 2 �� jt 3 � - (c j 1 + c j 2 + c j 3 ); (2)
� 3t+2 �



with solution



c n ∼



5 1
3 [−] 9t+12  - n ln n.
H 3(t+1) − H t+1


Why Is Dual-Pivot Quicksort Fast? 3


Oracle’s Java runtime library previously used classic Quicksort with ninther, and now uses
Yaroslavskiy’s Quicksort with tertiles-of-five; the average number of comparisons are asymptotically 1.5697n ln n vs. 1.7043n ln n. According to the comparison model, Yaroslavskiy’s algorithm is significantly _worse_ than classic Quicksort! Moreover, this result does not change
qualitatively if we consider _all_ primitive instructions of a machine instead of only comparisons [19, 9]. It is thus not surprising that researchers found the use of two pivots not promising [15, 6].
But if Yaroslavskiy’s algorithm actually uses more comparisons and instructions, how comes
it is still faster in practice? And why was this discrepancy between theory and practice not
noticed earlier? The reason is most likely related to a phenomenon known as the “memory
wall” [20, 13] or the “von-Neumann bottleneck” [1]: Processor speed has been growing considerably faster than memory bandwidth for a long time.

### **3. The Memory Wall**



Based on the extensive data for the STREAM
benchmark [12, 11], CPU speed has increased 10 [2]
with an average annual growth rate of 46% over
the last 25 years, whereas memory bandwidth, the

10 [1]

amount of data transferable between RAM and

CPU in a given amount of time, has increased by
37% per year. Even though one should not be too

10 [0]

strict about the exact numbers as they are averages
over very different architectures, a significant in- 1991 1995 2000 2005 2010 2015
crease in _imbalance_ is undeniable. Figure 1 gives Figure 1: Development of CPU speed
a direct quantitative view of this trend. against memory bandwidth over the last 25
If the imbalance between CPU and memory years. Each point shows one reported retransfer speed continues to grow exponentially, at sult of the STREAM benchmark [12, 11],
some point in the future any further improvements with the date on the x-axis and the ma
chine balance (peak MFLOPS divided by

of CPUs will be futile: the processor is waiting for

Bandwidth in MW/s in the “triad” bench
data all the time; we hit a “memory wall”.

mark) on a logarithmic y-axis. The fat line

It is debatable if and when this extreme will

shows the linear regression (on log-scale).

be reached [5, 13], and consequences certainly de
Data is taken from www.cs.virginia.edu/

pend on the application. In any case, however, the

stream/by_date/Balance.html.

(average) relative costs of memory accesses have
increased significantly over the last 25 years.
So when Quicksort with two pivots was first studied, researchers correctly concluded that it
does not pay off. But computers have changed since then, and so must our models.



10 [2]



10 [1]



10 [0]



1991 1995 2000 2005 2010 2015



Figure 1: Development of CPU speed
against memory bandwidth over the last 25
years. Each point shows one reported result of the STREAM benchmark [12, 11],

with the date on the x-axis and the ma
chine balance (peak MFLOPS divided by
Bandwidth in MW/s in the “triad” bench
mark) on a logarithmic y-axis. The fat line
shows the linear regression (on log-scale).
Data is taken from www.cs.virginia.edu/
stream/by_date/Balance.html.


### **4. Scanned Elements**

Our new cost model for sorting counts the number of _“scanned elements”_ . An element scan
is essentially an accesses “A[i]” to the input array A, but we count all accesses as one that
use the same index variable i _and_ the same value for i. For example, a linear scan over A


4 Sebastian Wild


entails n scanned elements, and several interleaved scans (with different index variables) cost
the traveled distance, summed up over all indices, even when the scanned ranges overlap. We
do not distinguish read and write accesses.
We claim that for algorithms built on potentially interleaved sequential scans, in particular
for classic and dual-pivot Quicksort, the number of scanned elements is asymptotically proportional to the amount of data transfered between CPU and main memory [9].
Scanned elements are related to cache misses [8], but the latter is a machine-dependent
quantity, whereas the former is a clean, abstract cost measure that is easy to analyze: One
partitioning step of classic Quicksort scans A exactly once, resulting in n scanned elements. In
Yaroslavskiy’s partitioning, indices k and g together scan A once, but index ℓ scans the leftmost
segment a second time. On average, the latter contains a third of all elements, yielding 3 [4] [n]

scanned elements in total.

Using these in recurrences (1) resp. (2) yields 1.5697n ln n vs. 1.4035n ln n scanned elements; the Java 7 Quicksort saves 12% of the element scans over the version in Java 6, which
matches the roughly 10% speedup observed in running time studies.

### **5. Conclusion**


Memory speed has not fully kept pace with improvements in processing power. This growing
imbalance forces us to economize on memory accesses in algorithms that were almost entirely
CPU-bound in the past, and calls for new cost models for the analysis of algorithms. For sorting
algorithms that build on sequential scans over their input, the proposed “scanned elements”
counts serve as such a model and give a good indication of the amount of memory traffic caused
by an algorithm. It is exactly this data traffic where dual-pivot outclasses classic Quicksort,
offering a plausible explanation for its superiority in practice.

### **References**


[1] J. B ACKUS, Can Programming Be Liberated from the Von Neumann Style? A Functional Style
and Its Algebra of Programs. _Communications of the ACM_ **21** (1978) 8, 613–641.


[2] J. L. B ENTLEY, M. D. M C I LROY, Engineering a sort function. _Software: Practice and Experience_
**23** (1993) 11, 1249–1265.


[3] M. D URAND, Asymptotic analysis of an optimized quicksort algorithm. _Information Processing_
_Letters_ **85** (2003) 2, 73–77.


[4] M. H. VAN E MDEN, Increasing the efficiency of quicksort. _Communications of the ACM_ (1970),
563–567.


[5] M. A. E RTL, The Memory Wall Fallacy. 2001.
[https://www.complang.tuwien.ac.at/anton/memory-wall.html](https://www.complang.tuwien.ac.at/anton/memory-wall.html)


[6] P. H ENNEQUIN, _Analyse en moyenne d’algorithmes : tri rapide et arbres de recherche_ . PhD Thesis,
Ecole Politechnique, Palaiseau, 1991.


[7] C. A. R. H OARE, Quicksort. _The Computer Journal_ **5** (1962) 1, 10–16.


Why Is Dual-Pivot Quicksort Fast? 5


[8] S. K USHAGRA, A. L ÓPEZ -O RTIZ, A. Q IAO, J. I. M UNRO, Multi-Pivot Quicksort: Theory and
Experiments. In: _ALENEX 2014_ . SIAM, 2014, 47–60.


[9] C. M ARTÍNEZ, M. E. N EBEL, S. W ILD, Analysis of Pivot Sampling in Dual-Pivot Quicksort.
_Algorithmica_ (2015).
[http://arxiv.org/abs/1412.0193](http://arxiv.org/abs/1412.0193)


[10] C. M ARTÍNEZ, S. R OURA, Optimal Sampling Strategies in Quicksort and Quickselect. _SIAM Jour-_
_nal on Computing_ **31** (2001) 3, 683–705.


[11] J. D. M C C ALPIN, _STREAM: Sustainable Memory Bandwidth in High Performance Computers_ .
Technical report, University of Virginia, Charlottesville, Virginia, 1991-2007. Continually updated
technical report.
[http://www.cs.virginia.edu/~mccalpin/papers/bandwidth/bandwidth.html](http://www.cs.virginia.edu/~mccalpin/papers/bandwidth/bandwidth.html)


[12] J. D. M C C ALPIN, Memory Bandwidth and Machine Balance in Current High Performance Computers. _IEEE Computer Society Technical Committee on Computer Architecture (TCCA) Newsletter_
(1995), 19–25.
[http://www.cs.virginia.edu/~mccalpin/papers/balance/index.html](http://www.cs.virginia.edu/~mccalpin/papers/balance/index.html)


[13] S. A. M C K EE, Reflections on the Memory Wall. In: _Proceedings of the first conference on com-_
_puting frontiers_ . 2004, 162–167.


[14] D. R. M USSER, Introspective Sorting and Selection Algorithms. _Software: Practice and Experi-_
_ence_ **27** (1997) 8, 983–993.


[15] R. S EDGEWICK, _Quicksort_ . Ph. D. Thesis, Stanford University, 1975.


[16] R. S EDGEWICK, Implementing Quicksort programs. _Communications of the ACM_ **21** (1978) 10,
847–857.


[17] R. C. S INGLETON, Algorithm 347: an efficient algorithm for sorting with minimal storage [M1].
_Communications of the ACM_ **12** (1969) 3, 185–186.


[18] S. W ILD, _Java 7’s Dual Pivot Quicksort_ . Master thesis, University of Kaiserslautern, 2012.
[http://nbn-resolving.de/urn/resolver.pl?urn:nbn:de:hbz:386-kluedo-34638](http://nbn-resolving.de/urn/resolver.pl?urn:nbn:de:hbz:386-kluedo-34638)


[19] S. W ILD, M. E. N EBEL, Average Case Analysis of Java 7’s Dual Pivot Quicksort. In: L. E PSTEIN,
P. F ERRAGINA (eds.), _ESA 2012_ . LNCS 7501, Springer, 2012, 825–836.
[http://arxiv.org/abs/1310.7409](http://arxiv.org/abs/1310.7409)


[20] W. A. W ULF, S. A. M C K EE, Hitting the Memory Wall: Implications of the Obvious. 1995.


