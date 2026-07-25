# Recovery metadata instructions

These rules apply under `config/recovery/`. Also follow root `AGENTS.md`.

- Human-authored JSON is the durable knowledge source. Generated indexes,
  catalogs, reports, audits, and context packs belong under ignored `build/`.
- Keep binary, source-shape, semantic, naming, and data status independent.
- Add a reviewed owner only after examining evidence. The operational catalog is
  not permission to bulk-classify owners.
- Preserve stable identity across semantic renames and record accepted, proposed,
  rejected, and unresolved names with confidence.
- Evidence summaries must be concise, falsifiable, and linked to durable sources.
- Preserve rejected probes and counterexamples so agents do not repeat them.

## Knowledge cards

A reusable source-to-output finding belongs in `compiler_patterns.json` with:

- trigger and preconditions;
- possible emitted effects and recognizable signatures;
- one clear rule and safe actions;
- exact stable-ID, owner, module, tag, compiler, or project scope;
- examples, counterexamples, related exceptions, and evidence.

Use `confirmed_rule` only for repeatable evidence, `contextual_heuristic` for a
bounded diagnostic path, and `owner_constraint` only for explicit owners or
stable identities. Compiler behavior cannot authenticate semantic names.

Every card must have a matching record in `knowledge_freshness.json` with its
validated commit/date, watched evidence/source paths, status, and supersession.
When watched inputs change, revalidate or mark the card stale rather than silently
continuing to inject it as current knowledge.

## Exceptions

Exceptions authorize specific unusual source forms. They must be path- and
rule-scoped. Empty rule lists suppress nothing. Temporary exceptions require a
removal condition. A card explaining behavior does not widen an exception.

Do not create a wave report as the only record of reusable knowledge. Keep the
wave as forensic evidence and distill the conclusion into structured metadata.

Validate:

```sh
python tools/recovery_index.py check
python tools/knowledge_cards.py check
python tools/knowledge_cards.py freshness
python tools/agent.py knowledge audit
python tools/agent.py check --base origin/main
```
