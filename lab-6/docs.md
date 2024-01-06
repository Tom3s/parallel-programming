# Hamilton Cycle

## Observations

Since this problem is NP-complete, we can't find a polynomial solution for it. So, we will use a brute force approach to solve it (using backtracking). This means that parallelizing the algorithm did not bring any improvements, since the algorithm is sequential.

## Time

100 vertices, edge density as a percentage of max

density | 75% | 55% | 100% | 50% | 35%
--- | --- | --- | --- | --- | ---
sequential | 25ms | 26ms | 26ms | 33ms | 228ms
parallel | 88ms | 945ms | 80ms | 545ms | 1732ms

We can see that the parallel version is slower by a factor of ~3.5 when a cycle is easily found, but significantly slower when the cycle is harder to find. On runs where there is no cycle, the parallel version didn't finish in a reasonable amount of time.

30 vertices, edge density of 10%
```
No Hamiltonian cycle found
Time for linear algorithm: 1,819 ms
Time for parallel algorithm: 21,428 ms
```

Not everything has to be parallelized. 🤷‍♀️

## Speedup

Since the starting point is the same, the threading is done on each step of the "DFS" algorithm. This way the two algorithms produce the same results. The speed difference is within the error margin, so we can't say that the parallel version is faster.