# Native matching Wave 97: typed `mdparty` contracts

Date: 2026-07-17

## Scope

This wave continues the application owner in
`REL/mdpartydll/mdparty.c`. It retains three source-shape corrections and
closes three functions totaling `0x182C` target text bytes. No configure entry
is flipped: the owner remains fallback-linked and `C-not-yet-matched`.

The Wave 96 baseline was 248/258 exact functions and `0x3A600 / 0x3F424`
exact target text bytes. The final Wave 97 object has 251/258 exact functions
and `0x3BE2C / 0x3F424` exact target text bytes. Seven represented functions
and `0x35F8` bytes remain.

## Recovered source contracts

### Read-only Bezier paths

`fn_1_22A8` only reads its three path vectors and passes them to the already
exact `fn_1_1404`, whose inputs are `const HuVecF *`. Restoring that same
read-only contract on the `fn_1_22A8` definition and late-inline declaration
reproduces the target's nested-helper allocation.

Before the correction, target and source were both `0x454` bytes and differed
only in a 17-row saved-register rotation: target retained `modelId` in `r28`
and the three vector pointers in `r29`-`r31`, while source rotated those four
values. With the recovered qualifiers, `fn_1_22A8` is byte-exact. Its exact
late-inline callers `fn_1_5324`, `fn_1_6324`, and `fn_1_737C` remain exact.
The retail REL contains no type metadata, so this is retained as a semantic,
byte-proven source shape corroborated by the read-only uses and exact callee;
it is not presented as a uniquely DWARF-authenticated spelling.

### Signed character arguments

`fn_1_156B4` and `fn_1_160A0` each call the exact late-inline helper
`fn_1_CA68`, whose first parameter is `s16 modelNo`. In each caller the Wave
96 residue was the same three-instruction conversion ownership mismatch:

```text
target: lha volatile -> extsh saved -> mulli saved
source: lha saved    -> extsh volatile -> mulli volatile
```

Exact same-translation-unit `fn_1_CA68` callers `fn_1_1B9E8` and
`fn_1_1F8B8` authenticate an explicit `(s16)entry->chrSel` argument at this
boundary. Restoring that typed argument expression in the two remaining
callers makes both `0x9EC` functions byte-exact. The cast records the call ABI;
it is not a register-forcing local.

## Rejected residue probes

- Reversing the `fn_1_2B0` zero comparison and spelling the zero as `0L` did
  not change its `li`/`cmplw` versus `cmplwi` residue. `NULL` is a `void *` in
  the active SDK header and is not a valid comparison with the `u32`
  `AMEM_PTR` return type. No spelling-only change is retained.
- Spelling `fn_1_2EBEC`'s threshold as `-200.0f` did not change its three-row
  constant-address temporary residue. No synthetic literal global was added.
- Folding the inner `fn_1_2F22C` retry assignment into its comparison was
  byte-neutral. The separated, readable statements remain.
- `fn_1_BBD8` still requires the compiler to keep exact `fn_1_BB30` out of
  line. The earlier diagnostic pragma closes one function only by regressing
  the other, so no inline-control pragma is admitted.

The retained source contains no inline assembly, fake local, register force,
synthetic data packet, or unproved compiler-control pragma.

## Name and tool evidence

`dtk rel info` finds only `_prolog`, `_epilog`, `_ctors`, and `_dtors` in the
retail REL. The richer target-object names come from DTK `.note.split`
metadata and are not original retail symbols. Consequently m2c, Ghidra,
FunctionID, or a sibling can suggest semantics but cannot authenticate an
internal MP6 name without a symbol-bearing artifact. No address name is
replaced in this wave.

The local MWCC Inspector build supports the exact GC/1.3.2 compiler and is a
useful frontend/inline diagnostic. `mwcc-debugger` targets GC/2.6 as an
approximation, while `powerpc-rs` is a disassembly/assembly library; neither
is name authority. The project already has the exact compiler archive, so the
external compiler bundle does not improve this owner's provenance.

## Remaining application work

| Function | Target bytes | Object evidence |
| --- | ---: | --- |
| `fn_1_2B0` | `0x150` | 98.095240%; zero-compare instruction form |
| `fn_1_400` | `0x24C` | 98.911570%; late-inline clone of the same residue |
| `fn_1_BBD8` | `0x580` | 91.514206%; target keeps exact `fn_1_BB30` out of line |
| `fn_1_2EBEC` | `0x338` | 99.902916%; constant-address temporary register |
| `fn_1_2F22C` | `0x1920` | 96.641790%; nested-inline allocator moves |
| `fn_1_38314` | `0x728` | 98.930130%; nested-inline volatile-register coloring |
| `fn_1_3EAC8` | `0x95C` | 99.791320%; tail return-value lifetime plus size residue |

Text completion is not owner completion. The target object's `.data` is
`0xF10` bytes while the current source object emits only `0x1D9`; initialized
global/table ownership remains a separate real recovery task. Target and
source `.rodata` are both `0x358`, and `.bss` is `0xA80` on both sides.

## Verification

The final relocation-aware report is:

`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave97_final_object.json`

It is generated with:

```powershell
rtk ninja -j1 build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk build/tools/objdiff-cli-windows-x86_64.exe diff -p . `
  -u mdpartydll/REL/mdpartydll/mdparty `
  -o build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave97_final_object.json `
  --format json -c functionRelocDiffs=data_value
```

The exact-function set gains only `fn_1_22A8`, `fn_1_156B4`, and
`fn_1_160A0`; no Wave 96 exact function is lost. The final serialized Ninja
build and explicit DTK checksum each report `137 files OK`. Separate `cmp`
gates prove `main.dol` and `mdpartydll.rel` byte-identical, and the final build
leaves `config/GP6E01/symbols.txt` unchanged.
