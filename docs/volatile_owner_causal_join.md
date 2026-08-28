# Volatile-owner causal join

`tools/volatile_owner_causal_join.py` is a downstream, diagnostic-only reducer
for authenticated same-session ownership failures. It joins an exact residual
row set from `focus_symbol_report/v1` to capture-local PCode, interference-graph,
virtual-register, final-color, physical-register, and def/use facts. A successful
result proves only that bounded causal join and ranks one caller-allowlisted
natural source class. It does not prove source spelling or originality.

The reducer never emits source or a patch and never grants retention, recovery,
board-admission, integration, or promotion authority. `PROVEN` means every gate
below closed. Every incomplete or competing evidence path produces deterministic
`UNKNOWN`; malformed, unsafe, or hash-mismatched inputs are rejected.

## Inputs

The context schema is `volatile_owner_causal_join_context/v1` and is described
by `tools/VOLATILE_OWNER_CAUSAL_JOIN_CONTEXT_V1.schema.json`. Its
`context_sha256` seals every field except itself. It externally binds the focus
artifact file and self-digest, strict and data objdiff report digests, the sealed
source-span file and self-digest, target and candidate object paths/size/digests,
an independent physical-relocation receipt path/file/payload digest, and the
ownership graph file and self-digest. It also supplies:

- a `residual_row_bindings` list from capture `row_id` to focus row, explicit
  zero-based candidate register-operand position, captured machine role, and
  semantic role (`index`, `base`, `result`, or a bounded arithmetic operand
  role). Several bindings may name distinct operand positions of one focus row;
- a `source_class_allowlist`; and
- explicit `source_class_hypotheses`, each naming the exact graph rows it covers.

The ownership graph remains
`mwcc_capsule_same_session_ownership_failure_graph/v1`. Older graphs are valid
inputs but lack the additive `volatile_owner_facts` section and therefore return
`UNKNOWN`. The additive section is authenticated by both
`volatile_owner_facts_sha256` and the outer `failure_graph_sha256`. It binds the
session, function, source, compiler, compiler-owned candidate object, raw event
hashes, pointer-free object identities, the closed residual inventory, and owner
facts. Only a `PRESENT` object identity plus capture classification `UNIQUE`
with one vreg, matching final color/physical register, one definition, and at
least one use can close a row. `UNKNOWN` hidden-object bindings never close a
row. PCode and IG tokens must carry the section session ID; row, fact,
definition, and use tokens must use the producer's canonical formats.
`PRESENT` object tokens must be `local-<session>-NNNNNN` or
`argument-<session>-NNNNNN`; hidden identities must be
`hidden-ig-<session>-NNNNNN`. Every raw event ID must begin with that same
session and use the canonical `-eNNNNNN` suffix.

Example invocation:

```sh
rtk python tools/volatile_owner_causal_join.py \
  --context C:\proof\volatile-owner-context.json \
  --context-sha256 <externally-recorded-context-sha256> \
  --focus C:\proof\focus.json \
  --source-spans C:\proof\source-spans.json \
  --ownership-graph C:\proof\ownership-failure-graph.json \
  --output C:\proof\volatile-owner-join.json
```

Exit status is `0` for `PROVEN`, `1` for evidence-complete parsing that remains
`UNKNOWN`, and `2` for rejected input.

The context self-digest detects internal mutation but is not its own trust root.
The public CLI and `build_from_paths` therefore require the caller to supply the
expected canonical `context_sha256` independently. The tool compares that value
before loading the other evidence. Omitting the CLI option is an argument error;
a mismatching external anchor is rejected. Direct `build_join` remains available
for deterministic in-memory unit construction, but it also requires
`expected_context_sha256` and rejects a substituted context before evaluating
positive evidence. `build_from_paths` passes through the caller-supplied value;
there is no pre-authenticated positive API path.

## Closure rules

Both strict and data report bindings are mandatory. Their target and candidate
diff row IDs must agree, and the data channel must name the strict residual set.
The focus artifact must contain an exact, independently authenticated physical
relocation receipt bound to the context receipt's file and canonical payload
hashes. At execution time the target object, candidate object, and receipt paths
are reread and checked. Output repeats only size/digest identities, never paths.

The caller's row bindings and the capture's
closed rows must cover that set exactly: missing and extra rows both fail. Each
bound capture role must select exactly one owner fact. Its physical register must
equal the register at `candidate_operand_index`; duplicate register occurrences
therefore require no guess. The target register comes from that same position in
the hash-bound focus artifact. Multiple positions in one `lhax` or `fmuls` row
may be bound independently.

Replacing every physical register with one placeholder must make the complete
target and candidate instruction text identical. Opcode, immediate,
displacement, relocation, punctuation, or other non-register drift therefore
yields `UNKNOWN` rather than being misclassified as a register cycle.

All changed candidate-to-target register pairs must be owner-bound. The changed
mapping must be bijective over the same register set, yielding a complete
permutation rather than a partial recoloring observation. Finally, exactly one
allowlisted source-class hypothesis must cover the complete graph row set.
Ambiguous hypotheses are not ranked.

The checked-in JSON Schema, `tools/VOLATILE_OWNER_CAUSAL_JOIN_V1.schema.json`,
describes the output policy surface. Every output repeats immutable evidence
identities and seals itself with `join_sha256`.
Its `PROVEN` branch requires nonempty owner bindings, complete and changed
register mappings, closed residual rows, and non-null context, data-report, and
physical-receipt evidence. An evidence-free document cannot validate as
`PROVEN`.

## Replay caution

Do not use the current `mgcall-rouletteexec-focus.json` as positive evidence.
Its input binding names the CallMg c3 report rather than the RouletteExec strict
report, and the current source/object identities have drifted from the cited
historical artifacts. Rows 825–828 remain a useful index/scale/base/result
diagnostic shape, but a positive replay requires immutable historical inputs and
a regenerated focus artifact bound to the correct report and object identities.
