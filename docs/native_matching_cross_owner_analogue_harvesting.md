# Cross-owner analogue harvesting

## Confirmed MP6 evidence

`src/REL/mdpartydll/stage.c` supplied natural same-game source hypotheses for
independently exact `mdsingdll` clusters.  The owners are different, so
the donor is evidence rather than merge authority; target globals, callbacks,
types, literals, and split ownership were rebound to `mdsingdll` before each
comparison.

| Donor stable ID | Target stable ID | Family | Target text |
| --- | --- | --- | ---: |
| `mdpartydll:fn_1_40A9C` | `mdsingdll:fn_1_2E68C` | layer-hook install | 44 bytes |
| `mdpartydll:fn_1_40AC8` | `mdsingdll:fn_1_2E6B8` | layer-hook reset | 36 bytes |
| `mdpartydll:fn_1_40EAC` | `mdsingdll:fn_1_2EA9C` | particle create/configure | 276 bytes |
| `mdpartydll:fn_1_40FC0` | `mdsingdll:fn_1_2EBB0` | particle destroy | 44 bytes |
| `mdpartydll:fn_1_40FEC` | `mdsingdll:fn_1_2EBDC` | particle activation | 232 bytes |
| `mdpartydll:fn_1_417DC` | `mdsingdll:fn_1_2F3CC` | particle display enable | 88 bytes |
| `mdpartydll:fn_1_41834` | `mdsingdll:fn_1_2F424` | particle data reset | 160 bytes |
| `mdpartydll:fn_1_42060` | `mdsingdll:fn_1_2FC50` | particle group reset | 256 bytes |
| `mdpartydll:fn_1_42160` | `mdsingdll:fn_1_2FD50` | particle model display disable | 44 bytes |
| `mdpartydll:fn_1_4218C` | `mdsingdll:fn_1_2FD7C` | particle group create/configure | 204 bytes |
| `mdpartydll:fn_1_42D2C` | `mdsingdll:fn_1_31198` | particle model enable | 124 bytes |
| `mdpartydll:fn_1_42DA8` | `mdsingdll:fn_1_31214` | particle group data reset | 260 bytes |
| `mdpartydll:fn_1_42EAC` | `mdsingdll:fn_1_31318` | particle group activation | 136 bytes |
| `mdpartydll:fn_1_433AC` | `mdsingdll:fn_1_31818` | particle family reset | 364 bytes |
| `mdpartydll:fn_1_43518` | `mdsingdll:fn_1_31984` | particle family teardown | 88 bytes |
| `mdpartydll:fn_1_43570` | `mdsingdll:fn_1_319DC` | particle family position | 124 bytes |
| `mdpartydll:fn_1_43724` | `mdsingdll:fn_1_31B90` | particle family disable | 84 bytes |
| `mdpartydll:fn_1_42B40` | `mdsingdll:fn_1_30730` | particle group create/configure | 404 bytes |
| `mdpartydll:fn_1_42CD4` | `mdsingdll:fn_1_308C4` | particle group destroy | 88 bytes |
| `mdpartydll:fn_1_43D78` | `mdsingdll:fn_1_321E4` | particle pair create/configure | 364 bytes |
| `mdpartydll:fn_1_43EE4` | `mdsingdll:fn_1_32350` | particle pair destroy | 88 bytes |
| `mdpartydll:fn_1_44300` | `mdsingdll:fn_1_3276C` | particle family visibility | 180 bytes |
| `mdpartydll:fn_1_4451C` | `mdsingdll:fn_1_32988` | particle family reset | 128 bytes |
| `mdpartydll:fn_1_44D48` | `mdsingdll:fn_1_331E0` | particle family teardown | 132 bytes |

The first three retained clusters total 632 target text bytes.  A later bounded
inventory found eight more analogues at a stable donor-to-target address delta
of `-0x12410`; one coherent six-function lifecycle attempt retained five exact
functions totaling 752 bytes and 58 physical relocations.  Across both batches,
ten mappings accounted for 1,384 exact target text bytes and 116 physical
relocations.  A second feedback pass expanded the safe catalog to 18 later
donors at stable address delta `-0x11B94`, attempted eight lifecycle functions,
and retained seven exact functions totaling 1,180 bytes and 72 physical
relocations.  A third feedback pass retained seven more exact helpers across
five independently consumed objects, adding 1,384 target text bytes and 98
physical relocations.  Across all feedback batches, 24 mappings now account for
3,948 exact target text bytes and 286 physical relocations.  Fresh active-object
reports showed exact instructions and zero
relocation differences; the first three DTK physical-relocation counts were 4,
45, and 9 respectively.  Each retained cluster was also
registered as `Object(Matching, ...)`, proved present in generated Ninja/link
inputs, linked with the remaining retail fallback consumers, passed the public
gate, and reproduced the retail REL in a serialized clean build.

The later family also authenticated a reusable owner translation: donor bases
`AE4`, `AE2`, `ADA`, and `B40` correspond to target objects at `bss_141E`,
`bss_141C`, `bss_1414`, and `bss_1478`; donor constants 0, 1, 200, and 275
correspond to target named objects at `rodata_398`, `rodata_3BC`, `rodata_438`,
and `rodata_470`.  These mappings rank candidates but do not authorize names or
values outside the relocation-proven family.

## Reusable search procedure

1. Build an eligible donor catalog from authenticated same-game, non-board,
   non-minigame owners.  Record stable IDs and ordered external-call/symbol
   relocation signatures; source names alone are not a fingerprint.
2. Compare those signatures with unclaimed target functions, then refine with
   ABI, control-flow shape, literal values, callback roles, and target-owned
   consumer evidence.  Rank exact ordered-call skeletons above loose API-set
   overlap.
3. Batch candidates by contiguous target range or shared declarations and
   types.  Attempt one coherent multi-function cluster instead of issuing a
   one-function turn for every candidate.
4. Translate only the donor's natural expression and control structure.  Bind
   all symbols, callbacks, types, counts, and literals from target evidence;
   never import donor ownership or semantic names by resemblance.
5. Compile once for the coherent hypothesis.  Retain any independently exact
   subset and revert unresolved speculative C.  Save compact negative evidence
   for false candidates so later inventory passes can suppress them.
6. Before integration, prove exact instructions and relocations, prove the
   generated object is actually consumed rather than silently replaced by
   fallback, replay linked consumers, and run the serialized retail gate.

## Limits and counterexamples

- API-set similarity without ordered relocations and compatible control flow is
  only a weak lead; common engine setup/teardown idioms create false positives.
- A donor contract cannot authenticate a target symbol name, private layout,
  callback identity, array count, or constant without target-side evidence.
- Large callback analogues may require earlier TU-visible auto-inline helpers;
  the 1,352-byte `fn_1_410D4 -> fn_1_2ECC4` and 1,932-byte
  `fn_1_418D4 -> fn_1_2F4C4` candidates were excluded rather than rewritten.
- `fn_1_4161C -> fn_1_2F20C` reached exact size and 99.866070%, but three
  conversion relocations bound compiler-local `@166` instead of target
  `lbl_1_rodata_3F8`; it remains rejected pending named pool ownership.
- `fn_1_435EC -> fn_1_31A58` reached 312/312 bytes and 99.423080%, but nine
  relocation-only references bound compiler-local `@190` instead of target
  `lbl_1_rodata_400`; it remains rejected pending named pool ownership.
- `fn_1_43F3C -> fn_1_323A8` and `fn_1_463E0 -> fn_1_34958` were suppressed
  because their target output depends on earlier TU-visible auto-inline bodies.
- `fn_1_443B4 -> fn_1_32820` repeated the known `@190` byte-to-float pool
  mismatch and was not reworked.
- `fn_1_44B1C -> fn_1_32FB4` reached 99.748% text, but local-initializer and
  adjacent-rodata ownership prevented a retail-exact split object.
- Adding a split or source file is insufficient if `configure.py` does not emit
  a matching object into generated build and link inputs.
- Exact subsets are valuable independently.  A failed neighbor does not justify
  reverting exact functions elsewhere in the cluster.

Evidence is retained in `src/REL/mdpartydll/stage.c`,
`src/REL/mdsingdll/mdsing_tail.c` through `mdsing_tail13.c`, the `mdsingdll`
split map, and their explicit matching-object registrations.
