# Agent quickstart

## 1. Inspect and prepare

Read root `AGENTS.md`, the nearest nested `AGENTS.md`, and this file. Do not load
all of `STATUS.md` or the wave archive.

```sh
python tools/agent.py doctor
python tools/agent.py hooks install
python tools/agent.py queue status
```

Warnings about missing retail inputs are acceptable for public-safe work, but
private DOL/REL proof cannot be claimed.

## 2. Claim one owner

Use a recovery-task issue. Claude and Codex must use separate worktrees,
branches, and build directories.

The safest setup is:

```sh
python tools/agent.py worktree create <owner> \
  --agent <claude-or-codex> \
  --base main \
  --retail <read-only-GP6E01-directory>
```

For an existing worktree:

```sh
python tools/agent.py queue claim <owner> --agent <claude-or-codex>
```

The orchestrator can populate batches and workers can take the next eligible
task:

```sh
python tools/agent.py queue claim-next \
  --agent codex \
  --capability rel \
  --batch menu-flow
```

## 3. Generate focused context

```sh
python tools/agent.py context function <symbol> \
  --owner <owner> \
  --symptom "signed extension" \
  --local-evidence \
  --budget 12000
```

The packet reserves space for selected knowledge cards, freshness state, local
objdiff summaries, target source, constraints, and acceptance criteria. Expand
only one named missing dependency.

## 4. Research, then edit

Before editing, record:

- owner and stable identity;
- selected rules, owner constraints, freshness warnings, and counterexamples;
- target instructions, relocations, sections, callers, and data references;
- consumer widths and semantic domains;
- sibling evidence and previous rejected probes;
- unresolved semantic questions.

Write a natural candidate first. Reconcile compiler shape one variable at a
time only after the source meaning and likely structure are clear.

## 5. Keep the real diff inside the claim

Declare any shared path before touching it:

```sh
python tools/agent.py queue update <owner> \
  --agent <agent> \
  --add-shared include/game/example.h
```

Before commits and handoff:

```sh
python tools/agent.py queue check-diff --base origin/main
```

This examines committed, staged, unstaged, and untracked files. The installed
pre-commit hook runs the same ownership check automatically.

## 6. Save reusable knowledge

Update `config/recovery/` with owner state, evidence, naming, debt, scoped
exceptions, knowledge cards, examples/counterexamples, and freshness records.
Do not leave the reusable conclusion only in an agent transcript or wave report.

## 7. Record worker proof

Commit the task and leave the worktree clean:

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

The proof is tied to the clean current commit. Editing afterward requires
re-verification.

## 8. Integrate serially

The integration worktree acquires exclusive resources, integrates the ready
commit, and runs the full private gates:

```sh
python tools/agent.py queue acquire-resource integration --agent integrator
python tools/agent.py queue acquire-resource retail-build --agent integrator
```

After the serialized build, checksum, consumer, and byte comparisons:

```sh
python tools/agent.py integration finalize <owner> \
  --agent integrator \
  --retail-gate pass \
  --checksum pass \
  --toolchain GC/1.3.2
```

Finalization confirms the integration tree contains the worker’s verified
claimed paths before setting the task to `done`.

## 9. Handoff and cleanup

Complete the PR template with evidence, cards, natural candidate, compiler
reconciliation, exact/relocation impact, consumers, worker proof, integration
proof, metadata changes, and remaining debt.

```sh
python tools/agent.py queue release-resource retail-build --agent integrator
python tools/agent.py queue release-resource integration --agent integrator
python tools/agent.py worktree close <owner>
```
