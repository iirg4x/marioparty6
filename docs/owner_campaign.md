# Autonomous MP6 owner campaign (v2)

This is the v2 entry point for cracking one MP6 owner. The campaign manifest
uses schema `owner_campaign/v1`; retained partial progress uses
`crack_frontier/v2`. The current per-cell harness remains documented in
[`crack_harness.md`](crack_harness.md) for legacy replay and migration only.
The normative workflow contract is
[`MP6_CRACKING_WORKFLOW_V2.md`](../MP6_CRACKING_WORKFLOW_V2.md).

## Campaign scope

Starting an owner task grants one owner-scoped campaign manifest. The manifest
hash-binds:

- owner, slash-form objdiff unit, source path, base commit, target object, and
  toolchain;
- the functions in scope and protected exact functions;
- allowed source/build paths and forbidden source constructs;
- snapshot, candidate, and final-owner proof commands; and
- a cancellation epoch and bounded time/storage limits, including a 256 KiB
  focus-evidence cap for compact residuals.

The lane may compile any natural-C candidate inside that scope until the
campaign is cancelled or closed. A manager is not on the candidate-selection,
compile, proof, or retention path. There is no global `STOP`, per-cell HMAC
permit, manager-issued approval, predicted-row admission, or precompile audit.
Infrastructure failure does not consume a candidate. A compiled no-gain
candidate is keyed by hash and is not repeated.

The intended lane entry point is:

```sh
rtk python tools/agent.py crack loop --campaign <campaign-manifest.json>
```

This is a live supervisor: it polls a bounded batch of independently produced
sealed proposals, selects at most one evidence-ranked winner for each distinct
function, and dispatches up to five functions concurrently. It continues until
the campaign closes, is
cancelled, reaches its bounded idle/watchdog policy, or encounters a terminal
infrastructure failure. The default idle timeout is 60 seconds and the
idle timeout and watchdog are both 30 minutes; use `--idle-timeout`, `--watchdog-seconds`, and
`--poll-interval` to tune a bounded run. For a single administrative/test
snapshot, add `--once`; that mode dispatches one batch and returns immediately.

The manifest itself is the authority boundary. Helpers must remain within its
path, command, source-shape, hash, and cancellation-epoch bindings.

### Initialize once

Campaign setup can be done once by the Sol parent from a draft. The draft must
include the owner identity, base commit, source/function scope, target and
toolchain bindings, the measurement-producer binding, and all three commands
(including `final_owner`); the initializer hashes the files, writes the
manifest atomically, and validates it with the campaign loader:

```sh
rtk python tools/agent.py owner-campaign initialize \
  --campaign build/owner-campaign/captrap.json \
  --draft build/owner-campaign/captrap.draft.json \
  --measurement-producer tools/owner_campaign_measure.py
```

The output is the compact campaign identity and manifest hash. A second call
with only the existing `--campaign` path is idempotent; changed or
hash-drifting draft inputs are rejected before replacing the manifest. When a
draft omits snapshot/candidate argv, the initializer supplies the production
`owner_campaign_measure.py` command, but it never invents the required
`final_owner` command.

### Bootstrap the first frontier

Before emitting any candidate descriptor, the Sol parent must establish each
selected function's current frontier. This is a reusable baseline boundary for
the whole batch: it binds the live source, target, toolchain, function, and
compact focus evidence once, and makes a repeated call a no-op while that
frontier is still current:

```sh
rtk python tools/agent.py owner-campaign snapshot \
  --campaign build/owner-campaign/<owner>.json \
  --function <function>
```

Bootstrap up to five distinct functions concurrently by repeating
`--function`; each function receives a distinct scratch worker, and output is
returned in the requested function order:

```sh
rtk python tools/agent.py owner-campaign snapshot \
  --campaign build/owner-campaign/<owner>.json \
  --function <function-a> \
  --function <function-b> \
  --function <function-c> \
  --workers 3
```

`--workers` must be between 1 and 5 and bounds each bulk group. Duplicate or
out-of-manifest functions fail before dispatch. For a single function,
`--worker 0..4` selects its isolated scratch explicitly; `reconstruct` accepts
the same single-function `--worker` option.

The command prints only the compact `owner_campaign_snapshot/v1` identity and
`focus_evidence_sha256` binding (or an ordered
`owner_campaign_snapshots/v1.snapshots` list for a bulk request). It does not dispatch candidates, read or
write legacy permits/STOP state, or enter the candidate loop. Workers consume
that returned frontier identity before writing their distinct descriptors to
the inbox.

### Target-first reconstruction

Candidate discovery starts from the retail object, not repository history.  A
snapshot produces a content-addressed
`owner_campaign_reconstruction_packet/v1` alongside the compact frontier.  The
packet binds the source, target and candidate objects, toolchain, source span,
residual identities, physical relocations, and bounded target/candidate
instruction windows.  It also records call/branch chronology and stack-relative
accesses that can be proved from the instruction stream.

Every cracking helper must read that packet before proposing source.  A
`READY` packet qualifies one natural-C change tied to one causal cluster.  A
broad-residual `UNKNOWN` packet is still actionable: it carries bounded
representative clusters/windows, full residual counts and digests, compact
whole-function frame/save-set/CFG/call/stack facts, and
`target_first_signal.next_action=DECOMPOSE`; the lane must crack those bounded
regions one at a time instead of abandoning the owner.  An `UNKNOWN` packet
whose signal is `PIVOT` means the required evidence is genuinely absent or
ambiguous, so the lane pivots without compiling a blind control or syntax
matrix.  History, donors, and prior source may corroborate a target-first
reconstruction, but they are optional and cannot block or authorize a proposal.
Static register names and stack offsets remain target pseudo-owners unless a
same-session compiler trace proves the source-to-vreg join.

The reconstruction packet is diagnostic only.  It never emits a source patch,
authorizes a compile, retains a candidate, or advances authority.  Exactness
still requires the normal strict/data/physical/sibling/source-link proof ladder.

### Sol lane protocol

The Sol parent owns orchestration, not candidate selection by management. At
startup it may fill up to five Luna/max worker slots with distinct open
functions (or bounded decomposition regions of broad functions), then collect
their sealed `owner_campaign_candidate/v1` descriptors in the campaign inbox:

```text
build/owner-campaign/inbox/<campaign-slug>/*.json
```

The available evidence classes are CFG/frame/topology;
lifetime/stack/register chronology; expression scheduling/types/promotions;
ABI/inline/header/TU visibility; and static data/pools/relocations/layout.
They are ranking labels, not a five-way same-function probe matrix. Each worker
independently reads its function's current reconstruction packet, reconstructs
one natural-C boundary from target evidence, and writes only a sealed
descriptor plus its source under the campaign's allowed build roots.
Donor/history search may corroborate a reconstruction but is never required.
The Sol parent then runs the inbox command above. It may continue from retained
frontiers, decompose a broad residual while other functions proceed, and pivot a
function after the configured time/no-gain budget. Python does not create or
control Codex subagents; the Sol parent performs that delegation in the task
runtime.

The inbox driver groups proposals by function and runs up to five independent
`select -> validate -> snapshot -> compile/proof` pipelines. A ready function
does not wait for an unrelated slow selector or snapshot. Each pipeline
arbitrates at most one winner for its function; the batch never spends five
slots on same-function syntax variants. Terminal descriptors and unshared candidate sources are deleted after measurement;
`infra_retry` descriptors and sources stay in place for retry. Empty inboxes
return an explicit `idle` result, not a false successful empty batch. The only
upward lane message after success is the completed, evidence-bound
`CRACK_REPORT/v1` (plus owner landing/push receipt when closure is complete).

## Lane topology

One Sol orchestrator is the only live-source writer. Up to five Luna/max
workers run end-to-end cracking lanes in parallel, each attached to a distinct
function or bounded decomposition region; they are not approval auditors or
same-function probe slots. The evidence classes above remain available when a
worker ranks its one winning source boundary.

Each worker may inspect evidence, create an isolated overlay, compile, measure,
and propose retention. Baselines, reconstruction packets, proposal validation,
candidate compilation, and proof hooks for distinct functions overlap as
streaming per-function pipelines. Workers do not wait for the manager, for the
slowest selector, or for the slowest baseline. GC and storage-limit maintenance
run once at the batch tail, outside the source/frontier hot path. The Sol parent
adopts each winner with a per-function frontier compare-and-swap (CAS), so only
the short live-source/frontier publication and final link are serialized.

A reconstruction assignment has a ten-minute search lease. Before that lease
expires, its worker returns exactly one current-frontier-bound natural-C
candidate or `NONE`. A freed slot is reassigned immediately to another function
or causal region; the parent never waits for the other four searches before
submitting a ready descriptor. `NONE` closes only the searched evidence class,
not the function, and therefore triggers target-region decomposition rather
than a donor/history retry or a same-shape syntax matrix.

## Baseline and cracking loop

`SNAPSHOT` compiles each current frontier once and stores compact focus evidence
keyed by `(frontier source, target object, toolchain, unit)`. Snapshots and
target-first reconstruction packets for distinct functions are produced in
parallel. Candidate cells compile only their candidate against the reusable
baseline; selection validation and candidate measurement/proof also overlap
across isolated function roots. A candidate does not reconstruct or
reauthorize another function's retained frontier.

The state machine is:

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

For each dispatch:

1. Select the most crackable remaining function, or the next bounded
   decomposition region for a broad function, from the current target-derived
   reconstruction packets.
2. Reduce it to the earliest independent target/candidate dataflow cause,
   rather than a percentage or a full mismatch list.  A broad packet's
   decomposition regions are the bounded work units; do not expand them back
   into a full-function syntax search.
3. Give each selected function one evidence-ranked source boundary; evidence
   classes are not serialized same-function alternatives.
4. Reconstruct and compile the first target-backed natural-C candidate for each
   selected function immediately. Do not compile controls merely to prove
   alternatives wrong.
5. Measure strict, data, size, physical relocations, and protected siblings in
   parallel across functions.
6. Retain each safe gain atomically; discard and fingerprint a
   neutral/regression.
7. Continue each function from its retained source. A retained gain resets that
   function's pivot counters.
8. When a function is exact, run its proof ladder and emit its report without
   blocking other functions.

The 30-minute watchdog is not an approval boundary. Two no-gain candidates in
one hypothesis family close that family; six compiled candidates or 15 active
minutes without a retained gain rotate the function/role. Infrastructure retry
is automatic and does not count against a hypothesis.

## Frontier retention and recovery

A candidate is retainable only when protected exact siblings have zero losses,
already-exact channels remain exact, strict/data/physical-relocation
differences and absolute data-size error do not increase, at least one strict
row/data row/size distance/physical distance improves, and natural-C plus all
source/target/toolchain bindings remain valid.

The campaign keeps one live champion and at most two speculative Pareto
frontiers per function. A retained frontier records its parent frontier hash,
generation, source/object/target/toolchain hashes, metrics, report receipts,
and its own `frontier_sha256`. A candidate that becomes stale is revalidated
against its immutable base. When the named function itself is unchanged, the
lane transplants that function edit onto the current source, refreshes the
function baseline and residual evidence, and queues it again without another
manager decision. Refresh and rebase work runs concurrently across distinct
functions. A compact self-hashed tombstone makes the handoff idempotent;
overlapping edits and ambiguous row remaps are retired without a retry loop.
Neutral or regressing candidates never change the live source.

Keep only this compact state:

```text
owners/<owner>/<function>/latest-frontier.json
owners/<owner>/<function>/frontier.pending.json
owners/<owner>/<function>/latest-failure.json
proof-cas/reports/<prefix>/<report-sha256>.json
owners/<owner>/exact-manifest.json
```

The tracked source is the frontier source; retained state does not duplicate
it. `frontier.pending.json` records a publication that may be completed or
discarded after a crash. Recovery checks the bound base/candidate source hashes
and deterministically finishes or discards that publication; it never infers
safety from timestamps.

The storage contract is:

| Resource | Bound |
| --- | ---: |
| Reusable worker scratch (soft / hard, per worker) | 384 MiB / 512 MiB |
| One cell temporary output | 64 MiB |
| One focus-evidence artifact | 256 KiB |
| Transient measurement envelope | 16 MiB |
| One frontier | 64 KiB |
| One exact report | 64 KiB |
| Unresolved-function negative/dedupe ledger | 1 MiB |
| Retained owner state | 16 MiB |
| Retained global state | 64 MiB |

After each cell, retain only the compact candidate key, outcome, metric deltas,
and reason code. Remove raw full-owner objdiff JSON, compiler logs, candidate
source/object copies, and disposable build roots. Exact reports and owner
manifests are content-addressed and are not garbage-collected.

## Exact proof and reporting gates

Percentages alone do not establish exactness. A function is exact only when
function bytes and size are equal; strict and data channels are 100% with zero
focus rows; physical relocations and effective targets are exact; protected
sibling losses are zero; and source, object, target, and toolchain hashes are
valid. The proof ladder then requires source-link, protected-sibling, and
full-owner/link proof.

When the exact predicate passes, the lane automatically writes one compact,
hash-bound `CRACK_REPORT/v1`. Upward communication is limited to that completed
report, the updated exact/total owner count, and the final owner landing/push
receipt. Candidate proposals, permits, routine progress, audit packets,
negative results, and telemetry stay inside the lane.

Before four additional Board owner lanes may start, all release gates in the
v2 contract must pass: manager-offline exact replays of `SetupMgType`,
`mbev_CapBomheiMove`, and `ev_CapBobleOMExec`; the 60/30-second per-replay and
180-second sequential bounds; concurrent replay plus duplicate/stale-candidate
isolation and 1.5x slowdown bound; a manager-offline live pilot with a retained
gain and a new exact report; candidate/proof latency bounds; zero manager
intervention; kill/restart frontier recovery; and the 512 MiB reusable-lane,
16 MiB retained-owner, and 64 MiB retained-global storage bounds. Raw cell
outputs must be removed after every measurement; the reusable compiler checkout
is deliberately retained and remains subject to the lane bound. Any false exact, sibling loss,
source collision, intervention, or missed output/time bound is a release
failure.

## Migration from the per-cell harness

The legacy `STOP`/HMAC permit/approval path is not the v2 authority model. The
migration preserves exact proof predicates, protected-sibling gates,
source-link checks, compact reports, and the hash-bound toolchain manifest,
then:

1. adds `owner_campaign/v1`, `crack_frontier/v2`, and the owner-loop command;
2. replaces approval/permit/`STOP` admission with owner-scope checks and the
   cancellation epoch;
3. folds current-residual generation into the reusable baseline cache;
4. uses isolated build roots and per-function frontier CAS instead of global
   transaction/compile locks;
5. deduplicates candidate results and retries infrastructure failures rather
   than using one-shot cell tombstones; and
6. imports each valid old frontier once, imports consumed compiled cells into
   the dedupe ledger, removes approval directories and abandoned per-cell
   worktrees, and runs the release gates.

The offline migration entry point is:

```text
rtk python tools/agent.py --root . owner-campaign import \
  --campaign build/owner-campaign/config.json \
  --legacy-exact build/legacy/exact-function.json \
  --legacy-consumed build/legacy/consumed-cell.json
```

The importer accepts the compact `owner_campaign_legacy_exact/v1` receipt and
the old harness `CRACK_REPORT/v1` exact report. It validates the campaign
owner/unit/base commit, clean source, target and toolchain hashes, strict/data
zero rows, equal bytes, exact physical relocation count, zero protected
losses, and all bound legacy proof summaries before publishing v2 CAS and an
exact manifest. A compiled `owner_campaign_legacy_outcome/v1` (or old
`crack_harness_result/v1`) with `no_gain`/`stale` status is written to the
function's dedupe ledger only; infrastructure failures and uncompiled cells
are not consumed, and improved legacy outcomes are not promoted. Inputs are
read-only and the import does not consult STOP or create permits. The
transaction is idempotent and rolls back newly published state on failure.

Existing lanes stay paused until those gates pass. The old harness page remains
available only for legacy replay and migration reference.
