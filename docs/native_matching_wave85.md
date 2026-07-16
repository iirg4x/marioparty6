# Native matching Wave 85: mdparty flow expansion

## Result

Wave 85 adds target-backed C for 34 previously unrepresented
`REL/mdpartydll/mdparty.c` functions. They account for `0xC2F8` target text
bytes. Relocation-aware objdiff proves 23 functions and `0x736C` bytes exact;
the other 11 bodies remain compiled, non-exact C and are not counted as exact.

The application owner advances from 181/258 to 215/258 represented functions
and from `0xEAB4` to `0x1ADAC` represented target text bytes. Exact application
evidence advances from 177 to 200 functions and from `0xDF8C` to `0x152F8`
bytes. The owner now has 43 unrepresented functions and 15 represented
residues, including the four residues retained before this wave.

`mdparty.c` remains `NonMatching` and fallback-linked. No application source
byte is linked into the final REL, and no fallback-linked byte is counted as
recovered source.

## Exact functions

Objdiff CLI 3.7.2, with `functionRelocDiffs=data_value`, reports 100% code and
effective-relocation agreement for these Wave 85 functions:

| Function | Target text |
| --- | ---: |
| `fn_1_5A18` | `0x598` |
| `fn_1_6A20` | `0x598` |
| `fn_1_9F50` | `0x688` |
| `fn_1_B008` | `0x4C0` |
| `fn_1_C28C` | `0x40C` |
| `fn_1_C698` | `0x3D0` |
| `fn_1_CA68` | `0x5F0` |
| `fn_1_11EAC` | `0x3B4` |
| `fn_1_13EC0` | `0x484` |
| `fn_1_14CC0` | `0x2A4` |
| `fn_1_16E84` | `0x4B8` |
| `fn_1_1948C` | `0x3D4` |
| `fn_1_19D24` | `0x400` |
| `fn_1_1A20C` | `0x6A4` |
| `fn_1_1A8B0` | `0x5B0` |
| `fn_1_1AE60` | `0x5A0` |
| `fn_1_1B400` | `0x5E8` |
| `fn_1_221F4` | `0x5F0` |
| `fn_1_24090` | `0x4EC` |
| `fn_1_2457C` | `0x428` |
| `fn_1_2E638` | `0x5B4` |
| `fn_1_31508` | `0x5B8` |
| `fn_1_3CA70` | `0x574` |
| **Total** | **`0x736C`** |

## Explicit WIP residues

These Wave 85 bodies are real C recovery, but they do not pass the object
gate and remain outside the exact count:

| Function | Target/source text | Relocation-aware score |
| --- | ---: | ---: |
| `fn_1_4A84` | `0x458 / 0x480` | 93.219420% |
| `fn_1_5324` | `0x57C / 0x560` | 96.603990% |
| `fn_1_6324` | `0x580 / 0x564` | 96.613640% |
| `fn_1_737C` | `0x584 / 0x568` | 95.271960% |
| `fn_1_D058` | `0x388 / 0x380` | 98.433630% |
| `fn_1_150D0` | `0x5E4 / 0x5DC` | 98.785150% |
| `fn_1_19860` | `0x4C4 / 0x4AC` | 93.301636% |
| `fn_1_329A4` | `0xC4C / 0xC4C` | 99.968230% |
| `fn_1_36518` | `0xD48 / 0xD48` | 99.970590% |
| `fn_1_38314` | `0x728 / 0x728` | 97.368996% |
| `fn_1_3BEA8` | `0xBC8 / 0xBC8` | 99.874010% |

The two near-exact late-flow bodies retain systematic compiler slot-coloring
residues. They were not promoted on equal size or high percentage. The larger
animation and callback residues likewise remain source-only until their full
instruction and relocation streams match.

## Recovered source facts

The accepted source shape comes from target instructions and existing exact
helpers, not black-box register forcing:

- `fn_1_150D0` selects animation slot 3. The target accesses `animId[3]` at
  offset `0x8`; the initial source candidate incorrectly used slot 2.
- `fn_1_5A18` and `fn_1_6A20` read and increment `OMOBJ::work[0]` at offset
  `0x4C`. Their callers initialize the same field. Using `work[3]` emitted
  offset `0x58` and was rejected.
- `fn_1_9F50` resets `HUSPR_ATTR_LINEAR` (`0x8`), exactly matching the target
  immediate. `HUSPR_ATTR_DISPOFF` (`0x4`) was rejected.
- `fn_1_B008` directly chain-assigns the interpolated scale to the three vector
  components. Removing the unretained scalar temporary restores the target
  stack layout and store order; explicit signed-16-bit member arguments close
  the two remaining call-boundary differences.
- `fn_1_1A20C` retains the player bank in a signed-16-bit local across the
  inlined group-attribute loop. That live value accounts for the target's
  additional saved register and removes the recomputed global load.
- `fn_1_D678` returns `s16`, not `BOOL`. The standalone function remains exact,
  while its inlined callers now reproduce the target halfword spill/load ABI.
- `fn_1_24090` uses the existing exact `fn_1_9E50` sprite-bank helper. Its two
  inlined copies account for the target's parameter captures and complete
  `0x4EC` body.
- `fn_1_38314` uses the existing `fn_1_4734` and `fn_1_D6B0` helpers for group
  hiding and callback completion. The remaining function-level residue is
  retained rather than disguised.
- `fn_1_D058` uses two authentic `fn_1_C200` animation-reset calls and compares
  the selected absolute rotation directly. This materially improves the
  source shape but does not yet satisfy the byte gate.
- `fn_1_329A4`, `fn_1_36518`, and `fn_1_3BEA8` initialize and reuse their
  signed-16-bit results as shown by the target register chronology. Equal-size
  post-recovery objects still have slot/register residues, so all three remain
  WIP.

No assembly, inline assembly, fake volatile, register-binding extension, byte
packet, source padding, or invented Matching configuration was introduced.

## Verification

The final object evidence is retained at
`build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave85_final_object.json` and
was generated with:

```powershell
rtk ninja -j1 build/GP6E01/src/REL/mdpartydll/mdparty.o
rtk ..\..\tools\objdiff\v3.7.2\objdiff-cli.exe diff -p . `
  -u mdpartydll/REL/mdpartydll/mdparty `
  -o build/GP6E01/src/REL/mdpartydll/mdparty_objdiff_wave85_final_object.json `
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
