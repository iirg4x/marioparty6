# Mario Party 6 source recovery

This repository reconstructs the US GameCube build of **Mario Party 6**
(`GP6E01`) as readable C/C++ and verifies promoted work against the retail
binaries. Original game files and generated binaries are not committed.

The active scheduling target is the non-minigame game loop: boot, menus, party
mode, boards, results, and ending. Minigame DLLs, instruction DLLs,
minigame-mode wrappers, and mic-quiz modes are currently excluded from that work
queue. [`STATUS.md`](STATUS.md) is the historical evidence snapshot, not the
default agent context.

## Recovery standard

A binary match is necessary proof, not a complete source-authenticity claim.
Raw IDs, opaque arrays, fake padding, synthetic literals, invented names, and
unexplained compiler-control techniques remain recovery debt even when the
output is exact.

The project tracks five dimensions independently:

```text
binary · source shape · semantics · naming · data domains
```

`src/game/mgdata.c` is the model semantic cleanup: it replaced byte-oriented
scaffolding with named domains, natural layout, consumer-backed widths, and
readable source without claiming new matching bytes.

Read:

- [`AGENTS.md`](AGENTS.md): repository-wide agent rules
- [`docs/agent_quickstart.md`](docs/agent_quickstart.md): shortest safe workflow
- [`docs/concurrent_agents.md`](docs/concurrent_agents.md): Claude/Codex same-PC coordination
- [`docs/recovery_standard.md`](docs/recovery_standard.md): evidence and promotion rules
- [`docs/context_workflow.md`](docs/context_workflow.md): index, knowledge cards, and context selection
- [`CONTRIBUTING.md`](CONTRIBUTING.md): branches, verification, and handoff
- [`docs/README.md`](docs/README.md): documentation index

## Agent-ready workflow

Use one command as the front door:

```sh
python tools/agent.py doctor
```

Claude and Codex use a shared local queue stored under Git's common directory.
It is visible across worktrees but is not committed:

```sh
python tools/agent.py queue status
python tools/agent.py queue claim <owner> --agent claude
```

The queue rejects duplicate owners, branches, worktrees, build directories,
source files, and overlapping shared paths. Codex uses `--agent codex`. Each
agent must have a separate worktree and its own `build/` directory.

Generate bounded context for the exact task:

```sh
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --budget 12000

python tools/agent.py context owner main:game/mgdata --budget 7000
```

Context packs automatically include up to five applicable source-to-output
knowledge cards before the source. Exact-target findings, owner constraints,
compiler-wide diagnostics, and known counterexamples are ranked separately so
an agent sees previously recovered rules without loading wave documents or
copying owner-specific tricks blindly.

Inspect the cards or extraction backlog directly:

```sh
python tools/agent.py knowledge function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty
python tools/agent.py knowledge audit
```

Run the public-safe branch gate before handoff:

```sh
python tools/agent.py check --base origin/main
```

The gate runs Python compilation and tests, owner and knowledge-card validation,
deterministic SQLite indexing, context/report smoke generation, repository
cleanup policy, local queue-health reporting, private/generated-path checks,
whitespace checks, and changed-line source-quality review. It does **not** claim
a retail build.

Committed recovery knowledge lives under `config/recovery/`. Generated SQLite,
reports, audits, and context packs live under ignored `build/context/`. Exact
owner, address, symbol, rule, safe action, evidence, and counterexample lookup
comes before fuzzy or whole-repository retrieval.

`tools/decompctx.py` remains available for preprocessed decomp.me context. It
serves a different purpose from the evidence-bounded agent context pack.

## Local build

Install the tools described in [`docs/dependencies.md`](docs/dependencies.md),
then legally extract the US disc into `orig/GP6E01/` while preserving the disc
layout.

```sh
python configure.py
ninja -j1
```

The exact input paths and hashes are defined by `config/GP6E01/config.yml`.
`configure.py` pins the DTK, binutils, compiler, `sjiswrap`, and wrapper
versions. See [`docs/getting_started.md`](docs/getting_started.md) for complete
setup and verification instructions.

Any C/C++, shared header, symbol, split, compiler flag, object status, or link
change requires the relevant relocation-aware object and consumer comparisons,
serialized DOL/REL build, DTK checksum, and explicit retail byte comparisons
before promotion.

## Repository layout

- `src/`: recovered game, board, SDK, library, and REL source
- `include/`: shared declarations, structures, and data domains
- `config/GP6E01/`: DOL inputs, symbols, splits, and retail checksums
- `config/dll/rels/`: REL symbol and split ownership
- `config/recovery/`: owner state, evidence, names, exceptions, and actionable recovery knowledge
- `docs/`: active documentation and retained forensic evidence
- `tools/`: build helpers, claim queue, declaration gates, indexing, knowledge selection, context generation, and checks
- `build/`: ignored generated output, isolated per worktree
- `orig/GP6E01/`: ignored locally extracted retail files

## Contribution boundaries

Work on an isolated `agent/<agent>-<owner>-<goal>` worktree branch. Claim the
owner before editing and declare shared files before touching them. Keep
uncertain semantics uncertain, preserve stable target identity across renames,
record rejected probes and reusable source-to-output findings, and never regress
an independently exact function to close another target.

No copyrighted game assets, retail binaries, rebuilt DOL/REL files, local queue
state, or generated analysis output may be committed.
