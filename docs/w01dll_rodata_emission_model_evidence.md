# MWCC `.rodata` emission model at `-O0`, and what it says about w01Dll

Date: 2026-07-28. Compiler under test: `Compilers/GC/2.7/mwcceppc.exe`
(MWCC 2.4.7, 2,068,992 bytes). Flags: the exact `cflags_rel` profile from
`configure.py` — `-O0,p -char unsigned -fp_contract off -sdata 0 -sdata2 0`
over `cflags_base` (`-proc gekko -align powerpc -enum int -fp hardware
-Cpp_exceptions off -inline auto -str reuse -RTTI off`). Source commit under
test: `a34972549072d7e0cd8a0fe01e3d7eecb02cc477`.

All probes were compiled outside the build tree into a scratch directory; the
repository working tree was never used as a probe surface.

## Why this was measured

`src/REL/w01Dll/world01.c` reaches 132/132 functions with `.text`, `.data` and
`.bss` byte-identical to the target, while `.rodata` is ordered differently and
4 bytes longer, so the object cannot be flipped to `Matching`. The campaign
write-up (branch `codex/w01dll-match`, commit
`7d0876c98b8ab91dd1a215e241ef2625fc9d7883`, `docs/w01dll_rodata_open_problem.md`)
characterised the gap as a single contiguous 104-byte block of 19 hot constants
that the target emits before any function is code-generated.

That branch is **local-only — it is not pushed to `origin`** — so it cannot serve
as a durable evidence link. The reusable compiler behaviour is distilled here and
the load-bearing claims were re-measured rather than inherited.

## Correction: `.rodata` is ONE region in strict creation order

Both the campaign write-up and the workspace field notes state that *"named
`.rodata` objects always precede the literal pool"*. **That is false**, and the
two-region `[named objects][literal pool]` model built on it is wrong.

Probe `p1.c` creates a pool literal *first*, a named const object *second*, and
another pool literal *third*:

```c
float useLit(float x)  { return x * 7.5f; }
const float namedTbl[3] = { 1.25f, 2.5f, 7.5f };
float useLit2(float x) { return x * 3.25f; }
```

Emitted `.rodata` (20 bytes):

```text
0x00  40f00000  7.5f        <- pool literal, created FIRST
0x04  namedTbl[0..2]        <- named object, created SECOND
0x10  40500000  3.25f       <- pool literal, created THIRD
```

The named object sits **between** two pool literals. Section order is simply
**creation order**, with no separation of named objects from pooled literals.

`p4.c` confirms the same model with all four construct classes in one TU:

```text
0x00  55.5f      pk$localstatic3$ei__Ff   WEAK   extern-inline local static (parse)
0x04  7.5f       @8                              pool literal (useLit codegen)
0x08  namedTbl                                   named object (definition point)
0x10  3.25f      @13                             pool literal (useLit2 codegen)
0x14  11.5f      @18                             pool literal (helper FIRST EXPANSION)
```

Predicted by strict creation order; contradicted by the two-region model, which
would place `pk` and `namedTbl` together at the front.

`p1.c` also shows **no deduplication**: `7.5f` appears twice, once as the pool
literal at `0x00` and once inside `namedTbl` at `0x0c`.

## When each construct creates its constants

Measured on this compiler and profile:

| construct | constants created | probe |
| --- | --- | --- |
| named file-scope `const` object, referenced | at its definition point | `p1`, `p4` |
| ordinary expression literal | at the codegen of the referencing function | `p1`, `p2`, `p4` |
| `static inline` body, called | at the **first expansion** site | `p2`, `p4` |
| `static inline` body, never called | nothing emitted | `p2b` |
| `static inline` with function-scope `static const`, never called | nothing emitted | `p2b` |
| `extern inline` + function-scope `static const` under `#pragma cplusplus on`, never called | at **parse** of the inline body | `p3`, `p4` |
| unreferenced file-scope `static const` | nothing emitted | campaign, not re-measured |

`p2.c` isolates the expansion-timing rule: `helper` is *defined first* but
*expanded second*, and its `11.5f` lands after `first`'s `22.5f`.

```text
p2.o .rodata:  0x00 41b40000 22.5f   (first, defined second)
               0x04 41380000 11.5f   (helper, defined FIRST, expanded second)
```

`p3.c` is the one construct that emits `.rodata` ahead of all function codegen at
**zero `.text` cost**:

```c
#pragma cplusplus on
extern inline float ei(float x) { static const float pk = 55.5f; return x * pk; }
#pragma cplusplus reset
float useA(float x) { return x * 66.5f; }
```

```text
p3.o  .text 20 bytes (useA alone; ei contributes none)
      .rodata 0x00 = 55.5f  pk$localstatic3$ei__Ff  WEAK OBJECT
              0x04 = 66.5f  @5
```

`ei` is never called, yet `pk` is emitted, weak, at offset 0. Compare `p2b.c`:
the same idea with `static inline` instead of `extern inline` emits **nothing**.
The distinction between the two is real and is the mechanism already recorded in
the card `mwcc-extern-inline-static-local-comdat-ownership`.

## `.text` byte-identity places zero constraint on `.rodata` order

Every `.text` -> `.rodata` reference is a relocation pair against a symbol; the
instruction immediates carry no offset. From `p4.o`'s `.rela.text`:

```text
off=0x0002 type=6 (ADDR16_HA) sym=@8  addend=0  instr=0x3c600000  imm=0x0000
off=0x0006 type=4 (ADDR16_LO) sym=@8  addend=0  instr=0x38630000  imm=0x0000
off=0x0016 type=6              sym=@13 addend=0  instr=0x3c600000  imm=0x0000
off=0x001a type=4              sym=@13 addend=0  instr=0x38630000  imm=0x0000
off=0x002a type=6              sym=@18 addend=0  instr=0x3c600000  imm=0x0000
off=0x002e type=4              sym=@18 addend=0  instr=0x38630000  imm=0x0000
```

All six immediate fields are `0x0000`. Permuting `.rodata` changes only each
symbol's section offset, which lives in the relocation, so `.text` bytes are
untouched. This is why w01Dll can hold `.text` byte-identical across 2,948
`.text` -> `.rodata` relocations at the *same* text offsets while `.rodata` is
reordered: the campaign measured every one of those relocations resolving to a
byte-equal value, making the difference a pure permutation.

The diagnostic consequence: a 100%-text match with reordered `.rodata` is
**never** a codegen problem. It is a source-structure problem — declaration
order, inline placement, or a construct whose constants are created at a
different time.

## The `.rodata` match criterion is prefix-identity, not percentage

Campaign measurement across all 306 objects marked `Matching` in `configure.py`:
54 have a `.rodata` section and **12 score below 100%** while shipping in a
passing build, as low as 85.7143% (`main/game/sprman`, `main/game/hsfdraw`,
`main/dolphin/mtx/quat`), including two RELs.

```text
selmenuDll  ours=132  target=136  prefix(132) IDENTICAL  tail=00000000
mdseldll    ours=606  target=608  prefix(606) IDENTICAL  tail=0000
sprman      ours=12   target=16   prefix(12)  IDENTICAL  tail=00000000
```

In every one the **target is larger** and the extra bytes are trailing zeros from
dtk's split rounding. So the criterion is prefix-identical content plus the
*direction* of the size difference. w01Dll is the only object where ours is
larger (1,092 against 1,088) *and* reordered, confirmed by linking: flipping it
to `Matching` produces a 141,252-byte `.rel` against the expected 141,244.

This part is quoted from the campaign's whole-project measurement and was not
re-run here; it requires a full build, which these probes deliberately avoid.

## Source forms with zero codegen effect at `-O0`

`p5a.c` versus `p5b.c` differ only by redundant parentheses, `register` on a
parameter and two locals, `const` on all three parameters, and `u += t` in place
of `u = u + t`. Result: `.text` 64 bytes on both sides, **byte-identical**.

`p7a.c` versus `p7c.c` differ only by redundant parentheses: byte-identical.

Genuine regrouping is not neutral. `p7a` `(a*b)*(c*d)` against `p7b`
`((a*b)*c)*d` — same operands, same order, same 16-byte size, different
destination registers:

```text
p7a:  fmuls f5,f1,f2 | fmuls f0,f3,f4 | fmuls f1,f5,f0
p7b:  fmuls f0,f1,f2 | fmuls f0,f3,f0 | fmuls f1,f4,f0
```

The balanced tree needs a second live temporary (`f5`); the left-leaning tree
reuses `f0`. Float temporaries are numbered by **expression-tree shape**, not by
liveness and not by the `register` keyword — and parentheses that do not change
the tree do not change the numbering.

## Inline expansion and frame allocation

`p6a.c` and `p6b.c` are identical except that the two locals inside the
`static inline` helper are declared in the opposite order. `.text` is 152 bytes
on both sides but **not identical** — four instructions differ, all of them
allocation fields:

```text
[0x044] p6a fmr-class f29  <->  p6b f30
[0x048] p6a fadds f30,f29,f31   p6b fadds f29,f30,f31
[0x04c] p6a fmuls ...,f30       p6b fmuls ...,f29
[0x050] p6a fmuls f27,f30,f29   p6b fmuls f27,f29,f30
```

Declaration order **inside an inline helper is load-bearing**: two helpers that
differ only in declaration order are genuinely different functions and cannot be
merged. In this small probe the inlinee's locals were enregistered rather than
stack-homed, so the effect appears in register numbers; the campaign observed the
same lever as stack-slot numbers on `fn_1_13CC`.

Passing a caller value **by pointer** is the zero-extra-cost form. `p6a`
(by pointer) is 152 bytes; `p6c`, doing the same work with the value passed by
value plus an out-pointer, is 172 bytes — 5 extra instructions at expansion
entry.

Not re-measured here, and carried from the campaign: an inlinee's locals are
allocated **ascending from the bottom of the frame**, below everything the caller
allocates, which is why a target layout with a group of locals underneath the
compiler temporaries indicates an inline helper rather than a nested block.
Plain nested blocks continue the enclosing block's descending run and were
measured to make `fn_1_13CC` worse (46 -> 49..81 differing slots).

## Eliminated for the w01Dll `.rodata` gap (campaign measurement)

- **Compiler build.** All eleven MWCC builds present (1.3.2, 2.0, 2.5, 2.6, 2.7,
  3.0a3, 3.0a3.2/3/4, 3.0a5, 3.0a5.2) produce `.rodata` 1,092 with the same
  ordering; 1.3.2, 2.0, 2.5, 2.6 and 2.7 also give byte-identical `.text`. The
  ordering is not a compiler-version property.
- **Inline flags.** `-inline on`, `-inline off`, `-inline all` leave the pool at
  1,092. `-inline deferred` and `-inline auto,deferred` do reorder it (to 1,108),
  confirming inline codegen timing drives pool creation order, but both wreck
  `.text` (85,088 and 104,196 against 91,304).
- **String pooling.** `-str reuse,pool` and `-str pool,readonly`: no change.
- **Zero-cost source routes.** Statements after `return`, `switch(1){case 7:}`,
  `if(0){}`, `goto` over a block, an uncalled `static inline`, and an
  unreferenced file-scope `static const` all emit nothing. A dead store costs 72
  bytes of `.text`; an uncalled `static` function is not dead-stripped.
- **Folding.** `x * 1.0f` becomes a copy and `x / 2.0f` strength-reduces to
  `* 0.5f`, so reciprocal tricks cannot seed the pool. Allocation order within
  one expression is not left-to-right: `a*0.5f + 2.0f` allocates `2.0f` first.

## What remains open

No zero-`.text`-cost source arrangement has been found that creates w01Dll's 19
hot constants before the first function is code-generated. The best construction
emits them from one dead `static` function before `_prolog`: `.rodata`
byte-identical at 1,088, but +308 bytes of `.text`.

The `extern inline` + `#pragma cplusplus on` route measured above is the only
construct that *does* prefix `.rodata` at zero `.text` cost, which makes it a
diagnostic lever — not an authenticated source shape. The shipped w01Dll solution
took a different route entirely: `#pragma section code_type ".text.common"` with
file-qualified ldscript selectors (`configure.py`
`rel_ldscript_replacements["w01Dll"]`, exception
`w01-world01-exact-source-shape`).

Also unconfirmed: whether dtk reconstructs `.rodata` faithfully per translation
unit at all. dtk splits the linked REL by address range, so a translation unit
contributing `.rodata` but no `.text` would be invisible to the splitter and its
constants attributed to the neighbouring object. That single mechanism would
explain why `world01.o` is the only one of 21 REL target objects whose first
`text -> rodata` reference (`0x78`) is not its lowest referenced offset (`0x10`).

## Reproducing

```sh
# probes are compiled outside the build tree; nothing here touches the worktree
mwcceppc.exe -nodefaults -proc gekko -align powerpc -enum int -fp hardware \
  -Cpp_exceptions off -O0,p -inline auto -maxerrors 1 -nosyspath -RTTI off \
  -char unsigned -fp_contract off -str reuse -multibyte -sdata 0 -sdata2 0 \
  -c -o probe.o probe.c
```

Then read the ELF `.rodata`, `.symtab` and `.rela.text` of the scratch object.
For real objects, prefer the project's own tooling — `dtk rel info <file>.rel`
for REL section layout, and objdiff's JSON `data_diff` (with
`-c ppc_calculate_pool_relocations=true -c function_reloc_diffs=none`) for which
`.rodata` bytes differ.
