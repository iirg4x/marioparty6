# Native matching Wave 101: `mdparty` call contracts and paired state

Date: 2026-07-17

## Scope

Wave 101 closes the final four `REL/mdpartydll/mdparty.c` functions and
recovers a real two-entry message owner in the party-mode state block. The
accepted code adds all former `0x2CDC` residual target text bytes:

| Function | Target text | Before | After |
| --- | ---: | ---: | ---: |
| `fn_1_2EBEC` | `0x338` | `99.902916%` | `100%` |
| `fn_1_2F22C` | `0x1920` | `96.641790%` | `100%` |
| `fn_1_38314` | `0x728` | `98.930130%` | `100%` |
| `fn_1_3EAC8` | `0x95C` | `99.791320%` | `100%` |
| **total** | **`0x2CDC`** |  | **exact** |

The application owner now has 258 of 258 exact functions and
`0x3F424 / 0x3F424` exact target text bytes. Its semantic initialized data was
already exact. This wave also restores target function emission order and the
real MWCC `.bss` definition chronology, making the complete source-linked REL
exact before the owner is configured `Matching`.

## `fn_1_2EBEC`: recovered return contract

The helper was reconstructed as `void`, but the target's last constant-pool
load remains live in `r4` because a non-void return carrier occupies `r3`.
Changing the declaration, definition, and post-definition inline redeclaration
to `s16` removes the only three operand differences:

```c
s16 fn_1_2EBEC(void)
{
    ...
}

inline s16 fn_1_2EBEC(void);
```

The missing source `return` is intentional target behavior, not an omission
added by this wave. An explicit `return;` under the former void contract is
byte-neutral and remains divergent. Both `s16` and `s32` generate the required
GPR carrier; the retained `s16` is authenticated by the adjacent party-flow
family and MP5's `MdMessWinChoiceNowGet`, which preserves the same Hudson
signed-short function-without-a-final-return source class. Its exact inline
copy in `fn_1_31508` remains exact, and the contract change does not perturb
any other residue.

## `fn_1_2F22C`: translation-unit call visibility

The real implementation and public header declare:

```c
void HuAudSStreamFadeOut(int streamNo, s32 speed);
```

That remains true. The recovered defect was a different question: whether
this translation unit saw that declaration. `mdparty.c` does not include
`game/audio.h`, and an unproved TU-local void declaration had been added during
reconstruction. Its removal restores the original C89 call visibility without
falsifying the callee ABI.

Recovered MWCC frontend source at
`compiler_and_linker/FrontEnd/C/CExpr.c:1243-1265` proves that a call with no
visible declaration is materialized as an old-style external function
returning signed `int`. That implicit result creates the target's otherwise
missing call-result carrier after `HuAudSStreamFadeOut`. The carrier changes
the interference and coalescing decisions across the expanded party-flow
function, reconciling all four source-only moves, all six target-only moves,
and all 712 register-operand differences. Target and source become the same
`0x1920` bytes with identical relocations.

Repository history independently authenticates the visibility state.
`git show bb14807^:src/REL/mdpartydll/mdparty.c` already calls
`HuAudSStreamFadeOut` with the same include family and no declaration.
`bb14807` later introduced the local void declaration while transcribing more
flow. Matching `mdseldll` carries its own explicit local void declaration,
confirming that visibility was translation-unit-local rather than a global
callee-type correction.

No `int HuAudSStreamFadeOut` declaration is added. The source simply stops
asserting a prototype that this original translation unit did not see.

## `fn_1_38314` and `fn_1_3EAC8`: recovered flow return

`fn_1_38314` was reconstructed as `void`, but it belongs to the same Hudson
signed-short flow-helper class as `fn_1_2EBEC`. Restoring its declaration and
definition to `s16`—while preserving the target's absent final return—solves
both remaining functions:

```c
s16 fn_1_38314(void)
{
    ...
}
```

Inside `fn_1_38314`, the non-void carrier provides the missing interference
that shifts one reused anonymous temporary web. All 69 former register-operand
differences across the exact `fn_1_4258`, `fn_1_9804`, and `fn_1_D6B0` inline
expansions disappear; the target and source are the same `0x728` bytes.

The sole caller is state 103 of `fn_1_3EAC8`, immediately before the final
state-104 call. Its ignored `s16` result is eliminated before code emission,
but it still consumes one frontend GPR temporary. This is exactly the source
event the backend oracle required. The current compiler had allocated through
`v256`, wrapped to `v32`, and coalesced the final call result directly from
`r3` to saved `r31`. The restored ignored result advances the wrap by one:

- the state-104 call result becomes `v33`, colored `r4`;
- the assignment emits target `mr r4,r3; mr r31,r4`;
- the following comparison temporaries become `v34`, `v35`, and `v36`, with
  target colors `r4`, `r3`, and `r0`.

Those are all six former aligned differences. `fn_1_3EAC8` now matches at its
target `0x95C`, including the extra target move. The result comes from the
recovered helper contract, not from an invented temporary or allocator force.

## Exact compiler allocator evidence

The compiler-source audit was converted into an executable GC/1.3.2 backend
oracle instead of a source permutation sweep. It hooks the exact compiler's
`colorinstructions` and `colorgraph` boundaries and records pre-allocation
PCode, the full GPR interference graph and priority order, and post-allocation
PCode for a selected function.

For `fn_1_2F22C`, the baseline capture contains:

- 202 PCode basic blocks;
- 1,617 pre-allocation PCode instructions;
- 230 material GPR graph nodes and their adjacency, cost, flags, and colors;
- 1,595 post-allocation PCode instructions.

The capture identifies the call-result lifetime directly. Removing the
reconstructed void declaration restores that lifetime, after which the normal
GC/1.3.2 allocator emits the complete target object. No compiler patch,
register constraint, inline assembly, volatile local, or randomized search is
used.

The same oracle captures `fn_1_38314`'s reused temporary web and
`fn_1_3EAC8`'s exact `v256` wrap. `Registers.c::check_temp_registers` in the
recovered compiler source confirms the wrap to the first temporary register.
The one-event shift predicted by that implementation is exactly the shift
produced by `fn_1_38314`'s restored ignored result.

## Recovered paired-message owner

`LBL_1_BSS_9F4` previously described offsets `0x10` and `0x14` as unrelated
`s32` fields, then indexed across them with pointer arithmetic. Target and
consumer evidence proves one array:

```c
typedef struct Lbl1Bss9F4 {
    ...
    s32 unk_10[2];
    s16 unk_18;
    s16 unk_1A;
} LBL_1_BSS_9F4;
```

The retail symbol fixes the complete object at `0x1C` bytes. Exact code uses
word offsets `0x10` and `0x14`, followed by halfwords at `0x18` and `0x1A`, so
the array is exactly two `s32` elements. Exact `fn_1_3AD84` scales an index by
four and loads from offset `0x10`; its exact producers constrain that index to
zero or one. Every fixed producer and consumer initializes, passes, or displays
the same pair.

All direct uses now spell `[0]` and `[1]`, and the dynamic users spell
`unk_10[unk_C]`. The layout, code, data, and relocations are byte-neutral in
all consumers. The address-based field name is retained because no symbol
evidence supports inventing a semantic retail name.

## Source order and `.bss` relocation closure

Per-function equality did not by itself prove the source-linked container.
The first REL link exposed two independent source-layout defects that objdiff's
name-based function pairing correctly does not treat as instruction defects.

`fn_1_26328` and `fn_1_28044` were individually exact but defined in the wrong
order. The target symbol table places the `0x1D1C` `fn_1_26328` at `0x26328`
and the `0x3F0` `fn_1_28044` at `0x28044`. The recovered source had emitted
`fn_1_28044` at `0x26328` and `fn_1_26328` at `0x26718`. Moving the complete
`fn_1_28044` definition after `fn_1_26328` restores the target text chronology
without changing either function body.

After that correction, every file-backed REL section, header field, import,
and module-0 relocation was exact. Only 5,158 of the 17,155 module-1
self-relocation records differed. All 5,158 had the same relocation site,
type, and target section as retail; only their `.bss` addends differed. This
isolated the final defect to zero-initialized owner order rather than code,
initialized data, external symbols, or relocation ordering.

The typed `extern` declarations must remain visible before the functions.
Moving the definitions above the code changes compiler visibility and regresses
the generated owner to `.text = 0x3BFB4` and `.bss = 0xA84`, so that shape is
rejected. With the real definitions retained after the code, GC/1.3.2 emits
the definition block in reverse. The former ascending tail therefore put
`lbl_1_bss_A6C` at `0x0` and `lbl_1_bss_0` at `0xA7C`. Ordering the same typed
owners from `lbl_1_bss_A6C` down through `lbl_1_bss_0` makes MWCC emit the
retail ascending layout, including its natural alignment gaps, and preserves
the exact `0xA80` section size.

The rebuilt module-1 stream, all `0x70A44` REL bytes, and retail SHA-1
`519debb149ef42eda1ab3b0a4d2b3132b4f3e3cc` then match. No padding owner,
relocation edit, linker-script exception, or binary packet is used.

## `mapdas` applicability

The splitting primer's [`mapdas`](https://github.com/intns/mapdas)
recommendation is conditional on possessing an authentic CodeWarrior linker
map. `mapdas` imports symbols, object/source paths,
demangled argument types, and file boundaries from such a map; it does not
infer those names from a retail REL. The current reference workspace has no
MP6 or authenticated sibling developer map (the only relevant `.map` search
hit is an unrelated `partyboard/rel.map`), and `mapdas` has no REL reader.
Consequently it supplies no new `mdparty` name or byte proof in this wave.

If an authentic map is later obtained, its paths and symbol ranges should be
checked against `config/dll/rels/mdpartydll/splits.txt` and `symbols.txt`
before renaming. Until then, address-based owners remain preferable to invented
retail names.

## Rejected alternatives

- Adding the authoritative `void SLBoardLoad(void)` prototype changes no
  semantics but regresses `fn_1_2F22C` to `96.162930%`; the target likewise
  preserves that call's original implicit/stale visibility.
- Rewriting `fn_1_3EAC8` as a natural `do/while` loop is byte-neutral and does
  not change its six aligned differences, so the rewrite is absent.
- Moving `.bss` definitions before the code changes MWCC visibility, shrinks
  `.text` to `0x3BFB4`, and grows `.bss` to `0xA84`; the target instead proves
  extern-before-use declarations with the definitions retained at the tail.
- The paired-message array is code-neutral; it is retained only because
  independent layout and indexed-access evidence proves the owner.
- No late-inline sweep, fake return value, unused temporary, register force,
  volatile qualifier, inline assembly, or callee ABI lie is retained.

## Complete application owner

All 258 application functions, all `0x3F424` application text bytes, semantic
initialized data, target function order, `.bss` owner offsets, and effective
relocations are exact. The already Matching stage owner contributes 57
functions and `0x73D8` text bytes. The separate 19-function compiler runtime
remains source-linked assembly under the authenticated original-assembly
exception and contributes no clean-C credit. With the application owner
flipped to `Matching`, the complete `mdpartydll.rel` is source-linked and
byte-identical.

## Object proof

The final relocation-aware report is:

`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave101_integrated.json`

It is reproduced with:

```powershell
rtk ninja -j1 build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk build/tools/objdiff-cli-windows-x86_64.exe diff `
  -1 build/GP6E01/mdpartydll/obj/REL/mdpartydll/mdparty.o `
  -2 build/GP6E01/src/REL/mdpartydll/mdparty.o `
  -o build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave101_integrated.json `
  --format json-pretty -c functionRelocDiffs=data_value
```

The report proves 258 represented functions, 258 exact functions, and all
`0x3F424` target text bytes. No previously exact function is lost.

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
