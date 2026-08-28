# Differential allocator causal solver

`tools/differential_allocator_causal.py` is a read-only, fail-closed ranker for
small register-allocation residuals. It does not edit source, compile a
candidate, retain a result, or advance authority.

## Evidence contract

The caller supplies a canonical `differential_allocator_causal_context/v1`
document and its SHA-256 as a separate trust anchor. The context binds:

- a `focus_symbol_report/v1` with identical strict/data residual rows;
- target and candidate physical instruction streams with CFG fingerprints;
- a self-digested physical-relocation receipt;
- byte-exact natural source spans and the candidate source file;
- one same-session ownership trace;
- compiler and tool binaries; and
- allowlisted natural source hypotheses plus hash-bound rejected controls.

Every JSON descriptor binds both the serialized file SHA-256 and the payload's
canonical self-digest, so the direct API cannot substitute a different
self-consistent packet under the same trusted context.

The context, source-span manifest, trace, and each owner fact bind one canonical
`session_id`; the context, source-span manifest, and trace also bind the same
trust-anchor SHA-256. A different session is a lawful `UNKNOWN`, never a
cross-session join.

Each trace owner must close this chain:

`source span -> Object -> VarInfo -> PCode def/use -> IG node -> vreg -> physical register`

It must also report monotonic birth, assignment, first-use, and last-use
positions and at least one interference neighbor. A missing edge is evidence,
not permission to infer the owner.

## Decision rules

Physical streams are closed documents and require four valid SHA-256
fingerprints: target/candidate CFG and target/candidate relocation topology.
Allocator inference does not start if CFG or physical relocations differ. The
solver then requires every focus row to be register-only, every row to be
covered by one authenticated owner, and the changed physical mapping to form a
closed permutation. Target pseudo-owners are derived from aligned target and
candidate operand positions; trace target-register values are verified claims.
Each owner exposes a target residual interval and row/operand anchors. The
solver computes the minimum causal frontier from caller-bound
hypotheses and verifies that the selected class suppresses every measured
rejected control.

Exactly one minimum class produces `RANKED_SOURCE_CLASS` and at most one
sealed `differential_allocator_one_cell_request/v1`. It names one composed
winning cell (`max_cells=1`, `matrix_expansion=false`). It is intentionally not
a `candidate_interaction_request/v1`, whose factorial axis contract would
expand two or three axes into four or eight cells. Ambiguous, incomplete, exact/zero-row, or
policy-incompatible evidence produces `UNKNOWN` with `first_missing_edge`.
Up to three tied classes may be listed diagnostically, but none is actionable.
Matrix searches, dead/fake owners, padding, inline assembly, and register
shaping are rejected at input validation.

## Usage

```text
python tools/differential_allocator_causal.py \
  --context <absolute-context.json> \
  --context-sha256 <caller-trusted-sha256> \
  --output <new-result.json>
```

Exit status is 0 only for `RANKED_SOURCE_CLASS`, 1 for a lawful `UNKNOWN`, and
2 for malformed or unauthenticated input. The output validates against
`tools/DIFFERENTIAL_ALLOCATOR_CAUSAL_V1.schema.json`.

## Replay boundary

Synthetic replay covers the ConfigPadMain missing-hook shape, ConfigPadOpen's
extra three-owner cycle and zero-row exact state, a two-GPR cycle, an
allocation/consumer chain, and generic measured-control suppression matching
the CapSpecial two-owner class. Current archived Config and MgCall artifacts do
not contain a fresh same-session packet with Object, VarInfo, PCode/IG, and
lifetime joins. They therefore cannot lawfully produce a live ranked result;
the exact blocker is the first absent trace edge, not a reason to invent one.
