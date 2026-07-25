# Mario Party 6 source recovery

This repository reconstructs the US GameCube build of **Mario Party 6**
(`GP6E01`) as readable C/C++ and verifies promoted work against the retail
binaries. Original game files and generated binaries are not committed.

The active scheduling target is the non-minigame game loop: boot, menus, party
mode, boards, results, and ending. `STATUS.md` is a historical evidence snapshot,
not the default agent context.

## Recovery standard

A binary match is necessary proof, not a complete source-authenticity claim.
Raw IDs, opaque arrays, fake padding, invented names, and unexplained compiler
controls remain recovery debt even when the output is exact.

The project tracks five dimensions independently:

```text
binary · source shape · semantics · naming · data domains
```

Read:

- [`AGENTS.md`](AGENTS.md): mandatory repository rules
- [`docs/agent_quickstart.md`](docs/agent_quickstart.md): shortest safe workflow
- [`docs/concurrent_agents.md`](docs/concurrent_agents.md): Claude/Codex bulk work
- [`docs/recovery_standard.md`](docs/recovery_standard.md): evidence and promotion
- [`docs/context_workflow.md`](docs/context_workflow.md): index and context design
- [`CONTRIBUTING.md`](CONTRIBUTING.md): verification and handoff

## Unified agent commands

Inspect the checkout and install lightweight local checks:

```sh
python tools/agent.py doctor
python tools/agent.py hooks install
```

Build or query the operational owner inventory generated from `configure.py` and
source includes:

```sh
python tools/agent.py catalog build
python tools/agent.py catalog query REL:mdpartydll:mdparty
```

The catalog records configured owners, source paths, status, size, includes, and
header consumers. It does not fabricate semantic-recovery claims.

## Parallel Claude and Codex work

Claude and Codex use a shared queue under Git’s common directory, but separate
worktrees, branches, and build directories:

```sh
python tools/agent.py queue add <owner> \
  --priority high \
  --batch board-pass-1 \
  --capability mwcc

python tools/agent.py worktree create <owner> \
  --agent claude \
  --base main \
  --retail <read-only-GP6E01-directory>
```

Workers may take dependency-ready work automatically:

```sh
python tools/agent.py queue claim-next \
  --agent codex \
  --capability rel \
  --batch menu-flow
```

The queue blocks duplicate owners, branches, worktrees, build directories,
source files, overlapping shared paths, and header-consumer conflicts. Priority
is preserved when a queued task is claimed.

Before commits, the real committed/staged/unstaged/untracked diff must fit the
claim:

```sh
python tools/agent.py queue check-diff --base origin/main
```

## Focused recovery context

```sh
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --symptom "saved register lifetime" \
  --local-evidence \
  --budget 12000
```

Context uses fixed section budgets and reserves space for exact-target rules,
owner constraints, compiler diagnostics, counterexamples, freshness warnings,
local objdiff summaries, source, and acceptance criteria. Historical wave bodies
are never loaded automatically.

Inspect cards or their extraction backlog directly:

```sh
python tools/agent.py knowledge function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --symptom "helper boundary"
python tools/agent.py knowledge audit
python tools/knowledge_cards.py freshness
```

## Worker proof and integration proof

A worker commits a clean candidate, runs the public gate, records object-level
proof, and stops at `ready`:

```sh
python tools/agent.py check --base origin/main

python tools/agent.py queue verify <owner> \
  --agent claude \
  --public-gate pass \
  --object-report build/GP6E01/<report>.json \
  --functions-exact <exact/total> \
  --relocations exact \
  --consumer <consumer>=exact \
  --toolchain GC/1.3.2

python tools/agent.py queue update <owner> \
  --agent claude --status ready
```

The proof is tied to the clean current commit. Any later edit requires a new
proof.

The integration worktree serializes machine-wide resources, integrates the
worker commit, runs DOL/REL, consumer, checksum, and retail-byte gates, then
finalizes the task:

```sh
python tools/agent.py queue acquire-resource integration --agent integrator
python tools/agent.py queue acquire-resource retail-build --agent integrator

python tools/agent.py integration finalize <owner> \
  --agent integrator \
  --retail-gate pass \
  --checksum pass \
  --toolchain GC/1.3.2
```

Finalization checks that every claimed path in the integration tree still
matches the worker’s verified commit before setting the task to `done`.

## Public versus private gates

The public-safe gate runs Python compilation/tests, metadata/card/freshness
validation, owner catalog generation, deterministic indexing, context/report
smoke tests, queue policy, whitespace, generated/private-path checks, and
changed-line source-quality review:

```sh
python tools/agent.py check --base origin/main
```

It does **not** prove a retail build. Source promotion also requires the local
serialized build, relocation-aware object reports, affected consumers, DTK
checksum, and explicit DOL/REL comparisons.

## Repository layout

- `src/`: recovered source
- `include/`: shared declarations and data domains
- `config/GP6E01/`: DOL symbols, splits, and retail checksums
- `config/dll/rels/`: REL ownership
- `config/recovery/`: owner state, evidence, names, exceptions, cards, freshness
- `docs/`: active documentation and forensic evidence
- `tools/`: build, queue, catalog, worktree, context, knowledge, and verification tools
- `build/`: ignored output, isolated per worktree
- `orig/GP6E01/`: ignored local retail inputs

No copyrighted game assets, retail binaries, rebuilt DOL/REL files, local queue
state, or generated analysis output may be committed.
