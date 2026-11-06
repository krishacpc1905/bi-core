# DBLP Parallel Batch Insertion Timings

## Summary

The existing parallel batch insertion path was run on the repository's DBLP
dataset using the same 1,000-edge insertion batch as the sequential
measurement in [`DBLP_SEQUENTIAL_TIMINGS.md`](./DBLP_SEQUENTIAL_TIMINGS.md).
Each thread configuration was run five times. Timings below are mean ± sample
standard deviation.

| Threads | Batch latency (ms) | Throughput (updates/s) | Speedup vs sequential | Total insertion time, including index load/save (s) |
|---:|---:|---:|---:|---:|
| 1 | 8.130 ± 0.143 | 122,995 | 0.921x | 1.534 ± 0.009 |
| 2 | 8.452 ± 0.300 | 118,310 | 0.886x | 1.604 ± 0.017 |
| 4 | 8.666 ± 0.185 | 115,393 | 0.864x | 1.632 ± 0.029 |
| 8 | 8.605 ± 0.078 | 116,206 | 0.870x | 1.606 ± 0.037 |
| 16 | 9.660 ± 1.389 | 103,520 | 0.775x | 1.610 ± 0.019 |
| 20 | 9.518 ± 0.623 | 105,066 | 0.786x | 1.606 ± 0.011 |

The sequential comparison is the five-run mean of **7.485 ms** from
[`DBLP_SEQUENTIAL_TIMINGS.md`](./DBLP_SEQUENTIAL_TIMINGS.md). Speedup is
sequential latency divided by parallel latency. All observed speedups are
below 1.0, so this parallel path was slower than the sequential baseline for
this workload. One thread was the fastest parallel configuration; higher
thread counts did not improve batch latency.

## Per-run batch latency

All values are algorithm-reported batch latency in milliseconds.

| Threads | Run 1 | Run 2 | Run 3 | Run 4 | Run 5 |
|---:|---:|---:|---:|---:|---:|
| 1 | 8.032 | 8.013 | 8.261 | 8.036 | 8.310 |
| 2 | 8.245 | 8.313 | 8.627 | 8.894 | 8.183 |
| 4 | 8.452 | 8.960 | 8.641 | 8.599 | 8.678 |
| 8 | 8.626 | 8.658 | 8.677 | 8.584 | 8.482 |
| 16 | 9.122 | 9.043 | 12.125 | 8.763 | 9.247 |
| 20 | 9.649 | 8.995 | 9.146 | 10.544 | 9.255 |

## Workload and method

- Starting index: `data/dblp/base.index` (12,281,059 edges).
- Update sequence: `data/dblp/sample.txt` (1,000 insertions).
- Expected result: `data/dblp/full.index`.
- The insertion executable was run with `BI_CORE_PARALLEL=1`,
  `OMP_DYNAMIC=FALSE`, `OMP_NUM_THREADS` set to the row's thread count, and
  `OMP_THREAD_LIMIT` set to that same count.
- The thread counts tested were 1, 2, 4, 8, 16, and 20, up to the 20 logical
  CPUs available on the host (Intel Core i7-13650HX, 14 physical cores).
- Compiler/build: GCC 15.2, `-O3`, C++0x mode, OpenMP.
- Batch latency is the executable's reported `Total query time`; whole-command
  wall time includes index/input loading and output-index serialization.
- Throughput is `1,000 updates / mean batch latency`.
- Speedup is `7.4852 ms / mean parallel batch latency`.
- Every one of the 30 output indexes compared byte-for-byte equal to
  `data/dblp/full.index` (SHA-256:
  `10bce51250e5e5734e9688d357fcf07eae54eac2fdd4a423e5a719b75ef36002`).

## Interpretation and limitations

The repository's parallel insertion path parallelizes endpoint-owned
adjacency-list updates, but seed discovery and core-index maintenance remain
sequential. The measured result is therefore for this existing partial
parallel path, not a fully parallelized core-index algorithm. The results
show that, for this 1,000-edge DBLP batch, parallel setup/merge overhead did
not pay off and increased as thread counts rose. The 16-thread measurements
also had higher run-to-run variance, including one 12.125 ms trial.
