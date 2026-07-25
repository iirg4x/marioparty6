# Tooling instructions

These rules apply under `tools/`. Also follow the root `AGENTS.md`.

- Keep public-safe agent and recovery tooling Python-standard-library-only unless
  a dependency is already required by the project.
- Every script must work both as `python tools/name.py` and through imports from
  `tools.tests`.
- Use `pathlib`, explicit UTF-8, typed interfaces, useful exit codes, and error
  messages that identify the failing path or record.
- Never read or mutate retail files for a public-safe check.
- Generated output belongs under `build/`; tools must create parent directories
  and avoid partially written files.
- The local claim queue belongs under Git's common directory, not inside one
  worktree. Queue updates must use a cross-platform atomic lock and atomic file
  replacement.
- Queue validation must reject duplicate owners, branches, worktrees, build
  directories, source owners, and overlapping shared paths.
- Tests for concurrent coordination must create synthetic Git worktrees and must
  not require network access, compilers, Ninja, or `orig/`.
- Prefer deterministic exact lookup before fuzzy search. Preserve stable target
  identities in generated output.
- Knowledge-card ranking must be deterministic and must not infer scope from
  prose. Exact counterexamples, stable identities, and owners outrank compiler-
  wide diagnostics.
- Owner constraints must never be selected for unrelated owners. A known
  counterexample must remain visible rather than being filtered out.
- Reserve context budget for selected rule cards before adding source and long
  evidence sections. Do not read wave-document bodies automatically.
- Enrich index search with card triggers, emitted effects, rules, safe actions,
  examples, and counterexamples.
- Tests must use temporary directories and synthetic fixtures; do not require a
  configured compiler, Ninja build, network access, or `orig/`.
- A source-quality rule must avoid comments and string literals, scan changed
  lines by default, and require a narrowly scoped exception.

Run:

```sh
python -m unittest discover -s tools/tests -v
python -m compileall -q tools
python tools/agent.py queue check
python tools/knowledge_cards.py check
python tools/agent.py check --base origin/main
```
