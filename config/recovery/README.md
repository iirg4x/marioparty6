# Recovery metadata

This directory is the committed knowledge layer for faithful source recovery.
It is intentionally JSON and standard-library-only so validation does not add a
package-manager dependency to the decompilation toolchain.

## Layout

- `project.json`: evidence hierarchy, status vocabularies, agent contract, and
  acceptance criteria.
- `owners/*.json`: one governed translation-unit owner per file.
- `names.json`: stable identities and semantic naming decisions.
- `exceptions.json`: authenticated, temporary, or forbidden unusual source
  shapes.
- `compiler_patterns.json`: reusable compiler observations with conditions,
  examples, and counterexamples.

Generated SQLite data and context packs belong under `build/context/` and must
not be committed.

## Owner manifest

An owner manifest records:

```json
{
  "id": "REL:module:owner",
  "module": "module",
  "source": "src/REL/module/owner.c",
  "status": {
    "binary": "partial",
    "source_shape": "plausible",
    "semantics": "partial",
    "naming": "address_only",
    "data": "typed_partial"
  },
  "evidence": [],
  "constraints": [],
  "debt": []
}
```

Keep the status dimensions independent. An exact binary owner can still have
partial semantics, address-only naming, and raw data domains.

Evidence entries require a kind from `project.json`, a confidence, a concise
summary, an accepted/rejected disposition, and normally a durable reference.
Do not paste a whole investigation transcript into a manifest.

## Stable naming

Use module plus target address when known. A semantic rename changes the source
name, not the stable identity. Keep unresolved proposals explicit and record
rejections to avoid repeated speculative work.

## Exceptions

An exception is not a blanket lint suppression. It documents a source form that
would otherwise resemble a matching shortcut. Scope it to one owner or path and
attach target or compiler evidence. `temporary` entries are visible debt;
`authenticated` entries may remain; `forbidden` entries document known bad
approaches.

## Validation

```sh
python tools/recovery_index.py check
python tools/recovery_index.py build
python tools/source_quality.py --changed origin/main --strict
```
