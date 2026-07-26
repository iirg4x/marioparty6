# GC/2.6 board player value-lifetime evidence

`src/board/player.c` is compiled with GC/2.6 using the board profile's final
`-O0,p` options. Two independently non-matching functions became strict exact
without pragmas or code-generation controls when the source restored explicit
value captures at the target's apparent use boundaries.

## Confirmed local results

- `PlayerTurn` became strict exact at 1,380 bytes. Explicit captures of
  `GwSystem.partyF` at three separate use sites, with `timeTurn` captured only
  after the short-circuit party/player checks, restored the target saved-register
  and signed-comparison lifetimes.
- `PlayerMetalOMExec` became strict exact at 816 bytes. Pointer truthiness plus
  an explicit terminal return restored the target compare and control-flow
  shape.
- Together the retained changes added 2 strict functions and 192 verified
  relocation-bearing target instructions while preserving all 138 prior exact
  functions.

The integrated object reports 140/165 strict functions. Public recovery checks
pass 78/78, all 137 retail outputs pass DTK checksum, and `main.dol` remains
byte-identical with SHA-1 `b897e6ade6b3a0cd2f9907689f38a3b19c327e70`.

## Boundary and counterexamples

This evidence is an owner-local contextual heuristic, not a general instruction
to introduce temporaries. Capsule return-shape probes and an
`mbPlayerMoveMain` lifetime simplification regressed output and were reverted.
Literal-pool-only BiriQ residuals were deliberately left untouched.

Use the heuristic only when repeated global or bitfield reads accompany a
saved-register or signed-comparison mismatch. Test each capture independently,
compare the complete exact-function set, and retain it only when strict
relocations and natural source structure also improve.
