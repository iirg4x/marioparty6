# Native Matching Wave 94 -- tutorial lifetimes and capsule-event extent

Wave 94 closes five `tutorial.c` functions and recovers one real
`capevent.c` global extent. The five functions add `0x620` exact target text
bytes; the tutorial owner advances from 35 to 40 exact paired functions with
no exact regression. Both owners remain `NonMatching` and fallback-linked.

## `tutorial.c` recovery

### `TutorialWatch`

The target gives the loop-local controller input and the later party-mode
value different saved-register lifetimes. Moving `partyF` into the wipe block
reproduces that ownership, while testing `tutorialGuideObj` directly removes
the source-only zero temporary. The function is exact at `0x208`.

### `mbTutorialViewSet`

The target narrows the space result when it is captured and repeats that
narrowing at the camera argument and return boundaries. The retained `int`
local with an explicit `s16` capture reproduces all three `extsh` operations.
The function is exact at `0xA4`.

### Guide-object call sites

`mbTutorialWinMesExec`, `mbTutorialWinCreate`, and
`mbTutorialWinKeyWait` now retain scoped `OMOBJ *guideObj` locals. Their target
prologues reserve the additional saved GPR, and the target loop back edges in
the first and third functions return to the global load rather than the
following move. Keeping those locals inside the loop blocks reproduces both
the lifetime and reload point. The three functions are exact at `0x184`,
`0xDC`, and `0x114` respectively.

## `capevent.c` global recovery

The target `ev_CapEffMoveOMObj` symbol is `0x20` bytes, proving eight pointers;
the former `GW_PLAYER_MAX` declaration emitted only `0x10`. Restoring the
eight-entry extent makes that global exact and agrees with the six adjacent
eight-entry capsule-effect object tables. All 83 previously exact represented
functions remain exact. The owner still has unresolved `.bss` order/extent and
unrepresented event code, so no owner-level claim is made.

## Rejected easy-win probes

- `board.c`'s three remaining paired functions contain only authenticated
  register-color cycles; the known edits are neutral or regress an exact
  inline/standalone counterpart.
- The apparent `effect.c` boolean-source fixes are collapsed by the owner's
  `-O4,p` optimizer and do not reproduce the target CFG. Widening
  `mbParManPosSet` emits the missing `extsh`, but schedules it before the LR
  spill instead of after it. None of those probes remains.
- `ev_Last5Coin40` needs an initialized but unread stack word with no sibling
  authentication, and `mbWipeSpecialFadeOutCreate` has only a ghost saved-GPR
  residue. Neither was changed.
- `single.c` and the last `opening.c` function have no unique easy source
  shape; their add-order and inline/allocator residues were left alone.

## Object verification

The retained relocation-aware reports are:

- `build/GP6E01/src/board/board_wave94_tutorial_final.json`
- `build/GP6E01/src/board/board_wave94_capevent_final.json`

Both were generated after serial object builds with
`functionRelocDiffs=data_value`. They prove 40 exact paired tutorial functions,
five newly exact functions totaling `0x620`, an exact `0x20`
`ev_CapEffMoveOMObj`, and no exact-function regressions. No owner became
complete, so this slice does not change `configure.py` and does not rerun or
claim the full container gate.
