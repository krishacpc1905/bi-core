# Running the DBLP Experiments

Run these commands from the repository root. The DBLP inputs and prebuilt
indexes are under the ignored `data/dblp/` directory; the commands below read
them but do not modify them. Outputs and temporary update lists are written
under `/tmp`.

## 1. Build the executables

```bash
make -C index
make -C insert
make -C delete
```

Requirements: `g++`, GNU Make, OpenMP support, and `/usr/bin/time`.

## 2. Initial index construction

The index builder contains OpenMP preprocessing loops. Set the thread
environment to one for a single-worker run:

```bash
OMP_NUM_THREADS=1 OMP_THREAD_LIMIT=1 OMP_DYNAMIC=FALSE \
  ./index/index.out data/dblp/base.txt /tmp/dblp-base.index
```

Use the following for parallel preprocessing, replacing `8` with the desired
thread count:

```bash
OMP_NUM_THREADS=8 OMP_THREAD_LIMIT=8 OMP_DYNAMIC=FALSE \
  ./index/index.out data/dblp/base.txt /tmp/dblp-base-parallel.index
```

The executable prints preprocessing and index-construction times separately.
To also measure total process wall time:

```bash
OMP_NUM_THREADS=1 OMP_THREAD_LIMIT=1 OMP_DYNAMIC=FALSE \
  /usr/bin/time -f 'wall_seconds=%e' \
  ./index/index.out data/dblp/base.txt /tmp/dblp-base.index
```

## 3. Sequential insertion

The default insertion path is sequential. It starts from `base.index`, inserts
the 1,000 edges in `sample.txt`, and writes a new index:

```bash
env -u BI_CORE_PARALLEL OMP_NUM_THREADS=1 OMP_THREAD_LIMIT=1 OMP_DYNAMIC=FALSE \
  /usr/bin/time -f 'wall_seconds=%e' \
  ./insert/insert.out \
  data/dblp/base.index \
  data/dblp/sample.txt \
  /tmp/dblp-after-insert-sequential.index
```

Compare the result with the expected full index:

```bash
cmp data/dblp/full.index /tmp/dblp-after-insert-sequential.index
```

## 4. Parallel insertion

Set `BI_CORE_PARALLEL=1` to select the parallel batch path. For example, with
20 threads:

```bash
BI_CORE_PARALLEL=1 OMP_NUM_THREADS=20 OMP_THREAD_LIMIT=20 OMP_DYNAMIC=FALSE \
  /usr/bin/time -f 'wall_seconds=%e' \
  ./insert/insert.out \
  data/dblp/base.index \
  data/dblp/sample.txt \
  /tmp/dblp-after-insert-parallel.index
```

Check correctness:

```bash
cmp data/dblp/full.index /tmp/dblp-after-insert-parallel.index
```

Run the tested thread configurations by changing `THREADS`:

```bash
for THREADS in 1 2 4 8 16 20; do
  BI_CORE_PARALLEL=1 OMP_NUM_THREADS="$THREADS" \
    OMP_THREAD_LIMIT="$THREADS" OMP_DYNAMIC=FALSE \
    ./insert/insert.out \
    data/dblp/base.index data/dblp/sample.txt \
    "/tmp/dblp-insert-${THREADS}.index"
  cmp data/dblp/full.index "/tmp/dblp-insert-${THREADS}.index"
done
```

## 5. Sequential and parallel deletion

Deletion uses `full.index` as its starting point and removes the same 1,000
sample edges.

Sequential:

```bash
env -u BI_CORE_PARALLEL OMP_NUM_THREADS=1 OMP_THREAD_LIMIT=1 OMP_DYNAMIC=FALSE \
  /usr/bin/time -f 'wall_seconds=%e' \
  ./delete/delete.out \
  data/dblp/full.index \
  data/dblp/sample.txt \
  /tmp/dblp-after-delete-sequential.index

cmp data/dblp/base.index /tmp/dblp-after-delete-sequential.index
```

Parallel with 20 threads:

```bash
BI_CORE_PARALLEL=1 OMP_NUM_THREADS=20 OMP_THREAD_LIMIT=20 OMP_DYNAMIC=FALSE \
  /usr/bin/time -f 'wall_seconds=%e' \
  ./delete/delete.out \
  data/dblp/full.index \
  data/dblp/sample.txt \
  /tmp/dblp-after-delete-parallel.index

cmp data/dblp/base.index /tmp/dblp-after-delete-parallel.index
```

To run deletion at each tested thread count:

```bash
for THREADS in 1 2 4 8 16 20; do
  BI_CORE_PARALLEL=1 OMP_NUM_THREADS="$THREADS" \
    OMP_THREAD_LIMIT="$THREADS" OMP_DYNAMIC=FALSE \
    ./delete/delete.out \
    data/dblp/full.index data/dblp/sample.txt \
    "/tmp/dblp-delete-${THREADS}.index"
  cmp data/dblp/base.index "/tmp/dblp-delete-${THREADS}.index"
done
```

## 6. Mixed insertion/deletion ratios

The repository has separate insertion and deletion executables rather than a
single mixed-update executable. The mixed measurements apply insertions
first and deletions second, starting from `base.index`. The exact deletion
candidate sequence used for the results is
[`DBLP_MIXED_DELETE_UPDATES.txt`](./DBLP_MIXED_DELETE_UPDATES.txt); the
insertion candidates are the first lines of `data/dblp/sample.txt`.

The following Bash loop runs all requested ratios with exactly 1,000 updates
per ratio. It runs both sequential and 20-thread parallel variants, saves
logs and result indexes under `/tmp`, and checks that the two final indexes
match:

```bash
set -euo pipefail

make -C insert
make -C delete
mkdir -p /tmp/dblp-mixed-runs

make_prefix() {
  local source=$1 count=$2 target=$3
  if (( count == 0 )); then
    : > "$target"
  else
    head -n "$count" "$source" > "$target"
  fi
}

for INSERT_COUNT in 1000 900 800 700 600 500 400 300 200 100 0; do
  DELETE_COUNT=$((1000 - INSERT_COUNT))
  INSERT_PCT=$((INSERT_COUNT / 10))
  DELETE_PCT=$((DELETE_COUNT / 10))
  RATIO="${INSERT_PCT}:${DELETE_PCT}"
  INSERT_FILE="/tmp/dblp-mixed-runs/insert-${RATIO}.txt"
  DELETE_FILE="/tmp/dblp-mixed-runs/delete-${RATIO}.txt"
  make_prefix data/dblp/sample.txt "$INSERT_COUNT" "$INSERT_FILE"
  make_prefix reports/DBLP_MIXED_DELETE_UPDATES.txt "$DELETE_COUNT" "$DELETE_FILE"

  for MODE in sequential parallel; do
    PREFIX="/tmp/dblp-mixed-runs/${MODE}-${RATIO}"
    CURRENT_INDEX=data/dblp/base.index

    if (( INSERT_COUNT > 0 )); then
      INSERT_OUTPUT="${PREFIX}-after-insert.index"
      if [[ "$MODE" == parallel ]]; then
        BI_CORE_PARALLEL=1 OMP_NUM_THREADS=20 OMP_THREAD_LIMIT=20 OMP_DYNAMIC=FALSE \
          /usr/bin/time -f 'wall_seconds=%e' \
          ./insert/insert.out "$CURRENT_INDEX" "$INSERT_FILE" "$INSERT_OUTPUT" \
          2>&1 | tee "${PREFIX}-insert.log"
      else
        env -u BI_CORE_PARALLEL OMP_NUM_THREADS=1 OMP_THREAD_LIMIT=1 OMP_DYNAMIC=FALSE \
          /usr/bin/time -f 'wall_seconds=%e' \
          ./insert/insert.out "$CURRENT_INDEX" "$INSERT_FILE" "$INSERT_OUTPUT" \
          2>&1 | tee "${PREFIX}-insert.log"
      fi
      CURRENT_INDEX=$INSERT_OUTPUT
    fi

    if (( DELETE_COUNT > 0 )); then
      DELETE_OUTPUT="${PREFIX}-final.index"
      if [[ "$MODE" == parallel ]]; then
        BI_CORE_PARALLEL=1 OMP_NUM_THREADS=20 OMP_THREAD_LIMIT=20 OMP_DYNAMIC=FALSE \
          /usr/bin/time -f 'wall_seconds=%e' \
          ./delete/delete.out "$CURRENT_INDEX" "$DELETE_FILE" "$DELETE_OUTPUT" \
          2>&1 | tee "${PREFIX}-delete.log"
      else
        env -u BI_CORE_PARALLEL OMP_NUM_THREADS=1 OMP_THREAD_LIMIT=1 OMP_DYNAMIC=FALSE \
          /usr/bin/time -f 'wall_seconds=%e' \
          ./delete/delete.out "$CURRENT_INDEX" "$DELETE_FILE" "$DELETE_OUTPUT" \
          2>&1 | tee "${PREFIX}-delete.log"
      fi
      CURRENT_INDEX=$DELETE_OUTPUT
    else
      mv "$CURRENT_INDEX" "${PREFIX}-final.index"
      CURRENT_INDEX="${PREFIX}-final.index"
    fi
  done

  cmp \
    "/tmp/dblp-mixed-runs/sequential-${RATIO}-final.index" \
    "/tmp/dblp-mixed-runs/parallel-${RATIO}-final.index"
  printf 'ratio=%s correctness=PASS\n' "$RATIO"
done
```

Each log contains the operation's `Total query time` and the GNU `time`
`wall_seconds` measurement. For mixed ratios, add the insertion and deletion
`Total query time` values to get total algorithm batch latency. Wall times
include reading and writing the intermediate index between operations.

## 7. Reading the results

- `Total query time` is the algorithm-reported update latency. It excludes
  input index loading and output index saving.
- `wall_seconds` is whole-command wall time, including process startup and
  input/output work.
- For a mixed ratio, sum the insert and delete query times; do not compare one
  stage alone with the full mixed batch.
- `cmp` exits successfully only if two index files are byte-for-byte equal.
- Parallel mode in this repository parallelizes adjacency-list updates;
  seed discovery and core-index maintenance remain sequential. The mixed run
  is therefore two sequentially applied update phases, not a simultaneous
  mixed-update algorithm.

Previously collected results are in [`DBLP_SEQUENTIAL_TIMINGS.md`](./DBLP_SEQUENTIAL_TIMINGS.md),
[`DBLP_PARALLEL_INSERT_TIMINGS.md`](./DBLP_PARALLEL_INSERT_TIMINGS.md),
[`DBLP_PARALLEL_DELETE_TIMINGS.md`](./DBLP_PARALLEL_DELETE_TIMINGS.md), and
[`DBLP_MIXED_RATIO_RESULTS.md`](./DBLP_MIXED_RATIO_RESULTS.md).
