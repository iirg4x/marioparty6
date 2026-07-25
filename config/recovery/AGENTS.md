# Recovery metadata instructions

These rules apply under `config/recovery/`. Also follow the root `AGENTS.md`.

- Human-authored JSON is the durable knowledge source. Generated SQLite,
  Markdown reports, audits, and context packs belong under ignored
  `build/context/`.
- Keep binary, source-shape, semantic, naming, and data status independent.
  Never promote a source-quality field only because an object matches.
- Add an owner only after its current state has been reviewed. Do not bulk-mark
  owners from file names or progress percentages.
- Preserve stable identity when changing a C symbol. Record proposed, accepted,
  rejected, and unresolved names with explicit confidence.
- Evidence summaries must be concise, falsifiable, and linked to a durable
  source. Record rejected probes when they would otherwise be repeated.
- A reusable source-to-output finding belongs in `compiler_patterns.json` with a
  trigger, possible emitted effects, known signatures, one clear rule, safe
  actions, scope, examples, counterexamples, and evidence.
- Use `confirmed_rule` only for repeatable evidence under stated conditions. Use
  `contextual_heuristic` for a valuable diagnostic path. Use
  `owner_constraint` only for explicit owners or stable identities.
- A compiler-wide rule is diagnostic and must not prescribe one owner’s source
  layout. An owner constraint must never be marked compiler- or project-wide.
- Preserve negative evidence. A counterexample is a first-class result and is
  ranked ahead of general rules for that target.
- Compiler behavior cannot authenticate semantic names.
- Exceptions must be scoped to an exact path and source-quality rule. Empty rule
  lists document a constraint but never suppress lint findings.
- A knowledge card explains what was learned; an exception authorizes a specific
  unusual construct. One does not widen the other.
- Temporary exceptions require a removal condition and remain visible debt.
- Do not weaken schema validation to accommodate malformed metadata.
- Do not create a new wave report as the only record of reusable knowledge.
  Distill the conclusion and retain the wave only as evidence.

Validate and inspect every edit with:

```sh
python tools/knowledge_cards.py check
python tools/agent.py knowledge audit
python tools/agent.py check --base origin/main
```
