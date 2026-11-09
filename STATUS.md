# Mario Party 6 recovery status

The current state of the supported `GP6E01` build on this branch. This page is regenerated from `progress/report.json` whenever recovered sources change; the history of how it got here is in the commits and pull requests.

Updated 2026-11-09 19:21 UTC.

## Progress

| Area | Code | Data |
| --- | ---: | ---: |
| Entire project | 83.25% | 68.46% |
| Main DOL | 91.95% | 78.94% |
| REL modules | 81.40% | 61.82% |

## REL modules

**111 of 128** modules have all of their code recovered as matching source.

Still partial (14):

| Module | Code | Functions |
| --- | ---: | ---: |
| Granite Getaway (`m605DLL`) | 55.0% | 387 of 760 |
| Cog Jog (`m624DLL`) | 49.1% | 283 of 654 |
| Sumo of Doom-o (`m653DLL`) | 48.1% | 327 of 697 |
| Cash Flow (`m623DLL`) | 47.3% | 379 of 813 |
| Pitifall (`m655DLL`) | 45.2% | 320 of 743 |
| Tricky Tires (`m610DLL`) | 41.1% | 381 of 859 |
| Light Breeze (`m639DLL`) | 41.0% | 279 of 645 |
| Pit Boss (`m671DLL`) | 36.7% | 235 of 609 |
| Treasure Trawlers (`m611DLL`) | 35.8% | 278 of 767 |
| Dark 'n Crispy (`m673DLL`) | 34.9% | 262 of 682 |
| Dust 'til Dawn (`m634DLL`) | 33.2% | 241 of 678 |
| Surge and Destroy (`m628DLL`) | 32.1% | 265 of 769 |
| Seer Terror (`m677DLL`) | 28.7% | 318 of 1079 |
| What Goes Up... (`m604DLL`) | 11.7% | 93 of 449 |

Not recovered here yet (3): `micquizishidll`, `micquizohdedll`, `mikeactdll`.

## Main DOL

**5547 of 5915** functions are recovered as matching source; 368 remain, all in files that are not yet matching as a whole:

- `gssdk_lib/asrpho`: 361
- `gssdk_lib/gsapi`: 7

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
