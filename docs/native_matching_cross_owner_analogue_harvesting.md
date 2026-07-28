# Cross-owner analogue harvesting

## Confirmed MP6 evidence

`src/REL/mdpartydll/stage.c` supplied natural same-game source hypotheses for
five independently exact `mdsingdll` functions.  The owners are different, so
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

The three retained clusters total 632 target text bytes.  Fresh active-object
reports showed exact instructions and zero relocation differences; their DTK
physical-relocation counts were 4, 45, and 9 respectively.  Each cluster was also
registered as `Object(Matching, ...)`, proved present in generated Ninja/link
inputs, linked with the remaining retail fallback consumers, passed the public
gate, and reproduced the retail REL in a serialized clean build.

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
- Adding a split or source file is insufficient if `configure.py` does not emit
  a matching object into generated build and link inputs.
- Exact subsets are valuable independently.  A failed neighbor does not justify
  reverting exact functions elsewhere in the cluster.

Evidence is retained in `src/REL/mdpartydll/stage.c`,
`src/REL/mdsingdll/mdsing_tail.c`, `mdsing_tail2.c`, `mdsing_tail3.c`, the
`mdsingdll` split map, and their explicit matching-object registrations.
