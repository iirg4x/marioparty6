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

## Permanent branch boundary

- The AI workspace branch must never be merged, squashed, rebased, or
  cherry-picked into `main`.
- Main-promotion tooling must create a new branch/worktree directly from `main`.
- Automatic transfer accepts only explicitly declared added/modified paths:
  `src/**/*.c` for `promote_recovered_c.py`; `include/**`, `config/**` (never
  `config/recovery/**`), and `configure.py` for
  `promote_supporting_change.py`.
- Every promoted Git blob must exactly equal the verified worker-commit blob.
- The C tool rejects headers, configuration, symbols, and splits; both tools
  reject tools, metadata, docs, workflows, benchmarks, generated output, and AI
  attribution.
- Promotion manifests remain under ignored `build/promotion/` in the AI
  workspace; never commit them to the clean branch.
- Supporting changes move only through `promote_supporting_change.py` onto
  their own `project/*` branch, with every affected Matching consumer
  re-verified; never copy them from this branch by hand.
- Tests must prove that a promotion branch contains no AI workspace files and
  that out-of-scope or attributed changes are rejected.

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
  proof; source promotion requires that exact commit.
- Machine-wide resources must be exclusive and explicit.
- Tests must create real temporary Git worktrees and cover simultaneous claims,
  stale commits, undeclared diffs, dependencies, resource locks, integration,
  and clean promotion.

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

## Blind benchmark invariants

- Freeze the candidate and record its SHA-256 before the retained source is
  revealed or scored.
- Preserve the exact evidence packet and target/candidate assembly.
- Record source path, source commit, retained-function SHA-256, candidate hash,
  freeze time, toolchain, and blindness assertions.
- Score assembly equivalence, retained-source fidelity, organicity, and
  reproducibility independently.
- A token-identical candidate may inherit retained-source debt; never convert
  source similarity directly into an organicity claim.
- Automated findings are review prompts, not proof that old source is inauthentic.
- Cases lacking raw candidate or assembly artifacts remain `legacy-reported`.

## Source-quality checks

A changed-line rule must ignore comments and string literals, scan the real diff
by default, and require a narrowly scoped exception.

Run:

```sh
python -m compileall -q tools
python -m unittest discover -s tools/tests -v
python tools/recovery_index.py check
python tools/knowledge_cards.py check
python tools/blind_recovery.py audit
python tools/agent.py catalog build
python tools/agent.py queue check
python tools/agent.py check --base <AI_BASE_COMMIT>
```
