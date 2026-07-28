# Concurrent Claude and Codex workflow

Claude and Codex may work simultaneously on one PC, but they must use separate
Git worktrees, branches, and build directories. Final retail integration is
serialized.

Pin `AI_BASE_COMMIT` to the selected commit of
`agent/recovery-context-workflow` before adding a batch. Worker worktrees store
that exact ref in their queue claim, and worker diff checks compare against it.
`main` is reserved for clean promotion worktrees created by the promotion
tools: `recovery/*` for C, `project/*` for supporting changes.

## One-time setup

From the integration checkout:

```sh
python tools/agent.py doctor
python tools/agent.py hooks install
python tools/agent.py catalog build
```

The operational owner catalog is generated from `configure.py` and source
includes. It inventories configured owners, source paths, matching status,
source size, and header consumers without making semantic-recovery claims.

The shared queue lives outside individual worktrees:

```text
<git-common-dir>/agent-coordination/queue.json
```

All worktrees from the same clone see it immediately. Existing schema-v1 queues
are migrated to schema v2 when written. Separate clones may share an explicit
`MP6_AGENT_QUEUE` path.

## Add a batch

The orchestrator adds tasks with dependency and scheduling information:

```sh
python tools/agent.py queue add main:board/tutorial \
  --priority high \
  --batch board-pass-1 \
  --capability mwcc \
  --change-class private-source

python tools/agent.py queue add REL:fileseldll:filesel \
  --priority critical \
  --depends-on main:game/filesel-data \
  --batch menu-flow \
  --capability rel \
  --change-class private-source
```

Priority is preserved when the task is claimed unless `--priority` is explicitly
provided again.

## Create isolated worktrees

The bootstrap command creates the branch, worktree, private build directory,
optional read-only retail link, and queue claim together:

```sh
python tools/agent.py worktree create main:board/tutorial \
  --agent claude \
  --base <AI_BASE_COMMIT> \
  --retail C:\retail\GP6E01
```

```sh
python tools/agent.py worktree create REL:fileseldll:filesel \
  --agent codex \
  --base <AI_BASE_COMMIT> \
  --retail C:\retail\GP6E01
```

The queue rejects a worktree that belongs to another clone, is on the wrong
branch, is not registered by `git worktree list`, or uses a build directory
outside that worktree.

Workers may instead claim an existing task from their own worktree:

```sh
python tools/agent.py queue claim-next \
  --agent claude \
  --capability mwcc \
  --batch board-pass-1
```

`claim-next` considers priority, completed dependencies, capabilities, expected
work/verification cost, and path conflicts.

## Declare every write path

The source owner is protected automatically. Shared headers, `configure.py`,
symbols, splits, and recovery schemas must be declared before editing:

```sh
python tools/agent.py queue update main:board/tutorial \
  --agent claude \
  --add-shared include/game/board.h
```

The generated include graph also blocks scheduling a consumer in parallel with
a task that declares one of its headers for modification.

Before every commit:

```sh
python tools/agent.py queue check-diff --base <AI_BASE_COMMIT>
```

This checks committed, staged, unstaged, and untracked paths. It fails when the
real Git diff escapes the claim or overlaps another agent’s paths. The managed
pre-commit hook runs this automatically.

## Base pins and stale defaults

`AI_BASE_COMMIT` has no committed storage: it lives in each task's `base_ref`.
A task added or claimed without an explicit `--base-ref` stores the symbolic
default (`agent/recovery-context-workflow`), which resolves to the local
integration branch pointer at every diff check. A worker lane rooted on
commits beyond that pointer will therefore see already-landed commits flagged
by `queue check-diff` and the managed pre-commit hook.

- Pin every task to an exact commit at creation: `queue add ... --base-ref
  <sha>` or `queue claim ... --base-ref <sha>`.
- Correct an active mis-pinned claim with `queue update <owner> --base-ref
  <ref>`. The ref must resolve and be an ancestor of the claimed branch, the
  resolved commit is stored, and the pin is frozen once the task is `ready` or
  has recorded verification.
- `MP6_AGENT_BASE` overrides the base for a single hook run only. It is an
  escape hatch for a wedged commit, not a substitute for repinning the claim.

## Worker loop

```sh
python tools/agent.py queue update <owner> \
  --agent claude --status researching

python tools/agent.py context function <symbol> \
  --owner <owner> \
  --symptom "saved register lifetime" \
  --local-evidence

python tools/agent.py queue update <owner> \
  --agent claude --status coding
```

During verification, commit first and leave the worktree clean. Then run the
public gate and record structured proof:

```sh
python tools/agent.py check --base <AI_BASE_COMMIT>

python tools/agent.py queue verify <owner> \
  --agent claude \
  --public-gate pass \
  --object-report build/GP6E01/<report>.json \
  --functions-exact 24/28 \
  --relocations exact \
  --consumer main:game/pause=exact \
  --toolchain GC/1.3.2

python tools/agent.py queue update <owner> \
  --agent claude --status ready
```

Verification is bound to the clean current commit. Any later edit invalidates
completion because `HEAD` no longer matches the verified commit.

Use `released` to return unfinished work to the queue and `cancelled` to stop it:

```sh
python tools/agent.py queue release <owner> \
  --agent claude --status released
```

## Serialized integration

Object work may run in parallel. The integration worktree must acquire exclusive
machine resources before a full build:

```sh
python tools/agent.py queue acquire-resource integration \
  --agent integrator --owner <owner>

python tools/agent.py queue acquire-resource retail-build \
  --agent integrator --owner <owner>
```

After cherry-picking or merging the worker commit, run the serialized retail
build, consumer checks, DTK checksum, and explicit DOL/REL comparisons. Then:

```sh
python tools/agent.py integration finalize <owner> \
  --agent integrator \
  --resource integration \
  --retail-gate pass \
  --checksum pass \
  --consumer main:game/pause=exact \
  --toolchain GC/1.3.2
```

Finalization compares every claimed path between the worker’s verified commit
and the integration commit. It refuses completion when integration changed or
omitted the verified source. The integration commit and proof are stored in the
queue before the task becomes `done`.

Release machine resources afterward:

```sh
python tools/agent.py queue release-resource retail-build --agent integrator
python tools/agent.py queue release-resource integration --agent integrator
```

Finally remove the completed worktree:

```sh
python tools/agent.py worktree close <owner>
```

## Status and recovery

```sh
python tools/agent.py queue status
python tools/agent.py queue status --all
python tools/agent.py queue check
python tools/agent.py doctor
```

Claims inactive for 24 hours are marked stale but continue protecting their
owners and paths. Queue state is local coordination data and must never be
committed as recovery evidence.
