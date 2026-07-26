# Mario Party 6 AI recovery workspace

> **Permanent branch boundary:** this branch is the AI-forward recovery
> workspace. It must never be merged, squashed, or rebased into `main`.
>
> `main` remains the clean, human-facing project. Only verified recovered
> `src/**/*.c` blobs may move to a fresh branch created from `main`.

Read [`AI_WORKSPACE.md`](AI_WORKSPACE.md) before doing any work on this branch.

This workspace reconstructs the US GameCube build of **Mario Party 6**
(`GP6E01`) while coordinating Claude, Codex, local tooling, evidence indexing,
blind benchmarks, and private verification. Those operating files intentionally
stay here.

The active scheduling target is the non-minigame game loop: boot, menus, party
mode, boards, results, and ending. `STATUS.md` is a historical evidence snapshot,
not the default agent context.

## Branch roles

### Human-facing `main`

`main` contains the ordinary project:

- recovered source;
- normal build configuration;
- readable human commit history and pull requests;
- no agent prompts, orchestration, queue metadata, knowledge-card machinery,
  blind-benchmark infrastructure, or AI attribution.

### This AI workspace

This branch contains:

- queue, worktree, scheduling, and integration orchestration;
- exact-first context generation and dependency indexing;
- knowledge cards, freshness state, and historical evidence distillation;
- agent instructions, hooks, benchmarks, and experimental CI.

The branches are permanent parallels. They are not intended to converge.

## Promoting recovered C to `main`

Promotion is a content transfer from one verified commit, not a merge or
cherry-pick of AI-workspace history.

```sh
python tools/promote_recovered_c.py plan \
  --base main \
  --source <verified-worker-commit> \
  --owner <queue-owner> \
  --path src/path/recovered.c

python tools/promote_recovered_c.py create \
  --base main \
  --source <verified-worker-commit> \
  --owner <queue-owner> \
  --path src/path/recovered.c \
  --branch recovery/<human-topic> \
  --worktree ../marioparty6-promotion-<topic> \
  --title "Recover <human-readable subsystem>"
```

The promotion tool:

- creates the branch directly from `main`;
- accepts only added or modified `src/**/*.c` files;
- copies exact blobs from the verified worker commit;
- rejects headers, tooling, metadata, prompts, workflows, reports, and generated
  output;
- rejects AI/agent attribution in source comments, branch names, and commit
  messages;
- verifies that the promotion diff contains only selected C files.

Supporting header or build changes must be recreated and reviewed separately in
the clean promotion worktree. See
[`docs/main_promotion.md`](docs/main_promotion.md).

## Recovery standard

A binary match is necessary proof, not a complete source-authenticity claim.
Raw IDs, opaque arrays, fake padding, invented names, and unexplained compiler
controls remain recovery debt even when output is exact.

The workspace tracks:

```text
binary · source shape · semantics · naming · data domains
```

Read:

- [`AGENTS.md`](AGENTS.md): mandatory workspace rules
- [`AI_WORKSPACE.md`](AI_WORKSPACE.md): permanent branch boundary
- [`docs/main_promotion.md`](docs/main_promotion.md): clean C-only transfer
- [`docs/agent_quickstart.md`](docs/agent_quickstart.md): worker workflow
- [`docs/concurrent_agents.md`](docs/concurrent_agents.md): Claude/Codex bulk work
- [`docs/recovery_standard.md`](docs/recovery_standard.md): evidence and promotion
- [`docs/context_workflow.md`](docs/context_workflow.md): index and context design
- [`docs/blind_recovery_benchmark.md`](docs/blind_recovery_benchmark.md): blind testing and organicity

## Unified workspace commands

```sh
python tools/agent.py doctor
python tools/agent.py hooks install
python tools/agent.py catalog build
python tools/agent.py queue status
```

Create isolated work:

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

Workers may claim dependency-ready tasks automatically:

```sh
python tools/agent.py queue claim-next \
  --agent codex \
  --capability rel \
  --batch menu-flow
```

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

Context reserves space for exact-target rules, owner constraints, compiler
diagnostics, counterexamples, freshness warnings, local objdiff summaries,
source, and acceptance criteria. Historical wave bodies are never loaded
automatically.

## Worker proof and integration proof

A worker commits a clean candidate, runs the public gate, records object proof,
and stops at `ready`:

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

The integration worktree serializes machine-wide resources and private retail
gates. After that proof, use `promote_recovered_c.py` to create the clean
human-facing branch from `main`.

## Blind recovery and organicity

Blind tests score four independent dimensions:

```text
assembly equivalence
retained-source fidelity
candidate organicity
artifact reproducibility
```

```sh
python tools/blind_recovery.py audit
python tools/blind_recovery.py organicity \
  src/gssdk_lib/asrpho/common/blocks/flfxblks/lkahead.c \
  --function ProcessLookAhead
```

The initial surrogate holdouts remain `legacy-reported` because their raw packet
and candidate artifacts were not preserved.

## Public versus private gates

```sh
python tools/agent.py check --base origin/main
```

The public-safe gate validates tooling and policy. It does **not** prove a retail
build. Source promotion also requires relocation-aware object reports, affected
consumers, a serialized DOL/REL build, DTK checksum, and explicit retail
comparisons in the clean promotion worktree.

## Repository layout

- `src/`: recovered source under active investigation
- `include/`: shared declarations and data domains
- `config/recovery/`: AI-workspace evidence, cards, queue policy, and freshness
- `benchmarks/blind_recovery/`: replayable blind-test artifacts
- `docs/`: workspace documentation and forensic evidence
- `tools/`: orchestration, context, benchmark, and verification tooling
- `build/`: ignored output, isolated per worktree
- `orig/GP6E01/`: ignored local retail inputs

No AI workspace file is promoted automatically to `main`. Only verified
`src/**/*.c` content may cross the boundary through a fresh `recovery/*` branch.
