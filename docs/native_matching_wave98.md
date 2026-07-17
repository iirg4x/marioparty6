# Native matching Wave 98: `mdparty` initialized data

Date: 2026-07-17

## Scope

This wave recovers the initialized-data owner in
`REL/mdpartydll/mdparty.c`. The Wave 97 source object emitted `0x1D9` data
bytes against the target split's `0xF10`. Wave 98 adds `0xD34` semantic bytes:
the source now emits `0xF0D`, and every one of those bytes is identical to the
corresponding target byte.

The remaining target bytes at `0xF0D..0xF0F` are three zero alignment bytes
between sections. They are not represented as a source global. The owner
remains fallback-linked and `C-not-yet-matched` because seven text functions
still diverge; no configure entry is flipped.

## Recovered owners

### Sprite resources and descriptors

The target bytes and the `fn_1_4834` consumers recover three related tables:

- `lbl_1_data_1C[32]` is a sprite-animation resource table expressed as
  `DATANUM(DATA_mdparty, fileNo)` values.
- `lbl_1_data_9C[17]` contains the exact member count for each of the 17
  sprite groups.
- `lbl_1_data_C0[58]` contains the complete sprite creation and placement
  descriptors.

MP5's `SpriteAnimFile`, `SpriteInfo`, and `MDSPR_INFO` provide the sibling
source family. MP6 consumers extend the descriptor with group/member ownership,
bank, and Z rotation. The recovered MP6 type is:

```c
typedef struct MdSprInfo_s {
    s16 groupNo;
    s16 memberNo;
    s16 animNo;
    s16 priority;
    s16 bank;
    HuVec2f pos;
    HuVec2f scale;
    float zRot;
} MDSPR_INFO;
```

It is `0x20` bytes. Five `s16` fields end at offset `0xA`; the compiler's
natural two-byte alignment places `pos` at `0xC`. The former explicit `pad`
member was not a logical field and is removed. All real fields are consumed by
`HuSprCreate`, `HuSprGrpMemberSet`, `HuSprPosSet`, `HuSprScaleSet`, or
`HuSprZRotSet`.

The group-count table is 17 logical `s16` values (`0x22` bytes), followed by
the same natural two-byte alignment before `MDSPR_INFO`. DTK's target split
attributes that alignment to the preceding symbol and reports `0x24`; the
source does not invent an eighteenth element. The corresponding BSS group-ID
array is likewise 17 logical entries with alignment before the next owner.

### Model resources and positions

`lbl_1_data_800[41]` recovers the complete model-animation resource sequence
as `DATANUM(DATA_mdparty, fileNo)` values. Its consumers load the resources
into the 41-entry animation pointer bank and select them at the recovered model
transition boundaries.

`lbl_1_data_8A4[67]` is a typed `HuVecF` position bank. All 67 vectors are
materialized as floats rather than packed words. Consumer ranges cover player
selection, character placement, team layouts, confirmation paths, and the
late party-flow presentation.

### Strings, state, and the selection graph

The tail recovery restores the camera diagnostic formats, model hook/object
names, animation names, six signed-halfword state entries, and the full
`LBL_1_DATA_E12_ENTRY[11]` directional-selection graph. Each graph entry has
eight signed neighbor slots plus the signed occupancy field at offset `0x10`;
the `fn_1_B748`/`fn_1_BBD8` family and selection-state consumers prove the
`0x12` stride and signed accesses.

All strings use the target ASCII bytes, LF line endings, one implicit NUL,
and natural alignment. The target contains no `.data` relocations. Four
`.rodata` pointer relocations reference the recovered `ys77120`, `ys77121`,
`ys77060`, and `ys77061` strings; the relocation-aware object report retains
them without mismatch.

## Source-authenticity boundary

The retail REL exposes only `_prolog`, `_epilog`, `_ctors`, and `_dtors`.
Address-based global names are therefore retained unless sibling parity is
strong enough to authenticate a type-family name such as `MDSPR_INFO`. No
internal MP6 symbol spelling is invented.

The already-present 14-word board-directory table is byte-correct, but the
target cannot distinguish one `[14]` array from two seven-entry rows or prove
signedness. This wave does not claim that unresolved spelling or reshape it.
Likewise, the three section-tail zeros and the two group-table alignment bytes
remain compiler/linker alignment, not source-visible padding owners.

No inline assembly, byte blob, synthetic padding global, fake array member,
register force, or compiler-control pragma is added.

## Object proof

The final relocation-aware report is:

`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave98_final_object.json`

It proves:

- 258/258 target functions remain represented;
- 251 functions and `0x3BE2C / 0x3F424` target text bytes remain exact;
- target/source `.rodata` are both exact at `0x358`;
- target/source `.bss` extents remain `0xA80`;
- all `0xF0D` source `.data` bytes equal target bytes `0x000..0xF0C`;
- the only section-length difference is target alignment `00 00 00` at
  `0xF0D..0xF0F`.

The semantic data comparison is reproduced with:

```powershell
rtk build/binutils/powerpc-eabi-objcopy.exe --dump-section `
  .data=build/GP6E01/src/REL/mdpartydll/mdparty_target_data.bin `
  build/GP6E01/mdpartydll/obj/REL/mdpartydll/mdparty.o
rtk build/binutils/powerpc-eabi-objcopy.exe --dump-section `
  .data=build/GP6E01/src/REL/mdpartydll/mdparty_source_data_wave98.bin `
  build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk cmp -n 3853 `
  build/GP6E01/src/REL/mdpartydll/mdparty_target_data.bin `
  build/GP6E01/src/REL/mdpartydll/mdparty_source_data_wave98.bin
```

The final serialized Ninja build and explicit DTK checksum each report
`137 files OK`. Separate `cmp` gates prove `main.dol` and `mdpartydll.rel`
byte-identical, and the final build leaves `config/GP6E01/symbols.txt`
unchanged.
