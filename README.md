# Mario Party 6 decompilation and source recovery

This repository reconstructs the US GameCube build of **Mario Party 6**
(`GP6E01`) as readable C/C++ source and verifies it against the retail binaries.
Original game files are not committed.

The active recovery target is the byte-identical non-minigame game loop: boot,
menus, party mode, boards, results, and ending. Minigame DLLs, instruction DLLs,
minigame-mode wrappers, and mic-quiz modes are currently outside that scheduling
target. See [STATUS.md](STATUS.md) for the evidence-backed progress snapshot.

## Recovery standard

A binary match is necessary proof, not a complete source-authenticity claim.
Raw metadata IDs, opaque arrays, fake padding, synthetic literals, unsupported
semantic names, and unexplained compiler-control techniques remain recovery
debt even when the output is exact.

The project tracks binary status separately from source shape, semantics,
naming, and data-domain recovery. Start with:

- [Source recovery standard](docs/recovery_standard.md)
- [Recovery index and context workflow](docs/context_workflow.md)
- [Agent instructions](AGENTS.md)

`src/game/mgdata.c` is the model semantic cleanup: named domains, natural
layout, consumer-backed widths, and readable source were recovered without
claiming new matching bytes.

## Token-efficient recovery workflow

Committed recovery knowledge lives in `config/recovery/`. A deterministic
SQLite index and bounded context packs are generated under `build/context/`.
The primary lookup path is exact owner, stable identity, symbol, evidence, and
compiler constraint—not a whole-repository prompt.

```sh
python tools/recovery_index.py check
python tools/recovery_index.py build
python tools/recovery_index.py query mdpartydll:0xBBD8

python tools/context_pack.py \
  --budget 12000 \
  --output build/context/mdparty_BBD8.md \
  function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty
```

`tools/decompctx.py` still generates preprocessed context for decomp.me.
`tools/context_pack.py` instead supplies a compact recovery contract, target
function, bounded owner signatures, evidence, known rejected probes, naming
debt, and acceptance criteria for agents and human review.

## Source-quality review

New compiler-shape controls must be authenticated, recorded as temporary debt,
or rejected. Review added source lines with:

```sh
python tools/source_quality.py --changed origin/main --strict
```

The check focuses on newly introduced pragmas, forced inline/no-inline controls,
`volatile` or `register` used for code generation, inline assembly, include-guard
overrides, synthetic padding, opaque blobs, and dead code-generation branches.
It does not retroactively fail unrelated historical code.

Generate a readable status matrix with:

```sh
python tools/recovery_report.py \
  --output build/context/recovery-report.md
```

## Building

The project uses [decomp-toolkit](https://github.com/encounter/decomp-toolkit),
Ninja, the pinned Metrowerks compilers, objdiff, and the repository helper tools.
Setup details remain in:

- [Dependencies](docs/dependencies.md)
- [Getting started](docs/getting_started.md)
- [`symbols.txt`](docs/symbols.md)
- [`splits.txt`](docs/splits.md)

After the original `GP6E01` files are extracted into `orig/GP6E01`, configure
and build with the repository’s normal `rtk` environment. The final promotion
gate is the serialized build plus the configured DTK checksum and explicit
DOL/REL byte comparisons documented in [STATUS.md](STATUS.md).

Public-safe metadata checks do not require original game files:

```sh
python -m unittest discover -s tools/tests -v
python tools/recovery_index.py check
python tools/recovery_index.py build
```

## Project structure

- `src/`: recovered game, board, SDK, library, and REL source
- `include/`: shared declarations, structures, data domains, and generated-style tables
- `config/GP6E01/`: DOL configuration, symbols, splits, and retail checksums
- `config/dll/rels/`: REL symbol and split ownership
- `config/recovery/`: source-authenticity status, evidence, names, exceptions, and compiler knowledge
- `docs/`: setup documentation and retained recovery evidence
- `tools/`: build helpers, declaration gates, indexing, context generation, and checks
- `build/`: generated and ignored build products, reports, index, and context packs
- `orig/GP6E01/`: locally extracted and ignored retail files

## Contribution rules

Work one translation-unit owner or tightly connected function cluster at a
time. Begin with evidence research, then produce a natural candidate before
compiler reconciliation. Never regress an independently exact function to make
one target match. Re-check consumers whenever a shared type, declaration,
structure, or data owner changes.

Keep uncertain semantics uncertain. Preserve stable target identity when
renaming. Record rejected probes and owner-specific compiler behavior so the
next contributor does not spend context and time rediscovering them.

No copyrighted game assets or original binaries should be committed.
