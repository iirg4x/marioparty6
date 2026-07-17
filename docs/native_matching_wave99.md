# Native matching Wave 99: `mdparty` ARAM null contract

Date: 2026-07-17

## Scope

Wave 99 closes the standalone character-motion preload helper and its inlined
copy in `REL/mdpartydll/mdparty.c`. Both functions were already semantically
complete. Their only target/source difference was the expression type used to
test an ARAM handle for zero.

The accepted change adds two exact functions and `0x39C` target text bytes:

| Function | Target text | Before | After |
| --- | ---: | ---: | ---: |
| `fn_1_2B0` | `0x150` | `98.095240%` | `100%` |
| `fn_1_400` | `0x24C` | `98.911570%` | `100%` |
| **total** | **`0x39C`** |  | **exact** |

The application owner now has 253 of 258 exact functions and
`0x3C1C8 / 0x3F424` exact target text bytes. Five functions and `0x325C` bytes
remain explicit compiler-shape residues, so the owner remains fallback-linked
and `C-not-yet-matched`.

## Recovered source shape

`CharMotionAMemPGet` returns an `AMEM_PTR`, the 32-bit ARAM-address handle used
by Hudson's character subsystem. The target does not compile its null test as
an unsigned-integer immediate comparison. It emits:

```asm
bl      CharMotionAMemPGet
li      r0, 0
cmplw   r3, r0
beq     ...
```

The former integer expression emitted `cmplwi r3, 0`, shortening both the
standalone body and the copy expanded into `fn_1_400` by four bytes. A
controlled GC/1.3.2 probe distinguishes the two language-level nodes:

- an integer-handle comparison against `0` emits `cmplwi`;
- a pointer-null comparison emits `li` plus `cmplw`;
- casting the ARAM handle to `void *` produces the exact target pair.

The retained source therefore expresses the null-address contract directly:

```c
if ((void *)CharMotionAMemPGet(GwPlayer[i].charNo) == NULL) {
    break;
}
```

This is a real type-domain correction, not a register force. Hudson sibling
source uses the same ARAM-address-to-`void *` cast family when an ARAM result is
consumed as an address. The cast changes no runtime value; it restores the
pointer comparison node proved independently in both MP6 target bodies.

Direct `NULL` was rejected because the active headers define it as
`((void *)0)` and MWCC correctly rejects comparing that pointer directly with
the integer `AMEM_PTR`. An explicit `0L` remains an integer constant and is
byte-neutral. No fake zero local, volatile qualifier, synthetic global,
register declaration, pragma, or assembly is retained.

## Compiler and tool evidence

The result follows the pinned compiler rather than a black-box permutation.
MWCC's expression kind controls immediate versus register-form comparison;
the controlled probe reproduces the target opcode sequence before the change
is tested in the full owner.

The supplied m2c tool was also run on target `fn_1_2F22C` as a lifetime
cross-check. It recovers the `0xE0` expanded stack extent and the same helper
flow, but it expands helper ABI mechanics into dozens of stack temporaries.
That output supplies no original local names or unique source boundary and is
not copied into the source. m2c remains useful for CFG and lifetime evidence,
not for authenticating Hudson spelling or overriding MWCC object proof.

The linked Ghidra, PowerPC, and inspector tooling has the same boundary:
function boundaries, relocations, SSA, instruction semantics, and compiler AST
behavior can constrain source shape, but a stripped retail REL cannot yield
internal source identifiers that are absent from the binary.

## Remaining residue audit

The other five functions were re-audited in parallel and left unchanged:

| Function | Target text | Current evidence |
| --- | ---: | --- |
| `fn_1_BBD8` | `0x580` | The target keeps exact `fn_1_BB30` out of line. Callee-scoped `dont_inline` can make the caller exact only by regressing the independently exact callee; caller-scoped control also regresses the body. No unique natural inline-control source is authenticated. |
| `fn_1_2EBEC` | `0x338` | Three operands select target `r4` instead of source `r3` for the same relocated `-200.0f` pool object. Calls, CFG, size, value, and relocations are exact; the missing `r3` interference is not attributable to a unique C node. |
| `fn_1_2F22C` | `0x1920` | Control flow, helper semantics, and relocation targets are represented; the residue is move, register, and stack coloring. Decompiler-expanded temporaries are not source-authentic evidence. |
| `fn_1_38314` | `0x728` | All 69 differences are register-color operands. Exact helpers, Matching `mdseldll`, and MP5 authenticate the current flow and declaration order; no alternative natural node is proven. |
| `fn_1_3EAC8` | `0x95C` | The first 576 instructions are exact. The tail reduces to one uncoalesced target move plus four downstream compare-color differences; inventing an inline-return wrapper or ghost temporary would be fakematching. |

The rejected `fn_1_BBD8` caller-scoped `dont_inline` diagnostic preserved exact
`fn_1_BB30` but reduced the caller to `83.383520%`; it is absent from source.
No residue was changed merely because a tool could synthesize a matching
temporary.

## Object proof

The final relocation-aware report is:

`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave99_integrated.json`

It is reproduced with:

```powershell
rtk ninja -j1 build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk ../../tools/objdiff/v3.7.2/objdiff-cli.exe diff -p . `
  -u mdpartydll/REL/mdpartydll/mdparty `
  -o build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave99_integrated.json `
  --format json-pretty -c functionRelocDiffs=data_value
```

The report proves 258/258 represented functions, 253 exact functions, and
`0x3C1C8` exact target text bytes. Both recovered bodies have exact sizes,
instructions, control flow, and relocations. No previously exact function is
lost, and the Wave 98 initialized-data image remains unchanged.

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
