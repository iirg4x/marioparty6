# Native matching Wave 100: `mdparty` navigation helper boundary

Date: 2026-07-17

## Scope

Wave 100 closes `fn_1_BBD8`, the character-selection navigation update in
`REL/mdpartydll/mdparty.c`. Wave 92 had already recovered its natural vector,
sector, and assignment-condition source. The remaining difference was not a
missing calculation or temporary: GC/1.3.2 automatically expanded the exact
`fn_1_BB30` neighbor-search helper into its caller, while the target retains a
direct call.

The accepted change adds one exact function and `0x580` target text bytes:

| Function | Target text | Before | After |
| --- | ---: | ---: | ---: |
| `fn_1_BB30` | `0xA8` | `100%` | `100%` |
| `fn_1_BBD8` | `0x580` | `91.514206%` | `100%` |

The application owner now has 254 of 258 exact functions and
`0x3C748 / 0x3F424` exact target text bytes. Four functions and `0x2CDC` bytes
remain compiler-shape residues, so the owner remains fallback-linked and
`C-not-yet-matched`.

## Recovered compiler boundary

The retained source scopes CodeWarrior's automatic-inline selection around
the helper definition:

```c
#pragma auto_inline off
s16 fn_1_BB30(...)
{
    ...
}
#pragma auto_inline reset
```

This is narrower than `dont_inline`. It prevents the small helper from being
marked as an automatic inline candidate while preserving normal inline
processing inside the helper and in the following caller. The resulting
object retains both target relocations:

- `fn_1_BB30 + 0x90`: `R_PPC_REL24` to `fn_1_B748`;
- `fn_1_BBD8 + 0x4CC`: `R_PPC_REL24` to `fn_1_BB30`.

The recovered compiler source explains the distinction directly.
`compiler_and_linker/FrontEnd/C/CInline.c:CInline_GenFunc` sets the automatic
inline flag only when `copts.auto_inline` is enabled, `dontinline` is disabled,
and the body passes the eligibility and size tests. `CPrep.c` exposes
`auto_inline` as the corresponding pragma option. This is the exact decision
that separated the prior source object from the target.

## Authenticity boundary

This is target/compiler-authenticated inline control, not a sibling-parity
claim. CodeWarrior's Power Architecture documentation defines the same
automatic-selection behavior, and genuine shipped MWCC source uses scoped
`auto_inline off/reset` around individual functions. One preserved example is
[Unreal Engine 2's `UnName.cpp`](https://github.com/jmarshall23/Unreal2004/blob/07249c526830e8ffa4e6a3b133c92896f44a686a/Core/Src/UnName.cpp#L24-L52).

A recursive search of the local MP4, MP5, MP7, Nintendo SDK, and Pikmin source
families found no Hudson/Nintendo instance of this spelling. MP5's
`MdPartyChrSelCursorUpdate` does authenticate the broader table-driven design:
it tracks occupied character slots and consults ordered `fixPos` candidate
tables. Its table dimensions and navigation procedure differ from MP6, so it
does not authenticate MP6's local identifier, exact table, or pragma.

The guard therefore remains recorded as source-quality debt under the
repository's provenance standard. It earns exact object credit because its
compiler effect and both surrounding function bodies are proved, but it is not
described as recovered Hudson spelling.

## Excluded alternatives

Every diagnostic was compiled in isolation with the pinned compiler and
compared with relocation values enabled:

| Candidate | Result |
| --- | --- |
| No pragma | `fn_1_BB30` stays exact; `fn_1_BBD8` becomes `0x5F4`, `91.514206%`, with the helper expanded. |
| K&R helper definition | The helper is still expanded; the caller becomes `0x608`, `89.423294%`. |
| `dont_inline` around the caller | It also suppresses the authentic `sqrtf` expansion; the caller becomes `0x4A0`, `83.383520%`. |
| `dont_inline` around the helper | The caller becomes exact, but the independently exact helper regresses to `0xB4`, `92.238100%`. |
| Move the helper after the caller | Both bodies compile exactly, but their emitted symbol order and addresses reverse. |
| Flat table initializer | Byte-neutral for the helper and does not prevent caller expansion. |
| `__declspec(noinline)` / `__attribute__((noinline))` | Both are rejected by the GC/1.3.2 C frontend; recovered parser source confirms no declaration-level noinline form. |
| `auto_inline off/on` or pragma push/pop | Whole-object byte-identical to the retained `off/reset` form. |

No inline-size sweep, fake statement, unused local, volatile/register force,
assembly, or source-order violation is retained.

## Object proof

The final relocation-aware report is:

`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave100_integrated.json`

It is reproduced with:

```powershell
rtk ninja -j1 build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk build/tools/objdiff-cli-windows-x86_64.exe diff `
  -1 build/GP6E01/mdpartydll/obj/REL/mdpartydll/mdparty.o `
  -2 build/GP6E01/src/REL/mdpartydll/mdparty.o `
  -o build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave100_integrated.json `
  --format json-pretty -c functionRelocDiffs=data_value
```

The report proves 258 represented functions, 254 exact functions, and
`0x3C748` exact target text bytes. Its only mismatches are `fn_1_2EBEC`,
`fn_1_2F22C`, `fn_1_38314`, and `fn_1_3EAC8`; no previously exact function is
lost.

## Full gate

The final serialized project gate uses:

```powershell
rtk ninja -j1
rtk build/tools/dtk.exe shasum -q -c config/GP6E01/build.sha1
rtk cmp orig/GP6E01/sys/main.dol build/GP6E01/main.dol
rtk cmp orig/GP6E01/files/dll/mdpartydll.rel `
  build/GP6E01/mdpartydll/mdpartydll.rel
rtk git diff --exit-code HEAD -- config/GP6E01/symbols.txt
```

The Ninja checksum and explicit DTK checksum each report `137 files OK`.
Both `cmp` commands exit zero, so `main.dol` and `mdpartydll.rel` remain
byte-identical to the originals. The final build leaves
`config/GP6E01/symbols.txt` unchanged.
