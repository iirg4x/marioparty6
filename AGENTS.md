# Mario Party 6 recovery instructions

## Objective

Recover the most likely original Mario Party 6 source. A byte-identical object,
DOL, or REL is required for final promotion, but a match obtained through
unproven compiler manipulation is not source recovery.

Evidence priority:

1. Same-game debug information, maps, symbols, source remnants, and target artifacts.
2. Target instructions, relocations, access widths, ownership, sections, and call contracts.
3. Same-game callers, consumers, archives, messages, and data domains.
4. Authenticated sibling source from the same Hudson/Mario Party lineage.
5. Controlled compiler experiments for source shape only.
6. Readability hypotheses, which must remain explicitly provisional.

Do not invent semantic names, types, padding, globals, branches, or literal
domains. An honest `unk_*` field or address symbol is preferable to a confident
but unsupported interpretation.

## Required work phases

1. **Evidence research:** inspect the target, owner, callers, consumers, data
   references, sibling evidence, and recorded compiler behavior. Do not edit
   source in this phase.
2. **Natural candidate:** write the cleanest evidence-supported C without
   forcing the final instructions. A natural nonmatching candidate is useful.
3. **Compiler reconciliation:** adjust only evidence-backed scope, signedness,
   expression order, temporary lifetime, loop form, declaration visibility, or
   helper boundaries.
4. **Adversarial review:** check for invented names, raw domains, fake storage,
   unexplained compiler controls, regressions, and consumer damage.

## Context discipline

Start every task from the deterministic recovery index, not from a full-repo
prompt or a long previous chat transcript.

```sh
python tools/recovery_index.py check
python tools/recovery_index.py build
python tools/context_pack.py --budget 12000 function fn_1_BBD8 --owner REL:mdpartydll:mdparty
```

Use exact owner IDs, stable identities, and current symbols first. Expand only
a named dependency or evidence document that remains unresolved. Do not attach
all of `STATUS.md`, every wave report, or an entire large translation unit by
default.

`tools/decompctx.py` remains the preprocessed context generator for decomp.me.
`tools/context_pack.py` is the evidence and token-budget layer for agents and
human review; the two tools solve different problems.

## Recovery status

Track these dimensions independently in `config/recovery/owners/`:

- `binary`: fallback, partial, exact
- `source_shape`: scaffold, plausible, evidence_backed, authenticated
- `semantics`: opaque, partial, recovered
- `naming`: address_only, provisional, partially_semantic, evidence_backed
- `data`: raw, typed_partial, typed, domain_recovered

Never promote semantic or source-quality status merely because binary status is
exact. Source-quality cleanup that gains zero matching bytes is valid recovery
progress and should be documented as such.

## Names and stable identity

Keep a permanent identity separate from a semantic name. REL address symbols
such as `fn_1_BBD8` map to stable identities such as
`mdpartydll:0xBBD8`. Record proposals in `config/recovery/names.json` with an
evidence confidence. Do not erase the stable identity when a semantic name is
accepted.

Compiler experiments may prove source shape. They cannot prove that a function,
field, or object has a particular meaning.

## Unusual source constructs

Every pragma, forced inline/no-inline control, `volatile` or `register` used for
code generation, inline assembly block, include-guard override, synthetic
padding object, opaque byte blob, or dead code-generation branch must be one of:

- authenticated in `config/recovery/exceptions.json` with evidence;
- temporary debt with a removal condition; or
- rejected.

Run the changed-lines review before committing:

```sh
python tools/source_quality.py --changed origin/main --strict
```

Do not copy an owner-specific authenticated oddity into another owner. The
compiler-pattern database must retain conditions and counterexamples.

## Verification

For every source change:

- do not regress independently exact functions;
- use relocation-aware object comparison;
- re-check affected consumers when a type, declaration, structure, or shared
  data owner changes;
- retain the relevant object-diff report path in the owner evidence;
- run the normal serialized DOL/REL and checksum gates before promotion;
- update semantic, naming, data, and source-shape debt separately.

The final review question is not only “does it match?” It is also “what evidence
shows that this is likely the source that produced the target?”
