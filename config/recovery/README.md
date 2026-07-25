# Recovery metadata

This directory is the committed knowledge layer for faithful source recovery.
It is JSON and Python-standard-library-only so validation adds no package-manager
dependency to the decompilation toolchain.

Follow the nearest [`AGENTS.md`](AGENTS.md) before editing these files.

## Layout

- `project.json`: project goal, evidence hierarchy, status vocabularies, agent
  contract, acceptance criteria, and workspace-readiness policy.
- `owners/*.json`: reviewed translation-unit owners. Coverage is intentionally
  incremental; do not create speculative bulk classifications.
- `names.json`: stable identities and semantic naming decisions.
- `exceptions.json`: authenticated, temporary, or forbidden unusual source
  shapes.
- `compiler_patterns.json`: reusable compiler observations with conditions,
  examples, and counterexamples.

Generated SQLite data, reports, and context packs belong under
`build/context/` and must not be committed.

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

Keep dimensions independent. An exact object can still have partial semantics,
address-only naming, raw data domains, or unresolved source-shape debt.
Conversely, a natural semantic candidate may be valuable before it reaches an
exact match.

Evidence entries require a kind from `project.json`, confidence, a concise
accepted/rejected summary, and normally a durable reference. Do not paste an
agent transcript into a manifest.

## Stable naming

Use module plus target address when known. A semantic rename changes the source
name, not the stable identity. Keep unresolved proposals explicit and record
rejections so later agents do not repeat speculative work.

Compiler probes may support source shape but cannot independently authenticate a
semantic name.

## Exceptions

An exception is not a blanket lint suppression. It documents a source form that
would otherwise resemble a matching shortcut.

- Scope it to one owner/path and exact source-quality rule.
- Attach target or compiler evidence.
- `authenticated` entries may remain.
- `temporary` entries are visible debt and require a removal condition.
- `forbidden` entries document approaches that must not be introduced.
- An empty `rules` list records a constraint but suppresses nothing.

## Workspace readiness

`project.json` also declares required agent entrypoints and inactive template
paths that must not exist. `python tools/agent.py doctor` checks this policy.
The intent is to keep one obvious workflow instead of parallel example
workspaces and stale setup instructions.

## Validation

Use the unified gate:

```sh
python tools/agent.py check --base origin/main
```

Lower-level commands remain available:

```sh
python tools/recovery_index.py check
python tools/recovery_index.py build
python tools/source_quality.py --changed origin/main --strict
```
