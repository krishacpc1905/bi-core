# Bi-Core Index and Dynamic Maintenance Architecture

## Scope

This repository implements a bipartite graph index and incremental edge
insertion/deletion. The three executables are separate builds of closely
related code:

- `index/` builds the index from an edge list.
- `insert/` loads a saved index and applies an insertion batch.
- `delete/` loads a saved index and applies a deletion batch.

The dynamic implementations are not one shared library: each has its own
`myG.cpp`, headers, file I/O, and executable. Their index records differ;
deletion additionally stores cached degree fields and an initialization flag.
The checked-in code does not contain general clique enumeration or separate
k-truss/k-nucleus executables. Its central maintained structure is the
bipartite core index, including its layer values.

## Graph and index representation

`myG` stores both sides of the bipartite graph in `m_vAdj[2]`:

- Side `1` is U and side `0` is V.
- Each edge occurs in both endpoint adjacency lists.
- `m_vD`, `m_vbA`, and the `m_iMax*` fields hold degrees, active-node flags,
  and graph/index maxima.
- `m_vK` stores the per-node diagonal/core index.
- `m_vIndexA` and `m_vIndexB` store the two directional index sequences.
- Index records contain core number, degree/support information, and layer.
  The delete implementation's `TPST_INDEX` also has `dm`, `dk`, and `bInit`
  fields for cached layer/core degree calculations.
- Candidate records identify a vertex, side, layer, and core-number pair.
  Raw, support, done, and same-layer vectors plus per-node flags drive
  maintenance.

Adjacency lists are kept sorted. `adjInsert` uses binary search and vector
insertion; `adjDelete` searches and erases from the vector. These operations
and their mirrored endpoint updates are not safe to run concurrently on
shared endpoints without coordination.

## Entry points and execution flows

### Index construction

`index/main.cpp` reads the text edge list with `file_readG`, calls
`myG::init()` to sort/deduplicate adjacency lists and compute degrees, calls
`myG::index()` to construct the index, then optionally saves it with
`file_saveBitIndex`.

`myG::index()` initializes the remaining-node sets, repeatedly peels the
diagonal `(k,k)` cores, and then performs the two directional peeling passes
for each successive diagonal level. `peelingUnit()` and `peeling()` remove
vertices in layer/frontier order and record the per-node index values.

The current worktree already contains an uncommitted OpenMP change in
`index/myG.cpp`/`index/Makefile` that parallelizes per-vertex initialization.
That is separate from dynamic update parallelism and should be preserved.

### Batch insertion

`insert/main.cpp` loads the saved graph/index, reads update edges using
`file_query`, times `myG::batch_insert()`, and optionally writes the updated
index.

Sequential `batch_insert()`:

1. Calls `simpleInsert()` sequentially for each edge. This grows arrays for
   new vertex IDs, updates both endpoint adjacency lists and degrees, and
   ignores edges already present.
2. Calls `findSeed()` sequentially for each successfully inserted edge.
   Seed discovery updates degree fields in `m_vK`/directional index records
   and appends candidates to shared `vCandidate` vectors.
3. Calls `maintain(2)`, then `maintain(0)`, then `maintain(1)`. `maintain()`
   sorts and deduplicates candidates by core pair, groups one pair at a time,
   calls `process()` to propagate layer/index changes, and writes the changed
   records back.

### Batch deletion

`delete/main.cpp` has the corresponding load, query, timed update, and optional
save flow, calling `myG::batch_delete()`.

Sequential `batch_delete()`:

1. Calls `simpleDelete()` sequentially for each requested edge. It removes
   both adjacency entries and updates degrees; missing edges are ignored.
2. Calls deletion `findSeed()` sequentially for each removed edge. It checks
   the affected index records and appends candidates. `checkDecLayer()` can
   lazily call `init()`, which writes cached degree fields.
3. Calls `maintain(2)`, `maintain(0)`, and `maintain(1)`. This sorts/groups
   candidates and calls `decProcess()`. The latter performs a core-decrease
   BFS-like propagation, followed by layer re-estimation/propagation; changed
   index entries and additional candidates are then applied.

## Parallelization assessment

The candidate/index loops are not automatically parallel just because they
are loops. The critical dependencies are writes to endpoint adjacency vectors,
per-node index records, per-node flags, and shared work queues.

| Operation | Opportunity | Dependencies and race risk | Safe direction |
|---|---|---|---|
| Batch adjacency update | Potentially parallel over edges with disjoint endpoints | Multiple edges can share either endpoint; each update changes both mirrored vectors, degrees, active flags, maxima, and may race on duplicate checks. New-ID resizes also invalidate concurrent access. | Precompute/resize first, canonicalize updates, then use endpoint ownership or conflict-free edge batches and a controlled merge. Keep each mirrored edge change logically atomic. |
| Insertion seed discovery | Good batch-parallel candidate-generation opportunity | `findSeed()` mutates endpoint degree fields and shared candidate vectors; updates can collide at a vertex and candidates may share index keys. | Use thread-local contribution/candidate buffers; aggregate degree deltas by `(side, vertex, peel type, base k)`, apply them deterministically, evaluate thresholds, then merge/deduplicate candidates. Preserve edge-specific candidate metadata. |
| Deletion seed discovery | Read-heavy work can be parallelized after preparation | `checkDecLayer()` lazily calls `init()`, which mutates cached fields; shared candidate vectors also race. | Initialize every required node/index cache before the parallel read phase; use thread-local candidates and a deterministic merge. |
| Maintenance frontiers | Potentially parallel, but requires the most care | `process()`/`decProcess()` update neighboring degree fields, layers, `m_vbRaw`/`m_vbSup`/`m_vbDone`, and shared frontier queues. Frontier vertices can share neighbors. | Use level-synchronous work: compute thread-local neighbor contributions, reduce by target node, apply updates once per target, and merge new frontier records at barriers. Do not parallelize the current mutation loops directly. |
| Alpha/beta directions | Possible future independent work | Both directions are ordered after the diagonal pass and share per-node state/flags and candidate-processing machinery. Disjoint-looking index vectors alone do not prove independence. | Keep the current order until an explicit read/write-set proof or isolated per-direction state exists. |
| Core-number/candidate groups | Limited without algorithm changes | `maintain()` processes sorted core-pair groups and may create candidates for later groups. Updates can affect neighboring records in those groups. | Preserve sorted sequential group order; parallelize within a group only with the frontier reduction strategy above. |
| Sorting/aggregation | Some parallel potential | Candidate sorting is currently `std::sort`; output order and duplicate handling must stay deterministic. | Profile first; deterministic sort/unique or thread-local aggregation can be parallelized after the larger update costs are addressed. |

The repository does not implement the paper's separate clique-size enumeration
path, so that is not an available optimization in this codebase.

## Implemented parallel path

The original sequential routines remain the default. Set
`BI_CORE_PARALLEL=1` to choose the separate batch-parallel entry points in the
insert/delete executables; `OMP_NUM_THREADS` configures worker count. Both
dynamic Makefiles enable OpenMP.

The implemented safe phase is adjacency maintenance, not κBCO-index
maintenance:

1. Deduplicate the input edge batch while preserving first-seen order.
2. Pre-grow insertion arrays and initialize new endpoint metadata serially.
3. Group updates by U endpoint and by V endpoint.
4. In a U-owned OpenMP pass, each worker owns distinct U adjacency vectors.
   Insertion detects existing edges and merges sorted additions; deletion
   detects present edges and removes them from sorted lists.
5. After the implicit OpenMP barrier, a V-owned pass updates the mirrored
   endpoint vectors for exactly the edges accepted in the U pass.
6. Merge the changed-edge list in input order, update global counts, then
   execute the existing sequential `findSeed()` and κ/β/α maintenance paths.

The two passes prevent concurrent writes to both ends of an edge: each pass
owns one side, with a barrier between them. The `inserted`/`deleted` result
array uses byte elements and each U group writes disjoint edge IDs; it is not
`vector<bool>`, whose packed bits could race. `m_vbA` and new-node index
initialization happen before the parallel loops. Per-node degree writes are
owned by one endpoint group. Small batches run the grouped loops serially to
avoid OpenMP startup overhead.

Seed discovery and index/frontier maintenance remain sequential. `findSeed()`
mutates per-vertex index degrees and shared candidate queues; maintenance
propagates through neighbors and updates shared degree/layer fields and flags.
This implementation does not claim parallel κBCO/onion-layer maintenance.
The next step for that work is a level-synchronous gather/reduce/apply design
with explicit per-frontier barriers and sequential-reference comparisons.

## Technology recommendation

OpenMP is the best first choice for this codebase: it is C++, uses GNU Make
with `g++`, and the current index build already enables OpenMP with
`-fopenmp`. It supports configurable worker counts through
`OMP_NUM_THREADS` and has no additional runtime/library dependency. Use
thread-local buffers and deterministic merges for candidate/contribution
generation; use explicit barriers between maintenance frontiers. Avoid
concurrent mutation of shared `std::vector` objects.

`std::thread` would require more thread lifecycle and reduction code. oneTBB
would add a dependency and build-system changes without an evident need for
its task graph in the initial implementation.

## Baseline and evaluation notes

The recorded original DBLP run in `log_index.txt` reports maximum original
IDs U=1,953,085 and V=5,624,219 (the internal 1-based array extents are
1,953,086 and 5,624,220), 12,282,059 edges, maximum core number 14, and
maximum layer 60. It reports 133.197 ms copy, 1,976.885 ms peeling, and
2,854.409 ms total indexing. `log_insert.txt` records a 7.305 ms update and
`log_delete.txt` records a 5.136 ms update. These are sequential baseline
results and must not be overwritten by later measurements.

The current DBLP split has a 1,000-edge sample. A separate deterministic
10,000-edge holdout and its matching base index were generated under `/tmp`
for local experiments; they are not repository deliverables.

For fair comparisons, build and run sequential and parallel binaries from the
same source revision, use identical graph/index snapshots and update files,
exclude index/file I/O consistently from query timing, and report repeated
runs. Record wall time, speedup, efficiency, thread count, and memory use.
Correctness comparisons should include complete saved-index equality where
applicable, plus small manually constructed and seeded random graphs to cover
duplicates, absent deletions, empty/single-edge batches, shared endpoints,
and conflicting updates.

The completed DBLP batch experiment and its limitations are recorded in
[`EXPERIMENTAL_REPORT.md`](./EXPERIMENTAL_REPORT.md), with the 10,000-edge
thread-count speedup plot in [`benchmark-speedup.svg`](./benchmark-speedup.svg).
Three-run medians showed no consistent speedup: every 10,000-edge parallel
configuration was slower than the sequential reference. Output equality was
verified for sequential/parallel insertion and deletion at 1,000 and 10,000
edges. Seed discovery and κBCO/onion maintenance remain sequential, so these
measurements do not demonstrate parallel index maintenance.

## Suggested implementation sequence

1. Keep the existing sequential routines as the reference path and establish
   repeatable tests for batch update behavior.
2. Extend parallel execution from endpoint-owned adjacency updates to
   thread-local seed candidate generation with deterministic merges, while
   retaining sequential index mutation.
3. Add level-synchronous, conflict-safe frontier processing for κ/core and
   onion-layer maintenance; compare every resulting index against the
   sequential path before benchmarking.
4. Benchmark identical 1, 10, 100, 1,000, and 10,000-edge batches at 1, 2, 4,
   8, and maximum available threads. Publish only measured results and
   identify any batch/thread combinations where parallel overhead loses.
