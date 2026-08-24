# Authenticated MWCC stack-home capture

`tools/capsule_stack_home_native.py` is a diagnostic-only native producer for
MWCC GC/2.6 Board functions. It records compiler `Object` and `VarInfo`
chronology around the audited numeric stack allocator and three `Object+0x2e`
write sites. It does not decide source admissibility, exactness, retention, or
promotion.

The installed capture contract is version 4:

- request: `mwcc_capsule_stack_home_native_request/v4`;
- events: `mwcc_capsule_stack_home_event/v4`;
- packet: `mwcc_capsule_stack_home_native/v4`;
- producer: `capsule-stack-home-native-4`; and
- summary: `mwcc_capsule_stack_home_summary/v1` with self-hash
  `summary_sha256`.

Older requests and packets are intentionally rejected. Regenerate them from
the current immutable inputs; do not edit a sealed request.

## Trust boundary

Every capture is bound to all of the following before the compiler launches:

- one canonical C function identifier and externally supplied function hash;
- one exact source file below an authenticated `src/board` path;
- the source SHA-256, size, and resolved path;
- the baseline report SHA-256, size, and resolved path;
- the pinned GC/2.6 compiler, wrapper/producer, debugger, emulator, and GDB;
- the compiler working directory and complete argument vector;
- the exact six compiler hook byte prefixes; and
- an external authority manifest outside the writable capture directory.

The external authority manifest must use
`mwcc_capsule_stack_home_authority/v1` and contain:

```json
{
  "schema": "mwcc_capsule_stack_home_authority/v1",
  "source": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
  "function": {
    "name": "mbev_CapTogezo",
    "sha256": "...",
    "source_sha256": "..."
  },
  "artifacts": {
    "source": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
    "baseline": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
    "compiler": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
    "producer": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
    "debugger": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
    "emulator": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
    "gdb": {"path": "ABSOLUTE", "size": 0, "sha256": "..."}
  }
}
```

The request manifest has the closed key set printed below. Artifact values may
be absolute paths or exact `{path,size,sha256}` descriptors. Descriptor form is
preferred for durable handoffs.

```json
{
  "function": "mbev_CapTogezo",
  "function_sha256": "...",
  "cwd": "ABSOLUTE COMPILER CWD",
  "argv": ["-nodefaults", "...", "-c", "ABSOLUTE SOURCE", "-o", "ABSOLUTE OUTPUT"],
  "source": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
  "baseline": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
  "compiler": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
  "producer": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
  "debugger": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
  "emulator": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
  "gdb": {"path": "ABSOLUTE", "size": 0, "sha256": "..."},
  "authority_manifest": {"path": "ABSOLUTE", "size": 0, "sha256": "..."}
}
```

The argument vector contains compiler arguments only; it must not repeat the
compiler executable. It must contain exactly one `-c` source and one `-o`
candidate output. The output must stay inside the new empty capture directory.

## Commands

Prepare authenticates files, hashes, paths, PE identity, hook prefixes, and the
external authority document without launching MWCC:

```powershell
rtk C:\Python313\python.exe tools\capsule_stack_home_native.py prepare `
  --manifest C:\absolute\request-manifest.json `
  --output-dir C:\absolute\new-empty-capture
```

Preflight re-authenticates the sealed request and live prerequisites without
launching the compiler:

```powershell
rtk C:\Python313\python.exe tools\capsule_stack_home_native.py preflight `
  C:\absolute\new-empty-capture\request.json
```

Capture is the only command that launches MWCC and the native transport:

```powershell
rtk C:\Python313\python.exe tools\capsule_stack_home_native.py capture `
  C:\absolute\new-empty-capture\request.json
```

`--trace` may be added for transport diagnostics; trace output is not authority
and is not part of source admission. Validate the resulting sealed packet:

```powershell
rtk C:\Python313\python.exe tools\capsule_stack_home_native.py validate `
  C:\absolute\new-empty-capture\trace.packet.json
```

Summarize first runs the same complete packet validation, then joins only the
requested exact compiler names through the sealed Object↔VarInfo pair to the
authenticated `Object+0x2e` pre/post events. Repeat `--name` in the desired
output order. `--output` must be a new absolute `.json` sibling of the packet:

```powershell
rtk C:\Python313\python.exe tools\capsule_stack_home_native.py summarize `
  C:\absolute\new-empty-capture\trace.packet.json `
  --name playerPosCur `
  --name playerRot `
  --name playerPosNext `
  --name coinVel `
  --name coinPos `
  --name togezoPos `
  --name motionId `
  --output C:\absolute\new-empty-capture\summary.json
```

Given identical packet bytes, canonical paths, and `--name` order, the summary
and `summary_sha256` are deterministic. The command exclusively creates the
output and refuses an existing file. It rejects a missing, `UNKNOWN`,
duplicate, or ambiguous exact name; a missing or non-unique VarInfo snapshot;
an Object without an authenticated stack-home write; a missing post-step; or a
slot observed for multiple Object identities.

The producer's deterministic non-MWCC contract check is:

```powershell
rtk C:\Python313\python.exe tools\capsule_stack_home_native.py self-test
```

## Packet interpretation

Raw process pointers are never serialized. Capture-local tokens such as
`object-000000` and `varinfo-000000` identify one compiler generation. The
`compiler_list.objects` inventory binds each Object token to exactly one
VarInfo token, datatype byte, and name status:

- `EXACT` means MWCC exposed one canonical C identifier;
- `UNKNOWN` has a null name; malformed, compiler-private, and pointer-like text
  is discarded rather than copied into the packet.

`varinfo_home_snapshot` reports the corresponding `VarInfo+0x26` home value.
`object_stack_write_pre/post` reports the authenticated `Object+0x2e` write
chronology. Names do not authenticate a source declaration span, inline owner,
or semantic role; those remain separate causal-join work.

The summary preserves the sealed `object_token` and `varinfo_token`, the
`VarInfo+0x26` snapshot, every paired `Object+0x2e` write, and sorted physical
`mapped_slots`. Each mapping says `owner:"UNKNOWN"`; the root says
`diagnostic_only:true`, `board_admission:false`, `exactness_claim:false`, and
`authority_advanced:false`. A mapped physical slot is not an ownership claim.

For the representative authenticated `mbev_CapTogezo` acceptance source
SHA-256 `1aa01bc8391ee18db9430de1a5da38a0d3e3d9e95ef0343987b1dd424418a236`,
the expected exact-name slot mapping is:

| Compiler name | Object | VarInfo | Stack-home slot |
|---|---|---|---:|
| `playerPosCur` | `object-000009` | `varinfo-000009` | 104 |
| `playerRot` | `object-000007` | `varinfo-000007` | 80 |
| `playerPosNext` | `object-000008` | `varinfo-000008` | 92 |
| `coinVel` | `object-000004` | `varinfo-000004` | 56 |
| `coinPos` | `object-000005` | `varinfo-000005` | 68 |
| `togezoPos` | `object-000006` | `varinfo-000006` | 152 |
| `motionId` | `object-000016` | `varinfo-000016` | 140 |

These tokens are capture-local and must be re-established by each new sealed
capture; the table records the acceptance case, not a reusable pointer map.

The packet is always diagnostic: `diagnostic_only:true`,
`board_admission:false`, `exactness_claim:false`, and no authority is advanced.

## Fail-closed behavior

Preparation, capture, validation, or summarization rejects stale hashes,
aliases/symlinks, a non-Board source path, mismatched function/source authority,
unexpected compiler bytes, wrong hook prefixes, duplicate/reused pointers,
Object↔VarInfo rebinding across a captured write, missing single-step post
events, partial functions, disconnects, nonzero compiler exit, malformed names,
unsupported packet fields, and ambiguous summary joins. A failed capture or
summary is evidence of no result, not an `UNKNOWN` ownership claim that callers
may promote.

## Operational ownership

Board orchestrators may request a bounded capture for an immutable source and
function. The Recovery Manager owns installation and cross-owner tooling. The
tool never edits production source, records a candidate, acquires integration,
or decides retention. Store the manifest, authority file, sealed request,
event stream, packet, validation output, summary, and their hashes in the
function's private artifact directory.
