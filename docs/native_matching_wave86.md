# Native matching Wave 86: mdparty selection flow

## Result

Wave 86 adds target-backed C for 26 previously unrepresented
`REL/mdpartydll/mdparty.c` functions. They account for `0xEA50` target text
bytes. Relocation-aware objdiff proves 8 functions and `0x3258` bytes exact;
the other 18 bodies remain compiled, non-exact C and are not counted as exact.

The application owner advances from 215/258 to 241/258 represented functions
and from `0x1ADAC` to `0x297FC` represented target text bytes. Exact application
evidence advances from 200 to 208 functions and from `0x152F8` to `0x18550`
bytes. The owner now has 17 unrepresented functions and 33 represented
residues, including the 15 residues retained before this wave.

`mdparty.c` remains `NonMatching` and fallback-linked. No application source
byte is linked into the final REL, and no fallback-linked byte is counted as
recovered source.

## Exact functions

Objdiff CLI 3.7.2, with `functionRelocDiffs=data_value`, reports 100% code and
effective-relocation agreement for these Wave 86 functions:

| Function | Target text |
| --- | ---: |
| `fn_1_17730` | `0x6E0` |
| `fn_1_228E8` | `0x5D0` |
| `fn_1_232A0` | `0x73C` |
| `fn_1_23AC0` | `0x5D0` |
| `fn_1_24A8C` | `0x6D4` |
| `fn_1_25160` | `0x5E0` |
| `fn_1_25740` | `0x5D0` |
| `fn_1_25D10` | `0x618` |
| **Total** | **`0x3258`** |

## Explicit WIP residues

These Wave 86 bodies are real C recovery, but they do not pass the object
gate and remain outside the exact count:

| Function | Target/source text | Relocation-aware score |
| --- | ---: | ---: |
| `fn_1_2B74` | `0x710 / 0x710` | 95.548676% |
| `fn_1_B748` | `0x3E8 / 0x3D4` | 94.696000% |
| `fn_1_BBD8` | `0x580 / 0x5F4` | 90.278410% |
| `fn_1_DA54` | `0x1120 / 0x1110` | 99.303830% |
| `fn_1_EB74` | `0xCC4 / 0xCD0` | 98.763770% |
| `fn_1_F838` | `0xCE0 / 0xCF8` | 97.833740% |
| `fn_1_1158C` | `0x920 / 0x918` | 99.143840% |
| `fn_1_12364` | `0x7B4 / 0x79C` | 96.612580% |
| `fn_1_12FD8` | `0xEE8 / 0xEB0` | 94.362686% |
| `fn_1_14510` | `0x7B0 / 0x790` | 96.674800% |
| `fn_1_156B4` | `0x9EC / 0x9E4` | 98.807880% |
| `fn_1_160A0` | `0x9EC / 0x9E4` | 98.807880% |
| `fn_1_18384` | `0x68C / 0x5E8` | 86.632460% |
| `fn_1_18AA4` | `0x9E8 / 0x980` | 94.611984% |
| `fn_1_30B4C` | `0x9BC / 0x9BC` | 99.871590% |
| `fn_1_31AC0` | `0xE44 / 0xE44` | 99.956190% |
| `fn_1_3736C` | `0xFA8 / 0xFA8` | 99.953094% |
| `fn_1_3EAC8` | `0x95C / 0x468` | 44.253757% |

Equal size or a high percentage is not accepted as byte proof. In particular,
`fn_1_30B4C`, `fn_1_31AC0`, and `fn_1_3736C` retain systematic register or
stack-slot allocation differences. `fn_1_3EAC8` remains structurally smaller
because its two exact large helpers are still emitted as calls rather than
target-shaped inline copies.

## Recovered source facts

The accepted source shape comes from target instructions, relocations, exact
helpers, and sibling framework evidence:

- `LBL_1_DATA_E12_ENTRY` has eight signed-16-bit direction links followed by a
  signed-16-bit active field. The target uses an `0x12` stride.
- `fn_1_12FD8` reads repeated directions through `HuPadDStkRep` and new A/B
  presses through `HuPadBtnDown`; the target references the two input globals
  independently.
- `fn_1_17730` owns separate `OMOBJ *` and sprite-member locals in its left and
  right branches. Restoring those branch scopes removes every register
  difference and proves the full `0x6E0` body exact.
- `fn_1_228E8` uses the authentic inline `fn_1_9B90` helper and the target's
  condition senses. `fn_1_232A0` uses authentic inline `fn_1_96E4`, the
  target's `>= 10` boundary, and the target pointer ownership. Both are exact.
- `fn_1_23AC0`, `fn_1_24A8C`, `fn_1_25160`, `fn_1_25740`, and `fn_1_25D10`
  recover the repeated player-selection transition family without shared fake
  abstractions; each independent function passes the object gate.
- `fn_1_3EAC8` uses the exact standalone `fn_1_34D10` and `fn_1_36264`
  helpers. The target inlines both bodies, but marking them inline at their
  first declarations suppressed the two standalone source symbols and reduced
  the full owner ledger to 239 represented functions. That shape was rejected;
  preserving the exact helper symbols keeps the state machine WIP at
  44.253757% instead of claiming a false 241/258 owner count.
- `fn_1_10518` remains unrepresented. The target does not yet prove how the
  animation index is divided between `fn_1_D058`'s `dataNo` and `dataOffset`
  arguments, so no equivalent-looking call was guessed.

No assembly, inline assembly, fake volatile, register-binding extension, byte
packet, source padding, or invented Matching configuration was introduced.

## Remaining unrepresented application functions

Seventeen functions and `0x15C28` target text bytes remain without accepted C:

| Function | Target text |
| --- | ---: |
| `fn_1_10518` | `0xFAC` |
| `fn_1_1B9E8` | `0x1D70` |
| `fn_1_1D758` | `0x10A0` |
| `fn_1_1E7F8` | `0x10C0` |
| `fn_1_1F8B8` | `0x1D70` |
| `fn_1_26328` | `0x1D1C` |
| `fn_1_2851C` | `0x107C` |
| `fn_1_29598` | `0x132C` |
| `fn_1_2A8C4` | `0x1314` |
| `fn_1_2BBD8` | `0xC9C` |
| `fn_1_2F22C` | `0x1920` |
| `fn_1_33724` | `0x15EC` |
| `fn_1_350F8` | `0x116C` |
| `fn_1_38A3C` | `0x1224` |
| `fn_1_39C60` | `0x1124` |
| `fn_1_3AD84` | `0x1124` |
| `fn_1_3CFE4` | `0x1AE4` |
| **Total** | **`0x15C28`** |

## Verification

The final object evidence is retained at
`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave86_final_object.json` and
was generated with:

```powershell
rtk ninja -j1 build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk ..\..\tools\objdiff\v3.7.2\objdiff-cli.exe diff -p . `
  -u mdpartydll/REL/mdpartydll/mdparty `
  -o build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave86_final_object.json `
  --format json-pretty -c functionRelocDiffs=data_value
```

The conservative container gate was then run without changing the
`NonMatching` owner:

```powershell
rtk ninja -j1
rtk build/tools/dtk.exe shasum -q -c config/GP6E01/build.sha1
rtk cmp.exe orig/GP6E01/sys/main.dol build/GP6E01/main.dol
rtk cmp.exe orig/GP6E01/files/dll/mdpartydll.rel `
  build/GP6E01/mdpartydll/mdpartydll.rel
```

Results:

- full Ninja checksum step: `137 files OK`;
- explicit DTK checksum: `137 files OK`;
- `main.dol`: byte-identical;
- `mdpartydll.rel`: byte-identical;
- no generated `symbols.txt` hunk remained after the final build.
