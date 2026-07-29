# GC/2.6 board capsule call-chronology evidence

`src/board/capsule.c` uses the board GC/2.6 `-O0,p` profile. Four bounded
target-only recovery passes added nine strict functions while preserving every
previously exact function. The results show that helper definition visibility,
expression-tree call evaluation, and local lifetime shape must be recovered
together.

## Confirmed definition and evaluation results

- `mbCapSelectMasuFrontNum` and `mbCapSelectMasuBackNum` are strict exact at
  200 bytes each.
- `mbCapSelectMasuNum` is strict exact at 72 bytes only when its definition
  precedes the Front/Back helper definitions, preventing GC/2.6 from
  auto-inlining the helpers into the caller.
- Spelling the sum as `FrontNum(...) + BackNum(...)` emits the target
  Back-then-Front call order for this expression under the pinned profile.
- `CapUse` is strict exact at 156 bytes when `work`, `result`, and `workData`
  follow the target-backed declaration chronology.
- `CapSelectMasuCheck` is strict exact at 204 bytes with positive nested
  eligibility tests and the target-backed `mbMasuMAttrGet` evaluation retained.
- `CapSelectMasuListGet` is strict exact at 176 bytes with direct nested calls;
  introducing a cached `linkMasuId` local changes the target allocation.
- `mbCapSelectMasuInit` is strict exact at 208 bytes when it is defined before
  `CapSelectMasuListGet`, preserving the target out-of-line call. Its `masuId`
  lifetime remains `int` and narrows only at the authenticated `s16` call
  boundary.
- `CapSelectMasuKill` is strict exact at 148 bytes when the selection-work check,
  object kill, pointer reset, and state cleanup retain the target's semantic
  chronology.
- `CapUseDeleteKill` is strict exact at 164 bytes when the delete-work check,
  object kill, and reset stores remain at their target-backed boundaries.

Together these functions add 1,528 exact text bytes and 51 verified
relocation-bearing target instructions. The capsule owner advanced from 81/165
to 90/165 strict functions. Public recovery checks pass 78/78, all 137 retail
outputs pass DTK checksum, and `main.dol` remains byte-identical with SHA-1
`b897e6ade6b3a0cd2f9907689f38a3b19c327e70`.

## Boundary and rejected shortcuts

These results do not authorize forcing a call order, suppressing inlining, or
retaining semantically dead work. Each source form is admitted only because the
target helper family, calls, and consumers authenticate it.

An `mbCapUse` reconstruction was reverted at 568/584 bytes because the target
needs authenticated variadic-call and return-width behavior that conflicts with
the current shared `gamemes` declaration. Local prototype hacks are not a valid
substitute for shared ABI review. Opaque capsule-list layouts and literal/BSS
identity-only candidates were also rejected. In particular, `mbCapNextGet`
reached instruction-exact 1,132/1,132 bytes but retained six data/literal
relocation-identity residuals; it is not call-chronology evidence and must wait
for natural section-ownership changes.

## Non-board GC/2.6 reinforcement

`endingdll` independently confirmed that the same final compiler profile is
the relevant boundary, rather than board ownership itself:

- `fn_1_27A0` became strict exact when the natural direct product grouping was
  restored, allowing MWCC to recover the retail call-evaluation chronology.
- `fn_1_B0A4` became strict exact when helper definition visibility and the
  saved-register lifetime were restored together.
- `fn_1_DF60` became strict exact at 396 bytes when three natural calls to the
  preceding exact scalar helper `fn_1_DF18` were allowed to expand under
  `-inline auto`; the result has zero text or relocation differences.
- `fn_1_8CFC` independently reproduced the same mechanism at a larger boundary:
  natural calls to preceding exact teardown helpers `fn_1_26D4`, `fn_1_3CD4`,
  and `fn_1_3F20` expand into the exact 644-byte lifecycle coordinator.

The exact evidence is bound to ending worker commits
`66bf6eb3cc97e3c225b85e891de4161244a1f05f` and
`8588d7fc1c443ba83cdc2c8b08dc6dcd37316c5b`; both clean subsets passed the
78-test public recovery gate. Operand-order and capture probes for
`fn_1_DE3C` did not reproduce the target and were reverted, so the result does
not authorize generic expression rewriting.

GC/1.3.2 evidence remains a separate candidate until it has enough independent
support; these observations do not silently broaden this card across compiler
families.

## Chained allocation-result lifetime

`mbCapInit` supplies an exact GC/2.6 boundary for assignment-chain length. The
natural two-destination form
`capsuleBorderObjId = borderObjData = HuMemDirectMallocNum(...)` compiles to the
312-byte target with zero instruction or relocation differences. Keeping the
later traversal alias as the separate statement `borderId = borderObjData`
preserves the target allocation result in `r25`.

Extending the chain through `borderId` is not equivalent compiler evidence: it
changes the saved-register and spill lifetime and scores 96.474360%. The rule is
therefore to recover the target-backed chain boundary, not to chain every alias
of a common result.
