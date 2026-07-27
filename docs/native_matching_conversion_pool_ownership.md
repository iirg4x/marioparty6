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

## Owner-level cluster evidence

Later application-owner work reproduced the same failure at larger scale:

- `miraclebookdll` functions `fn_1_3734`, `fn_1_479C`, `fn_1_4FD4`,
  `fn_1_6B8C`, and `fn_1_8A78` converged on instruction-identical conversion
  sequences while retaining relocation debt to the shared
  `lbl_1_rodata_F0`/`lbl_1_rodata_98` owners.
- `optionDll:fn_1_7728` reproduced all 844 instruction bytes, but its local
  `@489` relocation did not bind the retail `lbl_1_rodata_1E8` owner.
- `s01Dll:fn_1_590` reproduced all 748 instruction bytes with exact local
  rodata placement at target offset `0x28`; its only residual was the
  three-instruction signed-conversion relocation binding local `@365` instead
  of `lbl_1_rodata_28`.

When several natural bodies fail only at the same named pool owner, stop
shaping the consumers independently. Recover the owner-level data binding and
source chronology once, then re-enable the affected cluster together. Until
that ownership proof is exact, the functions remain unresolved even when all
ordinary instructions and literal values agree.

## Linked-owner closure

The completed `s01Dll` owner establishes an important boundary on the rule.
`fn_1_590` still named its signed-conversion constant `@365` in the intermediate
source object while retail called the same offset `lbl_1_rodata_28`. Once the
complete `0x78` application pool was source-owned, authenticated direct literals
made every application consumer bind that same pool naturally. The final linked
REL matched retail byte-for-byte at SHA-1
`7f0cfdb2d2b0b2c50b92675e5bef55d72cf94dd7`.

Therefore intermediate symbol spelling is diagnostic, not the final ownership
gate. A local name is acceptable only when complete section bytes, linked target
section/addend, all consumers, and the final REL are exact. Without that full
linked proof, the earlier rejection rule still applies.
