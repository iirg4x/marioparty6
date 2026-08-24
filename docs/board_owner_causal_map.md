# Full-owner causal map

`tools/board_causal_map.py` is the read-only checkpoint G entry point. It joins
one owner's authenticated matching history to every residual target-side
function in one bound objdiff report. It does not compile, edit source, write a
workbench record, retain a candidate, or advance recovery authority.

The command composes the installed workbench components instead of interpreting
their domains again:

- `match_workbench.build_matrix` validates the complete immutable candidate
  index and supplies accepted/rejected axis history;
- `match_workbench.build_function_telemetry` supplies function-scoped campaign
  telemetry where indexed focus history exists;
- `match_workbench.reduce_objdiff_cascades` supplies causal residual clusters;
- `match_workbench.decode_pool_ownership` supplies typed pool-owner evidence;
- `match_workbench.inspect_stack_residue` supplies stack-slot evidence;
- `match_workbench.plan_candidate_interactions` supplies the bounded factorial
  plan; and
- `match_workbench.list_donor_shapes` and `mwcc_fe_chronology.load_report`
  supply optional donor and tracer-receipt context.

The new layer is limited to closed input validation, immutable identity joins,
the per-function inventory, deterministic dependency closure, explicit coverage
lanes, and the final self-hash. A successful result always contains
`production_modified: false` and `authority_advanced: false`.

## Graph-first entry gate

Cross-file source-location discovery starts with the existing canonical Graphify
graph, not with a repository scan and not with a per-worktree rebuild:

```text
repository root:
D:\Games\Emulation\GameCube-Wii\_mp6_rebuild\port\mp6-native

canonical graph:
D:\Games\Emulation\GameCube-Wii\_mp6_rebuild\port\mp6-native\graphify-out\graph.json
```

From that repository root, query expanded codebase terms, then confirm each
selected node with `graphify path` and `graphify explain`. For example:

```powershell
rtk graphify query board-function-source-owner-capsule-matching --budget 1600
rtk graphify path capsule.c CapPlayerThrow
rtk graphify explain CapPlayerThrow
```

Bind the exact canonical graph size/SHA-256 plus the exact node ID, label,
`source_file`, and `source_location` returned by Graphify. The command rereads
the graph and rejects any requested location that does not match its node.

The fallback contract is deliberately narrow:

1. If the canonical graph exists, use `query`, `path`, and `explain` first.
2. Do not build another graph in an owner worktree.
3. If the graph lacks the needed fact or cannot be bound, use only an explicitly
   scoped, file-local inspection for investigation and omit `graph` from this
   request. The output then keeps `coverage.graph_source_locations` `UNKNOWN`.
4. Never infer or hand-author positive Graphify evidence from a fallback read.

Graph evidence is optional and diagnostic. Even an exact node join does not
prove target equivalence or original source shape.

## Invocation

Prepare a closed JSON request, including current byte sizes and lowercase
SHA-256 values for every file descriptor, then run:

```powershell
python tools/board_causal_map.py owner-causal-map-request.json --root . > owner-causal-map.json
```

Paths may be absolute or relative to `--root`. Output is canonical in content
and stable across repeated runs over unchanged inputs; the pretty-printed CLI
form is written only to stdout. Diagnostics go to stderr and return exit code 2.

## Request schema

The root is a closed object with `schema` equal to
`board_owner_causal_map_request/v1` and `schema_version` equal to `1`.
Unknown fields and duplicate JSON keys are rejected.

Required fields:

| Field | Contract |
|---|---|
| `owner` | Non-empty owner identity; must equal the immutable workbench session owner. |
| `source` | File descriptor plus `candidate_id`; both must identify the selected workbench candidate. |
| `target` | File descriptor; must equal the immutable workbench target. |
| `compiler` | `toolchain_key`, authenticated `compiler_sha256`, and complete compiler `context_sha256`; all must equal the session context. |
| `report` | File descriptor plus `kind` (`strict` or `data`); its bytes must equal that report in the selected candidate record. |
| `workbench` | `path`, immutable `session_id`, and `session_sha256`. |
| `interaction_request` | File descriptor for the installed factorial planner request. Its focus set must equal the report's complete residual-symbol set. |

A file descriptor is `{path, size_bytes, sha256}`. The command rereads every
descriptor and fails if its bytes changed.

Optional fields:

| Field | Contract |
|---|---|
| `target_assembly` / `candidate_assembly` | Bound assembly descriptors forwarded to the installed causal reducer for relocation context. |
| `donor_registry` | Bound donor-registry descriptor; listings remain diagnostic and function-scoped. |
| `tracer_receipts` | At most 32 receipt descriptors, each adding non-empty `focus_symbols`. Receipt source/compiler provenance must match this request. |
| `graph` | Canonical graph descriptor plus `source_locations`; each row has `function`, `node_id`, `node_label`, `source_file`, and `source_location`. |
| `telemetry` | Optional positive `elapsed_seconds`/`active_seconds` and nonnegative `tracer_runs`/`donor_searches`; omitted values become `null`. |

Minimal shape, with example hashes abbreviated here only for readability:

```json
{
  "schema": "board_owner_causal_map_request/v1",
  "schema_version": 1,
  "owner": "REL:board:capsule",
  "source": {
    "path": "game/src/board/capsule.c",
    "size_bytes": 12345,
    "sha256": "<64 lowercase hex>",
    "candidate_id": "candidate-0042"
  },
  "target": {
    "path": "build/orig/capsule.o",
    "size_bytes": 23456,
    "sha256": "<64 lowercase hex>"
  },
  "compiler": {
    "toolchain_key": "GC/2.6",
    "compiler_sha256": "<64 lowercase hex>",
    "context_sha256": "<64 lowercase hex>"
  },
  "report": {
    "path": "scratch/candidate-0042/strict.json",
    "size_bytes": 34567,
    "sha256": "<64 lowercase hex>",
    "kind": "strict"
  },
  "workbench": {
    "path": "scratch/match-workbench/capsule",
    "session_id": "capsule-session",
    "session_sha256": "<64 lowercase hex>"
  },
  "interaction_request": {
    "path": "scratch/capsule-interactions.json",
    "size_bytes": 4567,
    "sha256": "<64 lowercase hex>"
  }
}
```

The angle-bracket hash examples are documentation placeholders and are not
accepted by the command.

## Output schema

Successful output has `schema: board_owner_causal_map/v1`, `schema_version: 1`,
and these top-level sections:

| Section | Meaning |
|---|---|
| `bindings` | Revalidated request, source, target, compiler, report, workbench/matrix, interaction, optional assembly/donor, and optional graph identities. |
| `inventory` | Target/exact/residual counts and one row for every nonexact target-side function occurrence. |
| `rejected_context` | All rejected/no-go matrix axes and optional global donor rejections. |
| `next_axes` | Planner-ranked `generate_and_compile` cells. Every row expands every selected axis/level into `dependency_closure`, including hypothesis, control, source action, evidence, and admissibility. |
| `coverage` | Evidence availability by lane. Missing evidence is `UNKNOWN`; unavailable producer evidence may be `BLOCKED`; partial per-function coverage is `PARTIAL`. |
| `composition` | Component schemas/hashes, session/current-candidate identities, validated-candidate count, and the explicit delegate audit. |
| `limitations` | Non-authoritative interpretation boundaries. |
| `causal_map_sha256` | SHA-256 of canonical JSON for the complete output body before this field is added. |

Each `inventory.functions[]` row includes:

- stable report `identity`, `symbol`, and `occurrence`;
- `metrics` with target/candidate sizes, match percentage, diff-row count,
  diff-kind counts, exact/paired state, and paired symbol;
- the earliest address-ordered supported structural cluster or an explicit
  `UNKNOWN` reason;
- report/cluster relocation signals while keeping physical relocation authority
  `UNKNOWN`;
- typed pool summary/groups and stack-slot residue evidence;
- function-scoped rejected axes, optional donor records, optional graph nodes,
  available tracer receipts, telemetry, and component hashes.

Duplicate symbol occurrences and unpaired functions remain in the inventory,
but structural, pool, relocation, stack, and ambiguous symbol-only telemetry
lanes remain `UNKNOWN`.

Example abridged output:

```json
{
  "schema": "board_owner_causal_map/v1",
  "schema_version": 1,
  "owner": "REL:board:capsule",
  "status": "residuals_present",
  "inventory": {
    "residual_function_count": 1,
    "functions": [
      {
        "symbol": "CapPlayerThrow",
        "metrics": {
          "target_size": 1188,
          "candidate_size": 1192,
          "diff_rows": 7,
          "diff_kinds": {"DIFF_RELOC_MISMATCH": 2}
        },
        "earliest_structural_cause": {
          "status": "KNOWN",
          "classification": "relocation_or_data_mismatch"
        }
      }
    ]
  },
  "coverage": {
    "tracer": {
      "status": "UNKNOWN",
      "reason": "no source-aware tracer receipt was supplied; native producer checkpoint A is unavailable"
    },
    "physical_relocations": {
      "status": "UNKNOWN",
      "reason": "objdiff relocation signals are report-derived, not physical relocation proof"
    }
  },
  "production_modified": false,
  "authority_advanced": false,
  "causal_map_sha256": "<64 lowercase hex>"
}
```

The abridged example omits required successful-output fields and uses a hash
placeholder; it is explanatory, not a validation fixture.

## Fail-closed joins and authority

The command exits without a map when any of these joins fails:

- owner, target, workbench session, selected source/candidate, report bytes, or
  compiler/toolchain/context differ;
- any immutable candidate lacks compile attestation;
- the matrix does not contain exactly one consistent selected candidate;
- planner focus symbols are not exactly the residual set, or an observed
  planner candidate has different source/object hashes;
- optional donor, tracer, assembly, graph, or request descriptors changed;
- a tracer receipt has different source/compiler provenance;
- a graph function is absent from the report or its node facts differ; or
- an installed component returns `authority_advanced` other than `false`.

`KNOWN` means only that the named diagnostic lane was joined. It is not source
authenticity, semantic equivalence, physical relocation proof, linked-retail
closure, promotion approval, or candidate-retention authority.
