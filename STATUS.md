# Mario Party 6 recovery status

This page is the public project snapshot for the supported `GP6E01` build. It is intentionally short; detailed experiments belong in commits and pull requests rather than a permanent running notebook.

## Progress

Last published full-project snapshot: **September 15, 2026**

Byte percentages were generated from a verified full-project retail build with `tools/update_progress.py`.

| Area | Code | Data |
| --- | ---: | ---: |
| Entire project | 24.28% | 44.76% |
| Main DOL | 87.77% | 98.84% |
| REL modules | 10.82% | 10.49% |

Of the **82 unique numbered minigame modules** registered in `include/mgdata.inc`,
**11 are fully source-selected** (`m612dll`, `m616dll`, `m621dll`, `m630dll`, `m635dll`, `m640dll`, `m650dll`, `m651dll`, `m657Dll`, `m659Dll`, `m670dll`) and **71 remain**. This scope is distinct
from the game's 136 REL containers, which also include non-minigame modules.

Board recovery is **40 / 40 owners Matching**. Board Single closes the final
owner with **58 / 58 functions** matching instructions and effective physical
relocations. Its source-selected build, including the consistent gamework
provider interface, reproduces the retail DOL and all 136 RELs byte-for-byte.

**m650dll is now fully source-selected**: all 32,568 code bytes and 3,209
data/BSS bytes come from five source objects, with no original-object fallback.
All 57 application instruction bodies and both startup functions match their
effective physical relocations; the complete REL is byte-identical to retail.
Unknown record intervals and unreferenced storage are explicitly documented.
One unused player-value evaluation preserves a retail conversion whose original
source purpose remains unknown. The matrix hook preserves disclosed retail
four-row writes through a three-row matrix; this is not a portable safety fix.
Earlier modules retain their storage and legacy-behavior disclosures in source.
The main DOL has **352 / 396 build objects Matching**, with **44** left.
All newly enabled source objects participate in the verified retail-identical build.

Earlier aggregate REL build-object counts mixed reconstruction units and are
withdrawn. DTK splits change as source is recovered; those counts must not be
presented as minigames. Byte totals and the fixed registered-module census above
are the progress measures.

## Verification

The published snapshot passed the configured project build and retail hash checks:

```sh
ninja -j1
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

On Windows, use `build/tools/dtk.exe`.

All 137 configured outputs passed the retail checksum gate, and rebuilt `main.dol` compared byte-identical with retail SHA-1 `b897e6ade6b3a0cd2f9907689f38a3b19c327e70`.

## Recovery standard

A source file is marked Matching only when the configured compiler reproduces the target output. Matching is necessary, but recovered code should also remain readable and supported by evidence from callers, consumers, relocations, data layout, strings, and related source.

Unknown names or fields should stay explicit until their meaning is known. Compiler-specific constructs should not be added solely to force a match unless the target clearly requires them.

## Keeping this page current

Update this file only after a complete verified build changes the public progress
snapshot or verification gate. Keep it to current aggregate state; owner milestones,
investigation logs, and rejected experiments belong in commit history and recovery
evidence rather than this page.
