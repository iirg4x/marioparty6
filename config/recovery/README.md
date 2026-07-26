# Recovery metadata

This directory is the committed knowledge layer for faithful source recovery.
Generated SQLite, owner catalogs, reports, audits, context packs, and local queue
state belong under ignored build/Git-common paths and must not be committed.

Follow [`AGENTS.md`](AGENTS.md) before editing.

## Files

- `project.json`: project contract, evidence hierarchy, status vocabularies,
  context/coordination policy, and required tooling.
- `owners/*.json`: reviewed semantic recovery owners. Coverage is intentionally
  incremental.
- `names.json`: stable identities and semantic naming decisions.
- `exceptions.json`: scoped authenticated, temporary, or forbidden source forms.
- `compiler_patterns.json`: actionable source-to-output knowledge cards.
- `knowledge_freshness.json`: validated commits, watched inputs, status, and
  supersession for every card.

The generated operational owner catalog is separate. It inventories configured
source files and dependencies but does not promote semantic status.

## Owner state

Each reviewed owner tracks binary, source shape, semantics, naming, and data
independently. Exact output can coexist with address-only naming or semantic
debt. A natural semantic candidate may be useful before matching.

Include the configured compiler when known. Evidence must be concise,
falsifiable, accepted/rejected explicitly, and normally linked to a durable
report or source.

## Knowledge cards

Schema-v2 cards require:

```text
id and title
classification and category
compiler and confidence
triggering source condition and preconditions
possible emitted changes and known signatures
one rule and safe actions
explicit applicability scope
examples and counterexamples
related exceptions and evidence
```

Classifications:

- `confirmed_rule`: repeatable under stated conditions;
- `contextual_heuristic`: a high-value bounded diagnostic;
- `owner_constraint`: authenticated only for explicit owners/stable identities.

Counterexamples rank before general rules. Compiler-wide cards are diagnostics,
not permission to copy an owner’s source shape.

## Freshness

Every card must have a record in `knowledge_freshness.json`:

```json
{
  "status": "active",
  "validated_commit": "40-hex-sha",
  "validated_at": "YYYY-MM-DD",
  "watch_paths": ["source/or/evidence"],
  "supersedes": [],
  "superseded_by": null
}
```

A watched source/evidence change makes the card stale until revalidated. Stale
cards remain visible as warnings rather than silently disappearing.

## Stable names and exceptions

A semantic rename changes the source symbol, not stable target identity.
Compiler probes cannot independently prove semantic meaning.

Exceptions authorize specific unusual constructs and must be scoped to exact
paths and source-quality rules. A knowledge card explains behavior; it does not
widen an exception.

## Historical evidence

Wave reports remain forensic records, not prompt input. Distill reusable results
into owner evidence, cards, exceptions, names, or counterexamples. The audit
shows undistilled waves without assuming every wave contains a global rule.

## Validation

```sh
python tools/recovery_index.py check
python tools/knowledge_cards.py check
python tools/knowledge_cards.py freshness
python tools/agent.py knowledge audit
python tools/agent.py check --base <AI_BASE_COMMIT>
```
