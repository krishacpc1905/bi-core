# DBLP Sequential Index and Update Timings

## Result

The existing bipartite core-index implementation was run in its sequential
mode on the repository's DBLP dataset. The initial graph was
[`data/dblp/base.txt`](data/dblp/base.txt), with the 1,000-edge batch in
[`data/dblp/sample.txt`](data/dblp/sample.txt) inserted and then deleted.
Each reported value is the average of five runs; `±` is the sample standard
deviation.

| Stage | Internal algorithm time | Whole-command wall time |
|---|---:|---:|
| Initial graph preprocessing | 248.240 ± 19.988 ms | — |
| Initial index construction | **2,822.477 ± 59.638 ms** | 6.794 ± 0.119 s |
| Batch insertion (1,000 edges) | **7.485 ± 0.201 ms** | 1.484 ± 0.045 s |
| Batch deletion (1,000 edges) | **5.071 ± 0.042 ms** | 1.556 ± 0.040 s |

Preprocessing plus index construction took **3,070.717 ± 53.795 ms** in
the program's measured phases. Indexing wall time also includes reading the
edge list and saving the index. Update wall times include loading the index
and update file and saving the resulting index; the internal update times
exclude those I/O costs.

For the fixed 1,000-edge batch, the mean internal times correspond to
approximately 133,597 insertions/second and 197,192 deletions/second. These
are derived from the mean batch latencies, not separate throughput runs.

## Per-run measurements

All time values below are milliseconds except the whole-command wall-time
columns, which are seconds.

| Run | Preprocessing | Initial indexing | Initial wall | Insertion batch | Insertion wall | Deletion batch | Deletion wall |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 283.680 | 2,781.614 | 6.80 | 7.320 | 1.45 | 5.062 | 1.58 |
| 2 | 238.026 | 2,772.909 | 6.68 | 7.453 | 1.46 | 5.016 | 1.55 |
| 3 | 238.187 | 2,864.356 | 6.81 | 7.428 | 1.49 | 5.062 | 1.53 |
| 4 | 243.938 | 2,786.890 | 6.70 | 7.391 | 1.46 | 5.130 | 1.51 |
| 5 | 237.367 | 2,906.618 | 6.98 | 7.834 | 1.56 | 5.086 | 1.61 |

## Dataset, configuration, and validation

- `base.txt`: 12,281,059 edges; index header reports maximum U/V IDs
  1,953,085 and 5,624,219.
- `sample.txt`: 1,000 insertion/deletion edges.
- The experiment uses one initial index construction per run, inserts the
  sample batch, and then deletes the same batch from that inserted graph.
- The index was built with `OMP_NUM_THREADS=1`,
  `OMP_THREAD_LIMIT=1`, and `OMP_DYNAMIC=FALSE`. This implementation includes
  OpenMP preprocessing loops, so this setting makes the build single-worker.
- The dynamic executables used their default sequential `batch_insert()` and
  `batch_delete()` paths; `BI_CORE_PARALLEL` was not enabled.
- Compiler/build: GCC 15.2, `-O3`, C++0x mode, OpenMP enabled at build time.
- Host: Intel Core i7-13650HX, 14 physical cores, 20 available logical CPUs,
  approximately 14 GiB RAM.
- Whole-command wall time was captured with `/usr/bin/time`. The internal
  phase times are the executables' own reported timings.
- All five inserted indexes matched `data/dblp/full.index` byte-for-byte, and
  all five post-deletion indexes matched `data/dblp/base.index` byte-for-byte.
  The freshly constructed initial index also matched `base.index`.
- During the fifth deletion trial, output serialization hit the program's
  short-write assertion while temporary index files occupied most of `/tmp`.
  After removing intermediate scratch indexes, the same deletion was rerun
  successfully; only that successful run is included above.

The observed sequence is therefore:

```text
base graph --insert 1,000 edges--> full graph
full graph --delete the same 1,000 edges--> base graph
```
