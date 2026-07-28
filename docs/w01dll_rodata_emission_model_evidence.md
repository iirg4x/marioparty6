# MWCC `.rodata` emission model at `-O0`, and what it says about w01Dll

Date: 2026-07-28. Compiler under test: `Compilers/GC/2.7/mwcceppc.exe`
(MWCC 2.4.7, 2,068,992 bytes). Flags: the exact `cflags_rel` profile from
`configure.py` — `-O0,p -char unsigned -fp_contract off -sdata 0 -sdata2 0`
over `cflags_base` (`-proc gekko -align powerpc -enum int -fp hardware
-Cpp_exceptions off -inline auto -str reuse -RTTI off`). Source commit under
test: `a34972549072d7e0cd8a0fe01e3d7eecb02cc477`.

All probes were compiled outside the build tree into a scratch directory; the
repository working tree was never used as a probe surface. **Every probe source
in this document is reproduced in full below**, so each measurement can be
re-run from this file alone. See [Reproducing](#reproducing).

Provenance is marked throughout:

- **[measured]** — re-run for this document against the compiler named above,
  with the probe source printed here.
- **[campaign]** — quoted from the w01Dll campaign write-up on branch
  `codex/w01dll-match`, commit `7d0876c98b8ab91dd1a215e241ef2625fc9d7883`.
  **That branch is local-only and is not pushed to `origin`**, so these figures
  are not reproducible from this repository and are not treated as confirmed.
  They require a full build, which these probes deliberately avoid.

## Why this was measured

The campaign characterised a w01Dll `.rodata` gap — `.text`, `.data` and `.bss`
byte-identical to the target while `.rodata` was ordered differently and 4 bytes
longer — as a single contiguous 104-byte block of 19 hot constants that the
target emits before any function is code-generated. **[campaign]**

That framing is now historical. `configure.py:1146` carries
`Object(Matching, "REL/w01Dll/world01.c")`, and the exception
`w01-world01-exact-source-shape` records the retained owner as byte-exact across
`.text`, `.rodata`, `.data`, `.bss` and 5,673 relocations. The gap was closed by
`#pragma section code_type ".text.common"` with file-qualified ldscript
selectors (`configure.py:301` `rel_ldscript_replacements["w01Dll"]`, and
`src/REL/w01Dll/world01.c:399,486,6411`) — **not** by any of the source-only
constructs probed below. What survives is the reusable compiler behaviour, and
the record of which source-only routes were tried and rejected.

## Correction: `.rodata` is ONE region in strict creation order

Both the campaign write-up and the workspace field notes state that *"named
`.rodata` objects always precede the literal pool"*. **That is false**, and the
two-region `[named objects][literal pool]` model built on it is wrong.

**[measured]** `p1.c` creates a pool literal *first*, a named const object
*second*, and another pool literal *third*:

```c
/* p1.c */
float useLit(float x)  { return x * 7.5f; }
const float namedTbl[3] = { 1.25f, 2.5f, 7.5f };
float useLit2(float x) { return x * 3.25f; }
```

Emitted `.rodata` (20 bytes), `.text` 40 bytes:

```text
0x00  40f00000  7.5f     @5        <- pool literal, created FIRST
0x04  namedTbl[0..2]  (12 bytes)   <- named object, created SECOND
0x10  40500000  3.25f    @10       <- pool literal, created THIRD
```

The named object sits **between** two pool literals. Section order is simply
**creation order**, with no separation of named objects from pooled literals.

`p1.c` also shows **no deduplication between a named object and a pool literal**:
`7.5f` appears twice, once as the pool literal at `0x00` and once inside
`namedTbl` at `0x0c`. Pool literals *do* dedup against each other — see `p9`.

**[measured]** `p4.c` confirms the same model with all four construct classes in
one TU:

```c
/* p4.c */
#pragma cplusplus on
extern inline float ei(float x) { static const float pk = 55.5f; return x * pk; }
#pragma cplusplus reset
static inline float helper(float x) { return x * 11.5f; }
float useLit(float x)  { return x * 7.5f; }
const float namedTbl[3] = { 1.25f, 2.5f, 7.5f };
float useLit2(float x) { return x * 3.25f; }
float useHelper(float x) { return helper(x); }
```

`.rodata` 28 bytes, `.text` 60 bytes:

```text
0x00  55.5f      pk$localstatic3$ei__Ff   WEAK   extern-inline local static (parse)
0x04  7.5f       @8                              pool literal (useLit codegen)
0x08  namedTbl                (12 bytes)         named object (definition point)
0x14  3.25f      @13                             pool literal (useLit2 codegen)
0x18  11.5f      @18                             pool literal (helper FIRST EXPANSION)
```

Predicted by strict creation order; contradicted by the two-region model, which
would place `pk` and `namedTbl` together at the front.

> **Correction to an earlier revision of this document.** The p4 table was
> previously published as `namedTbl 0x08`, `3.25f 0x10`, `11.5f 0x14`. Those
> last two offsets were wrong: they implicitly gave `namedTbl` 8 bytes when it
> is `const float[3]` = 12 bytes. The measured offsets are `0x14` and `0x18`,
> and `.rodata` is 28 bytes, not 24. The construct ordering — the actual claim —
> is unaffected.

## When each construct creates its constants

| construct | constants created | probe |
| --- | --- | --- |
| named file-scope `const` object, referenced | at its definition point | `p1`, `p4` **[measured]** |
| ordinary expression literal | at the codegen of the referencing function | `p1`, `p2`, `p4` **[measured]** |
| `static inline` body, called | at the **first expansion** site | `p2`, `p4` **[measured]** |
| `static inline` body, never called | nothing emitted | `p2b` **[measured]** |
| `static inline` with function-scope `static const`, never called | nothing emitted | `p2b` **[measured]** |
| `extern inline` + function-scope `static const` under `#pragma cplusplus on`, never called | at **parse** of the inline body | `p3`, `p4` **[measured]** |
| unreferenced file-scope `static const` | **emitted in full, at its definition point** | `p11` **[measured]** |

The last row **refutes** an earlier claim in this document and in the campaign
notes that an unreferenced file-scope `static const` emits nothing. See `p11`.

**[measured]** `p2.c` isolates the expansion-timing rule: `helper` is *defined
first* but *expanded second*, and its `11.5f` lands after `first`'s `22.5f`.

```c
/* p2.c */
static inline float helper(float x) { return x * 11.5f; }
float first(float x)  { return x * 22.5f; }
float second(float x) { return helper(x); }
```

```text
p2.o .rodata (8 bytes):  0x00 41b40000 22.5f  @8   (first, defined second)
                         0x04 41380000 11.5f  @13  (helper, defined FIRST, expanded second)
```

**[measured]** `p2b.c` — an uncalled `static inline` carrying a function-scope
`static const` emits nothing:

```c
/* p2b.c */
static inline float uncalled(float x) { static const float pk = 55.5f; return x * pk; }
float useA(float x) { return x * 66.5f; }
```

```text
p2b.o  .text 20 bytes   .rodata 4 bytes: 0x00 42850000 66.5f @9   (55.5f absent)
```

**[measured]** `p3.c` emits `.rodata` ahead of all function codegen at zero
`.text` cost, via the COMDAT mechanism already recorded in the card
`mwcc-extern-inline-static-local-comdat-ownership`:

```c
/* p3.c */
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

`ei` is never called, yet `pk` is emitted, weak, at offset 0. Compare `p2b`: the
same idea with `static inline` emits nothing. The distinction is real.

**[measured]** `p11.c` — the plainer route to the same effect, with no pragma
and no weak symbol:

```c
/* p11.c */
static const float unusedTbl[3] = { 1.5f, 2.5f, 3.5f };
float live(float x) { return x * 4.5f; }
```

```text
p11.o  .text 20 bytes   <- identical size to live() alone (see p10a)
       .rodata 16 bytes: 0x00 unusedTbl (12 bytes, LOCAL)  <- never referenced
                         0x0c 40900000 4.5f @5
```

An unreferenced file-scope `static const` **is** emitted, in full, at its
definition point, at **zero `.text` cost**, ahead of the first function's
literal. So the `extern inline` + `#pragma cplusplus on` construct is *not* the
only zero-`.text`-cost way to prefix `.rodata`; a plain unreferenced
`static const` does it without any compiler control at all.

## `.text` byte-identity places zero constraint on `.rodata` order

**[measured]** Every `.text` -> `.rodata` reference is a relocation pair against
a symbol; the instruction immediates carry no offset. From `p4.o`'s `.rela.text`:

```text
off=0x0002 type=6 (ADDR16_HA) sym=@8  addend=0  instr=0x3c600000  imm=0x0000
off=0x0006 type=4 (ADDR16_LO) sym=@8  addend=0  instr=0x38630000  imm=0x0000
off=0x0016 type=6 (ADDR16_HA) sym=@13 addend=0  instr=0x3c600000  imm=0x0000
off=0x001a type=4 (ADDR16_LO) sym=@13 addend=0  instr=0x38630000  imm=0x0000
off=0x002a type=6 (ADDR16_HA) sym=@18 addend=0  instr=0x3c600000  imm=0x0000
off=0x002e type=4 (ADDR16_LO) sym=@18 addend=0  instr=0x38630000  imm=0x0000
```

All six immediate fields are `0x0000`, and the same pattern holds in `p1`, `p2`,
`p3`, `p10b` and `p11`. Permuting `.rodata` changes only each symbol's section
offset, which lives in the relocation, so `.text` bytes are untouched.

The diagnostic consequence: a 100%-text match with reordered `.rodata` is
**never** a codegen problem. It is a source-structure problem — declaration
order, inline placement, or a construct whose constants are created at a
different time.

**[campaign]** The same reasoning was applied at whole-object scale: w01Dll held
`.text` byte-identical across 2,948 `.text` -> `.rodata` relocations at the
*same* text offsets, every one resolving to a byte-equal value, making the
difference a pure permutation. Not re-run here.

## The `.rodata` match criterion is prefix-identity, not percentage

**[campaign]** Measurement across the objects marked `Matching` in `configure.py`
found 54 with a `.rodata` section, of which 12 score below 100% while shipping in
a passing build, as low as 85.7143% (`main/game/sprman`, `main/game/hsfdraw`,
`main/dolphin/mtx/quat`), including two RELs.

```text
selmenuDll  ours=132  target=136  prefix(132) IDENTICAL  tail=00000000
mdseldll    ours=606  target=608  prefix(606) IDENTICAL  tail=0000
sprman      ours=12   target=16   prefix(12)  IDENTICAL  tail=00000000
```

In every one the **target is larger** and the extra bytes are trailing zeros from
dtk's split rounding. The criterion is prefix-identical content plus the
*direction* of the size difference.

**Unreconciled.** The campaign quotes this as covering "all 306 objects marked
`Matching` in `configure.py`". The committed `configure.py` contains 293
`Object(Matching` entries, so the base figure does not reconcile against this
tree. The per-object numbers above are quoted as recorded and have not been
re-derived; treat the whole section as campaign-carried until a full build
reproduces it.

## Source forms with zero codegen effect at `-O0`

**[measured]** `p5a.c` versus `p5b.c` differ only by redundant parentheses,
`register` on a parameter and two locals, `const` on all three parameters, and
`u += t` in place of `u = u + t`.

```c
/* p5a.c */                          /* p5b.c */
float f5(float a, float b, float c) {  float f5(register const float a, const float b, const float c) {
    float t;                               register float t;
    float u;                               register float u;
    t = a * b;                             t = (a * b);
    u = t + c;                             u = ((t) + (c));
    u = u + t;                             u += t;
    return u;                              return (u);
}                                      }
```

Result: `.text` 60 bytes on both sides, **byte-identical**.

**[measured]** `p7a.c` versus `p7c.c` differ only by redundant parentheses:
byte-identical, 16 bytes.

```c
/* p7a.c */ float f7(float a, float b, float c, float d) { return (a*b)*(c*d); }
/* p7b.c */ float f7(float a, float b, float c, float d) { return ((a*b)*c)*d; }
/* p7c.c */ float f7(float a, float b, float c, float d) { return (((a)*(b))*(((c))*(d))); }
/* p7d.c */ float f7(float a, float b, float c, float d) { return (b*a)*(d*c); }
```

Genuine regrouping is not neutral. `p7a` against `p7b` — same operands, same
order, same 16-byte size, different destination registers:

```text
p7a:  fmuls f5,f1,f2 | fmuls f0,f3,f4 | fmuls f1,f5,f0
p7b:  fmuls f0,f1,f2 | fmuls f0,f3,f0 | fmuls f1,f4,f0
```

The balanced tree needs a second live temporary (`f5`); the left-leaning tree
reuses `f0`. Float temporaries are numbered by **expression-tree shape**, not by
liveness and not by the `register` keyword — and parentheses that do not change
the tree do not change the numbering.

### Commutative operand order: what it does and does not move

**[measured]** `p7d` commutes both products (`(b*a)*(d*c)`) against `p7a`. The
result is *not* neutral, but it is also not a temporary-allocation lever:

```text
p7a:  fmuls f5,f1,f2 | fmuls f0,f3,f4 | fmuls f1,f5,f0
p7d:  fmuls f5,f2,f1 | fmuls f0,f4,f3 | fmuls f1,f5,f0
```

Destination registers are **identical** (`f5`, `f0`, `f1`) and the tree shape is
unchanged. Only the A/C operand fields of the two leaf multiplies swap. So a
commutative source swap is a targeted control for an isolated *operand-order*
mismatch; it does not reallocate temporaries and is not a reason to sweep.

This does not license a mechanical sweep. The bounded protocol in the card
`mwcc-commutative-fmuls-source-swap-canonicalizes-neutral` still governs: run
**one** direct reversal, and if the emitted code does not move, record the
neutral result and stop. That card's own measurements are on GC/2.6 and
GC/1.3.2, where the reversal was observed to canonicalize; the GC/2.7 probe here
shows the operand fields following the source. Both can hold — the compilers and
the surrounding functions differ — and neither supports sweeping.

### Helper definition position is NOT neutral

**[measured]** This refutes a claim in an earlier revision of this document that
a helper's file position is an `-O0` no-op.

```c
/* p8a.c — helper defined ABOVE the caller */
static float helper(float x, float y) { float t = x * y; return t + 2.5f; }
float caller(float a, float b) { return helper(a, b) * 3.5f; }

/* p8b.c — prototype first, helper defined BELOW the caller */
static float helper(float x, float y);
float caller(float a, float b) { return helper(a, b) * 3.5f; }
static float helper(float x, float y) { float t = x * y; return t + 2.5f; }
```

```text
p8a  .text 148 bytes   .rodata 40200000 40600000   (2.5f then 3.5f)
p8b  .text 112 bytes   .rodata 40600000 40200000   (3.5f then 2.5f)
```

Under `-inline auto` the helper defined above the caller is expanded into it
(148 bytes, no call); defined below, it is not visible for automatic inlining
and a real call is emitted (112 bytes, `bl` present). Both `.text` **and**
`.rodata` order change. This agrees with the pre-existing card
`gc26-o0-definition-order-and-call-evaluation` ("helper definition order can
change automatic inlining") and with `p2`, where definition order and expansion
order diverge.

## Inline expansion and frame allocation

**[measured]** Declaration order inside an inline helper is load-bearing **when
the helper's locals are independent**, and neutral when the second depends on the
first. Two probe pairs bound this.

`p6a.c` / `p6b.c` — the helper's second local depends on the first:

```c
/* p6a.c */
static inline float helper(const float *pv, float k) {
    float s;
    float t;
    s = *pv * k;
    t = s + k;          /* t depends on s */
    return s * t;
}
float caller(float a, float b, float c) {
    float v;
    float w;
    v = a * b;
    w = v + c;
    return helper(&v, w) * v + w;
}
/* p6b.c is p6a.c with the helper's `float s;` and `float t;` exchanged */
```

```text
p6a  .text 140 bytes
p6b  .text 140 bytes   BYTE-IDENTICAL  <- declaration order neutral here
```

`p6d.c` / `p6e.c` — the helper's two locals are independent:

```c
/* p6d.c */
static inline float helper(const float *pv, float k) {
    float s;
    float t;
    s = *pv * k;
    t = k + 1.5f;       /* t independent of s */
    return s * t;
}
float caller(float a, float b, float c) {
    float v;
    float w;
    v = a * b;
    w = v + c;
    return helper(&v, w) * v + w;
}
/* p6e.c is p6d.c with the helper's `float s;` and `float t;` exchanged */
```

```text
p6d  .text 152 bytes
p6e  .text 152 bytes   NOT identical - three allocation fields differ:
  [0x03c] p6d fmuls f28,f31,f30   p6e fmuls f29,f31,f30
  [0x04c] p6d fadds f29,f0,f30    p6e fadds f28,f0,f30
  [0x050] p6d fmuls f27,f28,f29   p6e fmuls f27,f29,f28
```

Same size, exchanged `f28`/`f29`. Two helpers differing only in declaration
order are genuinely different functions and cannot be merged — but only when the
declarations are independent, which is the case worth probing.

**[measured]** Passing a caller value **by pointer** is the cheaper form. `p6a`
(by pointer) is 140 bytes; `p6c`, doing the same work with the value passed by
value plus an out-pointer, is 164 bytes.

```c
/* p6c.c */
static inline float helper(float vv, float k, float *pout) {
    float s;
    float t;
    s = vv * k;
    t = s + k;
    *pout = s;
    return s * t;
}
float caller(float a, float b, float c) {
    float v;
    float w;
    float o;
    v = a * b;
    w = v + c;
    return helper(v, w, &o) * v + w + o;
}
```

Note `p6c` also adds the `+ o` term, so the 24-byte delta is an upper bound on
the by-value cost rather than an isolated measurement of it.

**[campaign]** Not re-measured here: an inlinee's locals are allocated
**ascending from the bottom of the frame**, below everything the caller
allocates, which is why a target layout with a group of locals underneath the
compiler temporaries indicates an inline helper rather than a nested block.
Plain nested blocks continue the enclosing block's descending run. In the small
probes above the inlinee's locals were enregistered rather than stack-homed, so
the direction could not be observed.

## Rejected source-only routes for prefixing `.rodata`

**[measured]** unless marked.

- **Uncalled `static` function.** Not dead-stripped. `p10a` (one function) is
  `.text` 20 bytes; `p10b` adds an uncalled `static float dead(float)` and costs
  `.text` 40 bytes — and `dead`'s literal is created *first*, at `.rodata 0x00`,
  ahead of the live function's. So it does prefix the pool, but never at zero
  `.text` cost.

  ```c
  /* p10a.c */ float live(float x) { return x * 4.5f; }
  /* p10b.c */ static float dead(float x) { return x * 9.5f; }
               float live(float x) { return x * 4.5f; }
  ```

  ```text
  p10a  .text 20  .rodata 4:  0x00 40900000 4.5f
  p10b  .text 40  .rodata 8:  0x00 41180000 9.5f (dead)  0x04 40900000 4.5f (live)
  ```

- **Unreferenced file-scope `static const`.** Zero `.text` cost and *does*
  prefix `.rodata` — see `p11` above. Previously recorded as emitting nothing;
  that was wrong.

- **Declared-but-never-referenced locals.** Free. `p12a` and `p12b` differ by two
  unused `float` locals and are byte-identical at `.text` 36 bytes, `.rodata`
  empty. A leftover declaration need not be deleted to match.

- **Folding.** `p9.c`:

  ```c
  float mulOne(float x)  { return x * 1.0f; }
  float divTwo(float x)  { return x / 2.0f; }
  float mulHalf(float x) { return x * 0.5f; }
  ```

  `mulOne` compiles to `blr` alone — `x * 1.0f` is a copy and seeds nothing.
  `divTwo` strength-reduces to `* 0.5f`, so the pool holds `0.5f`, not `2.0f`.
  `.rodata` is 4 bytes total: `divTwo` and `mulHalf` share the one `0.5f` slot,
  so pool literals **do** dedup against each other (unlike named object versus
  pool literal, see `p1`). Reciprocal tricks cannot seed the pool.

- **[campaign]** Compiler build is not a lever: all eleven MWCC builds present
  (1.3.2, 2.0, 2.5, 2.6, 2.7, 3.0a3, 3.0a3.2/3/4, 3.0a5, 3.0a5.2) produced
  w01Dll `.rodata` 1,092 with the same ordering; 1.3.2, 2.0, 2.5, 2.6 and 2.7
  also gave byte-identical `.text`.

- **[campaign]** Flags are not a lever: `-inline on`, `-inline off`,
  `-inline all` left the pool at 1,092; `-str reuse,pool` and `-str pool,readonly`
  made no change. `-inline deferred` and `-inline auto,deferred` did reorder it
  (to 1,108), confirming inline codegen timing drives pool creation order, but
  both wrecked `.text` (85,088 and 104,196 against 91,304).

- **[campaign]** Other zero-cost statement forms — statements after `return`,
  `switch(1){case 7:}`, `if(0){}`, `goto` over a block — emit nothing. A dead
  store cost 72 bytes of `.text`.

- **[campaign]** Allocation order within one expression is not left-to-right:
  `a*0.5f + 2.0f` allocated `2.0f` first.

## What this leaves open

The w01Dll `.rodata` gap itself is **closed**: `world01.c` is
`Object(Matching)` and byte-exact, using the `#pragma section code_type` plus
ldscript-selector route recorded in the exception
`w01-world01-exact-source-shape`. The residual questions are tracked as debt on
the owner `w01Dll:REL/w01Dll/world01`:

1. **The accepted shape uses a compiler control, not a natural source order.**
   No zero-`.text`-cost *source-only* arrangement was found that creates the 19
   hot constants before the first function is code-generated; the best
   source-only construction emitted them from one dead `static` function before
   `_prolog`, reaching `.rodata` byte-identical at 1,088 but costing +308 bytes
   of `.text` **[campaign]**. The two zero-cost prefixing constructs measured
   here (`p3` extern-inline local static, `p11` unreferenced file-scope
   `static const`) are diagnostic levers; neither is an authenticated source
   shape and neither is what shipped.

2. **Whether dtk reconstructs `.rodata` faithfully per translation unit is
   unconfirmed.** dtk splits the linked REL by address range, so a translation
   unit contributing `.rodata` but no `.text` would be invisible to the splitter
   and its constants attributed to the neighbouring object. That single mechanism
   would explain why `world01.o` is the only one of 21 REL target objects whose
   first `text -> rodata` reference (`0x78`) is not its lowest referenced offset
   (`0x10`) **[campaign]**.

## Reproducing

Every probe source above is complete. Write each block to its named file in a
scratch directory outside the build tree, then:

```sh
MWCC="Compilers/GC/2.7/mwcceppc.exe"
FLAGS="-nodefaults -proc gekko -align powerpc -enum int -fp hardware \
  -Cpp_exceptions off -O0,p -inline auto -maxerrors 1 -nosyspath -RTTI off \
  -char unsigned -fp_contract off -str reuse -multibyte -sdata 0 -sdata2 0"

for p in p1 p2 p2b p3 p4 p5a p5b p6a p6b p6c p6d p6e \
         p7a p7b p7c p7d p8a p8b p9 p10a p10b p11 p12a p12b; do
  "$MWCC" $FLAGS -c -o "$p.o" "$p.c"
done
```

Then read each object's `.rodata`, `.symtab` and `.rela.text`. Section sizes and
byte-identity comparisons in this document are taken directly from the ELF
section headers and contents; `.rodata` symbol offsets come from `.symtab`
entries whose `st_shndx` is the `.rodata` section index.

For real objects, prefer the project's own tooling — `dtk rel info <file>.rel`
for REL section layout, and objdiff's JSON `data_diff` (with
`-c ppc_calculate_pool_relocations=true -c function_reloc_diffs=none`) for which
`.rodata` bytes differ.
