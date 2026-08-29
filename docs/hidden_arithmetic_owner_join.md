# Hidden arithmetic owner join

`tools/hidden_arithmetic_owner_join.py` is a read-only replay tool for archived
`mwcc_capsule_same_session_partial_evidence/v1` packages. It never launches a
compiler, edits source, admits a candidate, or advances retention authority.

The tool closes one narrow evidence chain:

`source Object -> PCode Object/IG node -> final allocator color -> independently decoded PPC operand`

Legacy GC/2.7 packets do not always contain a vreg identifier. The tool does
not substitute `operand_index`. A complete direct edge is reported as
`allocator_identity.kind=IG_NODE`,
`edge_mode=DIRECT_PCODE_OBJECT_TO_IG_COLOR`, and `legacy_vreg_id=null`.

## Hash-bound context

The input schema is
`tools/HIDDEN_ARITHMETIC_OWNER_JOIN_CONTEXT_V1.schema.json`. A context binds:

- the immutable partial package, trust root, hook validation, failure graph,
  PCode stream, machine stream, source, compiler, and trace-produced object;
- separate production target and candidate objects;
- the production focus artifact and strict/data report identities;
- explicit residual rows and a per-chain mapping from trace machine sites to
  production residual groups (trace indices are never assumed to be objdiff
  row indices);
- an independently decoded
  `mp6_physical_relocation_receipt/v1`, including normalized target/candidate
  rows, counts, and zero differences;
- named semantic-owner source spans, a bounded natural-source-class allowlist,
  and hash-bound measured rejected controls with structured boundary kinds.

The context contains no caller-asserted production gate or desired source
class. The replay derives exact size/CFG/calls from the complete focus rows,
data exactness from the `.data`/`.bss`/`.sdata2` section census, protected
siblings from matching strict/data identity digests, and physical exactness from
the independent receipt. It derives either
the reloaded-value or direct-expression boundary from the authenticated hidden
producer topology, then suppresses measured controls. An unsupported boundary,
an already-tested class, or missing structured competing-boundary evidence
yields `UNKNOWN`.

Objdiff-local relocation symbol ordinals are intentionally not compared across
objects. Nonresidual focus rows require relocation presence and type parity;
offset/addend/effective-target equivalence is proven by the independently
decoded physical relocation receipt.

A unique nested arithmetic producer machine event may remain `UNKNOWN` only
when its reason is exactly `ambiguous reaching definition` and the authenticated
PCode destination token/IG/color, decoded PPC destination, and consumer input
all agree. This seals only the producer output edge; the result explicitly
records `producer_input_ownership_claimed=false`.

```powershell
python tools/hidden_arithmetic_owner_join.py `
  --context C:\proof\hidden-arithmetic-context.json `
  --output C:\proof\hidden-arithmetic-result.json
```

The result schema is `tools/HIDDEN_ARITHMETIC_OWNER_JOIN_V1.schema.json`.
Ranked and UNKNOWN results bind the absolute implementation path, byte size,
and SHA-256. CLI output must be an absolute path under this repository's
`build/` directory. Symlinked outputs or parents are rejected. Publication uses
a same-directory flushed/fsynced temporary followed by atomic replacement; a
write or replacement failure preserves any existing result and removes the
temporary.
`RANKED_SOURCE_CLASS` is emitted only when every production and trace gate is
complete and every selected site has exactly one semantic input, one hidden
input, one unique earlier reaching definition, and matching independently
decoded PPC colors. The predicted scope contains every bound production
residual row. All other outcomes are fail-closed `UNKNOWN` with the first
missing or conflicting edge.

## Validation and safety

The focused synthetic suite is non-skipped and covers a complete join plus
ambiguity, color conflict, non-`fmuls`, missing/nonunique/late producers,
session/source/focus/receipt/relocation drift, insufficient control memory,
and attempts to inject an asserted source answer. It also proves that mutating
`operand_index` cannot change the semantic result.

Run it with:

```powershell
python -m unittest tools.tests.test_hidden_arithmetic_owner_join -v
```

The tool is diagnostic only. A ranked class is not source provenance, candidate
admission, retention, promotion, or permission to compile.
