# Concurrent Claude and Codex workflow

Claude and Codex may work at the same time on one PC, but they must use
**worktrees from the same Git repository**, separate branches, and separate
build directories.

## Local layout

```text
marioparty6-integration/        integration and final verification
marioparty6-claude-task/        Claude worktree
marioparty6-codex-task/         Codex worktree
```

Each worktree naturally has its own ignored `build/` directory. Retail inputs
may be exposed read-only to each worktree through a symlink or directory
junction, but the build outputs must not be shared.

## Shared queue location

The queue is stored outside every worktree under Git's common directory:

```text
<git-common-dir>/agent-coordination/queue.json
```

All worktrees created from the same repository see it immediately. Queue writes
use an atomic local lock and atomic file replacement so simultaneous Claude and
Codex claims cannot overwrite each other.

Separate clones do not share a Git common directory. When clones are required,
point both to one absolute queue path:

```text
MP6_AGENT_QUEUE=C:\path\to\shared\queue.json
```

or on Unix:

```sh
export MP6_AGENT_QUEUE=/absolute/path/to/shared/queue.json
```

Do not commit the queue. It contains coordination state, not recovery evidence.

## Create worktrees

From the integration checkout:

```sh
git worktree add ../marioparty6-claude-task \
  -b agent/claude-board-tutorial

git worktree add ../marioparty6-codex-task \
  -b agent/codex-fileseldll
```

## Add bulk tasks

The orchestrator can queue work before assigning it:

```sh
python tools/agent.py queue add src/board/tutorial.c \
  --source src/board/tutorial.c \
  --priority high \
  --target fn_80000000

python tools/agent.py queue add REL:fileseldll:filesel \
  --priority high
```

Use a configured recovery owner ID when one exists. Otherwise, use the source
path as the owner and pass `--source` explicitly.

## Claim work

From the worker's own worktree:

```sh
python tools/agent.py queue claim src/board/tutorial.c \
  --agent claude
```

```sh
python tools/agent.py queue claim REL:fileseldll:filesel \
  --agent codex
```

A claim records:

- owner and target;
- assigned agent;
- worktree and branch;
- build directory;
- status and priority;
- source owner;
- shared files that may be edited;
- last verified commit;
- timestamps and notes.

The claim is rejected when another active task already uses the same owner,
branch, worktree, build directory, source file, or overlapping shared path.

## Declare shared-file risk before editing

Central headers, `configure.py`, symbols, splits, and recovery schemas should be
declared before they are touched:

```sh
python tools/agent.py queue claim src/board/tutorial.c \
  --agent claude \
  --shared include/game/board.h \
  --shared configure.py
```

For an existing claim:

```sh
python tools/agent.py queue update src/board/tutorial.c \
  --agent claude \
  --add-shared include/game/board.h
```

If Codex already claims an overlapping path, the update fails before the edit.
One worker should then defer the shared change to the integration task.

## Track progress

```sh
python tools/agent.py queue update src/board/tutorial.c \
  --agent claude \
  --status researching

python tools/agent.py queue update src/board/tutorial.c \
  --agent claude \
  --status coding

python tools/agent.py queue update src/board/tutorial.c \
  --agent claude \
  --status verifying \
  --verified-commit HEAD
```

Available active states are:

```text
claimed · researching · coding · verifying · blocked · ready
```

## Inspect the queue

```sh
python tools/agent.py queue status
python tools/agent.py queue status --all
python tools/agent.py queue check
```

Claims with no update for 24 hours are marked with `*`. A stale claim still
protects its owner and paths until it is explicitly released or cancelled.

`python tools/agent.py doctor` also reports whether the current worktree has a
matching claim.

## Finish or release work

After verification and handoff:

```sh
python tools/agent.py queue release src/board/tutorial.c \
  --agent claude \
  --status done \
  --verified-commit HEAD
```

Use `released` when work is intentionally returned to the queue and `cancelled`
when it should not continue.

## Integration rules

The integration worktree should:

1. inspect `queue status` before merging worker results;
2. integrate only claims marked `ready` or `done`;
3. resolve shared-header or configuration changes centrally;
4. run the serialized private build and retail gates;
5. release any remaining integration claims.

Workers may compile different objects in parallel because their build
directories are isolated. Final retail verification remains serialized in the
integration worktree.
