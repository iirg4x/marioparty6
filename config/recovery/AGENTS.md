# Recovery metadata instructions

These rules apply under `config/recovery/`. Also follow the root `AGENTS.md`.

- Human-authored JSON is the durable knowledge source. Generated SQLite,
  Markdown reports, and context packs belong under ignored `build/context/`.
- Keep binary, source-shape, semantic, naming, and data status independent.
  Never promote a source-quality field only because an object matches.
- Add an owner only after its current state has been reviewed. Do not bulk-mark
  owners from file names or progress percentages.
- Preserve stable identity when changing a C symbol. Record proposed, accepted,
  rejected, and unresolved names with explicit confidence.
- Evidence summaries must be concise, falsifiable, and linked to a durable
  source. Record rejected probes when they would otherwise be repeated.
- Compiler patterns require conditions and counterexamples. An owner-specific
  behavior is not a global Metrowerks rule.
- Exceptions must be scoped to an exact path and source-quality rule. Empty rule
  lists document a constraint but never suppress lint findings.
- Temporary exceptions require a removal condition and remain visible debt.
- Do not weaken schema validation to accommodate malformed metadata.

Validate every edit with:

```sh
python tools/agent.py check --base origin/main
```
