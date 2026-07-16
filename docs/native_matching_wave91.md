# Native Matching Wave 91 -- `mdparty.c` helper-boundary recovery

Wave 91 proves four more `REL/mdpartydll/mdparty.c` functions exact and
replaces six additional hand-expanded regions with target-authenticated helper
calls and source expressions. The owner remains `NonMatching`: all 258
functions and all `0x3F424` target text bytes have source, but 27 functions and
`0x132C4` target bytes still have real matching work.

## Evidence basis

- MP6 target instructions, relocations, DTK split boundaries, and object-use
  analysis are authoritative. The project remains pinned to DTK `v0.9.2`
  (`4d039140f2d2ed80572b1949b76a5ff9b3094e06`).
- Exact same-TU helpers `fn_1_9B90`, `fn_1_C158`, `fn_1_C200`,
  `fn_1_C28C`, `fn_1_C698`, and `fn_1_CA68` authenticate the recovered
  argument capture, evaluation order, local lifetimes, and nested inline
  objects. Matching `mdseldll` confirms the same helper/redeclaration idiom.
- The MP5 framework at `marioparty5/src/REL/mdpartyDll` constrains party-flow
  ownership and adjacent-message arithmetic where the games retain parity. It
  is not treated as proof where M6 logic diverges.
- Recovered MWCC `CInline.c`, `CPrep.c`, `Coloring.c`, and `StackFrame.c`
  explain inline-size decisions, virtual-register creation order, coloring,
  and stack chronology. Compiler evidence is not permission to add fake
  locals, force registers, or invent an unidentifiable pragma value.

## Exact recovery

| Function | Target bytes | Wave 90 | Wave 91 | Recovered source shape |
| --- | ---: | ---: | ---: | --- |
| `fn_1_10518` | `0xFAC` | `96.899300%` | `100%` | Restore the target's `count >= current` main branch and trailing reset `else`; the former C inverted the physical branch topology. |
| `fn_1_12FD8` | `0xEE8` | `98.534590%` | `100%` | Restore exact `fn_1_9B90` so its bank argument is captured before sprite-group evaluation, plus the target's `> 2`/`> 3` wrap tests. |
| `fn_1_150D0` | `0x5E4` | `99.336870%` | `100%` | Recover the distinct player-init `j` lifetime and the target-proven `player`-before-`entry` declaration chronology. |
| `fn_1_2851C` | `0x107C` | `93.574410%` | `100%` | Replace four flattened animation loops with four independent exact `fn_1_C200` to `fn_1_C158` nested helper expansions. |

The four additions total `0x34F4` target bytes. Each has the exact target size,
instruction bytes, and effective relocations under
`functionRelocDiffs=data_value`.

## Retained non-exact recovery

- `fn_1_156B4` and `fn_1_160A0` rise from `98.807880%` each to
  `99.952760%` and `99.968506%`. Restoring exact `fn_1_C698` and
  `fn_1_CA68` also closes both eight-byte size deficits; the retained
  functions are now the exact target size `0x9EC`.
- `fn_1_18384` rises from `86.644394%` at `0x5E8` to `99.618140%` at the
  exact target size `0x68C`. Its setup now uses the real `fn_1_C200`,
  `fn_1_C158`, and `fn_1_C28C` helper boundaries instead of duplicated
  animation and model-transition bodies.
- `fn_1_18AA4` rises from `98.698740%` at `0x9E0` to `99.889590%` at the
  exact target size `0x9E8` by restoring `fn_1_C698`; the already recovered
  `fn_1_CA68` phase remains intact.
- `fn_1_33724` rises from `95.963646%` at `0x15C8` to `99.857445%` at the
  exact target size `0x15EC`, and `fn_1_350F8` rises from `95.689690%` at
  `0x1154` to `99.844840%` at the exact target size `0x116C`. The target
  proves four initialized semantic locals and preserves the adjacent-message
  expression `0xD0013 - (count - 1)` at both message sites.

These six functions remain WIP and receive no Matching credit.

## Bounded residue and rejected forcing

- `fn_1_156B4`, `fn_1_160A0`, `fn_1_18384`, and `fn_1_18AA4` now have
  exact sizes, CFGs, opcodes, constants, calls, and relocations. Their 5, 3,
  29, and 14 remaining rows are saved-register color cycles. An outer
  `modelNo` temporary was not admitted because neither target nor sibling
  evidence uniquely authenticates it.
- `fn_1_33724` and `fn_1_350F8` now have exact target sizes and source
  operations. Their residue is nested-inline halfword-slot chronology and
  saved-register coloring; declaration permutation was not swept.
- `fn_1_2F22C` has the same frame, call stream, branches, and semantic
  operations as the target. Its remaining structural rows are allocator
  moves, so no source rewrite was made.
- `fn_1_2EBEC` differs only in the register used as a float-literal base.
  `fn_1_30B4C` differs only in saved-register and paired halfword-slot colors.
  Both retain exact sizes, opcodes, CFGs, constants, calls, and relocations.
- `fn_1_3EAC8` is an authenticated inline-boundary residue: the target embeds
  exact `fn_1_34D10` and `fn_1_36264`, while the current object calls them.
  MWCC proves that `#pragma inline_max_size(N)` controls this threshold, but
  the binary proves only that `N` was large enough. No sibling or repository
  history authenticates the original numeric value, so no guessed pragma or
  manual body duplication was admitted.

## Object verification

The retained relocation-aware evidence is:

`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave91_final_object.json`

It was generated with:

```powershell
rtk ninja -j1 build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk build/tools/objdiff-cli-windows-x86_64.exe diff -p . `
  -u mdpartydll/REL/mdpartydll/mdparty `
  -o build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave91_final_object.json `
  --format json-pretty -c functionRelocDiffs=data_value
```

The object reports all 258 functions and all `0x3F424` target text bytes
represented, with 231 functions and `0x2C160` target bytes exact. The 27
remaining functions account for `0x132C4` target bytes. Together with exact
`stage.c`, the application/stage portion represents all 315 functions and all
`0x467FC` target bytes; 288 functions and `0x33538` bytes are exact.

## Full gate

After the retained source and documentation were frozen, the serialized full
build and explicit DTK checksum each reported `137 files OK`. Explicit
comparisons proved both `main.dol` and `mdpartydll.rel` byte-identical to their
originals. The final build left no `config/GP6E01/symbols.txt` hunk. The
application owner remains `C-not-yet-matched`; no configure flip or fallback
credit is part of this wave.
