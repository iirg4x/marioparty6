# Native Matching Wave 95 -- semantic `mgdata` recovery

Wave 95 replaces the byte-oriented `game/mgdata.c` scaffold with readable,
dependency-backed source while preserving the existing Matching owner and the
complete DOL/REL gate. This is a source-quality correction, not a new owner
promotion.

## Why the former source was insufficient

Historical commit `f5dedebb` recovered all three functions and the complete
83-record table, but optimized for byte closure. It left the table inline as
raw overlay, flag, message, data-directory, picture, and instruction-message
integers. It also retained explicit structure padding, a synthetic seven-byte
`lbl_802BF859` string object, a two-element `lbl_802BF868` tail array, and a
`_MATH_H` include-guard override.

The public comparison at
<https://github.com/mariopartyrd/marioparty6/compare/93ed1427d9a2...ac64347f9fd8>
made the missing semantic layer visible. Its 3,247-line `mgdata.inc` labels the
same 82 minigames plus the same `DLL_NONE` sentinel; it does not contain more
table coverage than the local source. The useful difference is named
dependencies and dimensional structure.

The comparison was used as evidence, not imported as authority. MP5
authenticates the separate `mgdata.inc`, natural padding, `MGDATA`, `MgNoGet`,
table sentinel, and score-calculation source shape. Active MP6 consumers
authenticate the M6 day/night/subgame dimensions and the byte-buffer globals.
Unsupported remote renames (`MBookEvtNo`, `MgBoardE3F`) were rejected in favor
of MP5/MP7 sibling parity. Duplicate flag terms and incorrect copied field
comments in the public table were also rejected.

## Recovered source shape

- `MgDataTbl` now lives in `include/mgdata.inc`, matching the MP5 owner shape.
  All 83 records use `DLL_*`, `MG_TYPE_*`, `MG_FLAG_*`, `MG_NAME_*`, `DATA_*`,
  `INSTPIC_*`, and `MG_INST_*` identifiers rather than packed hex literals.
- `include/messdir_enum.h`, `include/messdir_table.h`,
  `include/messnum/mg_name.h`, `include/messnum/mg_inst.h`, and
  `include/datanum/instpic.h` recover the corresponding message and asset index
  domains.
- `MGDATA` uses natural padding and the real
  `instPic[MG_TIME_MAX][MG_SUBGAME_MAX]`,
  `movie[MG_TIME_MAX][MG_SUBGAME_MAX]`, and
  `instMes[MG_INST_TYPE_MAX][MG_INST_PAGE_MAX]` dimensions. The former
  `unk03`, `unk07`, `instPicN`, and `movieN` layout scaffolding is gone.
- REL access widths prove `MgModeWork` is an aligned `u8[256]` work buffer and
  `MgBattleOrder` is `u8[GW_PLAYER_MAX]`, not a scalar `int`. The exported
  `MgBoard2Force` and `MiracleBookEvtNo` spellings remain because sibling
  evidence supports them and the binary cannot authenticate the proposed
  alternatives.
- `MgDecaScoreCalc` now contains the natural `OSReport("%d\n", mgScore)` call.
  The target string symbol is correctly bounded to four bytes, and
  `lbl_802BF868` is a scalar; alignment gaps remain linker/split ownership,
  not fabricated source arrays.
- The unused math dependency and `_MATH_H` override are gone. The existing
  overlay enum was separated into the dependency-light `game/omovl.h` owner,
  and the private decathlon interpolation points now have semantic
  `score`/`value` fields. No compiler-control macro or fake data remains.
- Matching `REL/selmenuDll/selmenu.c` now consumes the shared `MGDATA` header
  instead of carrying a second opaque 0x7C-byte declaration and duplicate
  minigame constants.

## Object and consumer proof

All comparisons use `functionRelocDiffs=data_value`.

- `main/game/mgdata`: 3/3 functions exact; `.text`, `.data`, `.bss`, `.sdata`,
  `.sbss`, and `.sdata2` all report 100%; 538/538 relocations agree. The source
  omits the target split's unowned four-byte `.data` and `.sdata` alignment
  tails rather than fabricating source padding.
- `main/game/gamemes`: 41/41 functions exact and `.text` 100%.
- `main/board/pause`: 22/22 functions exact and `.text` 100%.
- `mdseldll/REL/mdseldll/mdsel`: 113/113 functions exact and `.text` 100%.
- `selmenuDll/REL/selmenuDll/selmenu`: 25/25 functions exact and `.text` 100%.

Retained reports:

- `build/GP6E01/mgdata_semantic_candidate3.json`
- `build/GP6E01/mgdata_consumer_gamemes.json`
- `build/GP6E01/mgdata_consumer_pause.json`
- `build/GP6E01/mgdata_consumer_mdsel.json`
- `build/GP6E01/selmenu_mgdata_consumer.json`

## Full gate

- Full serialized `rtk ninja`: `137 files OK`.
- Explicit `dtk shasum -q -c config/GP6E01/build.sha1`: `137 files OK`.
- `main.dol` compares byte-identical; SHA-1
  `b897e6ade6b3a0cd2f9907689f38a3b19c327e70`.
- `selmenuDll.rel` compares byte-identical; SHA-1
  `7bc5aa5991425fdb03153f1611a39bb5fa8e0cd7`.
- `mdseldll.rel` compares byte-identical; SHA-1
  `e6c20b24cfca8135ed8c49ce59108100277444c0`.
- The final build preserves only the intentional two-line `symbols.txt`
  ownership correction for the four-byte string and scalar tail symbols.

## Rule carried forward

`Matching` proves emitted bytes, not authentic source quality. Raw metadata
IDs, opaque whole-tail arrays, explicit natural padding, synthetic literals,
and compiler-guard overrides remain recovery debt even when their object is
exact. Future `game/`, `board/`, and REL/DLL work must apply the same
consumer-width, archive-index, sibling-shape, and full-gate analysis before
calling such owners substantially recovered. Existing scope exclusions control
the work queue, not this source-quality standard.
