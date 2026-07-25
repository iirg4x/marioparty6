# Recovery metadata

This directory is the committed knowledge layer for faithful source recovery.
It is JSON and Python-standard-library-only so validation adds no package-manager
dependency to the decompilation toolchain.

Follow the nearest [`AGENTS.md`](AGENTS.md) before editing these files.

## Layout

- `project.json`: project goal, evidence hierarchy, status vocabularies, context
  card limit, agent contract, acceptance criteria, and workspace policy.
- `owners/*.json`: reviewed translation-unit owners. Coverage is intentionally
  incremental; do not create speculative bulk classifications.
- `names.json`: stable identities and semantic naming decisions.
- `exceptions.json`: authenticated, temporary, or forbidden unusual source
  shapes.
- `compiler_patterns.json`: actionable source-to-output knowledge cards with
  scope, examples, counterexamples, rules, and safe actions.

Generated SQLite data, reports, audits, and context packs belong under
`build/context/` and must not be committed.

## Owner manifest

An owner manifest records:

```json
{
  "id": "REL:module:owner",
  "module": "module",
  "source": "src/REL/module/owner.c",
  "compiler": "GC/1.3.2",
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

Include the compiler when known so compiler-wide diagnostic cards can be
selected. Do not infer a compiler from a neighboring owner without checking the
configured object.

Evidence entries require a kind from `project.json`, confidence, a concise
accepted/rejected summary, and normally a durable reference. Do not paste an
agent transcript into a manifest.

## Knowledge cards

`compiler_patterns.json` uses schema version 2. Every card requires:

```json
{
  "id": "gc132-example",
  "title": "Human-readable rule title",
  "classification": "confirmed_rule",
  "category": "type_contracts",
  "compiler": "GC/1.3.2",
  "confidence": "confirmed",
  "summary": "Compact finding",
  "conditions": "Compact compatibility/search text",
  "source_condition": {
    "change": "The source change that triggers the behavior",
    "requires": ["precondition"]
  },
  "emitted_effect": {
    "possible_changes": ["register allocation"],
    "known_signatures": ["recognizable objdiff result"]
  },
  "rule": "The coding or investigation rule",
  "safe_actions": ["bounded next action"],
  "applicability": {
    "compiler_wide": true,
    "project_wide": false,
    "owners": [],
    "stable_ids": [],
    "modules": [],
    "owner_tags": []
  },
  "examples": [],
  "counterexamples": [],
  "related_exceptions": [],
  "evidence": []
}
```

Classifications:

- `confirmed_rule`: repeatable finding under stated conditions. A
  compiler-wide rule is diagnostic, not a source template.
- `contextual_heuristic`: a proven high-value investigation path that still
  requires local evidence.
- `owner_constraint`: an authenticated source shape for explicit owners or
  stable identities only.

An owner constraint cannot be compiler- or project-wide. Every card needs at
least one safe action. Evidence paths, owner scopes, and related exceptions are
validated.

Selection ranks counterexamples first, then exact stable identities, owner
scope, module/tags, compiler-wide rules, and project-wide rules. The default
context limit is five cards.

Inspect and audit:

```sh
python tools/knowledge_cards.py check
python tools/knowledge_cards.py function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty
python tools/knowledge_cards.py audit
```

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

Knowledge cards explain what was learned and what to do. Exceptions authorize a
specific unusual construct. Referencing an exception from a card does not widen
its scope.

## Historical evidence

Wave reports remain forensic evidence, not default context. A reusable finding
must be distilled into an owner record, exception, or knowledge card. The audit
lists wave documents without a knowledge-card reference; it does not assume
every wave deserves a global rule.

## Workspace readiness

`project.json` declares required agent entrypoints and inactive template paths
that must not exist. `python tools/agent.py doctor` checks this policy.

## Validation

Use the unified gate:

```sh
python tools/agent.py check --base origin/main
```

Lower-level commands remain available:

```sh
python tools/recovery_index.py check
python tools/recovery_index.py build
python tools/knowledge_cards.py check
python tools/source_quality.py --changed origin/main --strict
```
