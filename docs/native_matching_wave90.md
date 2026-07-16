# Native Matching Wave 90 -- `mdparty.c` nested-helper recovery

Wave 90 proves nine more `REL/mdpartydll/mdparty.c` functions exact and
recovers target-backed source shape in eight additional functions. The owner
remains `NonMatching`: all 258 functions and all `0x3F424` target text bytes
have source, but 31 functions and `0x167B8` bytes still have real matching
work.

## Evidence basis

- MP6 target instructions, relocations, DTK split boundaries, and object-use
  analysis are authoritative. DTK's documented
  [function-boundary, relocation, section, object, and split analysis](https://github.com/encounter/decomp-toolkit#analyzer-features)
  supplies the REL ownership evidence; the project remains pinned to DTK
  `v0.9.2` (`4d039140f2d2ed80572b1949b76a5ff9b3094e06`).
- Matching same-game `REL/mdseldll/mdsel.c` authenticates the 180-degree
  interpolation helper and Bezier/model-facing helper families. The MP5
  framework at `marioparty5/src/REL/mdpartyDll` constrains party-flow
  ownership where the games retain parity. Neither sibling is treated as
  proof where its logic diverges.
- Recovered MWCC `CInline.c`, `CFunc.c`, `Coloring.c`, and `StackFrame.c`
  explain nested-inline argument objects, saved-register coloring, and local
  stack chronology. Compiler evidence narrows real C shape; it is not
  permission to add fake locals, forced registers, padding, or casts.

## Exact recovery

| Function | Target bytes | Wave 89 | Wave 90 | Recovered source shape |
| --- | ---: | ---: | ---: | --- |
| `fn_1_19860` | `0x4C4` | `93.301636%` | `100%` | Restore the existing `fn_1_9CF0` sprite-bank helper boundary and its two captured halfword arguments. |
| `fn_1_12364` | `0x7B4` | `96.612580%` | `100%` | Restore the explicit `s16` character-selection conversions authenticated by exact `fn_1_12EC8`, plus the real cross-TU `fn_1_45C40` prototype. |
| `fn_1_14510` | `0x7B0` | `96.674800%` | `100%` | Restore the same explicit conversions and prototype, and the target's sprite-bank value `4`. |
| `fn_1_4A84` | `0x458` | `95.935250%` | `100%` | Restore exact `fn_1_21D4` for both 180-degree Y trajectories; X retains the distinct 90-degree interpolation helper. |
| `fn_1_B748` | `0x3E8` | `94.696000%` | `100%` | Recover three distinct scoped route results, three nested candidates, and target-proven `j`/`i` declaration chronology instead of one flattened temporary. |
| `fn_1_D058` | `0x388` | `98.433630%` | `100%` | Recover `fn_1_C200` through nested exact `fn_1_C158` calls and the target's `dataOffset + dataNo` expression order. |
| `fn_1_DA54` | `0x1120` | `99.303830%` | `100%` | Restore the depth-two `fn_1_C200`/`fn_1_C158` inline objects for all four animation groups. |
| `fn_1_1158C` | `0x920` | `99.494865%` | `100%` | Restore the same nested helper objects for both animation groups. |
| `fn_1_26328` | `0x1D1C` | `94.029526%` | `100%` | Restore nested `fn_1_D058` helper captures, `>= count` wrap tests, and the target's positive-direction-first branch shape. |

The nine additions total `0x574C` target bytes. Each has the exact target
size, instruction bytes, and effective relocations under
`functionRelocDiffs=data_value`.

## Retained non-exact recovery

- `fn_1_18AA4` rises from `94.611984%` to `98.698740%` by restoring exact
  `fn_1_CA68` and the target-proven rotation intermediate.
- `fn_1_5324`, `fn_1_6324`, and `fn_1_737C` rise from
  `96.205130%`/`96.215910%`/`95.271960%` to
  `98.586890%`/`98.590910%`/`98.776210%` by restoring the shared
  `fn_1_22A8` Bezier/model-facing helper boundary.
- `fn_1_12FD8` rises from `94.362686%` to `98.534590%` through the same
  authenticated `s16` conversions and recovered prototype used by the two
  newly exact selection functions.
- The nested `fn_1_C200`/`fn_1_C158` recovery also improves `fn_1_10518` to
  `96.899300%` and `fn_1_150D0` to `99.336870%`.
- Matching `mdseldll` authenticates the `pos`, `modelPos`, `modelRot` order
  retained in `fn_1_22A8`; its standalone score rises to `99.548740%`.

These functions remain WIP and receive no Matching credit.

## Rejected probes and bounded residue

Every rejected probe was removed before the retained object was generated:

- `#pragma dont_inline` restores a direct `fn_1_BB30` call in `fn_1_BBD8`,
  but regresses independently exact `fn_1_BB30` from `100%` to `92.238100%`.
- Removing five wrapper `inline` redeclarations is byte-neutral under the
  active `-inline auto` compilation and therefore proves no MP6 source change.
- `CharMotionAMemPGet(...) == NULL` is illegal for the authenticated `u32`
  `AMEM_PTR`; an explicit cast is byte-neutral. Spelling `-200` as
  `-200.0f` is also byte-neutral.
- The remaining near-exact menu-flow family has exact sizes, CFGs, opcodes,
  constants, and relocations. Its residue is shared nested-inline stack-slot
  chronology or register coloring. No authenticated macro/declaration owner
  is available, so no per-caller dummy locals, register binding, or slot
  forcing were admitted.

## Object verification

The retained relocation-aware evidence is:

`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave90_integrated_object.json`

It was generated with:

```powershell
rtk ninja -j1 build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk build/tools/objdiff-cli-windows-x86_64.exe diff -p . `
  -u mdpartydll/REL/mdpartydll/mdparty `
  -o build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave90_integrated_object.json `
  --format json-pretty -c functionRelocDiffs=data_value
```

The object reports all 258 functions and all `0x3F424` target text bytes
represented, with 227 functions and `0x28C6C` target bytes exact. The 31
remaining functions account for `0x167B8` target bytes. Together with exact
`stage.c`, the application/stage portion represents all 315 functions and all
`0x467FC` target bytes; 284 functions and `0x30044` bytes are exact.

## Full gate

After the retained source and documentation were frozen, the serialized full
build and explicit DTK checksum each reported `137 files OK`. Explicit
comparisons proved both `main.dol` and `mdpartydll.rel` byte-identical to their
originals. The final build left no `config/GP6E01/symbols.txt` hunk. The
application owner remains `C-not-yet-matched`; no configure flip or fallback
credit is part of this wave.
