# Match workbench

`rtk python tools/agent.py match` is the central workflow for authenticated,
repeatable matching experiments. It is diagnostic-only: it does not compile,
edit recovered source, mutate the agent queue, or advance proof authority. The
workbench is not an operating-system sandbox. Its frozen request is a trusted
policy document, so a diagnostic process can still do anything the host grants
it. Review each job independently and declare its executable and every file
dependency before running it; descriptors are checked before and after the job.

Only `read_only`, `read_only_cpu`, `read_only_io`, and
`read_only_subprocess` resource classes may run in parallel. `compiler` (and
`compiler_heavy`), `native_debug` (native-debug), `proof` (and `proof_serial`),
`authority` (and `authority_mutator`), `integration`, and `retail_link`
(retail-link) are serialized resources and are rejected by `diagnose`. They
remain the responsibility of the existing serialized compiler, native-debug,
proof, authority, integration, and retail-link tools.

## Workflow

Use a workspace beneath `build/`; the example keeps request and job manifests
outside the workspace because `init` requires a new or otherwise empty one.

### 1. Freeze the session

Prepare an authenticated `match_workbench_request/v1` manifest containing the
target descriptor, owner/function identity, base commit and toolchain context,
and the diagnostic policy. A complete executable context must bind the real
`compile_cwd`, the outer `compiler`, every additional executable in
`compile_tools` (for example a compiler behind a wrapper), the exact
`compile_argv`, and all `compile_inputs`. Then initialize the immutable session:

```sh
rtk python tools/agent.py match init build/match-request.json \
  --workspace build/match --json
```

`init` snapshots the target into content-addressed storage and creates the
session, index, candidate, diagnostic, report-CAS, and job-output areas.

### 2. Deduplicate before compiling

Run `lookup` before spending compiler time. The source/context index identifies
an already measured candidate; `--source` is optional when an object is already
available. The object index identifies a prior candidate whose diagnostics may
be reused, but `lookup` never skips diagnosis by itself: run `diagnose` and its
full authenticated fingerprint will either reuse the exact cache entry or run
the missing job.

```sh
rtk python tools/agent.py match lookup \
  --workspace build/match \
  --source build/match-candidate.c \
  --object build/match-candidate.o --json
```

Treat `conflict` as a stop-and-review result: the same frozen source/context
was associated with a different object, so compile inputs or determinism must
be re-authenticated. With complete frozen compile context, a known
source/context can skip compilation; a known object can skip diagnostics.
Neither result advances proof authority.

### 3. Seal the compiler provenance

Before a compiled object can be recorded, seal its producer context in a
`match_workbench_compile_attestation/v1` file:

```sh
rtk python tools/agent.py match attest-compile \
  --workspace build/match \
  --source build/match-candidate.c \
  --object build/match-candidate.o \
  --output build/match-candidate.compile-attestation.json \
  --producer-kind serialized-build \
  --producer-argv-from-session --json
```

`--producer-argv-from-session` copies the immutable session argument vector.
Alternatively, repeat `--producer-arg` in exact argument order; for a real
compiler, the resulting array must equal the immutable session's
`compile_argv` exactly. The compiler and wrapper executables remain separately
bound by `compiler` and `compile_tools`, so `compile_argv` follows the session's
documented convention (arguments only or a wrapper-inclusive vector).
The attestation also binds the session's toolchain key, compiler descriptor,
all wrapper/tool descriptors in `compile_tools`, compile working directory,
response-file expansion binding, source bytes, and object bytes. The document
is self-hashed with `attestation_sha256` and always has
`authority_advanced:false`.

`attest-compile` is an attestation boundary, not a compiler launcher and not a
proof that an operating-system process ran. The caller is responsible for
running the declared producer command in the declared working directory. A
real compiler session with missing `compile_tools`, missing `compile_cwd`, an
empty command, a command different from `compile_argv`, changed source/object
bytes, or a changed compiler/wrapper descriptor fails before writing an
attestation. Legacy incomplete sessions therefore cannot mint new evidence.

### 4. Record one candidate and its reports

After the independently run compile/comparison, record the candidate:

```sh
rtk python tools/agent.py match record \
  --workspace build/match \
  --candidate-id c1 \
  --source build/match-candidate.c \
  --object build/match-candidate.o \
  --compile-attestation build/match-candidate.compile-attestation.json \
  --strict-report build/match-strict.json \
  --data-report build/match-data.json \
  --hypothesis "natural candidate" \
  --axis "register-lifetime" --json
```

`--data-report` is optional; `--compile-attestation`, `--strict-report`,
`--hypothesis`, and `--axis` are required. The record operation validates the
attestation against the destination session before mutating the candidate
index or CAS. A GC/2.7 object therefore cannot be recorded in a GC/2.6 session,
even when the source, function, or object name is identical. Source and object
blobs are reused by SHA-256. Reports are stored once in deterministic gzip
form, with their raw and compressed hashes, and candidate records are
immutable, self-hashed, and linked in order.

`prepare` uses the same boundary and requires `--compile-attestation`; it
validates the immutable initialized workspace and only emits a guarded record
request. It never creates the workspace or records the candidate.

### 5. Audit and migrate legacy compiler provenance

Older workbenches remain readable for history and matrix telemetry, but an
unattested legacy record cannot drive `lookup`, `materialize`, or diagnosis
reuse. Classify a workspace first:

```sh
rtk python tools/agent.py match provenance-audit \
  --workspace build/legacy-match \
  --manifest build/legacy-provenance.json \
  --output build/legacy-provenance-audit.json --json
```

The optional closed, self-hashed `match_workbench_provenance_manifest/v1`
maps candidate IDs to independently reconstructed compile attestations:

```json
{
  "schema": "match_workbench_provenance_manifest/v1",
  "schema_version": 1,
  "candidates": [
    {
      "candidate_id": "player-movenum-memcpy-rotx-v495",
      "attestation": "build/v495.compile-attestation.json"
    }
  ],
  "manifest_sha256": "SELF_HASH"
}
```

`provenance-audit` emits self-hashed
`match_workbench_provenance_audit/v1` JSON in immutable ordinal order. Every
row is `context_match`, `cross_context`, or `unattested`, and reports the
session/actual context hashes, toolchain key, compiler SHA-256, object/source
hashes, duplicate relation, and evidence source. Unknown or repeated candidate
IDs, malformed attestations, artifact mismatches, and manifest tampering fail
closed.
When `--output` is supplied, the exact result is written once as a durable
receipt. Repeating identical evidence is idempotent; conflicting bytes at that
path fail closed.

Create a clean destination session for the actual compiler context, then
import only records whose external attestations match it:

```sh
rtk python tools/agent.py match provenance-migrate \
  --source-workspace build/legacy-match \
  --destination-workspace build/match-gc27 \
  --manifest build/legacy-provenance.json \
  --output build/legacy-provenance-migration.json --json
```

`provenance-migrate` emits self-hashed
`match_workbench_provenance_migration/v1` JSON. It never changes the source
workspace. Imported records retain hypotheses, outcomes, focus symbols,
reports, heavy-time telemetry, source ordinals, and source-record receipts;
source/object/report CAS is revalidated, and duplicate-object/source relations
are deterministically re-derived in source ordinal order. Cross-context rows
are listed as `skipped_cross_context`, not coerced. Repeating the same migration
is idempotent. A destination candidate collision, source/context producing a
different object, CAS tampering, report corruption, unknown manifest entry, or
non-final partial append fails closed. Migration and audit remain diagnostic
and always report `authority_advanced:false`. Migration `--output` uses the
same write-once receipt contract as the audit command. Results describe the
authenticated final imported set, so an idempotent rerun is byte-identical; a
different session, manifest, or candidate set fails closed.

### 6. Diagnose bounded, authenticated read-only jobs

Declare jobs in a `match_workbench_jobs/v1` file. Each job must identify its
kind and safe resource class, authenticated executable, `argv`, real `cwd`,
authenticated `inputs`, contained relative `outputs`, and a timeout; the
policy's registered kinds and worker limit bound the run. Path-like executable
arguments and dependencies must be declared, not inferred.

```sh
rtk python tools/agent.py match diagnose \
  --workspace build/match \
  --candidate-id c1 \
  --jobs build/match-jobs.json \
  --max-workers 2 --json
```

Each job gets a private output root, bounded time and captured output, and a
content fingerprint. Identical fingerprints are reused or deduplicated within
the run. The `MATCH_WORKBENCH_READ_ONLY=1` environment marker expresses intent;
it is not enforcement or a sandbox boundary.

### 7. Render the deterministic matrix

```sh
rtk python tools/agent.py match matrix \
  --workspace build/match --json
```

`matrix` emits self-hashed JSON to stdout in stable candidate/fingerprint
order. Rows include strict/data compact focus, diagnostic status, and a
`next_action`; aggregate KPIs include candidate and unique-object counts,
duplicate/diagnosed counts, raw versus unique compressed report bytes and
storage reduction, diagnostic seconds, and exact-focus bytes per heavy minute.

### 8. Measure one function's recovery campaign

```sh
rtk python tools/agent.py match telemetry \
  --workspace build/match \
  --function ev_CapKamekkuOMExec \
  --elapsed-seconds 21600 \
  --active-seconds 7200 \
  --tracer-runs 0 \
  --donor-searches 1 \
  --output build/throughput/ev_CapKamekkuOMExec.telemetry.json
```

`telemetry` emits self-hashed `match_workbench_function_telemetry/v1` JSON.
`--output` optionally writes the same receipt once; identical replay is
idempotent and conflicting existing bytes are rejected.
Candidate/source/object counts, convergence, exact candidate identity, and
heavy-process seconds are derived from immutable workbench records. Elapsed and
active time plus tracer/donor counts are explicitly caller-attested. Missing
candidate timings remain missing: heavy-process crack/hour is withheld unless
every selected candidate has `heavy_seconds`. Human elapsed/active crack/hour
is reported separately from compiler/process throughput. This report is
diagnostic telemetry only; it does not authenticate physical relocations,
consumer closure, or promotion authority.

### 8a. Record event-derived campaign time at checkpoint F

Create a small, reviewed checkpoint file for each process definition. The file
must already exist and remain byte-exact for the duration of that campaign; its
path, size, SHA-256, checkpoint ID, and the current match telemetry tool are
bound into every event.

```sh
rtk python tools/agent.py match campaign-start \
  --campaign build/throughput/checkpoint-f-before.json \
  --campaign-id checkpoint-f-before-01 \
  --adoption-phase before \
  --checkpoint-id checkpoint-f-before \
  --workflow-checkpoint process/checkpoint-f-before.json \
  --at 2026-08-24T08:00:00+04:00

rtk python tools/agent.py match campaign-event \
  --campaign build/throughput/checkpoint-f-before.json \
  --event pause \
  --at 2026-08-24T09:15:00+04:00

rtk python tools/agent.py match campaign-event \
  --campaign build/throughput/checkpoint-f-before.json \
  --event resume \
  --at 2026-08-24T09:30:00+04:00

rtk python tools/agent.py match campaign-event \
  --campaign build/throughput/checkpoint-f-before.json \
  --event exact \
  --at 2026-08-24T10:10:00+04:00 \
  --telemetry-receipt build/throughput/ev_CapKamekkuOMExec.telemetry.json
```

All `--at` boundaries are required ISO-8601 timestamps with an explicit UTC
offset and are canonicalized to microsecond UTC. Events must be strictly
increasing. The only valid transitions are `start -> pause -> resume`; `exact`
requires active state and leaves the campaign active so the next function can
begin immediately. A repeated receipt or repeated exact function identity is
rejected.

The durable, self-hashed `match_workbench_campaign_timing/v1` ledger derives
wall time from `start` through the latest `exact` boundary and derives active
Sol time from only active intervals, excluding every recorded pause. It never
uses the function receipt's caller-attested elapsed or active fields. Each
`exact` event embeds and validates the existing function telemetry receipt, so
candidate counts, convergence, tracer/donor activity, exact bytes, and
heavy-process timing keep their existing definitions. Heavy-process rates stay
withheld if any embedded receipt lacks complete `heavy_seconds` coverage.

Tampered self-hashes, broken event chains, changed workflow/tool bytes,
duplicate receipts, invalid transitions, non-increasing or missing timestamps,
and exact events without receipts fail closed. Before appending any event, the
command also reopens the bound workflow/tool and every historical exact receipt;
drift in earlier live evidence blocks the append. Every ledger reports
`authority_advanced:false` and remains observational telemetry, not matching or
promotion proof.

### 8b. Compare observed crack/hour before and after adoption

Record the `after` sample with `campaign-start --adoption-phase after` and its
own adopted checkpoint file, then compare one or more non-overlapping campaign
receipts per phase:

```sh
rtk python tools/agent.py match campaign-compare \
  --before-campaign build/throughput/checkpoint-f-before.json \
  --after-campaign build/throughput/checkpoint-f-after.json \
  --output build/throughput/checkpoint-f-observed-comparison.json
```

Repeat `--before-campaign` or `--after-campaign` to combine serial campaign
segments. All inputs in one phase must bind the same exact workflow checkpoint;
the before and after bindings must differ, intervals must not overlap, after
must not start before the before measurement boundary, and no telemetry receipt
or exact function identity may appear twice. Exact function identity is the
session/function pair, so changing the recorded first-exact candidate ID cannot
launder a duplicate into either phase. At comparison time, the command reopens
and rehashes each bound workflow checkpoint, the match telemetry tool, and every
referenced function telemetry receipt; missing, changed, or differently parsed
live evidence is rejected. Campaigns without a positive event-derived exact
boundary are rejected rather than used in a rate claim.

`campaign-compare` emits deterministic, self-hashed
`match_workbench_campaign_comparison/v1` JSON with aggregate function,
candidate, exact-byte, wall, active-Sol, and (when completely covered)
heavy-process rates. Its `observed_change` reports absolute, ratio, and percent
changes. `attribution.causal_attribution` is always `false`: the checkpoint
binding supports an observed pre/post association, while function mix and other
uncontrolled campaign conditions prevent causal attribution. Output is
write-once/idempotent and always reports `authority_advanced:false`.

### 9. Reduce one function's objdiff cascade

```sh
rtk python tools/agent.py match cascade \
  --report build/GP6E01/reports/candidate.strict.json \
  --function ev_CapTeresaFadeMatHook
```

`cascade` binds the report bytes and reducer implementation, then emits
self-hashed `match_workbench_causal_reducer/v1` JSON. It clusters adjacent
instruction residuals, collapses repeated structural signatures into causal
families, and ranks bounded diagnostics for uniform stack-home deltas,
sign-extension/prototype seams, aggregate-copy lifetimes, branch topology, and
relocation/data mismatches. Optional `--target-asm` and `--candidate-asm` add
hashed listing context; `--full` retains bounded instruction pairs.

The explicit-else rule recognizes a narrow MWCC topology: a target conditional
branch enters the second of two adjacent branches to one epilogue while the
candidate branches directly to that epilogue. It recommends testing
`if (condition) { body } else { return; }`, the shortest successful axis for
`ev_CapTeresaFadeMatHook` c17. Recommendations remain diagnostic evidence, not
source provenance or retention authority; strict/data/physical-relocation,
section, consumer, and protected-sibling gates remain mandatory.

### 10. Decode typed pool-owner mismatches

```sh
rtk python tools/agent.py match pools \
  --report build/GP6E01/reports/candidate.strict.json \
  --function ev_CapKoopaReturn
```

`pools` (aliases `pool-decode` and `pool-owners`) emits self-hashed
`match_workbench_pool_decoder/v1` JSON. It resolves each side's object-local
relocation owner, decodes 16/32/64-bit big-endian values, infers the consumer
type from `lfs`/`lfd`/integer loads, and recognizes MWCC signed/unsigned
integer-to-double bias constants. Mismatches are ranked by causal severity:
relocation type/addend, literal type/value, missing consumer, unresolved bytes,
then value-equivalent owner identity or pool chronology.

This distinction prevents a strict label-only residual from being mistaken for
a semantic constant mismatch. For example, the CapSpecial c23
`ev_CapKoopaReturn` receipt classifies all 16 pool consumers as byte- and
relocation-equivalent named-target versus anonymous-candidate owners, with zero
semantic/contract mismatches. Such a result directs investigation toward
authenticated constant binding or TU first-use chronology; it never authorizes
inventing an extern label or reordering unrelated source.

### 11. Plan factorial source-axis interactions

```sh
rtk python tools/agent.py match interactions \
  --request build/owner/function-interactions.json
```

`interactions` (aliases `factorial-plan` and `interaction-plan`) accepts a
closed `candidate_interaction_request/v1` manifest and emits self-hashed
`match_workbench_interaction_plan/v1` JSON. Each axis declares one measured
control, two or more levels, the natural source action, evidence,
admissibility, and an explicit normalized topology token. The planner expands
the bounded Cartesian product and orders controls, single-axis cells, then
higher-order interactions. This makes the combined cell mandatory when two
individually neutral axes may interact.

Optional observations bind a complete selection to authenticated source and
object SHA-256 values. The output distinguishes identical source from merely
identical object code. A cell is skipped only when its complete explicit
topology matches another cell or measured hashes prove reuse; names and prose
are never treated as equivalence. Constraints and blocked source levels remain
visible but are not scheduled. The request is capped at eight axes, eight
levels per axis, and a caller-bounded product (256 cells by default).

The CapThrow Kamekku receipt encodes `{36.0f, 60.0f}` by
`{discard RNG result, assign result to existing time}`. It yields the four
required cells: baseline, two single controls, and the exact combined
interaction. Replaying the completed campaign reuses all four measured hashes;
the counterfactual unmeasured request schedules all four rather than stopping
after the two 99.990710% single controls.

This command is a read-only batch planner. It does not generate source,
compile, record candidates, decide source admissibility, retain a candidate,
or advance authority. Every generated cell still needs natural-source review
and strict/data/physical-relocation/section/protected-sibling gates.

Compact objdiff summaries are explicitly diagnostic-only, not canonical proof.
Even an exact compact focus produces the next action
`authenticate_report_binding_then_run_serial_proof_and_closure`; first bind the
caller-supplied report to the frozen target, object, and toolchain, then perform
serial proof and closure before making any authority, integration, or
retail-link claim. Set `context_complete: true` only when the request seals the
complete executable context: full persistent file identities for the compiler,
tool chain, exact dependency path set and fresh dependency provenance, the
build/rule descriptor, every include root and its name-tree fingerprint, the
selected environment/codepage/locale, runtime DLLs, canonical output/depfile,
and source-tree state plus any dirty-patch descriptor. Response-file arguments
must also carry an expanded-argv binding and descriptors for every response
file. These fields are caller-attested and rechecked before reuse; a v1/v2/v3
session that lacks any of them remains readable but can never set `skip_compile`.
File bytes alone are not sufficient: same-byte replacement, hard-link changes,
and reparse/junction substitution fail closed.

Timeout and output enforcement terminates the direct diagnostic process. This
trusted diagnostic runner is not an OS sandbox or process-tree jail; jobs must
not spawn detached children.

## Generated layout

With `--workspace build/match`, generated state stays beneath the ignored
`build/` tree:

```text
build/match/
  session.json       frozen request and target identity
  index.json         self-hashed candidate/diagnostic indexes
  candidates/        immutable candidate records
  diagnostics/       immutable diagnostic results
  cas/blobs/         target/source/object blobs by SHA-256
  cas/reports/       deterministic gzip reports by raw SHA-256
  job-output/        private per-fingerprint diagnostic outputs
```

Do not treat these generated records as source or proof artifacts to promote
to `main`; retain reusable conclusions in the appropriate recovery knowledge
or proof record after the serialized gates pass.
