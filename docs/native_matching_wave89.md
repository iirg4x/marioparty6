# Native Matching Wave 89 -- `mdparty.c` helper and camera recovery

Wave 89 proves three more `REL/mdpartydll/mdparty.c` functions exact and
improves two additional compiler shapes.  The owner remains `NonMatching`:
all 258 functions and all `0x3F424` target text bytes have source, but 40
functions and `0x1BF04` bytes still have real matching work.

## Evidence basis

- MP6 target instructions, relocations, DTK split boundaries, and object-use
  analysis are authoritative.  DTK's documented
  [function-boundary, relocation, section, and object analysis](https://github.com/encounter/decomp-toolkit#analyzer-features)
  supplies the split-object evidence; this project pins DTK `v0.9.2`
  (`4d039140f2d2ed80572b1949b76a5ff9b3094e06`).
- The authenticated MP5 framework at
  `marioparty5/src/REL/mdpartyDll` commit
  `e246f9d9850ff53ac684b971068fbf87fdcf6acb` constrains party-flow and helper
  source shape.  MP5 is not treated as proof where its logic diverges.
- Recovered MWCC `InstrSelection.c`, `CInline.c`, and `StackFrame.c` explain
  constant-node selection, inline-local creation, and stack-slot assignment.
  Compiler evidence narrows authentic C shapes; it is not permission to add
  fake locals, forced registers, padding, or pointer casts.

## Accepted recovery

| Function | Target bytes | Wave 88 | Wave 89 | Recovered source shape |
| --- | ---: | ---: | ---: | --- |
| `fn_1_2B74` | `0x710` | `95.548676%` | `100%` | Restore target evaluation order for sub-stick sampling and the two commutative camera-movement sums. |
| `fn_1_EB74` | `0xCC4` | `98.763770%` | `100%` | Restore the existing exact `fn_1_8950` helper boundary instead of flattening its two work-field stores. |
| `fn_1_F838` | `0xCE0` | `97.833740%` | `100%` | Restore the existing exact nested `fn_1_8CCC` helper boundary instead of flattening four work-field stores. |
| `fn_1_4A84` | `0x458` | `93.219420%` | `95.935250%` | Restore the duration lifetime and both preincrement timer comparisons; source size falls from `0x480` to `0x478`. |
| `fn_1_1158C` | `0x920` | `99.143840%` | `99.494865%` | Restore the target-proven combined loop/global initializer chronology. |

The three exact additions total `0x20B4` target bytes.  Their source objects
have the target sizes `0x710`, `0xCC4`, and `0xCE0`, and their effective
relocations compare exact with `functionRelocDiffs=data_value`.

## Rejected probes and remaining residue

Every rejected probe was removed before the retained object was generated:

- `CharMotionAMemPGet(...) == NULL` does not compile because the authenticated
  `AMEM_PTR` type is `u32`; changing `0` to `0L` is byte-neutral.  No cast or
  false pointer type was admitted for `fn_1_2B0`/`fn_1_400`.
- Reordering the three `HuVecF` locals in `fn_1_22A8` is byte-score neutral and
  does not recover the separate saved-register color cycle.
- A post-definition `inline` redeclaration of exact `fn_1_C200` neither fixes
  `fn_1_D058` nor `fn_1_DA54` and damages already-exact callers.  It was
  removed.
- Staging `entry->rot.y - 540.0f` through `value` regresses both symmetric
  `fn_1_156B4` and `fn_1_160A0` from `98.807880%` to `98.382675%`.
- Spelling the `fn_1_2EBEC` threshold as `-200.0f` is byte-neutral.  Target
  `lbl_1_rodata_354` and source `@6740` are the same four-byte pool object at
  `.rodata+0x354`; inventing a named global would be false ownership.

Eight near-exact large functions were also classified.  `fn_1_329A4`,
`fn_1_31AC0`, `fn_1_36518`, `fn_1_3736C`, `fn_1_38A3C`, `fn_1_39C60`,
`fn_1_3AD84`, and `fn_1_3CFE4` have identical target/source sizes, instruction
order, control flow, and relocations; every difference is an inline-clone
stack-slot offset.  Their seven shared helper bodies are independently exact.
MWCC proves that inline-local creation chronology controls those slots, but
the binary and available sibling do not identify a unique macro/declaration
shape.  The callers therefore remain untouched rather than receiving slot
padding or declaration-order sweeps.

## Object verification

The retained relocation-aware evidence is:

`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave89_integrated_object.json`

It was generated with:

```powershell
rtk ninja -j1 build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk build/tools/objdiff-cli-windows-x86_64.exe diff -p . `
  -u mdpartydll/REL/mdpartydll/mdparty `
  -o build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave89_integrated_object.json `
  --format json-pretty -c functionRelocDiffs=data_value
```

The object reports all 258 functions and all `0x3F424` target text bytes
represented, with 218 functions and `0x23520` target bytes exact.  The 40
remaining functions account for `0x1BF04` target bytes.  Together with exact
`stage.c`, the application/stage portion represents all 315 functions and all
`0x467FC` target bytes; 275 functions and `0x2A8F8` bytes are exact.

## Full gate

After the retained source and documentation were frozen, the serialized full
build and explicit DTK checksum each reported `137 files OK`.  Explicit
comparisons proved both `main.dol` and `mdpartydll.rel` byte-identical to their
originals.  The final build left no `config/GP6E01/symbols.txt` hunk.  The
application owner remains `C-not-yet-matched`; no configure flip or fallback
credit is part of this wave.
