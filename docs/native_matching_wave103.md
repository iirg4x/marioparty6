# Native matching Wave 103: `mdparty` declaration ownership

Date: 2026-07-17

## Scope

Wave 103 audits the unusually large declaration block in
`REL/mdpartydll/mdparty.c`, assigns every direct cross-owner declaration in the
governed C sources to a real header or an explicit target-backed exception,
and adds a configure-time gate that prevents the block from becoming a
matching shortcut.

This wave clarifies ownership and source shape. It does not add recovered text
bytes: `mdparty.c` and `stage.c` were already byte-exact before the cleanup.
Address-based function and object names remain semantic name-recovery debt.

## Declaration audit

The previous `mdparty.c` had 82 declarations carrying the `extern` keyword:

| Class | Count | Resolution |
| --- | ---: | --- |
| Objects defined later in the same translation unit | 73 | Moved to the private owner-declaration header `REL/mdpartyDll_globals.h`; every object is still defined exactly once in `mdparty.c`. |
| Linker-provided labels | 2 | `_ctors` and `_dtors` remain explicit, REL-symbol-authenticated exceptions. |
| DOL imports | 7 | Three now come from their real owner headers; four audio imports retain target-proven translation-unit visibility and exact authoritative signatures. |

The file also had 146 forward function prototypes. Of those, 138 name
functions defined in `mdparty.c`; the eight functions owned by `stage.c` now
live in the public `REL/mdpartyDll.h` interface.

The current `mdparty.c` therefore has six explicit `extern` declarations:
the two linker labels and four audio imports. The 73 same-owner declarations
have not been hidden to manufacture a gate result. The private header is a
compiler-visibility contract, and the new gate proves target authority and
one real owner definition for every entry.

## Recovered ownership

- `HuDataDirCloseAll` now comes from `game/data.h`.
- `mbSaveInit` and `mbSavePartyInit` now come from `game/board/main.h`.
- MP5's board implementation and mdparty caller authenticate the
  `mbSavePartyInit` argument meanings as `teamF`, `bonusStarF`, minigame pack,
  turn count, and four handicaps. The MP6 definition and header now use those
  types and names; its complete `0x184` target body remains exact.
- `HuAudFXPlayPan` now uses the authoritative `s16 pan` contract instead of
  the reconstructed `int` declaration.
- `stage.c` owns its data/BSS directly and exposes only the eight functions
  consumed by `mdparty.c` through the module header.

The full audio header is deliberately not included by `mdparty.c`. Wave 101
proved that the retail translation unit did not see the
`HuAudSStreamFadeOut` prototype. Adding `game/audio.h` changes
`fn_1_2F22C` by eight bytes, so the four declarations that this translation
unit did see are retained locally and checked byte-for-byte against
`game/audio.h`.

Likewise, `stage.c` retains exact signed declarations for `frandmod` and
`rand8`. Broad random-function headers change five target functions. These
two plain prototypes are now explicit policy entries with DOL symbol
authority; a prototype without the `extern` keyword cannot bypass the gate.

## Compiler-shape evidence

Moving the 73 `mdparty.c` definitions before the functions is not an
equivalent cleanup under GC/1.3.2. It changes the target compiler's one-pass
visibility and register allocation even when an earlier header declaration is
kept:

| Probe report | Result |
| --- | --- |
| `mdparty_objdiff_wave103_header_early.json` | Rejected: source `.text` becomes `0x39BD4` instead of `0x3F424`; only 154/258 functions remain exact, and `.bss` becomes `0xA84` instead of `0xA80`. |
| `stage_objdiff_wave103_header_early.json` | Accepted: all 57 functions and `0x73D8` text bytes remain exact with early stage-owned definitions. |
| `mdparty_objdiff_wave103_header.json` | Rejected full-audio-header form: 257/258 functions remain exact; `fn_1_2F22C` is eight bytes short. |
| `stage_objdiff_wave103_header.json` | Rejected broad-random-header form: five functions diverge and source text becomes `0x736C` instead of `0x73D8`. |

The accepted shape therefore keeps the `mdparty.c` definitions at the tail,
where their already-proven reverse definition chronology produces the retail
data/BSS layout. Stage definitions move to their natural owner-local position
because that form independently re-proves exact.

## Declaration gate

`tools/check_tu_declarations.py` runs before `configure.py` writes Ninja files.
The governed sources, policy, module headers, and owner headers are also Ninja
reconfiguration dependencies. DTK-mutated `symbols.txt` files are read by the
gate but intentionally are not timestamp dependencies, avoiding a configure
loop.

The gate enforces all of the following:

- every C-source explicit `extern` has an exact allowlisted declaration and
  authority in the correct DOL or mdpartydll REL symbol file;
- every private globals-header entry is a target-backed object with exactly
  one actual file-scope definition, not merely an identifier reference;
- every plain file-scope prototype either has one same-TU definition or is an
  exact target-backed import exception;
- the public module header contains exactly its eight policy functions, each
  with one source definition and REL function authority;
- comma-packed extern/import/owner declarations, unreviewed direct includes
  in governed sources and private/public module headers, C-local
  redeclarations of owned APIs, and checked import/owner signature drift are
  rejected.

Sixteen regression tests include negative cases for plain import prototypes,
initializer-only and tag-only fake global ownership, missing module
definitions, comma-packed and duplicate externs, phase-two line-splice
spelling, nested public-header import smuggling, and unreviewed headers:

```powershell
rtk python -m unittest tools.tests.test_tu_declarations
rtk python tools/check_tu_declarations.py `
  --policy config/GP6E01/tu_declarations.json
```

Both commands pass, and the standalone gate reports
`declaration gate: mdpartydll ownership OK`.

## Object proof

The final relocation-aware reports use
`-c functionRelocDiffs=data_value`:

- `build/GP6E01/mdpartydll/mdparty_objdiff_wave103_ownership.json` proves
  258/258 exact functions and all `0x3F424` target text bytes.
- `build/GP6E01/mdpartydll/stage_objdiff_wave103_ownership.json` proves 57/57
  exact functions and all `0x73D8` target text bytes; its data sections are
  also exact.
- `build/GP6E01/board_board_wave103_contract.json` proves the edited
  `mbSavePartyInit` function exact at `0x184`. The still-NonMatching board
  owner is not claimed exact as a whole.

Objdiff's mdparty `.bss` symbol-extent score remains affected by known natural
alignment accounting, and the source object's semantic initialized data ends
before the target section's three alignment bytes. Only the linked REL gate
is used to claim complete container equality.

## Full gate

The final serialized gate is:

```powershell
rtk ninja -j1
rtk build/tools/dtk.exe shasum -q -c config/GP6E01/build.sha1
rtk cmp orig/GP6E01/sys/main.dol build/GP6E01/main.dol
rtk cmp orig/GP6E01/files/dll/mdpartydll.rel `
  build/GP6E01/mdpartydll/mdpartydll.rel
rtk git diff --exit-code HEAD -- config/GP6E01/symbols.txt
```

The build and checksum report `137 files OK`. Both `cmp` commands exit zero,
so `main.dol` and `mdpartydll.rel` are byte-identical to retail. The final
build leaves `config/GP6E01/symbols.txt` unchanged.
