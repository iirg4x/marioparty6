# Board GC/2.6 transitive math-pool evidence

Validated through AI integration commit
`630e9d333ae7c693e214517d6db0453fcf30995b` with the pinned GC/2.6 owner
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

The shared-header counterexample is equally important. Changing
`include/math.h` itself from static to automatic locals improved gate and
roulette and caused no strict loss among the 40 audited board objects, but the
whole-project retail build then failed seven previously exact REL outputs:
`actmanDLL`, `bootDll`, `fileseldll`, `mdpartydll`, `mdseldll`, `selmenuDll`, and
`sequencedll`. Reverting the shared header restored all 137 retail checks. The
safe correction is therefore translation-unit-local, not project-wide.

`main:board/tutorial` supplied the original negative source-quality control: an
unreviewed guard override removed the prefix and exacted several functions, but
the public gate correctly rejected it. Exact same-game precedent and a narrow
path-specific quality exception are required before retaining that source
shape. `main:board/opening` similarly showed that correcting pool chronology
alone did not establish an owner-clean helper/linkage shape.

## Bounded conclusion

The signature is a diagnostic for include closure and constant ownership, not a
blind source template. Safe investigation compares strict and data-value
reports, raw `.sdata2` prefixes, the exact owner compiler command, and
preprocessed header visibility. An owner-local canonical-header selection may
be retained only when exact same-game peers authenticate it, strict functions
and relocations close, the public gate accepts a path-specific review, and the
whole project remains retail-exact. It never authorizes named fake literals,
padding, or a shared-header change made for one owner.
