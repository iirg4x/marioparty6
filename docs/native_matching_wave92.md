# Native Matching Wave 92 -- `mdparty.c` navigation source shape

Wave 92 recovers two target-proven source-shape details in `fn_1_BBD8` without
admitting an unproved inline-control pragma. The application owner remains
`NonMatching`: its exact count is unchanged at 231 of 258 functions and
`0x2C160 / 0x3F424` target text bytes.

## Retained recovery

The target prologue initializes the result, direction index, and direction
vector in that order. The retained declarations now reproduce that semantic
chronology:

```c
s16 next = 0;
s16 directionNo = 0;
HuVecF direction = { 0.0f, 0.0f, 0.0f };
```

The target register-usage graph also distinguishes the result assignment from
a later second read. Recovered MWCC `CodeGen.c` and `COptimizer.c` show that
arguments are considered before locals and that a local wins an equal usage
count. Folding the assignment into the comparison removes the extra `next`
reference and reproduces the target allocation when the target's direct
`fn_1_BB30` call boundary is present:

```c
if ((next = fn_1_BB30(*current, entries, directionNo, count)) != -1) {
```

This is also an established Hudson source idiom in Matching MP6 and MP5
owners. Under the retained build configuration, these two corrections improve
`fn_1_BBD8` from `90.278410%` to `91.514206%`; the remaining size difference
is solely the unresolved `fn_1_BB30` auto-inline boundary.

## Exact diagnostic and rejected controls

A bounded diagnostic placed `#pragma dont_inline on/reset` only around
`fn_1_BB30`. With the two retained source corrections, `fn_1_BBD8` became
byte-exact at target size `0x580`, with all 352 aligned instructions and the
direct `R_PPC_REL24 fn_1_BB30` relocation exact. The pragma was rejected
because it simultaneously changed independently exact `fn_1_BB30` from
`0xA8 / 100%` to `0xB4 / 92.238100%`. The target and source versions of that
callee both retain a direct `fn_1_B748` call; the regression is the missing
MWCC simple-optimizer pass and its dead saved-register temporary, not a
semantic helper difference.

Two other bounded probes were also rejected:

- defining `fn_1_BBD8` before `fn_1_BB30` preserved both per-function bodies,
  but emitted them in the opposite section order (`BBD8` then `BB30`) and
  therefore failed whole-object layout;
- making the local `order` table `const` under `dont_inline` enlarged
  `fn_1_BB30` to `0xC0` and reduced it to `84.5%`.

No pragma, reversed function order, `const` probe, fake local, register force,
manual inline body, or assembly remains in the source.

## Remaining easy-win audit

The Wave 92 audit covered all 27 non-exact functions, the MP4 and MP5 sibling
trees, 760 unreachable commits, and the recovered MWCC inliner, allocator, and
stack-frame sources.

- `fn_1_38314`, `fn_1_5324`, `fn_1_6324`, `fn_1_737C`, and `fn_1_2F22C`
  retain balanced register/stack lifetime differences rather than missing
  operations. Matching `mdseldll` authenticates the shared vector and angle
  source order used by the latter three.
- `fn_1_2B0` and `fn_1_400` differ at one zero-comparison node. Siblings use
  both `!value` and `value == 0`, and Matching MP6 `selmenuDll` proves the
  former does not produce the target node, so no cast or fake zero local was
  admitted.
- `fn_1_3EAC8` needs the exact `fn_1_34D10` and `fn_1_36264` bodies inlined.
  The binary authenticates the boundary but not a unique
  `inline_max_size(N)` value; no sibling or reachable/unreachable history
  authenticates one.
- The other residues have exact sizes, calls, constants, CFGs, and effective
  relocations and differ only in register or stack-slot coloring.

## Object verification

The retained relocation-aware report is:

`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave92_final_object.json`

It was generated with the pinned DTK object and objdiff configuration:

```powershell
rtk ninja -j1 build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk build/tools/objdiff-cli-windows-x86_64.exe diff -p . `
  -u mdpartydll/REL/mdpartydll/mdparty `
  -o build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave92_final_object.json `
  --format json-pretty -c functionRelocDiffs=data_value
```

The report proves 231 exact functions and `0x2C160` exact target bytes. The 27
remaining functions account for `0x132C4` target bytes. No configure flip was
made, so this WIP-only slice did not rerun or claim the full container gate.
