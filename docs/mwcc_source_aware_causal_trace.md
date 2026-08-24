# Source-aware MWCC causal trace

This checkpoint is a diagnostic-only join over authenticated GC/2.6 evidence.
It does not edit Board source, compile a candidate, retain a result, or advance
authority. Every causal evidence artifact sets `authority_advanced: false`.

## Components and non-duplication boundary

- `tools/capsule_stack_home_native.py` remains the only native producer of the
  six stack-allocation/write hooks. `capsule_same_session_capture.py` imports
  those hook identities and adds only the authenticated vreg/physical-register
  hooks and one event bus.
- `tools/mwcc_fe_chronology_native.py` supplies the pointer-free frontend
  Object/VarInfo chronology contract. A missing packet is reported as UNKNOWN.
- `tools/pcode_varinfo_correlator.py` supplies the direct ownership gate. Null,
  reused, duplicate, one-to-many, or unauthenticated identities never become a
  name join.
- `tools/donor_cfg_align.py` parses the exact bound source function and emits
  deterministic assignment, call-return, evaluation, and control-flow spans.

No raw pointer is serialized. Capture-local Object tokens include the sealed
session ID and cannot be replayed into another request.

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

First authenticate or capture one immutable same-session envelope (the existing
`prepare`, `preflight`, and `capture` commands remain unchanged). Then seal the
reviewed span binding and build the causal map:

```text
rtk C:\Python313\python.exe tools\capsule_same_session_capture.py seal-source-spans --input C:\proof\source-spans.unsealed.json --output C:\proof\source-spans.json

rtk C:\Python313\python.exe tools\capsule_same_session_capture.py causal-map --envelope C:\proof\same-session.envelope.json --trust-root C:\proof\trust-root.json --source-spans C:\proof\source-spans.json --output C:\proof\source-aware-causal-map.json
```

If an independently sealed frontend packet exists, add:

```text
rtk C:\Python313\python.exe tools\capsule_same_session_capture.py causal-map --envelope C:\proof\same-session.envelope.json --trust-root C:\proof\trust-root.json --source-spans C:\proof\source-spans.json --frontend-chronology C:\proof\frontend-chronology.json --output C:\proof\source-aware-causal-map.json
```

The packet's source and compiler hashes must equal the same-session envelope.
Its exact function and capture session ID must also equal the envelope; a packet
for another Board function or session is rejected even when source/compiler
hashes happen to match. Without it, `frontend_chronology.status` is `UNKNOWN`;
the tool never imports a chronology from a separate compiler process by name.

## Output interpretation

Each `joined_objects` row contains verified source spans, the authenticated
virtual register, at most one physical GPR/FPR assignment, stack write/home
chronology, and source call-return chronology for the exact assigned identity.
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
