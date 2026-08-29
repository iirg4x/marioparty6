# Approved crack harness

`tools/crack_harness.py` is the only execution front door for a resumed crack
cell. It schedules no lanes and never asks a user for authentication. Production
state is fixed at `build/crack-harness`; only Python tests may inject another
state root.

## STOP and assignment permit

The manager creates the global `build/crack-harness/STOP` authorization when
cracking is stopped. Resuming one function does not remove STOP. At lane
assignment the manager first issues a closed, HMAC-SHA256-signed
`crack_harness_resume_permit/v1`; STOP then authenticates its exact file SHA-256
and 256-bit `stop_nonce`. The permit
binds owner, task, function, campaign, approval ID, cycle-free approval identity,
command-set identity, source path, source/base/candidate hashes, base commit,
toolchain, target, `issued_at`, and a deadline no more than 1,800 seconds later.
One permit authorizes one exact cell. The later approval binds the exact permit
SHA-256, so analysis time counts and neither approval nor candidate may drift. STOP
is revalidated before every command, CAS, central record, and terminal write.
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
Limits cannot be elevated: one candidate, at most 1,800 seconds, 512 MiB
ephemeral data, and 16 MiB retained compact state per stable owner across all
campaigns.

The UTF-8 natural-C cell may use at most three hunks and 80 changed lines,
including insert/delete, wholly inside the function span. NUL, asm,
volatile/register shaping, padding, dead branches, and inline/optimization
forcing are rejected.

Every approval also carries a closed `selection` object. Its strategy is fixed
to `winning_cell_first`, its rank is exactly `1`, and it must name a non-empty
natural-C `source_class`. The selection binds an existing repository-local
evidence artifact by path and SHA-256, the approval candidate SHA, and the
canonical SHA-256 of `predicted_rows`. `alternatives_compiled` and
`negative_controls` are both exactly `0`, and `pivot_if_unranked` is `true`.
The complete approval—including this selection—is included in the permit
identity and therefore in the manager signature binding. Missing, drifted,
or mismatched selection data is rejected before admission. There is no
fallback compile for an unranked cell: without a valid signed approval and
permit, STOP remains in force and the harness does not run.

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
and `data` JSON, `physical.json`, self-digested phase receipts, and a final
self-digested evidence context under `CRACK_HARNESS_OUT_ROOT`. The harness
checks every receipt identity, phase nonce, artifact hash/size, target hash, and
baseline immutability. Missing, stale, mixed, or fabricated evidence fails closed.
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
Exact requires strict/data 100%, exact equal
byte counts and zero differences, zero focus rows, zero protected-sibling
losses, and equal physical counts with zero differences. Assessment supplies
only owner gain. The admission token/input key and object pair must be bound by
one typed central-record receipt; recording failure rolls back. Protected sibling
proof covers the union of strict and data exact-identity sets. A positive focus
gain is retainable only when strict/data byte sizes remain target-equal, data
match does not regress, both protected sets lose nothing, and physical
relocations remain exact. Data percent and differing-row count must both be
non-regressing. Otherwise the candidate is treated as no gain and rolled back.

No gain/failure leaves baseline and retains no run directory or result artifact;
only one overwrite-only latest campaign tombstone remains; failure may additionally
retain one tiny overwrite-only sealed diagnostic, never an attempt log. Positive nonexact gain CAS-copies the candidate
as `improved`, emits `PIVOT_REQUIRED`, and ends the campaign. Exact also writes
compact `CRACK_REPORT/v1`. Only one latest compact result and, for exact only,
its bound report survive per owner/function; no full source duplicate or
per-attempt or append-only candidate history survives. Approval, baseline,
candidate, permit,
worktree, objects, logs, and temp are deleted. Owner state is hard-capped at 16
MiB and all harness state at 64 MiB. Journal recovery restores an interrupted
CAS. If central record committed before the local terminal commit, recovery
first deletes only the exact source/object/assessment-bound central experiment,
then rolls source back; it never leaves a retained central success paired with
baseline source. Recovery retains a candidate only when the self-digested
terminal result, complete typed central-record receipt, record-commit digest,
journal binding, and exact central database row all agree; exact additionally
requires the hash-bound report. Forged or partial terminal files are deleted.

`no_gain`, `improved`, and `failed` return nonzero. Results use the closed
`tools/CRACK_HARNESS_RESULT_V1.schema.json`. There is no reset command; Git is
the rollback path after a retained terminal gain.

Only retained `improved` or `exact` outcomes invoke central `record`. Every
unretained or failed admitted attempt invokes canonical `discard`, which deletes
the pending admission and creates no experiment. Central admission rows are
overwrite-bounded to one pending row per owner/function; a successful retained
record deletes its consumed admission rather than preserving admission history.
`RecoveryMemory.record` itself accepts only `improved` or `exact`; direct
`no_gain`, `failed`, or `regressed` calls fail before inserting an experiment.
The central database uses a fixed Git-common-directory path; queue/memory
environment overrides are rejected. SQLite's page cap is hard-set to 64 MiB,
inputs are compact-field bounded, admissions and lane snapshots are bounded,
and only the latest retained experiment/report per owner/function survives.
Historical synchronization imports at most the latest exact retained record;
nonretained and append-only observations are discarded. A compile failure keeps
an overwrite-only, hash-sealed `latest-failure.json` diagnostic whose primary
cause is preserved even when rollback, discard, deletion, GC, or cleanup raises
an ordinary exception, interruption, or other `BaseException`; those later
errors are bounded secondary metadata and never escape in place of the primary.
Cleanup after an authoritative `exact` or `improved` terminal never changes the
crack status or rolls source back. The sealed result instead carries
`cleanup_status: cleanup_incomplete` and at most eight bounded secondary errors.
Startup retries only the contained disposable worktree/temp cleanup, then
reruns protected owner/global retention maintenance and atomically advances that
same latest result to `cleanup_status: complete`. Once exact/improved is sealed,
no later cleanup, cap, GC, or maintenance exception may escape as a crack
failure, roll source back, or create `latest-failure.json`; every such exception
is bounded secondary metadata on the same result. This does not create a
contradictory failure result or attempt history.
