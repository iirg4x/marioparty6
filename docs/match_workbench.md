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
and the diagnostic policy. Then initialize the immutable session:

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

Compact objdiff summaries are explicitly diagnostic-only, not canonical proof.
Even an exact compact focus produces the next action
`authenticate_report_binding_then_run_serial_proof_and_closure`; first bind the
caller-supplied report to the frozen target, object, and toolchain, then perform
serial proof and closure before making any authority, integration, or
retail-link claim. Set `context_complete: true` only when `compile_inputs`
authenticate the complete compiler dependency context; it is a caller-attested
closure, not something inferred from a partial list.

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
