# Native Matching Wave 93 -- board easy-win batch

Wave 93 closes every authenticated low-risk residue found in the bounded
`board/` audit. Four owners gain 23 exact target functions and `0x85C` exact
target text bytes without assembly, fake locals, register forcing, or a
configure flip. The owners remain `NonMatching` because each still has
unrepresented or non-exact target functions.

## Recovered source shape

### `shopevent.c`

`mbev_ShopBackMotCreate` is exact at `0x2AC`. Its target `0x90` frame places
the local motion-data table at `r1+0x24` and reserves exactly `0x40` bytes
before the saved-register area, proving 16 `int` entries rather than two. The
case-4 target sequence also preserves the sign-extended model ID in a saved
register before `mbObjAttrSet`; a case-local `MBMODELID` reproduces that
lifetime. The owner advances from 10 to 11 exact paired functions.

### `telop.c`

Five functions become exact:

- `TelopTimePauseHook`, `mbTauntClose`, and `mbTelopTimeChangeKill` use direct
  pointer truth tests, reproducing the target `lwz`/`cmplwi` nodes;
- `mbPadStkXGet` and `mbPadStkYGet` again call external `abs`. The local
  `stdlib.h` macro had expanded those calls to `__abs`, while the target has
  three `R_PPC_REL24 abs` relocations in each function. MP5's Matching
  `src/game/board/telop.c` authenticates both the direct truth-test idiom and
  the external-`abs` source context.

The owner advances from 10 to 15 exact paired functions and retains only
`mbBoardDataDirRead` as a represented residue.

### `capspecial.c`

The three Teresa fade helpers become exact by restoring the target's direct
pointer truth tests. `ev_CapKoopaDiceMotHook` becomes exact after recovering
the target do/while CFG and post-increment comparison: the counter advances
before the bottom motion-end test, and dice hit 27 is emitted from the body.
All 13 represented functions in this owner are now exact.

### `capsule.c`

Target instruction and call graphs recover the retained value-helper shapes,
the saved `capsuleUseEffMode` local, and the signed capsule-number lifetime.
MP5's Matching `MBCapsuleThrowCheck` authenticates the explicit two-branch
throw predicate. These six direct corrections also close the inlined
`mbCapMasuPlayerTypeSet` caller and align the existing helper/stub islands, so
the final owner comparison
advances from 18 to 31 exact paired functions (`0xD88` to `0xFA8` exact
bytes). The two remaining represented functions are still non-exact; the
owner is far from complete and remains fallback-linked.

## Rejected probes

Three missing `config.c` panel helpers were reconstructed only far enough to
test their target semantics. The ordinary struct-copy, `memcpy`, and
`HuCopyVecF` spellings all failed the target's paired-single copy shape;
`mbPausePanelAnmNoSet` retained a one-register return-contract mismatch. None
of those probes remains in source.

## Object verification

The four retained relocation-aware reports are:

- `build/GP6E01/src/board/board_wave93_shopevent_final.json`
- `build/GP6E01/src/board/board_wave93_telop_final.json`
- `build/GP6E01/src/board/board_wave93_capspecial_final.json`
- `build/GP6E01/src/board/board_wave93_capsule_final.json`

They were generated after serial object builds with
`functionRelocDiffs=data_value`. Final per-owner evidence is:

| owner | paired | exact | exact target bytes |
| --- | ---: | ---: | ---: |
| `shopevent.c` | 11 | 11 | `0xC80` |
| `telop.c` | 16 | 15 | `0x6A4` |
| `capspecial.c` | 13 | 13 | `0x1D8` |
| `capsule.c` | 33 | 31 | `0xFA8` |

No previously exact function regressed. No owner became complete, so this
slice does not change `configure.py`, does not claim Matching-owner bytes, and
does not rerun or claim the full container gate.
