# Narrow assignment values with wide MWCC lifetimes

The semantic width of an API result does not necessarily identify the storage
width that produced the retail code. Under `GC/2.6`, two independent REL
owners required a narrow value to enter an `s32` lifetime so that sign
extension occurred at the target consumers rather than at assignment.

## Authenticated observations

- In `s02Dll`, the target sign-extends the result after the narrow-producing
  call and repeats `extsh` at every later consumer. Declaring the owner field or
  local as `s16` caused MWCC to replace those consumer extensions with register
  moves. An `s32` lifetime populated by an explicit `(s16)` result cast
  reproduced the target producer/consumer pattern across the retained exact
  cluster.
- In `miraclebookdll:fn_1_7354`, `HuWinChoiceGet` supplies a semantically
  narrow choice value. An `s16` local delayed or removed the target extension
  nodes. Assigning the explicitly narrowed result into an `s32` local retained
  the wide lifetime and reproduced the target scheduling exactly.

The rule is about lifetime and extension placement, not about changing the
API's authenticated return type. A single isolated `extsh` is insufficient;
producer width, stored width, and every consumer must agree with the target.

## Recovery rule

When target code repeatedly sign-extends a semantically narrow result, compare
both an `s16` lifetime and an `s32` lifetime initialized through an explicit
narrow cast. Retain the wide lifetime only when the producer contract and all
consumer extension sites authenticate it, and require exact instruction and
relocation proof.
