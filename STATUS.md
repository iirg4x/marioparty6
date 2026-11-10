# Mario Party 6 recovery status

The current state of the supported `GP6E01` build on this branch, from the same snapshot as the [progress site](https://iirg4x.github.io/marioparty6/). It is regenerated whenever recovered sources change; the history of how it got here is in the commits and pull requests.

Updated 2026-11-10 12:53 UTC.

## Progress

| Area | Code | Data |
| --- | ---: | ---: |
| Entire project | 83.14% | 93.15% |
| Main DOL | 91.35% | 99.35% |
| REL modules | 81.40% | 89.22% |

## REL modules

**110 of 128** modules are complete: every byte is owned by a matching source file (8 more modules are empty).

Partial (15):

| Module | Code | Data |
| --- | ---: | ---: |
| Full Tilt (`m652DLL`) | 100.00% | 100.00% |
| Granite Getaway (`m605DLL`) | 55.01% | 74.15% |
| Cog Jog (`m624DLL`) | 49.14% | 54.76% |
| Sumo of Doom-o (`m653DLL`) | 48.09% | 75.92% |
| Cash Flow (`m623DLL`) | 47.32% | 61.53% |
| Pitifall (`m655DLL`) | 45.17% | 65.54% |
| Tricky Tires (`m610DLL`) | 41.10% | 70.81% |
| Light Breeze (`m639DLL`) | 41.04% | 66.44% |
| Pit Boss (`m671DLL`) | 36.65% | 53.77% |
| Treasure Trawlers (`m611DLL`) | 35.82% | 46.02% |
| Dark 'n Crispy (`m673DLL`) | 34.90% | 49.53% |
| Dust 'til Dawn (`m634DLL`) | 33.16% | 46.32% |
| Surge and Destroy (`m628DLL`) | 32.12% | 65.00% |
| Seer Terror (`m677DLL`) | 28.74% | 41.87% |
| What Goes Up... (`m604DLL`) | 11.69% | 42.42% |

Not recovered here yet (3): `micquizishidll`, `micquizohdedll`, `mikeactdll`.

## Main DOL

**5438 of 5914** functions are recovered as matching source; 476 remain:

- `gssdk_lib/asrpho`: 446
- `gssdk_lib/gsapi`: 30

## Verification

Every configured output must match retail:

```sh
ninja
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

On Windows, use `build/tools/dtk.exe`. All 137 configured outputs pass the retail checksum gate, and the rebuilt `main.dol` is byte-identical to retail (SHA-1 `b897e6ade6b3a0cd2f9907689f38a3b19c327e70`).

## Recovery standard

A source file is marked Matching only when the configured compiler reproduces the target output. Matching is necessary, but recovered code should also remain readable and supported by evidence from the original game.

Unknown names or fields stay explicit until their meaning is known. Compiler-specific constructs are not added solely to force a match unless the target clearly requires them.
