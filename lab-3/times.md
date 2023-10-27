# Times

## Unique thread for each element

- Time to calculate product for 5x3 matrix: `2ms`
- Time to calculate product for 10x10 matrix: `8ms`
- Time to calculate product for 50x50 matrix: `284ms`
- Time to calculate product for 100x100 matrix: `1072ms`
- Time to calculate product for 250x250 matrix: `8437ms`

## Batching each 2 row

- Time to calculate product for 5x3 matrix with batching: `0 ms`
- Time to calculate product for 10x10 matrix with batching: `0 ms`
- Time to calculate product for 50x50 matrix with batching: `2 ms`
- Time to calculate product for 100x100 matrix with batching: `11 ms`
- Time to calculate product for 250x250 matrix with batching: `117 ms`

## Thread pool + Batching

- Time to calculate product for 5x3 matrix with pool: `0 ms`
- Time to calculate product for 10x10 matrix with pool: `0 ms`
- Time to calculate product for 50x50 matrix with pool: `0 ms`
- Time to calculate product for 100x100 matrix with pool: `4 ms`
- Time to calculate product for 250x250 matrix with pool: `108 ms`

## No pool / Batching

- Time to calculate product for 5x3 matrix with array: `0 ms`
- Time to calculate product for 10x10 matrix with array: `0 ms`
- Time to calculate product for 50x50 matrix with array: `2 ms`
- Time to calculate product for 100x100 matrix with array: `9 ms`
- Time to calculate product for 250x250 matrix with array: `120 ms`

## Conclusions

- Bacthing helps _(can't be as parallel as a GPU :()_
- Thread pool helps (prolly better allocation than me when manually creating threads)
- Batching without a pool but better implementation comes close to the pool