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
