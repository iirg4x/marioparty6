# MP6 Cracking Workflow v2

## Required outcome

An owner lane independently turns nonmatching functions into strict/data/physical-relocation exact functions and reports completed `CRACK_REPORT/v1` artifacts. The manager is never on the candidate-selection, compile, proof, or retention path.

The hot loop optimizes exact functions per active hour. It does not optimize approval volume, diagnostics, telemetry, or negative experiments.

## Authority model

Starting an owner task grants one owner-scoped campaign manifest. It binds the owner, unit, source, base commit, target object, toolchain, protected exact functions, allowed paths, forbidden source constructs, proof commands, limits, and cancellation epoch.

This one manifest replaces global `STOP`, per-cell HMAC permits, manager-issued approvals, predicted-row admission, and precompile audits. A lane may compile any natural-C candidate within its owner scope until the campaign is cancelled or closed. Infrastructure failures do not consume candidates; compiled no-gain candidates are deduplicated by hash.

## Lane topology

One Sol orchestrator is the sole live-source writer. Up to five Luna/max
workers run concurrently as end-to-end crackers, each attached to a distinct
open function or bounded decomposition region of a broad function. They are
not approval auditors or five same-function probe slots. Each function gets at
most one evidence-ranked winning cell in a dispatch batch.

The available evidence classes are structural CFG/frame/topology; lifetime,
stack ownership, saved-register allocation, and declaration chronology;
expression trees, operand scheduling, casts, promotions, and constant folding;
ABI, prototypes, inline boundaries, definition visibility, headers, and TU
chronology; and static data, pools, strings, relocations, section ownership,
and linked layout. These classes rank a function's one candidate; they do not
force a serialized syntax matrix.

Each worker may inspect target-first evidence, create an isolated overlay,
compile, measure, and prove its candidate. Baselines, reconstruction,
proposal validation, candidate compilation, and proof hooks overlap across
distinct functions. Workers do not wait for the manager or another worker.
The Sol parent adopts each winner through per-function frontier
compare-and-swap; only short live-source/frontier retention and final linking
are serialized.

## State machine

```text
BOOT -> SNAPSHOT -> DISPATCH -> RUNNING
                         |          |
                         v          v
                 RETAINED/DISCARDED/EXACT/INFRA_RETRY
                         |
                         +-------> DISPATCH

EXACT(function) -> compact CRACK_REPORT/v1 -> next function
all functions exact -> source-link -> protected-sibling -> full-owner/link proof
```

`SNAPSHOT` compiles each current frontier once and caches compact focus
evidence by `(frontier source, target object, toolchain, unit)`. Each function
is an independent streaming pipeline: as soon as its selector has one winner
and its own snapshot is ready, that candidate compiles and runs proof hooks in
an isolated root without waiting for unrelated selectors or snapshots. Up to
five function pipelines run at once. Storage/GC maintenance runs once after the
batch. Only short live-source/frontier publication and final linking are
serialized.

## Cracking loop

1. Select up to five distinct most-crackable functions, or bounded
   decomposition regions for broad functions, from current target-first
   evidence.
2. Reduce each to its earliest independent cause, not a percentage or a full
   mismatch list.
3. Rank one natural-C winning cell per selected function; do not fill unused
   slots with same-function alternatives.
4. Compile the first target-backed candidate for each selected function
   immediately.
5. Measure strict, data, size, physical relocations, and protected siblings
   concurrently across the isolated functions.
6. Retain each safe gain atomically; discard and fingerprint a neutral or
   regression.
7. Continue each function from its retained source without reconstructing or
   reauthorizing another function's frontier. Broad residuals decompose while
   other functions continue.
8. When any function is exact, run its proof ladder and emit its report without
   blocking the other functions.

No declaration matrices, negative controls, or failure-explanation campaigns are required. A failed hypothesis is useful only as a compact dedupe constraint.

## Retention and partial gains

Large functions may require many retained turns. A partial gain is never rolled back merely because the function remains nonexact.

A candidate is retainable only when protected exact siblings have zero losses, already-exact channels remain exact, strict/data/physical differences and absolute data-size error do not increase, and at least one strict row, data row, size distance, or physical distance improves. Source must remain natural C and all source/target/toolchain bindings must remain valid.

Keep one live champion and at most two speculative Pareto frontiers per
function. A stale candidate is revalidated against its immutable base. When
the named function is unchanged, the lane composes it onto the current source,
refreshes baseline and residual evidence concurrently with the other stale
functions, and queues it again. A compact self-hashed tombstone makes the
handoff idempotent. Overlapping edits and ambiguous residual remaps are retired
without a retry loop. Neutral or regressing candidates never change live
source.

Exact means equal function bytes and size, strict/data 100% with zero focus rows, exact physical relocations and effective targets, zero protected-sibling losses, and valid source/object/target/toolchain hashes. Percent alone is never exact.

## Time budget and automatic pivots

The 30-minute budget is a watchdog, not an approval boundary:

- 0–2 minutes: load or create the cached frontier snapshot.
- 2–12 minutes: first parallel batch of up to five distinct function/region
  winners.
- 12–15 minutes: adopt gains immediately and rebase remaining work.
- 15–25 minutes: second wave on new causes or another function.
- 25–30 minutes: exact proof/report or automatic role/function rotation.

Two no-gain candidates in one family close that family. Six compiled candidates or 15 active minutes without a retained gain rotate the function or role. A retained gain resets the counters. Infrastructure retries are automatic and do not count against a hypothesis.

## Compact state and disk limits

Use persistent reusable scratch worktrees, never base/candidate worktrees per cell. Retain only:

```text
owners/<owner>/<function>/latest-frontier.json
owners/<owner>/<function>/frontier.pending.json
owners/<owner>/<function>/latest-failure.json
proof-cas/reports/<prefix>/<report-sha256>.json
owners/<owner>/exact-manifest.json
```

The tracked source is the frontier source; do not duplicate it in retained state.

- scratch: 384 MiB soft, 512 MiB hard;
- one cell temporary output: 64 MiB;
- compact focus evidence: 256 KiB;
- transient measurement envelope (focus plus bound proof bodies): 16 MiB, deleted after CAS;
- frontier and exact report: 64 KiB each;
- negative/dedupe ledger: 1 MiB per unresolved function;
- retained owner state: 16 MiB;
- retained global state: 64 MiB.

After each cell, retain only a compact candidate key, outcome, metric deltas, and reason code. Remove raw full-owner objdiff JSON, compiler logs, candidate sources and objects, and disposable build roots. Exact reports and owner manifests are content-addressed.

Crash recovery uses `frontier.pending.json` plus bound source hashes to deterministically finish or discard publication. It never infers safety from timestamps.

## Reporting contract

The lane sends upward only a completed compact `CRACK_REPORT/v1`, the updated exact/total count, and the final landing/push receipt. Candidate proposals, permits, routine progress, audit packets, negative results, and telemetry remain inside the lane.

The manager creates or resumes tasks, monitors liveness, fixes shared tooling, receives exact reports, and coordinates final landing. The manager does not select functions, source shapes, candidates, or retention decisions.

## Release gates for a candidate workflow release

All gates are blocking:

1. Replay `SetupMgType`, `mbev_CapBomheiMove`, and `ev_CapBobleOMExec` from frozen bases with the manager offline. Reproduce exact source/object hashes, zero strict/data/physical differences, zero sibling losses, and valid compact reports.
2. Each replay completes within 60 seconds wall and 30 seconds command-active; all three finish sequentially within 180 seconds.
3. Run the three replays concurrently plus a duplicate/stale candidate. Require no shared compile lock, corruption, source collision, or cross-lane artifact and at most 1.5× slowdown versus the slowest solo replay.
4. Run one manager-offline live pilot that retains a safe gain within 20 minutes and emits a new exact function report within 60 minutes.
5. Candidate freeze-to-compile latency must have p50 at most 10 seconds and p95 at most 30 seconds; proof-to-retain/report must be at most 10 seconds.
6. Require zero manager per-cell permits, source edits, compiles, or messages between dispatch and exact report.
7. Kill during compile and retention; restart must resume the last valid frontier without duplicate compilation or manual lock repair.
8. Peak ephemeral data must remain at most 512 MiB per lane, retained state at most 16 MiB per owner, and cleanup at most 32 MiB scratch.
9. One false exact, exact-sibling regression, source collision, manager intervention, or missed output/time bound fails release.

These gates quarantine the candidate workflow release, not owner cracking.
Existing owner lanes continue on the last verified release while the candidate
is tested.  No owner waits for another owner's pilot, exact report, or
adoption.  After every gate passes, each lane may adopt the release
independently at its next safe process boundary without rebasing or merging its
source branch. Before the released agent starts, a current-release validation
failure may select the single hash-bound, independently verified prior release.
There is no fallback after child-process start: execution or cleanup failures
stay lane-local and fail closed. A lane-local failure does not pause unrelated
lanes, which continue cracking.

## Migration from the per-cell harness

Preserve exact proof predicates, sibling gates, source-link checks, compact reports, and the hash-bound toolchain manifest. Replace approval/permit/`STOP` admission with owner-scope validation and a cancellation epoch; fold current-residual generation into the reusable baseline cache; use isolated roots and per-function frontier CAS instead of global compile locks; replace one-shot tombstones with candidate-result dedupe; migrate each valid frontier once; import consumed compiled cells; remove approval directories and abandoned per-cell worktrees; and then run every release gate above.
