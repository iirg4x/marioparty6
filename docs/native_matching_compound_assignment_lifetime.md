# MWCC compound-assignment consuming-condition lifetime

MWCC can assign a different lifetime to a value when its assignment remains
inside the condition that immediately consumes it. Exact evidence now spans
two application owners and the project GC/2.6 and GC/1.3.2 O0 profiles.

## Ending counterfactual

`endingdll:fn_1_193C` provides a controlled before/after result. The natural
separated form updated `work->time`, stored it, and then reloaded the field for
the comparison. It compiled to 488 bytes at about 99%. Writing the same
target-backed semantics as:

```c
if ((work->time += step) > duration) {
```

kept the updated floating-point value live across the store and comparison,
removed the reload, and produced the strict-exact 484-byte target with exact
relocations. The clean evidence commit is
`b602e20eca4402dad7d4d4b96a655206a50ceb9a`; its public recovery gate passed
78/78.

## Independent mdpartydll evidence

The fully source-linked `mdpartydll` owner repeats the same source shape in
strict functions under its GC/1.3.2 profile:

- `fn_1_5324` uses `(work->time += 1.0f) > work->duration` twice and is exact at
  `0x57C`.
- `fn_1_6324` repeats the paired update/condition flow and is exact at `0x580`.
- `fn_1_737C` retains the compound update inside the state scope and is exact at
  `0x584`.
- `fn_1_3BEA8` keeps an assigned call result inside its consuming equality; the
  recovered form removes a three-way saved-register cycle without changing
  operations, control flow, or relocations.

The target/source `mdparty.o` metadata identifies compiler version 2.4.2.1;
the Ending source object identifies 2.4.7.1. Thus the mechanism is not confined
to one owner or one of the two pinned application compiler profiles.

## Recovery boundary

This card authorizes one bounded combined-versus-separated source-shape probe
only when target order proves that the stored or returned value is consumed by
the immediately following condition. It does not authorize comma expressions,
duplicate side effects, assignment conditions invented only for register
allocation, or retaining a form that is byte-neutral or regresses another
exact function.
