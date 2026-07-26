# MWCC conversion-pool relocation ownership

Instruction identity is not sufficient to retain a recovered function when the
compiler binds an integer-to-floating conversion sequence to the wrong literal
owner. Three independent `GC/2.6` REL recovery clusters exposed the same
failure mode.

## Authenticated observations

- `openingDll` retained 31 of 41 application functions (`0x3268` text bytes,
  1214 aligned relocations). Every remaining function consumes the configured
  `lbl_1_rodata_E0` conversion constant; unsigned paths also consume
  `lbl_1_rodata_128`. `#pragma pool_data off` still emitted anonymous MWCC
  conversion constants, and adding an equivalent weak named constant emitted
  both owners rather than absorbing the anonymous pool.
- `s02Dll:fn_1_4B8` reproduced all `0x470` text bytes. Its sole mismatch was
  the three-instruction conversion relocation site: retail binds
  `lbl_1_rodata_60`, while the partial source object binds compiler-local
  `@373`.
- `s03Dll:fn_1_1238` reproduced all 536 text bytes. Its two conversion
  relocations bind compiler-local `@360` at partial-object rodata `+0x10`,
  while retail binds `lbl_1_rodata_A0`. Target chronology places the larger
  preceding `fn_1_76C` before this consumer, so that predecessor must be tested
  as the natural pool owner before the downstream function can be retained.

These are ownership mismatches, not value or instruction mismatches. A named
constant with the same bytes, an altered DTK split, or an assembly/object
rewrite would manufacture the desired relocation without recovering the
source owner.

## Recovery rule

When text and size are exact but a compiler-generated conversion literal binds
the wrong symbol, compare the exact relocation stream and preceding source
chronology before retaining the function. Restore an authenticated earlier
owner when the evidence supports one; otherwise leave the function unresolved.
Do not fabricate a literal, force an ABI construction, or move a split solely
to rename the relocation target.

The retained gate remains exact instructions, exact relocation ownership,
organic source, and linked consumer proof. A text-only 100% score does not
satisfy that gate.
