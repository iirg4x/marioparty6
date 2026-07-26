# Board GC/2.6 transitive math-pool evidence

Validated against AI workspace commit
`d3585141f19bd4ba3604f8275aa1f675a56b2fcd` with the pinned GC/2.6 owner
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

`main:board/tutorial` supplied a negative source-quality control: suppressing
the math include guard removed the prefix and exacted several functions, but the
public gate rejected that unreviewed guard override. `main:board/opening`
similarly showed that correcting pool chronology alone did not establish an
owner-clean helper/linkage shape.

## Bounded conclusion

The signature is a diagnostic for include closure and constant ownership, not a
source template. Safe investigation compares strict and data-value reports,
raw `.sdata2` prefixes, the exact owner compiler command, and preprocessed header
visibility. It does not authorize named fake literals, padding, header-guard
suppression, or copying an include layout between owners.
