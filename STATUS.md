# Mario Party 6 recovery status

The current state of the supported `GP6E01` build on this branch, from the same snapshot as the [progress site](https://iirg4x.github.io/marioparty6/). It is regenerated whenever recovered sources change; the history of how it got here is in the commits and pull requests.

Updated 2026-11-23 04:27 UTC.

## Progress

| Area | Code | Data |
| --- | ---: | ---: |
| Entire project | 98.49% | 99.75% |
| Main DOL | 91.35% | 99.35% |
| REL modules | 100.00% | 100.00% |

## REL modules

**128 of 128** modules are complete: every byte is owned by a matching source file (8 more modules are empty).

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
