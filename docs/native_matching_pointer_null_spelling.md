# MWCC pointer truthiness and null spelling

MWCC can emit different pointer-zero comparisons for source forms with
identical C semantics. A direct truth test or comparison with integer literal
`0` can use `cmplwi`, while typed `NULL` can materialize zero in a register and
use `cmplw`.

## Cross-profile exact evidence

Under `GC/2.6 -O0,p`, Ending cleanup callbacks matched the target's direct
`cmplwi` only when written as pointer truthiness. The equivalent explicit
`pointer != NULL` form emitted `li` plus `cmplw`. The retained pass-3 owner
commit is `1d3d7adbb34f6fbe8b35f8e3f3691e9d0b49e4cc`; its exact subset contained
54 functions and 458 physical source-object relocations.

Under `GC/1.3.2 -O0,p`, `mdbankdll:_epilog` independently reproduced the
mechanism in a destructor-list walker. Authenticated donor spelling
`while (*entry != 0)` emitted a direct `lwz`/`cmplwi` sequence and matched the
76-byte target. `while (*entry != NULL)` emitted `lwz`, `li r0,0`, and `cmplw`,
growing the function to 80 bytes. The strict-exact retained commit is
`d4b8fc22950f75b54f404971407d8838a8da9ce3`.

The two owners use different compiler profiles and different pointer guards,
so the reusable finding is the pointer-null source spelling rather than an
owner-specific macro or callback shape.

## Recovery rule

Use this only when the operand is independently authenticated as a pointer and
the remaining mismatch is isolated to immediate-zero comparison versus a
materialized null operand. Test one natural truthiness or literal-`0` spelling,
then require exact branches, text, relocations, and consumers.

Do not cast integer handles to pointers, change a public type, or rewrite
unrelated control flow to obtain `cmplw`. `NULL` is not globally incorrect;
the target instructions and the specific consumer decide which historically
natural spelling applies.
