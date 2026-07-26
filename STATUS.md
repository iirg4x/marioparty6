# Mario Party 6 recovery status

This page is the public project snapshot for the supported `GP6E01` build. It is intentionally short; detailed experiments belong in commits and pull requests rather than a permanent running notebook.

## Progress

Last published full-project snapshot: **July 26, 2026**

This snapshot was generated from a verified full-project retail build with `tools/update_progress.py`.

| Area | Code | Data | Matching owners |
| --- | ---: | ---: | ---: |
| Entire project | 13.29% | 32.85% | 326 / 912 |
| Main DOL | 47.94% | 74.86% | 295 / 396 |
| REL modules | 5.95% | 6.24% | 31 / 516 |

Within the main game flow at that snapshot:

- `src/game/`: **57 / 57 owners matching**
- `src/board/`: **14 / 40 owners matching**
- Newly matching board owner: `src/board/audio.c` (49 / 49 functions and 444 / 444 relocations exact)
- Fully source-linked flow modules: `actmanDLL`, `bootDll`, `fileseldll`, `selmenuDll`, `sequencedll`, `mdseldll`, `mdpartydll`, and `w01Dll`
- Exact shared GC/2.6 runtime owners now cover 15 additional non-minigame RELs, including the results, ending, option, single-player, miracle-book, bank, opening, and staff modules.
- `w01Dll` is source-linked and byte-identical to retail (`196d7075abbe6eec3031c9484d25216de9dc0889`).

The current priority is the non-minigame game flow: boot, menus, party mode, boards, results, and ending. Minigame modules and mic-quiz modes are not part of the active recovery target.

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

Update this file only after a complete verified build changes the public progress snapshot. Keep updates to the table and short milestone list; investigation logs and rejected experiments should stay in the relevant pull request or commit history.
