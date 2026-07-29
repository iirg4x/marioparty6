# GC/2.6 board player value-lifetime evidence

`src/board/player.c` is compiled with GC/2.6 using the board profile's final
`-O0,p` options. An initial five independently non-matching functions became
strict exact without pragmas or code-generation controls when the source
restored explicit value captures at the target's apparent use boundaries.

## Confirmed local results

- `PlayerTurn` became strict exact at 1,380 bytes. Explicit captures of
  `GwSystem.partyF` at three separate use sites, with `timeTurn` captured only
  after the short-circuit party/player checks, restored the target saved-register
  and signed-comparison lifetimes.
- `PlayerMetalOMExec` became strict exact at 816 bytes. Pointer truthiness plus
  an explicit terminal return restored the target compare and control-flow
  shape.
- `mbev_PlayerColMasuAllSet` became strict exact at 740 bytes. Removing cached
  `PLAYERCOLWORK` pointers and restoring the target-backed `BOOL`/declaration
  order recreated the target allocation and call sequence.
- `mbPlayerMatClone` became strict exact at 160 bytes. Staging the raw allocation,
  typed material, and active clone pointer separately restored the target value
  lifetimes without changing ownership or layout.
- `PlayerMoveOMExec` became strict exact at 916 bytes. Separate scoped captures
  at the state-read and state-write boundaries reproduced the target's distinct
  transition lifetimes, while the target object independently authenticated the
  double-precision `2.0` literal.
- Together the retained changes added 5 strict functions and 326 verified
  relocation-bearing target instructions while preserving all 138 prior exact
  functions, for 4,012 newly exact text bytes.

At that initial validation point, the integrated object reported 143/165 strict
functions. Public recovery checks passed 78/78, all 137 retail outputs passed
DTK checksum, and `main.dol` remained byte-identical with SHA-1
`b897e6ade6b3a0cd2f9907689f38a3b19c327e70`.

## Player pass #019 reinforcement

The exact Player pass integrated at
`784bf01cfef0ee49ac01d59b7c329603ae425fbb` adds seven independent positive
cases for this same bounded heuristic:

- `PlayerMove` moved from 89.704865% at 1,152/1,152 bytes to strict exact at
  1,152 bytes. Repeated `mbPlayerWorkGet` accesses, movement-state reads at
  their use boundaries, and the loop-local `masuIdPrev` update restored the
  target's saved-register and branch lifetimes.
- `mbev_PlayerColMasu` and `mbev_PlayerColCircleAdd` moved from
  93.521736% and 93.752750% to strict exact at 644 and 728 bytes. Hoisting the
  shared sort temporary, restoring declaration order and loop-variable reuse,
  and reloading collision work at each semantic use boundary closed the broad
  register-allocation cycles.
- `mbev_PlayerColMasuAdd`, `PlayerColCornerSet`, and `mbev_PlayerColSet` moved
  from 85.032260%, 85.017440%, and 93.151260% to strict exact at 620, 688, and
  476 bytes. Removing broad cached work pointers, retaining a function-lifetime
  `BOOL` capture, and restoring matrix declaration order reproduced the target
  reload and stack-allocation shape.
- `BiriQEffectCreate` moved from 94.905660% at 404/424 bytes to strict exact at
  424 bytes. Function-scope model IDs followed by block-scoped raw `hookData`
  and typed `MBPARTICLE *` stages recreated the target's three distinct chained
  lookup lifetimes without casts, pragmas, or fake operations.

These were not isolated compiler micro-probes. `PlayerMove` also restored
target-backed movement-field meanings and branch grouping, while the collision
functions restored compare spelling and loop source shape. They reinforce the
heuristic only where those semantic corrections expose the target's distinct
lifetimes; they do not prove that adding captures alone is sufficient.

The final object report is 154/165 strict and 155/165 data-value exact, with
1,639 verified object relocations out of 2,214 target relocations after
excluding `R_PPC_NONE` symbol-binding rows, and no exact regressions. Integration
used DTK 0.9.2, objdiff 3.7.2, and MWCC GC/2.6; 99/99 tests, the serialized
137/137 build and DTK checks, and the retail `main.dol` SHA-1 all passed.

## Boundary and counterexamples

This evidence is an owner-local contextual heuristic, not a general instruction
to introduce temporaries. Capsule return-shape probes and an
`mbPlayerMoveMain` lifetime simplification regressed output and were reverted.
The remaining BiriQ literal- or relocation-identity residuals are still not
value-lifetime evidence; the positive BiriQ result is limited to the
instruction- and relocation-exact `BiriQEffectCreate` staging sequence.

Use the heuristic only when repeated global, bitfield, owner-field, accessor, or
staged-pointer lifetimes accompany a saved-register or signed-comparison
mismatch. Test each capture independently, compare the complete exact-function
set, and retain it only when strict relocations and natural source structure
also improve.

## REL reinforcement

Two independent GC/2.6 REL owners reproduced the same bounded lifetime effect:

- `s01Dll:fn_1_F4` became strict exact when the board-number read was restored
  through the authenticated `MBBoardNoGet` inline boundary and the model-ID
  pointer kept its persistent `s16` lifetime. Replacing that boundary with a
  direct `GwSystem.boardNo` read scored only 97.726320.
- `miraclebookdll:fn_1_7998` became strict exact when only the animation
  amplitude was captured across its two model consumers. Broader speculative
  captures did not reflect the target and were removed.

These results broaden the evidence beyond `main.dol`, but not the permission:
the capture still needs an authenticated value boundary, a matching final
GC/2.6 owner profile, and exact instruction and relocation proof.
