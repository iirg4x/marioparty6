# w01Dll / world01.c — `.rodata` constant-pool ordering (open problem)

A self-contained write-up for anyone willing to look at this. Everything below
is measured, not inferred, unless it says otherwise.

## Summary

`src/REL/w01Dll/world01.c` matches the original in every section except
`.rodata`, where the literal pool is ordered differently and 4 bytes longer.
The object therefore cannot be flipped to `Matching`, and `w01Dll.rel` is not
link-verified.

| section | ours | target | |
|---|---|---|---|
| `.text` | 91,304 | 91,304 | **byte-identical** |
| `.data` | 2,052 | 2,052 | **byte-identical** |
| `.bss` | 5,668 | 5,668 | **identical** |
| `.rodata` | **1,092** | **1,088** | reordered |

All 132 functions in the unit are byte-exact. `runtime.o`, the other
translation unit in this REL, is fully byte-identical (`.text` 2,628 = 2,628,
`.rodata` 24 = 24), so the two-unit split is consistent and sums exactly to the
REL (`.text` 91,304 + 2,628 = 93,932; `.rodata` 1,088 + 24 = 1,112).

Toolchain: MWCC 2.4.7 (`build/compilers/GC/2.7/mwcceppc.exe`), effective flags
`-O0,p -char unsigned -fp_contract off -sdata 0 -sdata2 0`.

## What the difference actually is

It is a **pure permutation**. Both objects contain exactly 2,948
`.text -> .rodata` relocations, at the **same** text offsets, and every one
resolves to a byte-equal value. No constant is missing or extra.

Per objdiff's `data_diff`, **980 of 1,088 bytes are already byte-identical and
in the correct order**, including one unbroken 560-byte run. The entire defect
is a single contiguous **104-byte block** at the front of the pool, plus the
4 bytes of padding its displacement forces:

```
0x10  float  0.0                 0x40  float  1.0      0x58  float 12.0
0x14  (4-byte alignment pad)     0x44  float  3.0      0x5C  float  8.0
0x18  double 0.5                 0x48  float -9.0      0x60  double PI
0x20  double 2.0                 0x4C  float  6.0      0x68  float 180.0
0x28  double 4330000080000000    0x50  float -3.0      0x6C  (pad)
        (int->double magic)      0x54  float  9.0      0x70  double 180.0
0x30  float  0.5
0x34  float  1/3  (3eaaaaab)
0x38  float  2.0
0x3C  float  0.1
```

`0x00`/`0x08` immediately before this block are `_half`/`_three` from `sqrtf`
in `include/math.h`, emitted at parse time. Both sides have those identically.

**The target creates the 104-byte block before any function is
code-generated.** Walking `.text` forward, the target's first pool reference is
to offset `0x78`; slots `0x10`–`0x74` are referenced only later, by functions
scattered across the file. Our pool is plain first-use order (95% strictly
increasing, beginning `0x10, 0x14, 0x18, 0x1c`).

Nothing early references them: scanning the target's `.text` bytes
`0x000`–`0x460` for any floating-point primary opcode (48–55/59/63) finds
**exactly one** hit — an `lfs` at `0x454` inside `MB1_Create`, which is the
site taking `0x78`. `_prolog` (0x00), `_epilog` (0x54), `ObjectSetup` (0xA0)
and the first `0x35A` bytes of `MB1_Create` contain no floating-point
instructions at all.

**`world01.o` is unique.** Across all 21 dtk REL target objects, it is the only
one whose first `text -> rodata` reference does not point at the lowest
referenced `.rodata` offset (min `0x10`, first reference `0x78`). Every other
REL object — actman, boot, filesel, mdparty, mdsel, meschk, selmenu, sequence,
opening, stage, and every `runtime.o` — has `firstTextRef == minRoRef`.

## What setting `Matching` produces

```
build/GP6E01/w01Dll/w01Dll.rel: FAILED
built    141,252 bytes   sha1 1e705dad61de597a52b767c8174e0deea6b30185
expected 141,244 bytes   sha1 196d7075abbe6eec3031c9484d25216de9dc0889
20,338 of 141,244 bytes differ (14.40%), first at 0x27
```

The 8-byte size delta is the 4 extra `.rodata` bytes plus alignment; `0x27` is
in the REL header where section sizes and offsets live, so everything
downstream shifts.

## Eliminated by measurement

**Compiler and flags**
- All 11 MWCC builds present (1.3.2, 2.0, 2.5, 2.6, 2.7, 3.0a3, 3.0a3.2/3/4,
  3.0a5, 3.0a5.2) produce `.rodata` 1,092 with the same ordering. Five of them
  (1.3.2, 2.0, 2.5, 2.6, 2.7) also give byte-identical `.text`.
- `-inline on` / `off` / `all`, `-str reuse,pool`, `-str pool,readonly`: no
  change. `-inline deferred` and `-inline auto,deferred` **do** reorder the
  pool (to 1,108), confirming inline codegen timing is the driver, but both
  wreck `.text` (85,088 and 104,196 against 91,304).
- `-strip_partial`: strips 9–12 KB of real code.

**Source constructs that emit nothing at all** (no code, no literals) — MWCC
folds them in the front end *before* literal-pool allocation:
- statements after `return`; `switch (1) { case 7: ... }`; `if (0) { ... }`;
  `goto` over a block
- an uncalled `static inline` function, including one holding function-scope
  `static const`
- unreferenced file-scope `static const`

**Constructs that allocate literals but always cost `.text`**
- a dead store (`t = k*0.5f + 2.0f; t = 1.0f;`) — 72 bytes
- an uncalled `static` function — MWCC 2.7 does not dead-strip it

**Other**
- Moving `static inline` definitions: no-op across 19 positions. An inline
  body's constants are created at first *expansion*, never at parse.
- Named `.rodata` objects **do** precede the literal pool, but MWCC never
  dedups a pool literal against a named object, so a const table yields two
  copies of every value.
- `x * 1.0f` folds to a copy; `x / 2.0f` strength-reduces to `* 0.5f` and
  allocates only `0.5f`, so reciprocal tricks cannot seed the pool.
- Allocation order within one expression is **not** left-to-right:
  `a*0.5f + 2.0f` allocates `2.0f` first.

## Best known construction (not deployable)

One dead `static` function placed immediately before `_prolog`, evaluating the
19 constants in order, yields `.rodata` **byte-identical at 1,088**, `.data`
and `.bss` identical, and no new sections — at a cost of **+308 bytes of
`.text`**. With `__declspec(section ".init")` the `.text` cost disappears and
every object-level gate passes, but the 424 bytes become a `.init` section that
dtk carries into the REL (141,980 vs 141,244). The original REL has 19 sections
with only 6 non-empty and **no `.init`**, so that construct is not what the
original had.

## Open question

What in the original translation unit creates 19 constants before the first
function is code-generated, at zero `.text` cost?

One hypothesis not yet resolved: dtk splits the linked REL into objects **by
address range**, so a translation unit contributing `.rodata` but **no
`.text`** would be invisible to the splitter — its constants would be
attributed to whichever object owns the neighbouring text. That single
mechanism would explain every fact above, including why only this object of 21
is affected. It has not been confirmed, and it has also not been verified that
dtk reconstructs `.rodata` faithfully per translation unit.

## Reproducing

```bash
# current state
objdiff-cli diff -p . -u w01Dll/REL/w01Dll/world01 \
  -c ppc_calculate_pool_relocations=true -c function_reloc_diffs=none \
  -o out.json --format json
# then read left/right sections[].data_diff — ordered equal / DIFF_INSERT /
# DIFF_DELETE runs with base64 payloads

dtk rel info w01Dll.rel        # REL section layout
```

`tools/matchsearch.py` in this repo compiles candidate sources outside the
build tree and scores them on instruction-stream alignment; the fuller
methodology notes live in the `mp6-rebuild` workspace at
`docs/decomp-matching.md` §9–12.
