# DLL compiler-pattern reinforcements (2026-07-28)

This report preserves reviewed evidence from the isolated non-board DLL recovery
domain. It reinforces existing compiler cards; it does not claim full owner,
linked REL, checksum, semantic-name, or retail completion.

## Definition visibility is captured when a helper body is compiled

Under GC/1.3.2 `-O0,p -inline auto`, mdbank commit
`559e018f8db67ada420a8daedc2dfa0b187123de` made `fn_1_281C` exact at 108
bytes while only a prototype for later `fn_1_11900` was visible. Later
`fn_1_28B4` expanded `fn_1_281C` exactly at 412 bytes, but its nested
`fn_1_11900` operation remained a call relocation. A definition becoming
visible later did not retroactively change the already compiled helper body.
Pass 13 retained 11 exact functions, 1,968 bytes, 1,620 exact
relocation-bearing instructions, and passed 78 public tests.

## Named data ownership remains part of function exactness

GC/2.6 mdpresult commit
`f1905818ca4395c3f49cba18a6a722c0ba57f7cf` added an authenticated
owner-local named `.rodata` closure and made six otherwise instruction- and
size-exact consumers relocation-exact without changing their text. Pass 2
retained 16 functions and 3,672 bytes; all 28 cumulative functions had exact
instruction, relocation, and consumer proof and passed 99 public tests.

GC/1.3.2 mdbank commit
`7317f2ad621254675609b11d059a17d18ca0e1fc` preserved five unresolved
standalone/callback/inline-consumer bodies as negative evidence. Natural
conversion consumers produced compiler-local `@992` or `@977` owners while
retail required distinct named constants `lbl_1_rodata_290` or
`lbl_1_rodata_2A0`. Commit
`e9854880392b13462806824b13d11b9ce837911e` independently showed natural
aggregate initializers in `fn_1_780` and `fn_1_8F0` binding compiler-owned
local symbols where retail required named objects; `memcpy` changed code and
did not authenticate ownership. All unresolved bodies were reverted. The
retained 94-function subset passed 78 public tests with exact relocation and
consumer proof.

These cases do not authorize fabricated constants, value-equal anonymous
relocations, `memcpy` substitutions, or retention based on text alone.

## Declaration order and first use remain bounded register-color levers

In mdbank commit `559e018f8db67ada420a8daedc2dfa0b187123de`,
expressing the natural evaluation order in `fn_1_3100` corrected the standalone
body and the `fn_1_3164` automatic-inline copy together. In mdpresult commit
`92f135446bbb597e82b17341cdc1614c89827145`, declaring the inner-loop index
before the outer-loop index made `fn_1_B178` exact. The counterexample in the
same commit is `fn_1_C358`: a separate second-loop index reproduced the target
save set but left a seven-row register-color cycle, so the function was fully
reverted. The bounded rule is therefore diagnostic, not permission to add
register-shaping locals.

## Pointer truthiness reinforcement and scope limit

GC/2.6 mdpresult commit
`e2175e88c550f2b4565ffda326f49a0ff38823e4` retained a repeated destructor
family with natural pointer truthiness and a terminal local null assignment.
The 12-function pass was exact on its first compile, added 1,568 bytes, had 95
real and 29 tracked relocation rows exact per side, and passed 99 public tests.
This reinforces the existing pointer-guard spelling card; it does not imply
that `NULL` is globally wrong or that integer handles may be cast to pointers.

## Proof boundary

The cited worker commits are clean, source-only partial-owner commits. Reports
remain in each isolated worker build directory. Every retained subset passed
its public gate and exact object/relocation/consumer checks. No cited owner was
marked ready or promoted from these partial results, and no linked retail claim
is made here.
