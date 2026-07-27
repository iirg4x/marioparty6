# AI recovery worker quickstart

## 1. Understand the branch boundary

Read root `AGENTS.md`, `AI_WORKSPACE.md`, the nearest nested `AGENTS.md`, and
this file.

This branch never merges into `main`. It is the AI recovery workspace. Only
verified recovered `src/**/*.c` blobs may move through a fresh main-based
`recovery/*` branch.

Do not load all of `STATUS.md` or the wave archive.

## 2. Inspect and prepare

```sh
python tools/agent.py doctor
python tools/agent.py hooks install
python tools/agent.py queue status
```

Warnings about missing retail inputs are acceptable for public-safe work, but
private DOL/REL proof cannot be claimed.

## 3. Claim one owner

Claude and Codex use separate worktrees, branches, and build directories:

```sh
python tools/agent.py worktree create <owner> \
  --agent <claude-or-codex> \
  --base <AI_BASE_COMMIT> \
  --retail <read-only-GP6E01-directory>
```

Or claim an existing eligible task:

```sh
python tools/agent.py queue claim-next \
  --agent codex \
  --capability rel \
  --batch menu-flow
```

## 4. Generate focused context

```sh
python tools/agent.py context function <symbol> \
  --owner <owner> \
  --symptom "signed extension" \
  --local-evidence \
  --budget 12000
```

The packet reserves space for selected cards, freshness state, local objdiff
summaries, target source, constraints, and acceptance criteria. Expand only one
named missing dependency.

## 5. Research, then edit

Before editing, record:

- owner and stable identity;
- rules, owner constraints, freshness warnings, and counterexamples;
- target instructions, relocations, callers, and data references;
- consumer widths and semantic domains;
- sibling evidence and rejected probes;
- unresolved semantic questions.

Write a natural candidate first. Reconcile compiler shape one variable at a time.

## 6. Keep the real diff inside the claim

Declare shared paths before touching them:

```sh
python tools/agent.py queue update <owner> \
  --agent <agent> \
  --add-shared include/game/example.h
```

Before commits and handoff:

```sh
python tools/agent.py queue check-diff --base <AI_BASE_COMMIT>
```

The check covers committed, staged, unstaged, and untracked files.

## 7. Save AI-workspace knowledge

Update `config/recovery/` with owner state, evidence, naming, debt, exceptions,
cards, examples/counterexamples, and freshness. These records stay on the AI
branch and are never promoted to `main`.

## 8. Record worker proof

Commit and leave the worktree clean:

```sh
python tools/agent.py check --base <AI_BASE_COMMIT>

python tools/agent.py queue verify <owner> \
  --agent <agent> \
  --public-gate pass \
  --object-report build/GP6E01/<report>.json \
  --functions-exact <exact/total> \
  --relocations exact \
  --consumer <consumer>=exact \
  --toolchain GC/1.3.2

python tools/agent.py queue update <owner> \
  --agent <agent> --status ready
```

Any later edit requires re-verification.

## 9. Run private integration serially

The AI integration worktree acquires exclusive resources and runs the full
private gates:

```sh
python tools/agent.py queue acquire-resource integration --agent integrator
python tools/agent.py queue acquire-resource retail-build --agent integrator
```

This verifies the worker commit. It still does not make the AI branch mergeable.

## 10. Promote only recovered C

From the AI workspace:

```sh
python tools/promote_recovered_c.py create \
  --base main \
  --source <verified-worker-commit> \
  --owner <queue-owner> \
  --path src/path/recovered.c \
  --branch recovery/<human-topic> \
  --worktree ../marioparty6-promotion-<topic> \
  --title "Recover <subsystem>"
```

The new worktree is based directly on `main` and contains only the selected exact
C blobs. It contains none of this branch's tools, prompts, metadata, benchmarks,
workflows, or history.

If a header or build change is necessary, promote it separately with
`tools/promote_supporting_change.py` onto its own `project/*` branch (see
[`supporting_change_promotion.md`](supporting_change_promotion.md)). Never copy
it to `main` by hand.

## 11. Verify and open the human-facing PR

In the clean promotion worktree:

```sh
python <ai-workspace>/tools/promote_recovered_c.py audit \
  --root . \
  --base main \
  --head HEAD \
  --source <verified-worker-commit>
```

Rerun object, consumer, DOL/REL, checksum, byte, readability, and semantic-debt
review. Push the clean `recovery/*` branch and open a normal PR to `main` without
AI attribution or operational details.

## 12. Cleanup

```sh
python tools/agent.py queue release-resource retail-build --agent integrator
python tools/agent.py queue release-resource integration --agent integrator
python tools/agent.py worktree close <owner>
```
