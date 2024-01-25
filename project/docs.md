# Simulating the N-body problem

## Algorithm

The N-body problem can be simplified into separate `Body` classes, with each a `calculateUpdate` and an `applyUpdate` function.
Then we specify the number of `ticks` the simulation should run for and the `deltaTime` between each tick.
This allows us to run the simulation sequentially, or also in parallel.

## Synchonization for parallelism

The synchronization is done by a `Barrier` class, which is initialized with the number of threads that will be running.
Each thread then calls `barrier.arrive_and_wait()` after calculating the update and after applying the update (twice per tick, to ensure correctness).
This ensures that all threads are synchronized before calculating the next tick.

## Performance

n - number of bodies
5000 ticks

| n | sequential | parallel  (n threads) | % diff |
|---|---|---|---|
| 5 | 32ms | 725ms | 22.65x slower | 
| 15 | 140ms | 180ms | 1.28x slower |
| 30 | 330ms | 285ms | 1.15x faster |
| 100 | 2.8s | 1.25s | 2.24x faster |
| 500 | 1m 14s | 35.68s | 2.07x faster |

## Conclusion

The parallel version is faster for larger n, but slower for smaller n ( \< 30~ ).
