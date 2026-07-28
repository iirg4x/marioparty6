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

## Call-spanning owner pointer capture is a bounded cross-profile rule

Under GC/2.6 `-O0,p`, mdpresult commit
`495f99498e937df3a3bb936b1b74c89d8e039b90` made `fn_1_17D94` strict exact
after one natural `HUSPR_GROUPID *` capture preserved the named group base in
the target nonvolatile-register lifetime across its call. Direct indexed or
scalar access shortened that lifetime and removed the target save. The pass
retained 12 exact functions and 608 bytes and passed 99 public tests.

Under GC/1.3.2 `-O0,p`, mdbank commit
`c708b3c74d5cc3de1a924be08354553153d8f8ab` isolated the same mechanism in
exact `fn_1_1DDC` and `fn_1_1EE8`. One authenticated `MDBANK_MOVE_WORK *`
kept the owner base across calls and repeated field accesses; direct global
field expressions rematerialized the symbol. The pass retained three exact
functions and 932 bytes, preserved protected `.rodata` byte-for-byte, and
passed 78 public tests. An earlier five-function GC/1.3.2 cluster at commit
`6c447c4c1103570ad2a2cb99bd1e4a0660320630` independently supported stable
owner-pointer reuse across calls and relocation-bearing accesses.

The counterexample is GC/2.6 mdpresult `fn_1_CE60` at commit
`92f135446bbb597e82b17341cdc1614c89827145`: a local object capture extended a
saved-register lifetime absent from retail, and direct global access was exact.
The reusable rule is therefore limited to target-proven call-spanning address
lifetime and named relocation ownership; it does not authorize pointer locals
as generic register-allocation controls.

## Consumer triangulation closes natural aggregate ownership

Under GC/2.6, staff commit
`a7625189a25dcde90cf59a3d9f07195fe0f2b3bc` replaced temporary offset overlays
with the public `HU3D_MODEL.hookData` -> `HU3D_PARTICLE` ->
`HU3D_PARTICLE_DATA` consumer chain while preserving strict-exact bytes.  The
pass retained 25 functions, 0x9BC text bytes, and 125 physical relocations; the
initial offset-only HSF interpretation was rejected.

Under GC/1.3.2, mdbank commit
`b8534c38ea3641e08dade5dc455e82cb24c56d15` proved that configured
`lbl_1_bss_1998` size 0x50 owns a camera aggregate rather than a pointer slot.
Callbacks and state consumers authenticated center, rotation, zoom, callback,
and state offsets; twelve new functions, 1,396 text bytes, and 94 physical
relocations became exact.  Pass 7 commit
`40babb7895ee0ed54a171e2017516f619d26e91b` independently used configured
extent divided by repeated consumer count to bind eight more owner arrays and
close two functions totaling 1,312 bytes.

The cross-profile rule is to triangulate configured extent, public field/stride
types, and every independent consumer before retaining an opaque overlay or a
first-word pointer interpretation.  Matching one offset is not semantic or
layout proof.

## Catalog-authenticated fixed arity selects natural source shape

Under GC/2.6, miraclebook commit
`d6a793893a0ed411baa415c9f7fbee8d9e29fa6d` used the authenticated five-entry
resource catalog to express five direct resource calls in `fn_1_7998`.  The
loop form retained 40 bytes of loop control absent from retail; the direct
sequence made the 4,212-byte function strict exact.  The same function's
authenticated helper early return bound its inlined branch to the caller
epilogue, whereas a rewritten switch `break` targeted the continuation.

In the fully recovered `s01Dll`, commit
`9ab7a158bd74541b0c72745def3d6f9686655be7` used an authenticated 16-entry
motion-order workspace in `fn_1_AFC`.  That natural fixed permutation reproduced
the 0x13C target body exactly; an earlier destructive pseudo-swap was only
54.632910% and was reverted.

The reusable diagnostic is not "always unroll."  First authenticate the small
resource count from catalog, configured extent, or consumers.  Then follow the
target shape: a direct call sequence explains absent loop control, while a
bounded local permutation explains retained index/load/store shuffle lifetime.
No pragma, fake table, or literal fabrication is authorized.

## Initialization timing is distinct from declaration order

Under GC/1.3.2, mdbank commit
`75986189698d2131f54a7033c781f41e8c741a6f` made `fn_1_83CC` exact at 756
bytes and 87 relocation-bearing instructions by declaring the authenticated
camera pointer normally but assigning it only after window/light cleanup.  An
initializer at the declaration was structurally and size exact yet scored
97.8836% because it began the saved-register lifetime too early.  Declaration
order and initialization/first-use position must therefore be tested as
separate bounded variables when target calls/stores prove the later lifetime.

## Retained inline source has multiple output contexts

Ending pass 19 at clean head
`bccac84672a5c8553cd012f6e6db389f105cf768` reproduced every opcode and
relocation of `fn_1_A964` and reached 99.984920%, leaving seven stack-slot
argument differences.  Natural declaration changes that improved those slots
regressed independently exact `fn_1_A6D8`, while file-data/animation chronology
changes regressed exact `fn_1_A58C`; all candidates were reverted.  A helper
edit must therefore replay its standalone body and every retained inline
consumer.  A higher score in one clone never outweighs an exact-context
regression.

## Proof boundary

The cited worker commits are clean, source-only partial-owner commits. Reports
remain in each isolated worker build directory. Every retained subset passed
its public gate and exact object/relocation/consumer checks. No cited owner was
marked ready or promoted from these partial results, and no linked retail claim
is made here.
