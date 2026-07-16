# Native Matching Wave 88 -- complete `mdparty.c` function coverage

Wave 88 closes the last unrepresented `REL/mdpartydll/mdparty.c` function and
proves four large party-flow functions exact.  The owner now has source for all
258 target functions and all `0x3F424` target text bytes.  It remains
`NonMatching`: 43 represented functions still have real compiler-shape
residue, so the fallback-linked owner is not counted as decompiled source.

## Evidence basis

The recovery uses three independent evidence layers:

- MP6 target instructions, relocations, DTK split boundaries, and object-use
  analysis are authoritative.
- The authenticated MP5 framework at
  `marioparty5/src/REL/mdpartyDll` commit
  `e246f9d9850ff53ac684b971068fbf87fdcf6acb` supplies sibling source shape for
  party-mode flow, signed character-number staging, and wait/helper macros.
- MWCC's recovered `StackFrame.c`, `Coloring.c`, and `CInline.c` explain the
  observed local-list, register-color, argument-conversion, and nested-inline
  behavior.  Compiler behavior is used to choose among target-equivalent C
  shapes, never to justify a fake local or forced register.

The project pins DTK `v0.9.2` (`4d039140f2d2ed80572b1949b76a5ff9b3094e06`).
Its generated symbol boundary fixes `fn_1_2F22C` at `.text:0x2F22C`, size
`0x1920`, followed by `fn_1_30B4C` at `0x30B4C`.  The owner split remains the
single `mdparty.c` text range `0x0..0x3F424`.

## Last missing function

`fn_1_2F22C` is recovered as helper-level C rather than a transcription of
expanded ABI mechanics.  The target proves the save-enable branch, import from
`GwSystem`, board-state setup, continuation prompt, accept path, both
cancel-confirm loops, warning loop, and final cleanup.

| Evidence | Result |
| --- | ---: |
| target text | `0x1920` |
| compiled source text | `0x1918` |
| relocation-aware objdiff | `96.559700%` |
| semantic relocation targets | `62/62` agree |
| actual paired ELF relocations | `601` |
| paired `R_PPC_NONE` provenance rows | `311` |
| total paired relocation/provenance records | `912` |
| same-symbol paired records | `777` |
| pooled-literal data-equivalent records | `135` |

The CFG and every non-move opcode align.  The remaining structural residue is
ten `mr` instructions (six target-only and four source-only); the other
differences are register and stack coloring.  Seven unpaired `R_PPC_NONE`
provenance records occur on that move-only residue.  No neighboring function
score changes.  Those moves were not chased with speculative temporaries.

## Exact signed-width recovery

Six direct `(s16)entry->chrSel` argument nodes recover the original signed
width at the `fn_1_C698` and `fn_1_CA68` inline boundaries.  Adjacent exact MP6
functions emit the same `lha`/`extsh`/`mulli` chronology, and MP5 stages the
same semantic character number through an `int charNo`.  All four affected
functions now have zero relocation-aware differences:

| Function | Target text | Result |
| --- | ---: | ---: |
| `fn_1_1B9E8` | `0x1D70` | 100% |
| `fn_1_1D758` | `0x10A0` | 100% |
| `fn_1_1E7F8` | `0x10C0` | 100% |
| `fn_1_1F8B8` | `0x1D70` | 100% |
| **total** | **`0x5C40`** | **100%** |

An outer `s32 modelNo` staging probe was rejected: it added `0x4..0x8` bytes
per function and reduced every score.  The direct signed-width expression is
the byte-proven source shape; no extra local remains.

## Control-flow correction

`fn_1_38A3C` now expresses the target's unavailable-board branch directly:
the disabled case plays the rejection sound and continues, while the enabled
case falls through to acceptance.  This preserves semantics but recovers the
target branch orientation.

| Function | Before | After | Remaining evidence |
| --- | ---: | ---: | --- |
| `fn_1_38A3C` (`0x1224`) | `99.768300%` | `99.955210%` | 52 stack-slot operands |

A shared `fn_1_4258` to `fn_1_39AC` nested-helper probe was also rejected.
It kept `fn_1_4258` exact and improved four callers, but structurally regressed
`fn_1_38314`; none of that mixed probe remains.

Two additional source-shape probes were rejected without retention.  Folding
the `fn_1_30B4C` assignment into its condition was byte-neutral: the function
remained `0x9BC/0x9BC`, `99.871590%`, with the same 20 operand differences.
Expanding the exact `fn_1_3CA0` body one level shallower inside `fn_1_4020`
regressed exact `fn_1_4020` and every exact direct caller.  The regression
supports retaining the byte-exact helper boundary; the unretained negative
probe is not used as original-source proof.  Replacing the final manual
confirmation sequence in `fn_1_3CFE4` with exact helper `fn_1_2EF24` was also
rejected: it grew the source from `0x1AE4` to `0x1AE8`, enlarged the frame from
`0xD0` to `0xE0`, and reduced the score from `99.941895%` to `99.468910%`.

## Owner ledger

| `mdparty.c` state | Functions | Target text bytes |
| --- | ---: | ---: |
| exact C | 215 | `0x2146C` |
| represented residue | 43 | `0x1DFB8` |
| unrepresented | 0 | `0x0` |
| **target owner** | **258** | **`0x3F424`** |

Together with exact `stage.c`, the application/stage portion represents all
315 functions and all `0x467FC` target text bytes.  Of those, 272 functions and
`0x28844` bytes are exact.  The authenticated 19-function compiler runtime is
a separate original-assembly owner and contributes zero clean-C bytes.

## Object verification

The integrated relocation-aware object evidence is retained at
`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave88_integrated_object.json`
and is generated with:

```powershell
rtk ninja -j1 build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk ..\..\tools\objdiff\v3.7.2\objdiff-cli.exe diff -p . `
  -u mdpartydll/REL/mdpartydll/mdparty `
  -o build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave88_integrated_object.json `
  --format json-pretty -c functionRelocDiffs=data_value
```

That object proof reports 258/258 represented functions, 215 exact functions,
and `0x2146C` exact target text bytes.  No configure flip, assembly,
fakematch device, source padding, or generated-symbol edit is part of this
wave.

## Full gate

The final serialized project gate passed after the accepted source and this
handoff were frozen:

```powershell
rtk ninja -j1
rtk build\tools\dtk.exe shasum -q -c config\GP6E01\build.sha1
rtk cmp.exe orig\GP6E01\sys\main.dol build\GP6E01\main.dol
rtk cmp.exe orig\GP6E01\files\dll\mdpartydll.rel `
  build\GP6E01\mdpartydll\mdpartydll.rel
```

Both the Ninja checksum step and the explicit DTK checksum report
`137 files OK`.  Both `cmp` commands exit zero: `main.dol` and
`mdpartydll.rel` are byte-identical to the originals.  The final build leaves
no `config/GP6E01/symbols.txt` hunk.
