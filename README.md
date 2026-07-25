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
- [`docs/recovery_standard.md`](docs/recovery_standard.md): evidence and promotion rules
- [`CONTRIBUTING.md`](CONTRIBUTING.md): branches, verification, and handoff
- [`docs/README.md`](docs/README.md): documentation index

## Agent-ready workflow

Use one command as the front door:

```sh
python tools/agent.py doctor
```

Generate bounded context for the exact task:

```sh
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --budget 12000

python tools/agent.py context owner main:game/mgdata --budget 7000
```

Run the public-safe branch gate before handoff:

```sh
python tools/agent.py check --base origin/main
```

The gate runs Python compilation and tests, recovery metadata validation,
deterministic SQLite indexing, context/report smoke generation, repository
cleanup policy, private/generated-path checks, whitespace checks, and
changed-line source-quality review. It does **not** claim a retail build.

Committed recovery knowledge lives under `config/recovery/`. Generated SQLite,
reports, and context packs live under ignored `build/context/`. Exact owner,
address, symbol, evidence, and compiler-constraint lookup comes before fuzzy or
whole-repository retrieval.

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
- `config/recovery/`: source-quality state, evidence, names, exceptions, and compiler knowledge
- `docs/`: active documentation and retained recovery evidence
- `tools/`: build helpers, declaration gates, index/context generation, and checks
- `build/`: ignored generated output
- `orig/GP6E01/`: ignored locally extracted retail files

## Contribution boundaries

Work on an isolated `agent/<owner>-<goal>` branch or worktree. One agent should
own one translation unit or tightly connected function cluster. Keep uncertain
semantics uncertain, preserve stable target identity across renames, record
rejected probes, and never regress an independently exact function to close
another target.

No copyrighted game assets, retail binaries, rebuilt DOL/REL files, or generated
analysis output may be committed.
