# Board gap and tail ownership evidence

This record distinguishes meaningful C object bytes from section padding and
synthetic symbol extents in the GP6E01 board objects. It supports the
`dtk-synthetic-gap-not-source-ownership` knowledge card.

## Result

At AI commit `1a502fe7d6c1b55707df714a611fb0b56b538d18`, fresh strict
objdiff reports showed exact functions, text, and effective relocations for the
owners below. Their named source objects also matched. The residual raw section
sizes were explained by DTK `.hidden gap_*` symbols, end alignment, final-symbol
extent, or targetless header constants:

| Owner | Exact code/relocations | Residual classification |
|---|---:|---|
| `main:board/audio` | 49/49; 444/444 | Target `.sdata` and `.sbss` each have a four-byte tail gap. |
| `main:board/camera` | 70/70; 607/607 | `viewData` is exactly 18 bytes; target `.data` adds an explicit six-byte gap. |
| `main:board/guide` | 25/25; 312/312 | Named tables match; `.data` and `.sdata2` each add four non-owned tail bytes. |
| `main:board/pause` | 22/22; 537/537 | Named tables occupy 388 exact bytes; target `.data` ends with a four-byte gap. |
| `main:board/window` | 67/67; 449/449 | `mbWinTopNo` is an authenticated `u8`; DTK extends the final target `.sbss` extent through seven tail bytes, and `.sdata2` has a four-byte gap. |

`main:board/comchoice` and `main:board/exit` provide the inverse case: their
source objects contain targetless header-local `sqrtf` constants with no
effective target relocation. The linker discards those bytes, so retaining or
manufacturing target ownership would be incorrect.

## Verification boundary

Gap classification is permitted only when all of these are true:

1. Every target function is strict exact, including relocation identity.
2. Every named source-owned data object agrees in bytes and effective relocations.
3. Symbol-table inspection identifies the residual as a gap, alignment tail,
   final-symbol extent, or targetless discarded pool.
4. The serialized retail build passes `config/GP6E01/build.sha1` and the rebuilt
   `main.dol` matches retail SHA-1
   `b897e6ade6b3a0cd2f9907689f38a3b19c327e70`.

A mismatch inside a named object, a nonexact effective relocation, or linked
output drift is a real recovery defect. It must not be dismissed as padding.

## Source-link closure validation

At AI base `abb7aed4b0386183e917baa60dac25aba802d194`, a fresh isolated
configuration linked `board/audio.c` from source. Objdiff 3.8.0 reported 49/49
strict-exact and 49/49 data-value-exact functions with no missing definitions
or order inversions, while DTK 0.9.2 reported 444 relocations in each object.
The serialized full build then reported `137 files OK`,
and `main.dol` matched retail SHA-1
`b897e6ade6b3a0cd2f9907689f38a3b19c327e70`. This closes
`main:board/audio`; its four-byte target `.sdata` and `.sbss` tails are
non-owned padding.

The same test does not yet close `main:board/scroll`. Although Scroll is 31/31
strict-exact and 31/31 data-value-exact at function level, linking Audio and
Scroll from source produced only `136 files OK`: `main.dol` SHA-1 became
`c496c9f6191624dbf447217e2d0ab6f6d4584184`, with 63 changed bytes. Restoring
only Scroll to its retail object while leaving Audio source-linked restored all
137 checksums. Scroll therefore remains a linked data-ownership defect, not a
harmless-gap exception.

The practical rule is therefore to inspect ownership before editing C. Never
widen a real declaration, add opaque storage, or insert padding solely to make
raw section extents equal.
