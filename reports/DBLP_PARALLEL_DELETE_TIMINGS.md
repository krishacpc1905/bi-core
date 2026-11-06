# DBLP Parallel Batch Deletion Timings

## Summary

The existing parallel batch deletion path was run on the repository's DBLP
dataset using the 1,000-edge batch in `data/dblp/sample.txt`. Each thread
configuration was run five times. Timings are mean ± sample standard
deviation.

| Threads | Batch latency (ms) | Throughput (updates/s) | Speedup vs sequential | Total deletion time, including index load/save (s) |
|---:|---:|---:|---:|---:|
| 1 | 5.565 ± 0.142 | 179,695 | 0.911x | 1.522 ± 0.011 |
| 2 | 5.704 ± 0.096 | 175,316 | 0.889x | 1.572 ± 0.005 |
| 4 | 5.798 ± 0.184 | 172,461 | 0.875x | 1.580 ± 0.007 |
| 8 | 5.964 ± 0.060 | 167,684 | 0.850x | 1.598 ± 0.025 |
| 16 | 7.001 ± 1.002 | 142,837 | 0.724x | 1.622 ± 0.059 |
| 20 | 6.803 ± 0.667 | 146,998 | 0.746x | 1.586 ± 0.013 |

The sequential comparison is the five-run mean of **5.071 ms** from
[`DBLP_SEQUENTIAL_TIMINGS.md`](./DBLP_SEQUENTIAL_TIMINGS.md). Speedup is
sequential latency divided by parallel latency. All measured speedups are
below 1.0; the parallel path was slower than the sequential baseline for
this workload. One thread was the fastest parallel configuration, and batch
latency generally increased with additional threads.

## Per-run batch latency

All values are algorithm-reported batch latency in milliseconds.

| Threads | Run 1 | Run 2 | Run 3 | Run 4 | Run 5 |
|---:|---:|---:|---:|---:|---:|
| 1 | 5.737 | 5.400 | 5.438 | 5.644 | 5.606 |
| 2 | 5.690 | 5.589 | 5.696 | 5.856 | 5.689 |
| 4 | 5.700 | 5.632 | 6.110 | 5.779 | 5.771 |
| 8 | 5.973 | 5.926 | 5.964 | 5.898 | 6.057 |
| 16 | 8.777 | 6.788 | 6.479 | 6.435 | 6.526 |
| 20 | 6.399 | 6.709 | 7.941 | 6.715 | 6.250 |

## Workload and method

- Starting index: `data/dblp/full.index` (12,282,059 edges).
- Update sequence: `data/dblp/sample.txt` (1,000 deletions).
- Expected result: `data/dblp/base.index` (12,281,059 edges).
- The deletion executable was run with `BI_CORE_PARALLEL=1`,
  `OMP_DYNAMIC=FALSE`, `OMP_NUM_THREADS` set to the row's thread count, and
  `OMP_THREAD_LIMIT` set to the same count.
- Thread counts were 1, 2, 4, 8, 16, and 20, up to the 20 logical CPUs
  available on the Intel Core i7-13650HX (14 physical cores).
- Compiler/build: GCC 15.2, `-O3`, C++0x mode, OpenMP.
- Batch latency is the executable's reported `Total query time`; total
  deletion wall time includes loading the index and update file and saving
  the output index.
- Throughput is `1,000 updates / mean batch latency`.
- Speedup is `5.0712 ms / mean parallel batch latency`.
- All 30 output indexes compared byte-for-byte equal to `data/dblp/base.index`
  (SHA-256:
  `ab8d8f8e9a7145623c3f234162affc41c26c7d1c1edddb786c55de21abf3f05e`).

## Interpretation and limitations

The existing parallel deletion path parallelizes endpoint-owned adjacency
list updates, while seed discovery and core-index maintenance remain
sequential. The results describe this partial parallel path, not a fully
parallelized core-index algorithm. For this 1,000-edge batch, parallel
overhead outweighed the benefit of the parallel adjacency phase. The 16- and
20-thread runs also show greater variability than smaller thread counts.
