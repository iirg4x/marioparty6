# Mario Party 6 recovery status

The current state of the supported `GP6E01` build on this branch, from the same snapshot as the [progress site](https://iirg4x.github.io/marioparty6/). It is regenerated whenever recovered sources change; the history of how it got here is in the commits and pull requests.

Updated 2026-11-11 19:16 UTC.

## Progress

| Area | Code | Data |
| --- | ---: | ---: |
| Entire project | 86.56% | 94.61% |
| Main DOL | 91.35% | 99.35% |
| REL modules | 85.54% | 91.61% |

## REL modules

**113 of 128** modules are complete: every byte is owned by a matching source file (8 more modules are empty).

Partial (14):

| Module | Code | Data |
| --- | ---: | ---: |
| Granite Getaway (`m605DLL`) | 59.42% | 74.52% |
| Sumo of Doom-o (`m653DLL`) | 53.24% | 76.33% |
| Cash Flow (`m623DLL`) | 51.09% | 61.83% |
| Cog Jog (`m624DLL`) | 50.85% | 54.76% |
| Pitifall (`m655DLL`) | 46.59% | 65.54% |
| Tricky Tires (`m610DLL`) | 44.62% | 71.16% |
| Light Breeze (`m639DLL`) | 42.59% | 66.44% |
| Treasure Trawlers (`m611DLL`) | 40.00% | 46.46% |
| Dark 'n Crispy (`m673DLL`) | 39.81% | 50.02% |
| Pit Boss (`m671DLL`) | 38.26% | 53.77% |
| Dust 'til Dawn (`m634DLL`) | 38.04% | 46.80% |
| Surge and Destroy (`m628DLL`) | 36.52% | 65.39% |
| Seer Terror (`m677DLL`) | 31.85% | 42.10% |
| What Goes Up... (`m604DLL`) | 13.56% | 42.42% |

Not recovered here yet (1): `mikeactdll`.

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
