# Mario Party 6 recovery status

This page is the public project snapshot for the supported `GP6E01` build. It is intentionally short; detailed experiments belong in commits and pull requests rather than a permanent running notebook.

## Progress

Last published full-project snapshot: **July 27, 2026**

This snapshot was generated from a verified full-project retail build with `tools/update_progress.py`.

| Area | Code | Data | Matching owners |
| --- | ---: | ---: | ---: |
| Entire project | 13.47% | 32.87% | 328 / 914 |
| Main DOL | 48.28% | 74.87% | 296 / 396 |
| REL modules | 6.08% | 6.25% | 32 / 518 |

Within the main game flow at that snapshot:

- `src/game/`: **57 / 57 owners matching**
- `src/board/`: **15 / 40 owners matching**
- One-time bulk board-source promotion: **182 newly strict-exact functions** across 16 partial owners, with zero exact-function regressions.
- The promoted partial owners are `capevent`, `capmove`, `capselect`, `capspecial`, `capsule`, `dice`, `last5`, `mgcall`, `player`, `scroll`, `shopevent`, `single`, `snpc`, `telop`, `tutorial`, and `wipe`; they remain NonMatching until fully recovered.
- Newly matching board owner: `src/board/board.c` (35 / 35 functions and 618 / 618 relocations exact)
- Newly matching board owner: `src/board/audio.c` (49 / 49 functions and 444 / 444 relocations exact)
- Newly strict-exact board sources: `src/board/gate.c` (11 / 11 functions, 168 / 168 relocations) and `src/board/roulette.c` (21 / 21 functions, 291 / 291 relocations)
- Fully source-linked flow modules: `actmanDLL`, `bootDll`, `fileseldll`, `selmenuDll`, `sequencedll`, `mdseldll`, `mdpartydll`, `s01Dll`, and `w01Dll`
- Exact shared GC/2.6 runtime owners now cover 15 additional non-minigame RELs, including the results, ending, option, single-player, miracle-book, bank, opening, and staff modules.
- `s01Dll` is source-linked and byte-identical to retail (`7f0cfdb2d2b0b2c50b92675e5bef55d72cf94dd7`), including all 44 application functions and 741 relocations.
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
