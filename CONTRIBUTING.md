# Contributing to the AI recovery workspace

## Permanent separation from `main`

This branch is the AI-forward recovery laboratory. It must never be merged,
squashed, rebased, or cherry-picked wholesale into `main`.

`main` is human-facing. Only verified content may be copied to fresh branches
created directly from `main`: recovered `src/**/*.c` blobs to `recovery/*`
branches, audited supporting changes to `project/*` branches.

See `AI_WORKSPACE.md`, `docs/main_promotion.md`, and
`docs/supporting_change_promotion.md`.

## Queue and isolate one task

A task names one owner or connected function cluster, stable identity, research
question, expected consumers, and verification scope.

Claude and Codex on one PC use separate worktrees, branches, and build
directories:

```sh
python tools/agent.py worktree create <owner> \
  --agent <claude-or-codex> \
  --base main
```

For bulk scheduling:

```sh
python tools/agent.py queue add <owner> \
  --priority high \
  --depends-on <owner> \
  --batch board-pass-1 \
  --capability mwcc \
  --change-class private-source
```

## Claim every write

Declare central headers, `configure.py`, symbols, splits, and recovery schemas
before editing:

```sh
python tools/agent.py queue update <owner> \
  --agent <agent> \
  --add-shared include/game/example.h
```

Before commits and handoff:

```sh
python tools/agent.py queue check-diff --base origin/main
```

The check covers committed, staged, unstaged, and untracked paths. Install local
hooks with:

```sh
python tools/agent.py hooks install
```

## Recovery workflow

1. Claim one owner.
2. Generate focused context with symptoms and local objdiff evidence.
3. Research target instructions, relocations, callers, consumers, selected
   rules, freshness warnings, and rejected probes.
4. Write natural evidence-supported C.
5. Reconcile compiler shape one variable at a time.
6. Review for invented semantics and match-only constructs.
7. Update AI-workspace evidence and knowledge.
8. Commit, record worker proof, and mark the task `ready`.
9. Run private retail integration serially.
10. Promote only the verified C blobs to a fresh main-based branch.

## Worker verification

```sh
python tools/agent.py check --base origin/main

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

Proof is tied to a clean current commit. Any later edit invalidates it.

## Serialized integration

The AI integration worktree may acquire exclusive resources and run the full
DOL/REL, consumer, checksum, and byte-comparison gates. Completing those gates
does not make the AI branch mergeable.

## Clean C-only promotion

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

The command creates a branch from `main`, copies exact C blobs, and rejects:

- anything outside `src/**/*.c`;
- AI/tooling files and metadata;
- headers and build configuration;
- AI/agent attribution in comments, branch names, or commit messages;
- blobs that differ from the verified worker commit.

If a header, symbol, split, or build change is required, promote it separately
with `tools/promote_supporting_change.py` onto its own `project/*` branch and
review it as a separate human-facing change (see
`docs/supporting_change_promotion.md`). Never copy it to `main` by hand.

In the promotion worktree, run:

```sh
python <ai-workspace>/tools/promote_recovered_c.py audit \
  --root . \
  --base main \
  --head HEAD \
  --source <verified-worker-commit>
```

Then rerun object, consumer, DOL/REL, checksum, byte, and readability review.
Only the clean `recovery/*` branch is pushed for a pull request to `main`.

## What stays on the AI branch

Never promote automatically:

- agent instructions, prompts, queues, locks, hooks, or orchestration;
- knowledge cards, freshness records, owner metadata, or benchmark artifacts;
- workflow files, wave reports, AI documentation, or generated reports;
- commit trailers naming Claude, Codex, agents, or AI provenance.

These records are useful internally but do not belong in the human-facing
project.

## Source rules

Do not regress independently exact functions. Do not introduce pragmas,
forced-inline controls, allocation-oriented `volatile`/`register`, inline
assembly, fake storage, or dead branches without scoped evidence.

Semantic names require evidence. Keep uncertain identifiers unknown.

Blind tests must freeze candidates before reveal and preserve replayable
artifacts. Source similarity, organicity, and binary equality remain separate.

## Handoff

An AI-workspace handoff states owner, stable identity, evidence, candidate,
compiler reconciliation, exact/relocation impact, consumers, proof, remaining
debt, and the exact C paths eligible for clean promotion.

A `main` pull request is different: it is human-facing, contains only clean
project changes, and discusses source behavior and verification—not agents,
prompts, orchestration, or token usage.
