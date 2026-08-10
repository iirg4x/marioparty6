# Mario Party 6 recovery status

This page is the public project snapshot for the supported `GP6E01` build. It is intentionally short; detailed experiments belong in commits and pull requests rather than a permanent running notebook.

## Progress

Last published full-project snapshot: **August 10, 2026**

This snapshot was generated from a verified full-project retail build with `tools/update_progress.py`.

| Area | Code | Data | Matching build objects |
| --- | ---: | ---: | ---: |
| Entire project | 14.41% | 38.64% | 343 / 925 |
| Main DOL | 53.32% | 89.26% | 310 / 396 |
| REL modules | 6.16% | 6.56% | 33 / 529 |

Current board recovery: **17 / 40 source owners matching** and **66 / 72
Towering Treetop stub seams strict-exact and promoted**. `src/board/tutorial.c`
is matching at **55 / 55 functions**. Current promoted partial-owner recovery
includes `src/board/capevent.c` at **200 / 237 strict-exact functions** and
**202 / 237 data-value-exact functions**; `src/board/capmove.c` at **17 / 24**
and **18 / 24**; `src/board/capsule.c` at **111 / 165** and **112 / 165**;
`src/board/capspecial.c` at **18 / 44** and **19 / 44**; `src/board/config.c` at
**31 / 51** and **30 / 51**; `src/board/captrap.c` at
**11 / 20** and **11 / 20**; `src/board/coin.c` at **36 / 52** and **36 / 52**;
`src/board/dice.c` at **53 / 68** and **54 / 68**; `src/board/last5.c` at **6 /
10** and **6 / 10**; `src/board/mgcall.c` at **26 / 38** and **26 / 38**;
`src/board/player.c` at **158 / 165** and **159 / 165**; `src/board/single.c`
at **42 / 58** and **43 / 58**; `src/board/snpc.c` at **43 / 85** and **47 /
85**; `src/board/star.c` at **63 / 90** and **71 / 90**; `src/board/telop.c`
at **30 / 31 strict-exact and data-value-exact functions**; and
`src/board/wipe.c` at **32 / 33 strict-exact and data-value-exact functions**.
The latest recovery batch adds **61 strict-exact functions** and **33,632 text
bytes** across 11 partial owners with zero exact-function regressions. The
board-owner and Treetop seam totals are unchanged because these files remain
partial owners.

Eight main-DOL MusyX runtime owners now build from recovered C: `synthvoice.c`
at **21 / 21**, `synthdata.c` at **27 / 27**, `hw_aramdma.c` at **13 / 13**,
`snd_midictrl.c` at **35 / 35**, `s_data.c` at **7 / 7**, `synth.c` at
**27 / 27**, `synthmacros.c` at **52 / 52 strict-exact functions**, and
`snd3d.c` at **16 / 16 strict-exact and data-value-exact functions**. The
complete `snd3d.c` owner contributes **8,760 text bytes** with exact retained
relocations; its authenticated extra donor csects are discarded by the linker.
Their combined source-linked build remains retail-identical. `hardware.c`
remains `NonMatching` until its whole-object link closure is exact.

The MusyX DSP image owner now builds from sibling-authenticated original data:
the **7,872-byte** `dspSlave` payload and its `dspSlaveLength` value are exact,
both objects have zero relocations, and the complete DOL remains
retail-identical. This data-only owner has **0 / 0 functions** and earns zero
clean-C credit.

The current partial MusyX batch adds **45 strict-exact functions** and
**15,224 text bytes**, plus **44 data-value-exact functions** and **19,376
text bytes**, with zero exact-function regressions. No owner or configuration
status changes are claimed.

The canonical Runtime `New.cp` owner now builds from recovered C++ allocation
operators with **372 text bytes** and **152 data bytes**; the complete DOL
remains retail-identical.

The main-DOL MSL `e_exp` owner landed in **1a848a3** at **1 / 1
strict-exact and data-value-exact function**, with **0x21C `.text`**, **0x30
`.rodata`**, **0x78 `.sdata2`**, and **21 / 21 relocations**. Candidate SHA-256
`e55b8b0d03c26a8288513bd2348af240d316e06d0f88cc9c6508d0b75878f717`, target
SHA-256 `c967b5c71b6e6c89bd7d53366b117c3c2815e784c45b4f9ede995aef1c03bd7d`,
and validation report SHA-256
`6a214d3ecc35f5a52904c67cf5eb7197dfa79ac28f76989074b25f4bd93b6fd4` remain
authenticated; **165 tests pass** and the complete DOL remains
retail-identical.

The main-DOL GSSDK `SlidingHisto_Init` function now matches exactly at **1 / 1
function** and **0x194 `.text` bytes**, with **21 / 21 relocations** and no data
delta. The `slidhist.c` owner remains `NonMatching` until its remaining three
nonexact functions close.

The TRK exception-vector owner now builds from a sibling-authenticated
standalone assembly exception covering **7,988 exact `.init` bytes**. Its 48
target-specific handler relocations resolve exactly, and the complete DOL
remains retail-identical. This owner is tracked separately from recovered
clean C.

REL recovery is stored under canonical application owners: **181 synthetic
pass/tail/address-derived C files have been removed across eleven DLLs**. Their
compiled comparison objects retain **538 / 666 exact recovered functions**;
incomplete application owners remain `NonMatching` as whole units until their
full text, data, relocation, and consumer closure is exact.

The complete `motchkDll` application owner now builds from recovered C with
**9 / 9 strict-exact and data-value-exact functions**, **8,144 text bytes**,
and exact `.rodata`, `.data`, `.bss`, and **1,014 relocation rows**. Partial
`w11Dll` and `m616dll` function-island reconstructions are not tracked or
credited because no authenticated original translation-unit path is known;
both modules use retail fallback. The canonical `s03Dll/s03.c` recovery is
retained as incomplete evidence but remains `NonMatching`, so the full S03
application owner also uses retail fallback until owner-wide closure.
The MDBank comparison set advances to **113 / 116 strict-exact functions**,
and MDPResult advances to **121 / 206 strict-exact** and **129 / 206
data-value-exact functions**. All affected RELs remain retail-identical.

The canonical `fileseldll` application owner is strict-exact at **39 / 39
functions**, **35,732 text bytes**, and **2,900 relocations**. Its source has no
raw hexadecimal domains or synthetic padding, uses target-backed resource and
message names, and scores **94 / 100** on the source-organicity review. The save
owner remains strict-exact at **23 / 23 functions**, **11,856 text bytes**, and
**831 relocations**. The complete REL remains retail-identical.

The canonical `mdpartydll` stage owner is strict-exact at **57 / 57
functions** and **29,656 text bytes**. Its particle targets and constant
domains are typed and named, with no raw hexadecimal literals, header-guard
override, or source pragma; the exact pool-data mode now lives in the owner
compiler profile. The complete REL remains retail-identical with SHA-1
`519debb149ef42eda1ab3b0a4d2b3132b4f3e3cc`.

`Matching build objects` is a reconstruction/configuration metric, not a count
of semantic owners in the original game. Its denominator can increase when one
autogenerated DTK fallback span is subdivided around newly recovered C objects;
the code and data byte percentages are the fixed-denominator progress measures.

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
