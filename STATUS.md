# Mario Party 6 recovery status

This page is the public project snapshot for the supported `GP6E01` build. It is intentionally short; detailed experiments belong in commits and pull requests rather than a permanent running notebook.

## Progress

Last published full-project snapshot: **October 9, 2026**

Byte percentages were generated from a verified full-project retail build with `tools/update_progress.py`.

| Area | Code | Data |
| --- | ---: | ---: |
| Entire project | 55.84% | 65.59% |
| Main DOL | 90.36% | 99.25% |
| REL modules | 48.52% | 44.27% |

Sink or Swim (`m625Dll`) selects all **96 functions** from **8 complete translation units**; its linked REL matches retail.

Rocky Road (`m644dll`) selects all **148 functions** from **8 complete translation units**; its linked REL matches retail.

Speak Up (`micquizdll`) selects all **373 functions** from **3 complete translation units**; its linked REL matches retail.

Trick or Tree (`m662dll`) selects all **97 functions** from **5 complete translation units**; its linked REL matches retail.

Solo Mode (`mdsingdll`) selects all **306 functions** from **3 complete translation units**; its linked REL matches retail.

Conveyor Bolt (`m631Dll`) selects all **128 functions** from **4 complete translation units**; its linked REL matches retail.

Mic Mode (`mdmicdll`) selects all **326 functions** from **3 complete translation units**; its linked REL matches retail.

Treetop Bingo (`mgmbingodll`) selects all **140 functions** from **8 complete translation units**; its linked REL matches retail.

Battle Bridge (`mgmbattledll`) selects all **145 functions** from **6 complete translation units**; its linked REL matches retail.

Mario Party 6 (`micquizseldll`) selects all **173 functions** from **2 complete translation units**; its linked REL matches retail.

Decathlon Park (`mgmdecathlondll`) selects all **120 functions** from **8 complete translation units**; its linked REL matches retail.

Banana Shake (`m675Dll`) selects all **237 functions** from **8 complete translation units**; its linked REL matches retail.

Endurance Alley (`mgmrenshodll`) selects all **123 functions** from **8 complete translation units**; its linked REL matches retail.

Ball Dozers (`m627dll`) selects all **94 functions** from **3 complete translation units**; its linked REL matches retail.

Ending (`endingdll`) selects all **159 functions** from **3 complete translation units**; its linked REL matches retail.

T Minus Five (`m658dll`) selects all **146 functions** from **4 complete translation units**; its linked REL matches retail.

Option Mode (`optionDll`) selects all **61 functions** from **5 complete translation units**; its linked REL matches retail.

staffdll (`staffdll`) selects all **97 functions** from **2 complete translation units**; its linked REL matches retail.

DOL source: `src/gssdk_lib/asrpho/common/blocks/flfxblks/pitchco.c` now builds from source; `main.dol` matches retail byte for byte.

Pokey Punch-out (`m621dll`) selects all **86 functions** from **4 complete translation units**; its linked REL matches retail.

Mount Duel (`mgmtournamentdll`) selects all **125 functions** from **9 complete translation units**; its linked REL matches retail.

REL source cleanup: `miraclebookdll`, `w05Dll`, `ttwarsdll`, `m657Dll`, `mgmfreedll`; reviewed source cleanup and generated progress; all selected RELs match retail byte for byte.

DOL source cleanup: `src/game/THPSimple.c`, `src/game/actman.c`, `src/game/data.c`, `src/game/decode.c`, `src/game/hsfman.c`, `src/game/hsfmotion.c`, `src/game/mgtimer.c`, `src/game/mic.c`, `src/game/saveload.c`; reviewed source cleanup and generated progress; `main.dol` matches retail byte for byte.

Wrasslin' Rapids (`m664dll`) selects all **71 functions** from **3 complete translation units**; its linked REL matches retail.

Miracle Book (`miraclebookdll`) selects all **58 functions** from **3 complete translation units**; its linked REL matches retail.

DOL source: `src/gssdk_lib/asrpho/common/blocks/fftmod.c` now builds from source; `main.dol` matches retail byte for byte.

DOL source: `src/Runtime.PPCEABI.H/Gecko_ExceptionPPC.cpp` now builds from source; `main.dol` matches retail byte for byte.

Party Mode (`mdpartydll`) selects all **334 functions** from **3 complete translation units**; its linked REL matches retail.

Shared C++ framework (`framework:octtree_debug.cpp`): **30 complete source files** compile once; all **21 consumer RELs** match retail. This publishes already exact code.

Burnstile (`m668DLL`) selects all **91 functions** from **6 complete translation units**; its linked REL matches retail.

Boot (`bootDll`) selects all **68 functions** from **4 complete translation units**; its linked REL matches retail.

Thirsty Gulch (`s01Dll`) selects all **63 functions** from **3 complete translation units**; its linked REL matches retail.

DOL source: `src/gssdk_lib/asrpho/common/blocks/flblocks/acne.c` now builds from source; `main.dol` matches retail byte for byte.

DOL source: `src/gssdk_lib/asrpho/rec1600/creaspt.c` now builds from source; `main.dol` matches retail byte for byte.

DOL source: `src/dolphin/mic/mic.c` now builds from source; `main.dol` matches retail byte for byte.

Towering Treetop (`w01Dll`) selects all **151 functions** from **2 complete translation units**; its linked REL matches retail.

Mode Select (`mdseldll`) selects all **132 functions** from **2 complete translation units**; its linked REL matches retail.

File Select (`fileseldll`) selects all **88 functions** from **4 complete translation units**; its linked REL matches retail.

openingDll (`openingDll`) selects all **60 functions** from **2 complete translation units**; its linked REL matches retail.

E. Gadd's Garage (`w02Dll`) selects all **137 functions** from **3 complete translation units**; its linked REL matches retail.

DOL source: `src/gssdk_lib/asrpho/rec1600/creasp.c` now builds from source; `main.dol` matches retail byte for byte.

Castaway Bay (`w05Dll`) selects all **119 functions** from **3 complete translation units**; its linked REL matches retail.

w10Dll (`w10Dll`) selects all **46 functions** from **3 complete translation units**; its linked REL matches retail.

DOL source: `src/gssdk_lib/asrpho/common/blocks/exev_dp.c` now builds from source; `main.dol` matches retail byte for byte.

DOL source: `src/gssdk_lib/asrpho/common/blocks/pitchwin.c` now builds from source; `main.dol` matches retail byte for byte.

Faire Square (`w03Dll`) selects all **64 functions** from **3 complete translation units**; its linked REL matches retail.

DOL source: `src/gssdk_lib/asrpho/common/blocks/flfxblks/combiner.c` now builds from source; `main.dol` matches retail byte for byte.

resultDll (`resultDll`) selects all **76 functions** from **6 complete translation units**; its linked REL matches retail.

Stamp By Me (`m649Dll`) selects all **190 functions** from **8 complete translation units**; its linked REL matches retail.

Talkie Walkie (`m667dll`) selects all **172 functions** from **5 complete translation units**; its linked REL matches retail.

DOL source: `src/gssdk_lib/asrpho/common/blocks/flfxblks/slidhist.c` now builds from source; `main.dol` matches retail byte for byte.

DOL source: `src/msm/msmsys.c` now builds from source; `main.dol` matches retail byte for byte.

DOL source: `src/gssdk_lib/asrpho/common/blocks/flfxblks/trigglr.c` now builds from source; `main.dol` matches retail byte for byte.

Shared C++ framework (`framework:camera.cpp`): **28 complete source files** compile once; all **21 consumer RELs** match retail. This publishes already exact code.

DOL source: `src/gssdk_lib/gsapi/callbacks.c` now builds from source; `main.dol` matches retail byte for byte.

Clockwork Castle (`w06Dll`) selects all **76 functions** from **3 complete translation units**; its linked REL matches retail.

Shared C++ framework (`framework:random.cpp`): **22 complete source files** compile once; all **21 consumer RELs** match retail. This publishes already exact code.

Shared C++ framework (`framework:new.cpp`): **1 complete source files** compile once; all **21 consumer RELs** match retail. This publishes already exact code.

Pixel Perfect (`m636dll`) selects all **64 functions** from **8 complete translation units**; its linked REL matches retail.

Jump the Gun (`m643DLL`) selects all **128 functions** from **5 complete translation units**; its linked REL matches retail.

Snowflake Lake (`w04Dll`) selects all **134 functions** from **3 complete translation units**; its linked REL matches retail.

w11Dll (`w11Dll`) selects all **40 functions** from **3 complete translation units**; its linked REL matches retail.

DOL source: `src/gssdk_lib/asrpho/common/blocks/undersam.c` now builds from source; `main.dol` matches retail byte for byte.

TT Wars (`ttwarsdll`) selects all **50 functions** from **3 complete translation units**; its linked REL matches retail.

Motion Check (`safdll`) selects all **37 functions** from three complete translation units; its linked REL matches retail.

Hyper Sniper (`m646Dll`) selects all **166 functions** from ten complete translation units; its linked REL matches retail.

Pop Star (`m629Dll`) selects all **45 functions** from two complete translation units; its linked REL matches retail.

Minigame Instructions (`instDll`) selects all **49 functions** from six complete translation units; its linked REL matches retail.

Mini-game Tour (`mgmfreedll`) selects all **109 functions** from native source. Its three translation units, data, and linked REL reproduce retail.

Of the **82 unique numbered minigame modules** registered in `include/mgdata.inc`,
**22 are fully source-selected** (`m602Dll`, `m608dll`, `m612dll`, `m616dll`, `m618dll`, `m621dll`, `m629Dll`, `m630dll`, `m632dll`, `m633dll`, `m635dll`, `m637dll`, `m638Dll`, `m640dll`, `m646Dll`, `m650dll`, `m651dll`, `m656DLL`, `m657Dll`, `m659Dll`, `m668DLL`, `m670dll`) and **60 remain**. This scope is distinct
from the game's 136 REL containers, which also include non-minigame modules.

Board recovery is **40 / 40 owners Matching**. Board Single closes the final
owner with **58 / 58 functions** matching instructions and effective physical
relocations. Its source-selected build, including the consistent gamework
provider interface, reproduces the retail DOL and all 136 RELs byte-for-byte.

Mini-Game Mode (`mdminidll`) selects all **259 functions** from **3 complete translation units**; its linked REL matches retail.
Fresh compilation verifies instruction bytes and effective relocations for every
function, with zero protected-function losses. All six allocated sections and
the complete linked REL are retail-identical. Composed translation units supply
the functions that share inline definitions and constant pools; their standalone
fallback entries are not selected. M630 remains retail-identical with its own
typed audio-call context unchanged. Minigame Mode is not one of the 82 numbered
minigame modules, so the numbered-module census above is unchanged.

The generated byte percentages above use the build's standard object accounting;
the separate 259-function count includes verified composed-source selections.
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
