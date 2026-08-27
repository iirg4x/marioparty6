# Source-aware MWCC causal trace

This checkpoint is a diagnostic-only join over authenticated GC/2.6 and
GC/2.7 evidence. It does not edit Board source, retain a candidate, or advance
authority. A native capture may run the exact request-bound compiler in a
private worktree, but every causal evidence artifact sets
`authority_advanced: false`.

## Components and non-duplication boundary

- `tools/capsule_stack_home_native.py` remains the only native producer of the
  six stack-allocation/write hooks. `capsule_same_session_capture.py` imports
  those hook identities and adds only the authenticated vreg/physical-register
  hooks, the authenticated GC/2.7 machine-emission hook, and one event bus.
- `tools/mwcc_fe_chronology_native.py` supplies the pointer-free frontend
  Object/VarInfo chronology contract. A missing packet is reported as UNKNOWN.
- `tools/pcode_varinfo_correlator.py` supplies the direct ownership gate. Null,
  reused, duplicate, one-to-many, or unauthenticated identities never become a
  name join.
- `tools/donor_cfg_align.py` parses the exact bound source function and emits
  deterministic assignment, call-return, evaluation, and control-flow spans.

No raw pointer is serialized. Capture-local Object tokens include the sealed
session ID and cannot be replayed into another request.

## Authenticated GC/2.7 transport

The immutable request remains wrapper-first. For the one authenticated pair
below, the capture tool may execute the compiler directly because sjiswrap
v1.1.1 replaces its image in the same PID too quickly for a reliable debugger
selection boundary:

- sjiswrap SHA-256:
  `27a3c5d4f263e4eb96e5619cfcda22f45d33ccd121104c7ff6a37e15b3f427cd`
- GC/2.7 `mwcceppc.exe` SHA-256:
  `04ece8178961bdbaeebe2d4e5922ed542c4d82b2fc3de996c41c9e193bd49eea`

The executed argv is derived by removing exactly `argv[0]` from the bound
request. The tool seals both argv forms, cwd, wrapper/compiler descriptors, and
`execution.mode = authenticated_direct_compiler`. No other wrapper/compiler
pair may use this path. An unrecognized pair, argv drift, non-ASCII wrapper
contract, image drift, or compiler-selection ambiguity fails before a hook is
trusted.

Windows may emit two initialization breakpoints for this direct WOW64 launch.
Only out-of-image breakpoints from the authenticated compiler PID can consume
that bounded allowance. The first owned hook clears it; a later unowned
breakpoint, an in-image unowned breakpoint, or a breakpoint from another PID
fails closed. Each worktree must still serialize its own compiler transaction
with `build/.compiler-lane.lock`; there is no global retail-build lease.

## Closed GC/2.7 hook profile and machine-emission join

GC/2.7 requests carry one closed, ordered 13-hook profile. Its six stack hooks
are `function_filter` at `0x00433492`, `allocation_pre` at `0x0043367E`,
`allocation_post` at `0x00433683`, and the three Object writes at
`0x004F9D74`, `0x004F9E11`, and `0x004F9E98`. Their exact compiler-image
prefixes are, respectively, `e87d650c00598a44240450`,
`89432e8b530e8b420201e84821f00105e40c`,
`89432e8b4b0e8b410201e84821f00105dc0c`, and
`89432e8b4b0e8b410201e84821f00105d80c`. The allocation call remains at
`0x0043367E`, but its target moves from GC/2.6 `0x004F9CE0` to GC/2.7
`0x004F9C00`. The three Object writes move by the same `-0xE0` delta. These
sites are GC/2.7-specific; do not copy the GC/2.6 stack profile into a GC/2.7
request.

The other seven hooks are the direct allocation hook at `0x0043598B`, three
physical-assignment commits at `0x004D0E65`, `0x004D0F6E`, and `0x004D0A7B`,
two PCode-color observations at `0x005086C4` and `0x005086C8`, and the
machine-emission hook at `0x004EB21F`. The GC/2.6 post-assignment hook at
`0x004D03E8` is not part of this profile. A 9-hook request, a request that
retains `0x004D03E8`, or any missing, extra, reordered, or prefix-modified row
is rejected before the compiler launches. A private backend can implement the
13 sites but cannot add authority or replace the request's compiler-selected
profile.

`preflight` also maps every selected hook address back into the authenticated
GC/2.7 PE file and compares the exact preferred-base bytes before any process
is created. This disk-image gate is independent of the later live mapped-image
gate. A stale address or prefix therefore fails during package verification;
relocation-aware live validation still runs after the wrapper selects the
same-PID compiler image.

The machine-emission site is authenticated by prefix
`8b178b0a030dd00b5e0001e989018b43`. At that boundary the native adapter
observes the compiler-internal PCode node, emitted function-relative byte
offset, and exact four output bytes. The PCode address is converted immediately
to a session-local `pcode-<session>-<ordinal>` token and is never serialized.

Each proven instruction appears as a pointer-free `machine_emission` event.
It records:

- PCode token, emitted byte offset, instruction index, compiler opcode enum,
  exact PPC bytes/word, mnemonic, registers, and immediate;
- memory operation, width, and effective `r1`-relative stack offset when the
  address is provable;
- prior emitted instruction indices that supply the address or stored value;
- `owner_joins` only when the complete Object/vreg/physical-register chain is
  exact; `physical_owner_joins` may also use the same-PCode-token
  Object/IG/final-color observation when no vreg identity is authenticated.
  Operand indices are never promoted to vreg identities.

The decoder supports the bounded stack and arithmetic forms needed by the
current evidence: `addi`, scalar D-form loads/stores, non-quantized PSQ
loads/stores, scalar `fmuls`, and paired-single `ps_mul`. Arithmetic events
record two source FPRs, one destination FPR, exact reaching definitions,
`arithmetic_op = multiply`, and `arithmetic_type = f32` or `paired-single`.
The same capture-local PCode token joins authenticated Object/IG final colors
to those physical operands without serializing compiler addresses. It returns
UNKNOWN and clears reaching-definition state for a missing/reused PCode token,
reversed or duplicate output offset, unsupported opcode/operand, descriptor
mismatch, ambiguous address definition, nonzero indexed base, or PSQ
quantization other than zero. UNKNOWN is evidence, never an inferred owner.
Objdiff reports the retail MoveNum seam at aligned rows 117--122. The
fresh candidate capture numbers the corresponding six scalar machine events
113--118: loads 113/114/117 cover `[0x08,0x14)` and stores 115/116/118 cover
`[0x14,0x20)`. Binding plans must use capture-local indices, never copy
objdiff row numbers into a machine-event manifest.

Because hook manifests are compiler-bound, an existing GC/2.7 request created
before this hook was installed must be regenerated and resealed. GC/2.6 and
other compiler requests retain their previous hook set.

## Required bindings

The same-session envelope, frontend packet, correlator report, sealed span
manifest, and causal map each carry `authority_advanced: false`. The same-session
request and external trust root bind the session ID, function
and function SHA-256, source, compiler, wrapper, debugger/transport, argv, cwd,
hook bytes, tool paths, and output paths. The sealed source-span manifest must
repeat the capture's function/function hash/session/source descriptor exactly.
Each span binds one capture-local Object token to one exact source identity,
UTF-8 byte range, line range, and text SHA-256. A span claimed by two tokens is
rejected.

An unsealed reviewed span file has this closed shape (no `manifest_sha256`):

```json
{
  "schema": "mwcc_source_span_bindings/v1",
  "function": "mbCapListDebug",
  "function_sha256": "<64 hex from envelope.context>",
  "session_id": "session-<16 hex from envelope.context>",
  "source": {"path": "C:\\...\\capsule.c", "size": 123, "sha256": "<64 hex>"},
  "spans": [{
    "object_token": "local-session-<16 hex>-000000",
    "identity": "listData",
    "role": "declaration",
    "byte_start": 100,
    "byte_end": 108,
    "line_start": 12,
    "line_end": 12,
    "text_sha256": "<SHA-256 of exactly source[100:108]>"
  }],
  "authority_advanced": false
}
```

Allowed roles are `declaration`, `read`, `write`, `call_return`, and
`evaluation`. The role is evidence metadata; it never overrides token, source,
or chronology validation.

## Exact workflow

First prepare and authenticate one immutable same-session request. `preflight`
must report all 13 GC/2.7 hooks before `capture` is permitted:

```text
rtk C:\Python313\python.exe tools\capsule_same_session_capture.py prepare --manifest C:\proof\manifest.json --output-dir C:\proof\capture --trust-root C:\proof\trust-root.json

rtk C:\Python313\python.exe tools\capsule_same_session_capture.py preflight C:\proof\capture\request.json --trust-root C:\proof\trust-root.json

rtk C:\Python313\python.exe tools\capsule_same_session_capture.py capture C:\proof\capture\request.json --trust-root C:\proof\trust-root.json
```

`capture` is the only command above that launches the authenticated compiler;
run it under the owner worktree's `build/.compiler-lane.lock`. When the exact
same capture-tool bytes are still running the analysis, seal the reviewed span
binding and build the causal map directly:

```text
rtk C:\Python313\python.exe tools\capsule_same_session_capture.py seal-source-spans --input C:\proof\source-spans.unsealed.json --output C:\proof\source-spans.json

rtk C:\Python313\python.exe tools\capsule_same_session_capture.py causal-map --envelope C:\proof\same-session.envelope.json --trust-root C:\proof\trust-root.json --source-spans C:\proof\source-spans.json --output C:\proof\source-aware-causal-map.json
```

For a completed runtime package, first seal one external post-capture trust
root per lane. The output must be outside the raw capture directory: that
directory remains a closed four-file producer boundary and any extra file is
rejected.

```text
rtk C:\Python313\python.exe tools\mwcc_post_capture_trust.py --package-request C:\proof\package\backend-request.json --package-receipt C:\proof\package\package-receipt.json --measurement C:\proof\active.closed.json --child-request C:\proof\package\backend-output\retained\capture\request.json --envelope C:\proof\package\backend-output\retained\capture\same-session.envelope.json --lane retained --output C:\proof\package\post-capture\retained\post-capture-trust-root.json
```

Repeat with the v491 child request/envelope and `--lane v491`. The sealer
must run while the producer path still contains the exact capture-time 13-hook
tool bytes. It authenticates that producer, the package request and receipt,
child request, compiled object, streams, envelope, and complete positive
active-time receipt. It launches nothing and always emits
`authority_advanced: false`. After that root is sealed, a later analysis-tool
revision may consume it without pretending to have the producer's historical
hash. Use the external root explicitly for that post-capture analysis:

```text
rtk C:\Python313\python.exe tools\capsule_same_session_capture.py causal-map --envelope C:\proof\package\backend-output\retained\capture\same-session.envelope.json --trust-root C:\proof\package\post-capture\retained\post-capture-trust-root.json --source-spans C:\proof\source-spans.json --post-capture-analysis --output C:\proof\source-aware-causal-map.json
```

If an independently sealed frontend packet exists, add:

```text
rtk C:\Python313\python.exe tools\capsule_same_session_capture.py causal-map --envelope C:\proof\package\backend-output\retained\capture\same-session.envelope.json --trust-root C:\proof\package\post-capture\retained\post-capture-trust-root.json --source-spans C:\proof\source-spans.json --frontend-chronology C:\proof\frontend-chronology.json --post-capture-analysis --output C:\proof\source-aware-causal-map.json
```

The packet's source and compiler hashes must equal the same-session envelope.
Its exact function and capture session ID must also equal the envelope; a packet
for another Board function or session is rejected even when source/compiler
hashes happen to match. Without it, `frontend_chronology.status` is `UNKNOWN`;
the tool never imports a chronology from a separate compiler process by name.

## Repaired handoff and v2 ownership contracts

### Compiler-output handoff

The pre-launch authentication still requires the capture output directory to be
empty and records the exact evidence paths that the capture owns. After the
authenticated compiler is created, the compiler's own explicit `-o` path is the
one permitted regular, non-symlink file. The handoff reuses the pre-launch
authentication and permits that exact compiler output; it does not treat the
compiler-created object as a stale capture artifact. Any pre-existing or
unowned file, capture stream, envelope, or partial evidence file still fails
closed. Partial-output cleanup is limited to known capture evidence paths and
does not delete arbitrary files. This race-handling rule is specific to the
authenticated request and does not broaden the compiler or wrapper allowlist.

### Diagnostic preservation at a final ownership-join UNKNOWN

`capture` has one opt-in escape hatch for evidence loss, not for validation:
`--partial-evidence-dir C:\proof\partial-evidence`. It applies only when the
strict envelope reaches one of the three authenticated final ownership-join
failures (missing exact Object/vreg edge, missing exact physical-register edge,
or missing exact same-session physical assignment). Every earlier failure,
hook/profile drift, request/trust mismatch, noncanonical chronology, ambiguous
or reused identity, backend cleanup failure, or output-boundary violation still
returns nonzero and publishes nothing.

The partial-evidence directory must be an absolute, canonical, previously
nonexistent path outside the raw capture directory. The compiler request must
name exactly one explicit `-o` path, that path must contain the compiler-owned
regular nonsymlink object created after the authenticated empty-output proof,
and the raw capture directory may contain only `request.json`, the three
capture-owned files, and that object when `-o` deliberately names the capture
directory. Stale or unowned entries fail closed. Successful publication is an
atomic directory rename; the raw stack/PCode/envelope files are then removed,
leaving the request boundary reusable without deleting the compiler object.

The immutable, hash-bound package contains:

- `stack.events.jsonl`, `pcode.events.jsonl`, and the separately filtered
  `machine.events.jsonl`;
- the unaccepted `candidate-envelope.json`;
- `hook-validation.json`, binding request, source, compiler, wrapper,
  debugger, transport, argv, cwd, hook profile, empty-output proof, and
  compiler-owned object;
- `ownership-failure-graph.json`, preserving every observed machine join,
  reaching definition, present/missing/conflicting
  Object-to-vreg-to-physical edge, and the first absent edge in canonical event
  order. Located `fmuls` events whose final reaching-definition join remains
  ambiguous retain their decoded physical operands, every independently known
  source-register definition, the explicit missing source registers, and the
  complete capture-local PCode operand/IG-owner chronology. Hidden IG owners
  remain `UNKNOWN`; the graph never promotes them to source Objects;
- `partial-evidence.json`, the self-hashed package manifest and artifact
  digests.

The command returns status 2 because the package has `status: UNKNOWN`,
`diagnostic_only: true`, `board_admission: false`, `exactness_claim: false`,
and `authority_advanced: false`. It is suitable for ranking a bounded natural-C
probe, but it cannot satisfy a causal-map ownership join, retain source, prove
matching, or advance closure/promotion authority.

```text
rtk C:\Python313\python.exe tools\capsule_same_session_capture.py capture C:\proof\capture\request.json --trust-root C:\proof\trust-root.json --partial-evidence-dir C:\proof\partial-evidence
```

### Source-span v2 and ownership modes

The normalized manifest schema is `mwcc_source_span_bindings/v2`. It has a
closed `objects` inventory in addition to source spans. The default
`ownership_mode` is `scalar_register`; those rows retain the ordinary
Object-to-IG/vreg-to-physical-register uniqueness gate. A `stack_interval` row
is an explicit diagnostic exception for an authenticated stack-only aggregate;
the object must be a `HuVecF` of exactly 12 bytes, and its final post-allocation
stack home must derive one exact ABI-adjusted half-open interval. Every access
must be wholly contained in that interval, with no partial overlap, alias,
duplicate final allocation, mixed mode, mixed session, or placeholder token.

`stack_interval` ownership does not satisfy a scalar-register row and does not
hide missing scalar crosswalk evidence. It may establish an authenticated
source/stack ownership row without a physical register or vreg, while ordinary
scalar rows continue to require the physical ownership chain. For the current
MoveNum seam, the source intervals are `posNorm` raw slot `0` to machine
`[0x08,0x14)`, and `pos` raw slot `12` to machine `[0x14,0x20)`; the target
paired sequence is aligned at objdiff rows 117--122. The current candidate
capture uses local machine indices 113--118 and three 4-byte read/write pairs.
Those scalar loads/stores cover the same 12-byte intervals, which is diagnostic
ownership evidence only. `paired_codegen_proof` is true only when
the authenticated address-definition chain and the paired `psq_l`/`lfs`/
`psq_st`/`stfs` machine edges are complete; scalar coverage alone cannot claim
that proof.

The source chronology guard distinguishes reviewed C source text from runtime
addresses: an ordinary unary address-of expression such as `&pos` or
`&savedPos[playerNo]` is allowed in a source call argument, while hexadecimal,
decimal, pointer-key, or other serialized runtime-address text remains
forbidden. This distinction applies only to source-expression chronology and
does not relax the pointer-free event or token formats.

Unknown evidence is dependency-scoped. An unsupported opcode, ambiguous
reaching definition, duplicate or reused identity, or incomplete regalloc
invalidates only the source dependency connected to that evidence. A `bl` or
`fneg` outside the dependency interval does not poison unrelated rows; an
unknown dependency touching the stack interval keeps that interval UNKNOWN.
UNKNOWN is never converted into an owner by inference.

### Capture-local template normalization

Reviewed templates may contain placeholders, but they are not sealed evidence.
`normalize-source-spans` binds one reviewed template and one closed binding plan
to one already-authenticated envelope, strips template-only fields, and emits a
fresh capture-local v2 manifest. The plan must have exactly these top-level
fields: `schema`, `function`, `function_sha256`, `session_id`, `source`,
`envelope`, `template`, `objects`, `bindings`, and `authority_advanced`, with
schema `mwcc_source_span_binding_plan/v1` and `authority_advanced: false`.
Plan object rows contain exactly `identity`, `ownership_mode`, `object_type`,
and `byte_size`; binding rows contain exactly `identity`, `role`, `byte_start`,
`byte_end`, `dependency_id`, and `machine_instruction_indices`. The normalized
output supplies the fresh capture-local Object tokens and must be sealed only
after its source, function, envelope, and session checks pass.

Example, with all paths bound to the same fresh capture:

```text
rtk C:\Python313\python.exe tools/capsule_same_session_capture.py normalize-source-spans --envelope C:\proof\same-session.envelope.json --trust-root C:\proof\trust-root.json --template C:\proof\MoveNum.source-spans.unsealed.json --binding-plan C:\proof\MoveNum.binding-plan.json --output C:\proof\MoveNum.source-spans.v2.json
```

The resulting v2 file is capture-local and sealed by normalization, so it can
be passed directly to `causal-map`. A stale template, stale session, or
placeholder token is rejected. `seal-source-spans` remains the command for a
reviewed manifest that already contains exact capture-local tokens.

### Fresh Player GC/2.7 runtime packages

`tools/player_gc27_runtime_package.py` turns an authenticated forensic package
into a fresh, closed current-source package. `materialize` and `validate` are
read-only with respect to the compiler: neither command launches MWCC or the
capture backend. The generated source snapshot is audit evidence only; the
request and compiler argv must continue to name the exact immutable owner
source path. The launcher is rebound to this tooling worktree and must use only
this worktree's `build/.compiler-lane.lock`.

```text
rtk C:\Python313\python.exe -B tools/player_gc27_runtime_package.py materialize --forensic-root C:\proof\forensic-package --output-root C:\proof\fresh-package --current-tool C:\tooling\tools\capsule_same_session_capture.py --source-span-template C:\owner\MoveNumOMExec.source-spans.unsealed.json --source C:\owner\player.c --template-manifest C:\owner\manifest.json
rtk C:\Python313\python.exe -B tools/player_gc27_runtime_package.py validate --package-root C:\proof\fresh-package
```

Both commands fail closed on an existing output root, symlinks, unexpected or
missing files, forensic hash drift, output collisions, stale hooks, session
disagreement, or a request/manifest schema mismatch. The one session identity
must match `session-[0-9a-f]{16}` exactly. The hook manifest must contain the
closed 13-hook GC/2.7 profile, including machine emission at `0x004EB21F`, and
must exclude stale `0x004D03E8`. A `VALID` result is package eligibility only;
it does not authorize or imply a live capture.

### Measured execution receipts

`tools/mwcc_execution_receipt.py` is the append-only execution journal for this
runtime task. A request is invalid unless `active_seconds` is a measured,
strictly positive value, `active_seconds_measured` is true, and its separate
`mwcc_active_seconds_measurement/v1` receipt is complete. The measurement
receipt uses a monotonic/perf-counter clock, ordered non-overlapping intervals,
and interval durations whose exact sum equals `active_seconds`. Null, zero,
estimated, or unmeasured active time is not accepted. Each journal row is
hash-chained, repeats the authenticated request descriptors, and preserves
`diagnostic_only: true` with `authority_advanced: false`.

Start the measurement immediately before the authorized one-shot action and
stop it immediately afterward. The closed validation/append workflow is:

```text
rtk C:\Python313\python.exe tools/mwcc_execution_receipt.py measure-start C:\proof\active.open.json
rtk C:\Python313\python.exe tools/mwcc_execution_receipt.py measure-stop C:\proof\active.open.json C:\proof\active.closed.json
rtk C:\Python313\python.exe tools/mwcc_execution_receipt.py validate-request C:\proof\execution-request.json
rtk C:\Python313\python.exe tools/mwcc_execution_receipt.py append C:\proof\execution-request.json C:\proof\execution-receipts.jsonl
rtk C:\Python313\python.exe tools/mwcc_execution_receipt.py validate-journal C:\proof\execution-receipts.jsonl
```

`append` may only extend the existing journal under its append lock; it does not
rewrite a prior row or replace the request. Validation must pass before a live
capture is considered eligible.

## Verified runtime records

The records below are the sealed results for task
`4af0c0e3ce3a4e2b96dfa7dd2f5258df`. They are diagnostic-only: every package,
capture, map, and execution receipt has `diagnostic_only: true`,
`board_admission: false`, and `authority_advanced: false`. Active time is
mandatory; a null, estimated, or zero duration is not accepted.

### MoveNum current-source final12

Package root:
`build/player-gc27-tracer-runtime-v1/movenum-current-source-v566-final12`
(`session-8e7a3c19b4d260f1`). It uses the closed, ordered 13-hook profile,
including `gc27_machine_emit` at `0x004EB21F`; the stale GC/2.6
`0x004D03E8` hook is absent. Retained and v491 child objects both reproduce
`e0806715086606bc51162bbbb90208aa1aa5cd99944738f79a8c515d58cde686`.

The exact package request SHA-256 is
`87cfa8bc18397da5ab536f9516935713b6cc36d9651494e9a78818de8927b9c1`.
The measured active-time receipt is `27.3668218` seconds, SHA-256
`77722f5b5eb58e2a7c0f36e634b1f574f179f001a7722ee15bbe33f8a911c783`.
The v2 maps prove scalar copy pairs `113->115`, `114->116`, and `117->118`;
both `posNorm` and `pos` stack-interval joins are
`MATCHED_AUTHENTICATED`, there is no seam `UNKNOWN`, and
`paired_codegen_proof` is deliberately `false` because the candidate emits
scalar loads/stores rather than target paired codegen. The final v3 map
artifacts are:

- retained: file SHA-256
  `841c3acee17a9a68a589edfd7c1575ea16861fea5f83662622894b9d877b3e95`,
  internal `f4f4125b57994e78ec6882036a41498380d7ed04a23bcac211572cddb3728664`;
- v491: file SHA-256
  `fbe054c9e41d35decff36f1216c149067c68d02b74a9ed27c141a2978a6db4e2`,
  internal `2bbfa2c44800df5cc27cc52396514454331b9fb44f854da84a3453ee5ac3a4e7`.

The final12 execution request is
`post-capture/movenum-final12.execution-request-v3.json`, SHA-256
`bf609561d5dd55e9230859847072aaad388ef4f1798c01bc11d3306bd62256a1`;
the append-only journal is
`post-capture/execution-receipts-v3.jsonl`, SHA-256
`6759c4609c65970938192ce061fb6c2a505db41a6e6a6443b681685cfd01d985`,
with head `d714f1af6871fa79f25add3f45679666da95f067f398e0a3a1764312df0f0add`.

The exact one-shot invocation sealed by the package is:

```text
rtk C:\Python313\python.exe -B C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-current-source-v566-final12\inputs\launch_movenum_capture.py --request C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-current-source-v566-final12\backend-request.json --backend C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-current-source-v566-final12\backend\movenum-pcode-color-v523\physical_capture_backend_v523.py --run-output C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-current-source-v566-final12\backend-output --plan-output C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-current-source-v566-final12\live-execution-plan.json --execute
```

### Radius current-source final2

Package root:
`build/player-gc27-tracer-runtime-v1/radius-current-source-v567-final2`
(`session-9a6d1b3c5e7f2048`). It was captured under the same exact 13-hook
profile and the private `build/.compiler-lane.lock`; the current capture tool
SHA-256 is `af3bd03d4476a909de097abf1a1a1d8c023d3a2b01a5b76c43c7d09650d89ec4`.
Both mirrored child objects are
`e0806715086606bc51162bbbb90208aa1aa5cd99944738f79a8c515d58cde686`.

Each physical envelope contains 19 committed physical events; each raw
same-session envelope contains 303 events, with 260 PCode and 43 stack events.
The retained raw envelope file SHA-256/internal envelope hash are
`228cd76e133d5a73036519dcf014a35e04e077a7724bb721c0d76ff2bb9b78b0` /
`38d29212cb1e78e0cfc7d2fad16e1c16ce0c6c4efe30a05f0bb7cec67606cc51`.
The v491 raw file/internal hashes are
`580b28e6ffd0c1adaab50a2bd7743456851289895d5b370c03ddf285056749ea` /
`60bee6cf7f99db63cdc3878d41c6cf9cdad7c860be35423a57c3a2bb07308e79`.
The source-span joins are deliberately `UNKNOWN`: the predeclared target
intervals conflict with the capture-local final homes, so no token or stack
lifetime edge is invented.

The package request SHA-256 is
`a7a876e37a225b384197a3fb31c393082a2d51d659862c1e808e1870e5ee8941`.
Measured active time is `20.2293315` seconds; the measurement receipt SHA-256
is `c631842ea9d4afb38752ceeef7bf699cbdebc9aaaa0ef43f5f4db27c8e008de6`.
The retained and v491 post-capture trust roots are
`2b21c2d22881a2d86bc33bcab39012872abc771b94608f112925160037ca05ed` and
`2499af7295d4bc7c13b84ca8ea54d84721c97bc64262b122595278746751c9ac`.
The live execution plan SHA-256 is
`a2784cf9b0e2548a9a62627326979fbe1ebcefc03a81eb57586b55a27f7c4cd6`.
The execution request is `post-capture/radius-final2.execution-request.json`,
SHA-256 `c235eb2229d5b5cce14d31e03740c6587d91a00ae0f5a4de40a609b5a9fcc8d5`;
the journal SHA-256 is
`b2ac6019dcc8914c4964a26a8e57b144c7b85762634f499af62f37b2ed1c1552`,
with head `92492d0db78e367e642d412ba102819c7504434af01de30017efc5f3a5ee8ac2`.

The exact one-shot invocation is:

```text
rtk C:\Python313\python.exe -B C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\radius-current-source-v567-final2\inputs\launch_movenum_capture.py --request C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\radius-current-source-v567-final2\backend-request.json --backend C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\radius-current-source-v567-final2\backend\movenum-pcode-color-v523\physical_capture_backend_v523.py --run-output C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\radius-current-source-v567-final2\backend-output --plan-output C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\radius-current-source-v567-final2\live-execution-plan.json --execute
```

### Historical MoveNum v523e replay v4

The prepared replay package is
`build/player-gc27-tracer-runtime-v1/movenum-v523e-historical-replay-v4/prepared`
under session `session-523e000000000003`. Preparation and comparison never
launch MWCC:

```text
rtk C:\Python313\python.exe -B tools/player_gc27_historical_replay.py prepare --request C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-v523e-historical-replay-v4\historical-replay-request.json --output-root C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-v523e-historical-replay-v4\prepared
rtk C:\Python313\python.exe -B C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-current-source-v568-resealed2\inputs\launch_movenum_capture.py --request C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-v523e-historical-replay-v4\prepared\backend-request.json --backend C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-current-source-v568-resealed2\backend\movenum-pcode-color-v523\physical_capture_backend_v523.py --run-output C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-v523e-historical-replay-v4\prepared\backend-output --plan-output C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-v523e-historical-replay-v4\prepared\live-execution-plan.json --execute
rtk C:\Python313\python.exe -B tools/player_gc27_historical_replay.py compare --package-root C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-v523e-historical-replay-v4\prepared --output C:\Users\Anony\.codex\mp6-wt-player-gc27-tracer-runtime-v1\build\player-gc27-tracer-runtime-v1\movenum-v523e-historical-replay-v4\prepared\comparison-receipt.json
```

Replay v4 compiled the retained exact object
`e4058cfd9b366a16f4f42e5c56821896c6bb9f40eb94cad1bc7393bda4c63cf7` and v491
exact object `ee588fbff9f167fc9ddb88a0f57938ddca7c2592de0afe18fc3afbc5ccfe1f82`.
Each lane has 12 physical events and 988 PCode rows. The replay envelopes are
`1b9ff6cffcc9a5796745d127786f917478dcb32ebf1053b88101aab063d9b2d1` and
`0c48925eec808836c6dce85b75ee0dc6eb4a519c4141980c08075f54615836b9`; the
comparison receipt SHA-256 is
`2dc01fe36e430dddba59c0d573177f22ec63a5cf8793407c723d37b811964b0b` and its
status is `MATCH` for both lanes with no differences. Active time is measured
at `92.9903873` seconds; measurement receipt SHA-256 is
`40c0818a3fe42ab6833350c98e52acb94895030630198625dc99ade913b55152`.
The execution request SHA-256 is
`53f49971a9ff702f9d4c218f0d8c2f372352dd8566bb4f98abc00d98220a3ea0`, the
journal SHA-256 is
`b337e29d87753dfecfd22696fae16fceaadef0509981610ef8a891772969c747`, its head is
`d026bb8d524d93da41b265952bed260ef6129c8cf8760903ac8f0ab2bbe494c2`, and
the prepared replay plan SHA-256 is
`ddd21d6198687d5f418665e694c016c33413631474ba3e7017fb5976b363f542`.
The launched live execution plan is separately bound at
`ad1a0dfa8eacbed6e30773bd173ef5d8315f504af33a7c39367b8fb360778cfe`;
the backend request and prepared package receipt are
`5ef9a574e2b0bee3defaaae3126ed8bf56eb12507a71f4037c4bbfc0c165ff2c` and
`3fe1852da1f0c1e9f399f18be91a38cfd54efe717cd48093dfffc6edb03ac5dc`.
Historical replay proves physical/PCode equivalence only; its source-span
join remains `UNKNOWN` by contract.

### Preserved fail-closed attempts and transport resolution

The v2 historical replay package is preserved as a no-run diagnostic record;
its stale-tool-bound request was rejected before compiler selection when the
same-session capture tool bytes changed. It was not retried in place. Replay
v3 did launch and produced 12 events/988 PCode rows in each lane, but retained
comparison failed closed on the semantic expected-object reference
(`226f5aa6...` versus compiled `e4058cfd...`); v491 matched. v4 keeps both
historical semantic roles explicit, revalidates the semantic reference, and
then produces the two-lane `MATCH`. These records are retained to prevent
rediscovery and are not success claims.

The runtime repair used the authenticated direct GC/2.7 compiler transport
alternative (`execution.mode = authenticated_direct_compiler`) because the
sjiswrap same-PID memexec selection boundary is unreliable. The direct path
still seals the wrapper/compiler descriptors, argv, cwd, exact 13-hook profile,
compiler SHA, output ownership, private lock, and trust roots; it is not a
general wrapper bypass. The package/root sealing is absolute-path and
cross-CWD validated, and package materialization/validation never launches
MWCC. Fresh capture-local tokens and separate retained/v491 manifests remain
required for source joins.

### Requirement status

- Compiler selection/transport: **PASS** through the authenticated direct
  GC/2.7 path; wrapper selection remains fail-closed when ambiguous.
- Exact closed profile: **PASS** for both current-source packages: 13 hooks,
  machine `0x004EB21F`, stale `0x004D03E8` absent.
- MoveNum historical replay: **PASS** in v4 (`MATCH` for both lanes).
- MoveNum source/stack causal join: **PASS** for final12, including the
  authenticated stack intervals and scalar copy pairs; paired proof remains
  deliberately false.
- Radius nonempty physical/PCode capture: **PASS** (19 physical events,
  303 raw events, 260 PCode, 43 stack per lane); source-span join is **UNKNOWN**
  because the predeclared homes conflict with capture-local homes.
- Active-time accounting: **PASS** for final12, Radius final2, and replay v4;
  all receipts contain measured positive seconds and validate against their
  append-only journals.
- Diagnostic/promotion boundary: **PASS**; no source retention, Board
  admission, commit, push, or authority advancement is claimed.

## Output interpretation

Each `joined_objects` row contains verified source spans, the authenticated
virtual register, at most one physical GPR/FPR assignment, stack write/home
chronology, and source call-return chronology for the exact assigned identity.
`machine_emission` events add the emitted instruction/stack/lifetime or
arithmetic edge. Their `owner_joins` list stays empty unless the same session
authenticated the complete Object→vreg→physical-register chain. A
`physical_owner_joins` row may still be exact when the same PCode token's
authenticated color observation directly binds an Object/IG owner to that
physical operand; this never invents the missing vreg edge.
`source_evaluation_chronology` records call evaluation order from the bound
function text. The report also hashes every composing tool source. The
correlator's deterministic `report_sha256` is SHA-256 over canonical JSON of
the complete correlator report with only `report_sha256` omitted.

`MATCHED_AUTHENTICATED` means only that the captured identity edge is complete.
It is not source provenance, matching proof, or permission to edit/retain. Any
missing span, missing PCode/frontend packet, duplicate name/token, reused
identity, conflicting physical assignment, source drift, compiler drift, argv
or cwd drift, hook drift, or manifest tamper yields UNKNOWN or a nonzero
fail-closed exit.
