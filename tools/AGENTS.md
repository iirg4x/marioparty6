# Tooling instructions

These rules apply under `tools/`. Also follow root `AGENTS.md`.

- Keep public-safe tooling Python-standard-library-only unless the project already
  requires the dependency.
- Every script must support direct `python tools/name.py` execution and imports
  from `tools.tests`.
- Use `pathlib`, UTF-8, typed interfaces, useful exit codes, and errors that name
  the failing path or record.
- Never read or mutate retail inputs during public-safe checks.
- Generated output belongs under `build/`; use atomic replacement for durable
  generated state.

## Queue and worktree invariants

- The queue lives under Git’s common directory or `MP6_AGENT_QUEUE`, never in one
  worktree.
- Queue schema updates must migrate existing local state safely.
- Lock and write operations must be cross-platform and atomic.
- Claims must validate Git common directory, registered worktree path, branch,
  and build directory containment.
- Preserve queued priority unless the claim explicitly overrides it.
- Reject duplicate owners, branches, worktrees, build directories, sources,
  shared paths, and header-consumer conflicts.
- Actual diff checks must include committed, staged, unstaged, and untracked
  paths.
- Worker proof must be tied to a clean current commit. `ready` requires worker
  proof; `done` for source work requires serialized integration proof.
- Machine-wide resources must be exclusive and explicit.
- Tests must create real temporary Git worktrees and cover simultaneous claims,
  stale commits, undeclared diffs, dependencies, resource locks, and integration.

## Catalog, context, and knowledge invariants

- The operational owner catalog may inventory configuration and dependencies but
  must not infer semantic recovery status.
- Prefer deterministic exact lookup before fuzzy search.
- Knowledge selection must derive scope from structured fields, never prose.
- Counterexamples, stable identities, and owners outrank compiler-wide rules.
- Owner constraints must never leak to unrelated owners.
- Symptom filtering may narrow compiler-wide diagnostics, but must retain exact
  target constraints and counterexamples.
- Reserve section budgets for knowledge, constraints, stable identity, and
  acceptance criteria before clipping source or historical evidence.
- Local objdiff parsers must be schema-tolerant and summarize rather than ingest
  unbounded reports.
- Freshness records must identify validated commits, watched paths, and
  supersession state.
- Wave-document bodies are never automatic prompt context.

## Source-quality checks

A changed-line rule must ignore comments and string literals, scan the real diff
by default, and require a narrowly scoped exception.

Run:

```sh
python -m compileall -q tools
python -m unittest discover -s tools/tests -v
python tools/recovery_index.py check
python tools/knowledge_cards.py check
python tools/agent.py catalog build
python tools/agent.py queue check
python tools/agent.py check --base origin/main
```
