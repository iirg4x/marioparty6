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

### 3. Record one candidate and its reports

After the independently run compile/comparison, record the candidate:

```sh
rtk python tools/agent.py match record \
  --workspace build/match \
  --candidate-id c1 \
  --source build/match-candidate.c \
  --object build/match-candidate.o \
  --strict-report build/match-strict.json \
  --data-report build/match-data.json \
  --hypothesis "natural candidate" \
  --axis "register-lifetime" --json
```

`--data-report` is optional; `--strict-report`, `--hypothesis`, and `--axis`
are required. Source and object blobs are reused by SHA-256. Reports are
stored once in deterministic gzip form, with their raw and compressed hashes,
and candidate records are immutable, self-hashed, and linked in order.

### 4. Diagnose bounded, authenticated read-only jobs

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

### 5. Render the deterministic matrix

```sh
rtk python tools/agent.py match matrix \
  --workspace build/match --json
```

`matrix` emits self-hashed JSON to stdout in stable candidate/fingerprint
order. Rows include strict/data compact focus, diagnostic status, and a
`next_action`; aggregate KPIs include candidate and unique-object counts,
duplicate/diagnosed counts, raw versus unique compressed report bytes and
storage reduction, diagnostic seconds, and exact-focus bytes per heavy minute.

### 6. Measure one function's recovery campaign

```sh
rtk python tools/agent.py match telemetry \
  --workspace build/match \
  --function ev_CapKamekkuOMExec \
  --elapsed-seconds 21600 \
  --active-seconds 7200 \
  --tracer-runs 0 \
  --donor-searches 1
```

`telemetry` emits self-hashed `match_workbench_function_telemetry/v1` JSON.
Candidate/source/object counts, convergence, exact candidate identity, and
heavy-process seconds are derived from immutable workbench records. Elapsed and
active time plus tracer/donor counts are explicitly caller-attested. Missing
candidate timings remain missing: heavy-process crack/hour is withheld unless
every selected candidate has `heavy_seconds`. Human elapsed/active crack/hour
is reported separately from compiler/process throughput. This report is
diagnostic telemetry only; it does not authenticate physical relocations,
consumer closure, or promotion authority.

### 7. Reduce one function's objdiff cascade

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

### 8. Decode typed pool-owner mismatches

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

### 9. Plan factorial source-axis interactions

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
