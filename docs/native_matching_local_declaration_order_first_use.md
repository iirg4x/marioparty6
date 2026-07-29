# MWCC local declaration order and first-use chronology

Adjacent natural locals can rotate saved-register assignments under MWCC O0
without changing the function's structure. This is a narrow final-mile lever,
not permission to shape registers: it applies only after types, control flow,
calls, size, and relocations already agree.

## Exact cross-profile evidence

Under `GC/2.6 -O0,p`, `endingdll:fn_1_12838` became strict exact when its
sequential same-type loop locals remained in one outer scope. Natural
declaration and first-use chronology assigned the first, second, and third
indices to `r31`, `r30`, and `r29`, with the group and model owners in `r27`
and `r28`. The retained owner commit is
`4c1f6f0aad290b5f5fad322ce3fb2f0540c39b75`.

Under `GC/1.3.2 -O0,p`, `mdbankdll:fn_1_13764` independently confirmed the
same mechanism. A natural typed loop initially declared `HU3D_PARTICLE
*particle` before `HU3D_PARTICLE_DATA *data`, producing particle/data in
`r31`/`r30` and a score of 98.833336%. Declaring `data` before `particle`
preserved identical semantics and the 120-byte structure while rotating the
owners to the target `r31`/`r30` data/particle assignment. Text and
relocations became strict exact at clean commit
`06884d858d93eb06f693b49c0425b118152e2cb0`.

These examples use different compilers and different aggregate families, so
the reusable mechanism is local lifetime chronology rather than an
owner-specific typedef or helper.

## GC/2.6 O0 source-local FPR ranking

`main:board/telop:mbTelopTimeSprRotSet` isolates the mechanism more precisely
for `GC/2.6 -O0,p`. The exact-sized 664-byte candidate had twelve operand-only
differences: retail used the first sine, second sine, and shared scale
lifetimes in `f31`, `f30`, and `f29`, while source used `f29`, `f30`, and
`f31`.

Native compiler inspection after frontend optimization measured the source
locals' `VarInfo.usage` values as 4 for the first sine, 4 for the second sine,
and 9 for the shared scale. This agrees with MWCC's non-optimizing
`allocate_local_FPRs` path: it repeatedly chooses the highest-usage eligible
source local, resolves ties by local-list traversal, and assigns descending
nonvolatile FPRs before PCode graph coloring handles compiler temporaries.

A diagnostic-only control raised those static use counts to 11, 10, and 9
without emitting instructions. The result was strict exact at 664 bytes with
zero differences, proving that source-local usage ranking owned the cycle.
The discarded-read control was then reverted because it was register shaping,
not recovered source. `register` was byte-neutral, the authenticated `HuAbs`
macro emitted branches, and inline absolute-value helpers grew the frame, so
none was retained.

## Bounded negative evidence

`meschkdll:fn_1_188` demonstrates the stop condition. Legal block-start
declarations and nested-scope relocation improved an exact-sized 1,696-byte
function from 71 differences to nine pure argument-register differences at
99.893870%. The final `dirWin`/`nextMessNo` `r20`/`r22` cycle did not respond
to an adjacent declaration swap or a natural alias, so every nonexact probe
was reverted. The retained source blob is
`8fbfcbdd8abaad2b1217503d3f9bb8b16f8d8f3d`.

Declaration order and first use must therefore be tested separately and only
once per authenticated hypothesis. A neutral result means another type, AST,
or definition boundary owns the remaining cycle.

## Recovery rule

Use this rule only when the function is exact-sized and remaining differences
are a small cycle among same-width saved-register operands. Authenticate local
types and pointer depth, test one natural adjacent declaration order and one
independently justified first-use chronology, then require exact text,
relocations, consumers, and exact-neighbor preservation.

Never retain redundant locals, fake scopes, volatile qualifiers, register
keywords, discarded reads, or width changes merely to choose registers. A
no-code use-count control may diagnose which local owns a cycle, but it is not
promotable source. If the bounded natural controls do not close the cycle,
revert them and preserve the negative evidence.
