# Approved crack harness

`tools/crack_harness.py` is the only execution front door for a resumed crack
cell. It schedules no lanes and never asks a user for authentication. Production
state is fixed at `build/crack-harness`; only Python tests may inject another
state root.

> **v2 migration:** This page documents the legacy per-cell approval,
> `STOP`, and HMAC-permit path. It is not the owner-campaign entry point. See
> [`owner_campaign.md`](owner_campaign.md) for the owner-scoped manifest,
> autonomous five-lane loop, frontier CAS, compact state, and release gates;
> keep this page for legacy replay and migration reference until those gates
> pass.

## Manager-only current residual baseline

`tools/crack_current_residual.py` is a manager-only baseline materializer for
the approval's current source. It compiles that current source once in a
detached disposable worktree under the serialized build lock, then publishes a
compact, current-source-bound `crack_current_residual_evidence/v1` artifact
with focus, physical, and object evidence plus bounded report receipts. It
requires residual rows; an already exact current base is not
materialized. It does not admit or compile a candidate, create an approval,
permit, admission, or history record, or advance authority (`authority_advanced`
is `false`). The artifact is bound into the winning-cell selection and supplies
the rows that approval predictions may name.

Use repository-relative placeholders for the owner, unit, function, hashes, and
span; do not guess a function-specific span:

```sh
rtk python tools/crack_current_residual.py \
  --root . \
  --base-commit <40-hex-commit> \
  --owner <owner> \
  --unit <slash-form-objdiff-unit> \
  --function <C-function> \
  --source <src/path.c> \
  --source-sha256 <64-hex-source-sha256> \
  --target-sha256 <64-hex-target-object-sha256> \
  --toolchain-key <64-hex-toolchain-manifest-sha256> \
  --span-start <START_LINE> \
  --span-end <END_LINE> \
  --output <build/current-residual.json>
```

## STOP and assignment permit

The manager creates the global `build/crack-harness/STOP` authorization when
cracking is stopped. Resuming one function does not remove STOP. At lane
assignment the manager first issues a closed, HMAC-SHA256-signed
`crack_harness_resume_permit/v1`; STOP then authenticates its exact file SHA-256
and 256-bit `stop_nonce`. The permit
binds owner, task, function, campaign, approval ID, cycle-free approval identity,
command-set identity, source path, source/base/candidate hashes, base commit,
toolchain, target, `issued_at`, and a deadline no more than 1,800 seconds later.
One permit authorizes one reviewed candidate cell. The later approval binds the exact permit
SHA-256, so analysis time counts and neither approval nor candidate may drift. STOP
is revalidated before every command (including admission discard), CAS, central
record, and terminal write.
The permit deadline must be no later than the bound approval expiry; both clocks
are rechecked at every checkpoint immediately before execution and retention.

The fixed issuer is `mp6-crack-manager`. The manager key is exactly 32 raw random
bytes in
`C:\Users\Anony\.codex\manager-secrets\mp6-crack-harness.key`; its fixed key ID
is `7a40e303f395fcfb25894819a19ad75488430e16a7d590aa1bc6370738b8591f`.
The key is outside the repository and is never inherited by child commands. The
permit fields are `schema`, `permit_id`, `issuer`, `resume` (`true`),
`owner`, `task_id`, `function`, `campaign_id`, `approval_id`,
`approval_identity_sha256`, `commands_sha256`, `source_relpath`,
`source_sha256`, `base_sha256`, `candidate_sha256`, `base_commit`,
`toolchain_key`, `target_sha256`, `stop_nonce`, `issued_at`, `deadline`,
`key_id`, and `signature`. `signature` is lowercase hex HMAC-SHA256
over canonical compact JSON of every other permit field. STOP contains only
`schema`, `stopped` (`true`),
`authorized_permit_sha256`, and the same `stop_nonce`.

The manager materializes the packet through the production issuer; lanes do not
handcraft HMACs or edit STOP:

```sh
python tools/agent.py crack issue --draft build/approvals/cell.draft.json --approval-out build/approvals/cell.json --permit-out build/approvals/cell.permit.json
```

The draft uses a zero `permit_sha256` placeholder. The issuer validates the
cell and clean repository, writes the signed permit, binds its raw file SHA into
the final approval, publishes STOP last, reloads all three through production
validators, and returns a dry-run-ready command. Any partial publication
restores the prior STOP and removes the new outputs. If either rollback step
fails, the issuer writes a bounded self-hashed
`build/crack-harness/PACKET_ROLLBACK_REQUIRED.json`, revokes STOP, and blocks
issue, dry-run, and run until a manager repairs the named paths.

This protects against forged lane permits and repository-controlled keys. It is
not a security boundary against a malicious process already running as the same
Windows user, which can read that user's files despite ACLs. The operating
boundary is a non-malicious same-user lane plus an ACL-restricted manager key;
no lane prompts the user or accepts a self-asserted issuer.

## Approval and commands

Validate approval with `tools/CRACK_HARNESS_APPROVAL_V1.schema.json`. It binds
Git `base_commit`, configure `unit`, the SHA-256 key of the closed central
toolchain manifest, target, source/baseline/candidate hashes, a
hash-bound function span, predicted rows, limits, and every command descriptor.
`unit` is the generated slash-form objdiff name (for example,
`main/board/captrap`), not the colon-form owner identity. The harness and
evidence bundle share one side-effect-free validator, so malformed, traversing,
owner-style, empty-segment, and leading/trailing-slash unit names fail during
dry-run before permit use.
Limits cannot be elevated: each `(base_sha256, candidate_sha256)` pair is
one-shot for an owner/function (a new campaign ID cannot retry that same
base+candidate attempt), at most 1,800 seconds, 512 MiB ephemeral data, and
16 MiB retained compact state per stable owner across all campaigns. A positive
safe improvement is retained as the one current signed, monotonic frontier for
that function;
a later permit may continue from that frontier/current source with a new
candidate. Only an exact result with a valid, bound `CRACK_REPORT/v1` closes the
function.

The UTF-8 natural-C cell may use at most three hunks and 80 changed lines,
including insert/delete, wholly inside the function span. NUL, preprocessor
directives, structural changes to the approved function boundary, and nested
function definitions are rejected; legitimate body edits and nested control
blocks remain allowed. asm, volatile/register shaping, padding, dead branches,
and inline/optimization forcing are also rejected.

Every approval also carries a closed `selection` object. Its strategy is fixed
to `winning_cell_first`, its rank is exactly `1`, and it must name a non-empty
natural-C `source_class`. `expected_terminal` is either `exact` or `improved`;
the former predicts a fully exact function and the latter predicts one positive,
safe, measurable frontier improvement. Every result repeats that prediction and
seals `terminal_expectation_met`: `exact` is satisfied only by an exact result,
while `improved` is satisfied by either an improved or exact result. A safe
partial gain from an exact-predicted cell is still retained monotonically, but
its result explicitly marks the exact prediction unmet; the harness never
silently downgrades an exact prediction or rolls back useful progress. An incremental, exploratory, or
negative-control cell is still rejected before permit use. The selection binds
an existing repository-local
`crack_winning_cell_evidence/v2` artifact by path and SHA-256, the approval
candidate SHA, and the canonical SHA-256 of `predicted_rows`. Validate the
artifact with `tools/CRACK_WINNING_CELL_EVIDENCE_V2.schema.json`. Its closed
contents repeat and must exactly match owner, function, candidate, predicted-row
digest, rank, strategy, controls, pivot decision, and source class; it also
binds 1-16 repository-local evidence inputs by path/role/hash and carries an
explicit earliest divergence, predicted effect, and exact predicted row list.
`predicted_rows` is non-empty and unique in both the approval/result schemas and
the runtime validators.
The harness parses and verifies those contents and every input hash rather than
accepting the artifact as an opaque assertion. `alternatives_compiled` and
`negative_controls` are both exactly `0`, and `pivot_if_unranked` is `true`.
Evidence inputs must be immutable: the approval's separately hash-bound live
source is forbidden as an evidence input, including through a hard-link alias.
Use the sealed base artifact when the current source bytes are evidence. This
prevents a valid base hash from becoming a deterministic post-retention failure
when the harness atomically applies an exact or improved candidate.
The complete approval—including this selection—is included in the permit
identity and therefore in the manager signature binding. Missing, drifted,
or mismatched selection data is rejected before admission. There is no
fallback compile for an unranked cell: without a valid signed approval and
permit, STOP remains in force and the harness does not run.

The selection also binds one `crack_luna5_audit/v1` artifact. It must contain
five distinct read-only Luna/max PASS receipts for the fixed roles
`exact_candidate_recovery`, `source_provenance`, `retry_safety`,
`permit_pipeline`, and `adversarial_security`. Every receipt binds the same
controller commit and candidate, a unique agent and immutable artifact, and
proves that the auditor neither compiled nor mutated source. Duplicate agents,
roles, outputs, commit drift, non-max effort, or a non-PASS result fail before
permit use.

Descriptors hash-pin executable and script. The fixed registry requires
`candidate_compile_admission.py` for admission/record,
`crack_evidence_bundle.py` for compile/evidence, and the exact
`crack_harness.py proof-adapter` front door for
proof/assessment extraction. The adapter derives closed payloads from real
hash-bound baseline/candidate objdiff reports, objects, source, and the physical
receipt through `focus_symbol_report.py`; arbitrary repo-local hooks are rejected.
The only accepted manifest key is
`b6764a1e5883ea1a096bfe4f8b888b93f1740f0f4046eb6149e0fe1d64cc6d90`,
which includes pinned Ninja 1.13.2. Admission must be the exact supported
`tools/candidate_compile_admission.py admit` argv. Other argv use `{RUN_ROOT}`
and `{OUT_ROOT}` for disposable inputs/outputs. The hash-pinned compile/proof
scripts themselves resolve only through `{CONTROLLER_ROOT}`, so a detached base
commit cannot substitute obsolete or absent harness tooling.

## Execution and proof

```sh
python tools/agent.py crack dry-run --approval build/approvals/cell.json
python tools/agent.py crack run --approval build/approvals/cell.json --permit build/approvals/assignment.permit.json
python tools/agent.py crack status
```

`run` holds one transaction lock through terminal output, verifies clean tracked
state and exact HEAD, creates a detached worktree under monitored temp, overlays
the candidate, and runs compile/proofs there. Before overlay it runs the fixed
`crack_evidence_bundle.py` front door under the harness's serialized build lock
with `CRACK_HARNESS_PHASE=baseline`; after overlay it reruns with phase
`candidate`. A hash-bound approval context supplies owner/function/unit/source,
commit, source hashes, target hash, and the exact central toolchain-manifest
SHA-256. A label or version string is not a valid `toolchain_key`.
The bundle must write
`target.o`, `baseline-candidate.o`, `candidate.o`, baseline/candidate `strict`
and `data` JSON, `baseline-physical.json`, `physical.json`, self-digested phase receipts, and a final
self-digested evidence context under `CRACK_HARNESS_OUT_ROOT`. The harness
checks every receipt identity, phase nonce, artifact hash/size, target hash, and
baseline immutability. Missing, stale, mixed, or fabricated evidence fails closed.
The signed permit is recorded as one-shot when execution begins. The approved
base+candidate attempt is reserved only after the candidate process has been
created and assigned to containment, and its durable marker is published before
the contained process is resumed. Admission, worktree, unit, configuration,
baseline, assignment, or resume/setup infrastructure failures therefore require
a fresh signed permit but cannot consume an uncompiled base+candidate attempt; a
published pre-resume reservation is rolled back when resumption fails.
Temp is metered and production
writes are polled and rejected. Combined streamed output is capped at 1 MiB;
timeout/overrun kills the process tree. On Windows the root process is created
suspended, assigned to a kill-on-close Job, and only then resumed, closing the
fast-child spawn race. Termination is followed by a bounded root wait and Job
active-process census; the Job is not closed until all descendants are quiescent.
Every filesystem component, including `.git`, is checked for symlink/reparse
indirection and monitored. Child environments cannot override Git, recovery
queue, or recovery-memory locations. The worktree is force-removed and
pruned. Incomplete disposable cleanup is retried under STOP at startup.

Typed proofs bind owner/function, candidate source, approved target object,
candidate object, and report hash; assessment binds the same source/object pair.
Every numeric proof and assessment field must be finite; JSON `NaN` and
infinities are rejected before any comparison or retention decision.
Exact requires strict/data 100%, exact equal byte counts and zero differences,
zero focus rows, zero protected-sibling losses, and equal physical counts with
zero differences. An `improved` result is deliberately allowed to retain
nonzero focus rows, physical-relocation residuals, or a changed function size,
but only when owner gain is positive, physical distance from the target does not
increase, protected siblings do not lose exactness, and data does not regress.
Physical distance is `abs(target_count - candidate_count) + differences`, where
`differences` is the number of nonidentical physical-relocation entries.
Size distance is `abs(target_bytes - candidate_bytes)`. Assessment binds both
the sealed baseline and candidate data byte counts and supplies
`size_diff_delta = candidate_size_distance - baseline_size_distance`.
Assessment supplies the strict-score gain plus data/size/physical deltas;
the runtime rejects non-finite or non-positive gains. Improved cells are
measurable progress, not proof of a crack: they are copied into the live source
frontier, do not invoke central `record`, and do not produce a
`CRACK_REPORT/v1`. The admission token/input key and object pair must still be
bound by typed receipts, and protected sibling proof covers the union of strict
and data exact-identity sets.

After an improved cell, the harness keeps exactly one overwrite-only,
self-hashed compact frontier for that owner/function. Validate frontier files
with `tools/CRACK_HARNESS_FRONTIER_V1.schema.json`; the manager HMAC and
`frontier_sha256` digest cover the retained body. The signed assessment stores
the baseline/candidate byte counts and the changes in size and
physical-relocation distance. Frontier validation recomputes the size delta
from those bound byte counts; the runtime rejects a positive size or physical
delta. Equal nonzero size distance is retainable only when another measurable
owner gain is positive and every other nonregression gate passes. The harness removes the run
directory, disposable inputs, logs, and per-candidate source copy. A later
candidate may continue from the signed frontier/current source: its approved
base must equal that source, and its base+candidate pair must be new. A
pre-size-distance frontier lacks these signed byte counts and is intentionally
invalid; rematerialize it from the retained source before continuing.
no_gain or failed cell leaves the prior frontier untouched, removes its
disposable state, and consumes only that same base+candidate attempt; it does
not close the function or force a pivot. A later exact result closes the
function only when its valid bound `CRACK_REPORT/v1` is sealed. Pre-candidate
infrastructure failure consumes neither a candidate attempt nor the function's
retained frontier. This report is exact-only. No full source
duplicate, per-attempt log, append-only candidate history, or raw compiler
history survives. Approval, baseline, candidate, permit, worktree, objects,
logs, and temp are deleted after each cell. Owner state is hard-capped at 16
MiB and all harness state at 64 MiB. Journal recovery restores an interrupted
CAS. If an exact central row committed before the complete local terminal was
sealed, recovery preserves the candidate, central row, journal, and a self-hashed
`RECOVERY_REQUIRED` marker; it does not invalidate that authoritative row or
roll source back. If no authenticated central row survives, a stray local exact
commit is rolled back with source and becomes one bounded, self-hashed failure
diagnostic rather than a recovery marker. Recovery retains a candidate only when
the self-digested terminal result, complete typed central-record receipt,
journal binding, exact central database row, and hash-bound report all agree.
`record.commit.json` is required while present and for the initial exact handoff,
but normal completed cleanup removes it; later startup validation uses the
sealed result/report receipts plus the exact central row. Forged or partial
terminal files are deleted.

New function tombstones use `crack_harness_function_tombstone/v2`. They are
written only after the approved candidate has been overlaid, Popen has returned,
and containment assignment has succeeded, but immediately before the contained
process is resumed. They bind the approval, base, candidate, and
`candidate_execution_started: true` execution-boundary reservation. Ordinary
admission, baseline, command, assignment, resume, or proof infrastructure
failures before that boundary do not consume a base+candidate attempt. A failed
pre-resume setup rolls back both the local tombstone and central reservation only
after both halves were fully published and still match the rollback snapshot.
If central publication throws
after persisting its ledger half, the central consumed-cell row is deliberately
retained even if the local marker is rolled back; that partial-publication case
fails closed rather than risking duplicate execution. A bounded
`consumed-cells.json` ledger independently preserves the same base+candidate
fact;
missing or conflicting local/central markers fail closed.

There is no general retry or tombstone reset for the same base+candidate
attempt. A legacy v1 tombstone may be reconciled exactly once only when the
approval carries the strict optional
`crack_harness_legacy_reconciliation/v1` descriptor. That descriptor binds the
immutable v1 tombstone and prior sealed failure by path and digest, the prior
approval, the same candidate and legacy controller commit, and compact
historical proof of equal target/candidate bytes, strict/data 100%, and zero
rows. It is authority-free by itself: the complete approval identity still
requires the external manager HMAC permit. A durable `retry-used.json` marker is
written after Popen/containment and before candidate resumption, and prevents a
second reconciliation. If setup or resume fails before the execution boundary,
a fully published, unchanged local/central reservation is rolled back together.
If only the central half persisted before publication failed, that half remains
consumed fail-closed. Once resume succeeds, every surviving reservation remains
consumed permanently.
V2 tombstones, partial historical results, malformed or missing artifacts, and
ordinary permits remain permanently fail-closed for their consumed
base+candidate pair. A new candidate may continue from the latest retained
frontier/current source under a new manager-signed permit; failure text or
provenance alone can never release that same attempt reservation.

`improved`, `no_gain`, and `failed` return terminal results; `no_gain` and
`failed` return nonzero. Results use the closed
`tools/CRACK_HARNESS_RESULT_V1.schema.json`. There is no reset command; Git is
the rollback path after an incorrectly retained frontier.

Only `exact` invokes central `record`; the function closes only after the exact
result and its valid bound `CRACK_REPORT/v1` are sealed. An `improved` attempt copies its
candidate into the current source frontier and invokes canonical `discard` for
the pending admission; it creates no central experiment. Every no-gain or failed
admitted attempt also invokes canonical `discard`. Central admission rows are
overwrite-bounded to one pending row per owner/function; a successful retained
record deletes its consumed admission rather than preserving admission history.
`RecoveryMemory.record` retains legacy API validation for `improved` and
`exact`, but this harness invokes it only for `exact`; direct `no_gain`, `failed`,
or `regressed` calls fail before inserting an experiment.
The central database uses a fixed Git-common-directory path; queue/memory
environment overrides are rejected. SQLite's page cap is hard-set to 64 MiB,
inputs are compact-field bounded, admissions and lane snapshots are bounded,
and only the latest retained experiment/report per owner/function survives.
Historical synchronization imports at most the latest exact retained record;
nonretained and append-only observations are discarded. Every failed reviewed
command attaches a bounded command receipt with return code (or explicit null
before exit), active time, sealed stdout/stderr hashes, cleanup errors, and
whether any `.o` object was observed in the disposable tree. A compile failure
keeps an overwrite-only, hash-sealed `latest-failure.json` diagnostic whose primary
cause is preserved even when rollback, discard, deletion, GC, or cleanup raises
an ordinary exception, interruption, or other `BaseException`; those later
errors are bounded secondary metadata and never escape in place of the primary.
If a command exits zero but descendant/job or output-reader quiescence cannot be
proved, its receipt preserves that zero primary exit and the exact cleanup
error, but the harness fails closed before consuming the output as evidence.
Cleanup after an authoritative `exact` terminal never changes the
crack status or rolls source back. The sealed result instead carries
`cleanup_status: cleanup_incomplete` and at most eight bounded secondary errors.
Startup retries the contained disposable worktree/temp cleanup and the exact
approval/base/candidate/permit set authenticated by the manager-HMAC-signed
`crack_harness_attempt/v2` receipt, then reruns protected owner/global retention
maintenance. Before deleting any root disposable it writes a manager-HMAC
`root-cleanup.receipt.json` beside the exact result. That compact receipt binds
the exact result's `attempt_sha256`, approval identity, retained source, and the
four canonical disposable roles, paths, and expected hashes. Root deletion is
fail-fast; the transaction journal is removed before the approval, the approval
before the attempt receipt, and the attempt receipt only after the other roots
are gone. It advances
the same latest result to `cleanup_status: complete` only after the receipt
signature and result bindings validate, every listed root path is rechecked as
absent, and the attempt receipt, transaction journal, and recovery marker are
gone. Once exact is sealed,
no later cleanup, cap, GC, or maintenance exception may escape as a crack
failure, roll source back, or create `latest-failure.json`; every such exception
is bounded secondary metadata on the same result. This does not create a
contradictory failure result or attempt history.

Root disposables are removed in fail-fast order with the approval deleted last,
so an expired but still hash-bound approval remains sufficient for automatic
startup cleanup. If an external actor deletes that approval while another root
disposable remains, startup deliberately writes or preserves a recovery lock and
leaves the remaining files untouched: even the signed attempt receipt cannot
replace the missing approval for rollback or cleanup. Manager repair or review
is required for that tamper boundary. Likewise, externally deleting
`attempt.json` before root cleanup cannot advance an exact result to
cleanup-complete while any signed-manifest path remains; the presealed cleanup
receipt is evidence of the intended paths, not deletion authority by itself.
