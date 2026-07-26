# Board GC/2.6 transitive math-pool evidence

Validated through AI integration commit
`6edb65c3076abb1d90ed2ffa1419cd65f41ed3f0` with the pinned GC/2.6 owner
commands and DTK `0.9.2` (`4d039140f2d2ed80572b1949b76a5ff9b3094e06`).

## Repeated signature

`main:board/star` and `main:board/dice` candidate objects begin `.sdata2`
with two 8-byte function-local statics emitted by the retained `sqrtf`
implementation: `_half` and `_three`. Their retail objects do not contain this
16-byte prefix. Later literal bytes remain valid, but strict SDA21 relocation
identities shift.

For the nine-function dice cluster, 31 unique target/candidate relocation pairs
had identical widths and data bytes under value-aware comparison. Removing the
direct `humath.h` include was byte-neutral because other retained headers kept
the same transitive math closure.

For the star cluster, the two math statics plus the early existing
`SignMdlTbl` formed a 24-byte candidate prefix. Eight clustered functions were
instruction-identical apart from relocation identity. The remaining map
function also had an independent stack-frame mismatch and must not be explained
by the pool finding.

`main:board/gate` and `main:board/roulette` convert the repeated signature into
a confirmed owner-local rule. Both retail objects require the existing
automatic-local implementation in `dolphin/math.h`, selected before transitive
generic `math.h` visibility using the same `_MATH_H` chronology already present
in exact board owners such as branch, status, and player. Gate closes from 7/11
to 11/11 functions and 168/168 relocations. Roulette closes from 12/21 to 21/21
functions and 291/291 relocations. Their named data is exact; the remaining raw
section-size differences are target-side synthetic gaps rather than C-owned
padding.

The same authenticated owner-local boundary produces partial but independently
exact gains in two larger owners. Tutorial rises from 39/55 to 44/55 strict
functions, closing TutorialWatch, both tutorial view functions, and both sprite
display functions without losing an exact neighbor. Dice rises from 39/68 to
44/68, closing DiceProcExec, DiceObjCreate, DiceObjOMExec, DiceObjHit, and
DiceNumObjReset. All retained exact-function relocations remain strict, both
path-specific quality exceptions pass, and the 137-output retail gate remains
exact.

The shared-header counterexample is equally important. Changing
`include/math.h` itself from static to automatic locals improved gate and
roulette and caused no strict loss among the 40 audited board objects, but the
whole-project retail build then failed seven previously exact REL outputs:
`actmanDLL`, `bootDll`, `fileseldll`, `mdpartydll`, `mdseldll`, `selmenuDll`, and
`sequencedll`. Reverting the shared header restored all 137 retail checks. The
safe correction is therefore translation-unit-local, not project-wide.

`main:board/tutorial` supplied the original negative source-quality control: an
unreviewed guard override removed the prefix and exacted several functions, but
the public gate correctly rejected it. After gate and roulette established
exact same-game precedent, a narrow path-specific exception authenticated the
same tutorial source boundary and the five gains became retainable. This does
not turn the pattern into a blanket rewrite: applying it to `main:board/star`
was byte-neutral at 60/90 strict functions, confirming that Star's early
`SignMdlTbl` chronology and map-view frame mismatch are independent.
`main:board/opening` similarly showed that correcting pool chronology alone did
not establish an owner-clean helper/linkage shape.

## Bounded conclusion

The signature is a diagnostic for include closure and constant ownership, not a
blind source template. Safe investigation compares strict and data-value
reports, raw `.sdata2` prefixes, the exact owner compiler command, and
preprocessed header visibility. An owner-local canonical-header selection may
be retained only when exact same-game peers authenticate it, strict functions
and relocations close, the public gate accepts a path-specific review, and the
whole project remains retail-exact. It never authorizes named fake literals,
padding, or a shared-header change made for one owner.
