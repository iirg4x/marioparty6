# Native matching Wave 87: close the `mdparty` source holes

Wave 87 materializes 16 more target-backed functions in
`REL/mdpartydll/mdparty.c`.  The source now represents 257 of the target's
258 functions and `0x3DB04/0x3F424` target text bytes.  Only
`fn_1_2F22C` (`0x1920`) remains unrepresented.

This is still a WIP owner, not a clean-C promotion.  The object has 211 exact
functions (`0x1B82C`) and 46 represented residues (`0x222D8`), so
`mdparty.c` remains `NonMatching` and fallback-linked.

## Source evidence

The framework reference is the authenticated Mario Party 5 checkout at
commit `e246f9d9850ff53ac684b971068fbf87fdcf6acb`, especially
`src/REL/mdpartyDll/mdparty.c`, `stage.c`, `object.c`, and `effect.c`.
MP5 supplies the owner split, state-machine family, and plausible original
source boundaries.  MP6 target instructions, relocations, globals, constants,
and call order decide every MP6 body; sibling parity alone is not treated as
proof.

The recovered MP6 source uses the already-proven TU-local inline helpers for
window state, object callbacks, model interpolation, and transition cleanup.
That restores the original call-level source shape instead of preserving
decompiler-expanded ABI mechanics.

| Function | Target text | Objdiff | Recovered responsibility |
| --- | ---: | ---: | --- |
| `fn_1_10518` | `0xFAC` | 94.175476% | connected-controller count, bounded selector, arrows, and animation banks |
| `fn_1_1B9E8` | `0x1D70` | 99.033970% | four-panel entry and solo/team character placement |
| `fn_1_1D758` | `0x10A0` | 99.981200% | panel/character exit, shared UI reset, and cleanup |
| `fn_1_1E7F8` | `0x10C0` | 99.361010% | dual UI-state reset and exit transition |
| `fn_1_1F8B8` | `0x1D70` | 99.033970% | final panel and solo/team character placement |
| `fn_1_26328` | `0x1D1C` | 94.029526% | configuration navigation, wrapping, animation, and cursor positioning |
| `fn_1_2851C` | `0x107C` | 93.906160% | persisted configuration import and four-model entry |
| `fn_1_29598` | `0x132C` | **100%** | exact four-model transition |
| `fn_1_2A8C4` | `0x1314` | **100%** | exact four-model transition |
| `fn_1_2BBD8` | `0xC9C` | **100%** | exact four-model exit |
| `fn_1_33724` | `0x15EC` | 96.063440% | human-side configuration loop and completion bounds |
| `fn_1_350F8` | `0x116C` | 95.994620% | paired configuration loop and completion bounds |
| `fn_1_38A3C` | `0x1224` | 99.768300% | board availability/selection loop and cancel confirmation |
| `fn_1_39C60` | `0x1124` | 99.938010% | character confirmation loop with selected-character insertion |
| `fn_1_3AD84` | `0x1124` | 99.938010% | team-message confirmation loop |
| `fn_1_3CFE4` | `0x1AE4` | 99.941895% | four-state party-configuration summary loop |
| **Wave total** | **`0x14308`** | 3 exact | 16 newly represented functions |

The three exact functions contribute `0x32DC` target bytes.  The other 13
bodies remain explicit residues; their source is useful decompilation progress
but is not counted as byte-identical C.

`fn_1_2F22C` was deliberately not transcribed.  Its target control flow still
has unresolved helper boundaries and bitfield ownership, and the independent
decompiler view discarded four blocks.  Adding a body at that evidence level
would be speculative, so the target's last `0x1920` bytes remain an explicit
hole.

## Owner audit

The relocation-aware all-symbol audit uses target function sizes rather than
source-size estimates:

| State | Functions | Target text |
| --- | ---: | ---: |
| exact | 211 | `0x1B82C` |
| represented residue | 46 | `0x222D8` |
| unrepresented | 1 | `0x1920` |
| **target owner** | **258** | **`0x3F424`** |

Together with exact `stage.c`, the application/stage portion now represents
314 of 315 functions and `0x44EDC/0x467FC` target text bytes.  Of those, 268
functions and `0x22C04` bytes are exact.  The authenticated 19-function
compiler runtime remains a separate original-assembly owner and contributes
zero clean-C bytes.

## Verification

The final object evidence is retained at
`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave87_final_object.json` and
is generated with:

```powershell
rtk ninja -j1 build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk ..\..\tools\objdiff\v3.7.2\objdiff-cli.exe diff -p . `
  -u mdpartydll/REL/mdpartydll/mdparty `
  -o build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave87_final_object.json `
  --format json-pretty -c functionRelocDiffs=data_value
```

The conservative fallback-linked container gate is:

```powershell
rtk ninja -j1
rtk build/tools/dtk.exe shasum -q -c config/GP6E01/build.sha1
rtk cmp.exe orig/GP6E01/sys/main.dol build/GP6E01/main.dol
rtk cmp.exe orig/GP6E01/files/dll/mdpartydll.rel `
  build/GP6E01/mdpartydll/mdpartydll.rel
```

That gate reports `137 files OK`; both `main.dol` and `mdpartydll.rel` compare
byte-identical.  No configure flip was made, no assembly or fakematch device
was added, and generated `symbols.txt` has no retained hunk.
