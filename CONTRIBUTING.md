# Contributing

This repository accepts human- and agent-assisted work. The goal is faithful
source recovery with retail binary identity as the final objective gate—not byte
closure through unexplained source tricks.

## Queue and isolate one task

A recovery task must name one owner or tightly connected function cluster, its
stable identity, research question, current status, expected consumers, and
verification scope.

Claude and Codex on the same PC must use separate worktrees, branches, and build
directories. Prefer the atomic bootstrap:

```sh
python tools/agent.py worktree create <owner> \
  --agent <claude-or-codex> \
  --base main
```

For bulk scheduling, the orchestrator may add dependencies, batches,
capabilities, and expected cost:

```sh
python tools/agent.py queue add <owner> \
  --priority high \
  --depends-on <owner> \
  --batch board-pass-1 \
  --capability mwcc \
  --change-class private-source
```

Workers can take the next eligible task with `queue claim-next`. Existing queued
priority is preserved unless a new `--priority` is explicitly supplied.

## Claim every actual write

The owner source is protected automatically. Declare central headers,
`configure.py`, symbols, splits, and recovery schemas before editing:

```sh
python tools/agent.py queue update <owner> \
  --agent <agent> \
  --add-shared include/game/example.h
```

Before commits and handoff:

```sh
python tools/agent.py queue check-diff --base origin/main
```

This checks committed, staged, unstaged, and untracked paths against the active
claim and every other task. The managed pre-commit hook runs it automatically:

```sh
python tools/agent.py hooks install
```

## Recovery workflow

1. Inspect the operational owner catalog and claim one task.
2. Generate focused context with symptoms and local objdiff evidence.
3. Research target instructions, relocations, callers, consumers, data domains,
   selected rules, freshness warnings, and previous rejected probes.
4. Write a natural evidence-supported candidate.
5. Reconcile compiler shape one variable at a time.
6. Review adversarially for invented semantics and matching-only constructs.
7. Update owner metadata, debt, cards, examples/counterexamples, and freshness.
8. Record worker proof and mark the task `ready`.
9. Integrate and run retail proof serially before finalization.

A compiler-wide card is diagnostic. An owner constraint never transfers to an
unrelated owner. A stale card must be revalidated before being relied on as a
final source-shape rule.

## Files that must not be committed

Never commit:

- retail inputs under `orig/`;
- generated files under `build/`;
- `build.ninja`, `objdiff.json`, or `ctx.c`;
- local queue/resource-lock state;
- editor, agent, hook, or virtual-environment state;
- rebuilt DOL/REL binaries or extracted game assets.

Generated reports may be referenced by path in structured proof, but durable
findings belong in `config/recovery/`.

## Verification matrix

| Change | Worker public gate | Object/consumer proof | Integration retail gate |
| --- | --- | --- | --- |
| Documentation only | Required | Not normally | Not normally |
| Python tools or metadata | Required | Not normally | Not normally |
| Private C implementation | Required | Required | Required before `done` |
| Shared header/type/data/symbol | Required | Every affected Matching consumer | Required |
| Flags/status/splits/link/configure | Required | Required | Required |

## Worker verification

Commit the candidate and leave the worktree clean:

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

Verification fails if the worktree is dirty, the branch/worktree/build assignment
is invalid, or the actual diff escapes the claim. Any later commit invalidates
the proof.

Documentation/tooling/metadata tasks do not need object fields, but still require
a clean verified commit and passing public gate.

## Serialized integration

The integration worktree acquires exclusive resources before full builds:

```sh
python tools/agent.py queue acquire-resource integration --agent integrator
python tools/agent.py queue acquire-resource retail-build --agent integrator
```

After integrating a `ready` worker commit and passing private gates:

```sh
python tools/agent.py integration finalize <owner> \
  --agent integrator \
  --retail-gate pass \
  --checksum pass \
  --consumer <consumer>=exact \
  --toolchain GC/1.3.2
```

Finalization compares every claimed path between the worker’s verified commit
and the integrated tree. It refuses `done` if integration changed or omitted the
verified source.

Release resources and remove completed worktrees afterward.

## Source and knowledge rules

Do not regress independently exact functions. Do not introduce pragmas,
forced-inline controls, code-generation `volatile`/`register`, inline assembly,
fake storage, or dead branches without scoped evidence.

Semantic names and fields require evidence. Keep uncertain identifiers unknown.

A reusable source-to-output finding must record:

- exact trigger and preconditions;
- possible emitted changes and recognizable signatures;
- one clear coding/investigation rule;
- safe actions;
- explicit scope;
- examples, counterexamples, related exceptions, and evidence;
- validated commit/date, watched paths, and supersession state.

## Commits and handoff

Separate semantic cleanup, compiler reconciliation, shared-interface work,
knowledge extraction, and tooling when independently reviewable.

Useful commit trailers/details:

```text
Owner: REL:mdpartydll:mdparty
Stable-Identity: mdpartydll:0xBBD8
Agent: claude / codex
Queue-Status: ready / done
Verified-Commit: <sha>
Knowledge-Cards: reviewed or changed IDs
Functions-Exact: before -> after
Relocations: exact / changed / not run
Consumers: names and results
Public-Gate: pass
Retail-Gate: pass / not run
Evidence: durable path
```

Complete the PR template. A handoff must be usable without reading the original
agent transcript.
