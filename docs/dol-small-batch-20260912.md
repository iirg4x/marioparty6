# Small DOL batch, 2026-09-12

## Retained closure

| Owner | Functions exact | Code bytes | Effective relocations |
| --- | ---: | ---: | ---: |
| gssdk flblocks/mtxopt | 3/3 | 600 | 1/1 |
| gssdk flblocks/dctlift | 4/4 | 1156 | 27/27 |

Three previously nonexact functions closed: QrPreMult, ProcessDCTLift and
InitDCTLift. The other four exact functions remain exact. Strict and
data-value objdiff agree; dctlift's 56-byte constant section is exact.

The real source-selected Ninja/MWLD build in the existing main-derived proof
workspace passed all 137 configured retail checksums. Direct comparison also
passed for the DOL and 136 RELs. DOL SHA256:
`172ae27aa6fcc9074de286b07ae4ac9f9152175fa913fffc3f5b8365e139ffec`.
The normal Ninja objects reproduced the isolated candidates byte-for-byte:

- mtxopt source `6b447ccde2854c683aa291062fc054a8a64900a83b57ad6bd759b812a6091fe6`,
  object `1f8bf4cafc6925b030e0c312ffb8a487cb7994e9bf8e3d8819963e761b636428`.
- dctlift source `ff4313f2f71be2e4218d93b0a236ef692c4d38aeee60594667f48fb2d6d89a52`,
  object `f01557913f5711efadb86f98132c910c5afe9e17d03648b8aeaf042d2486d006`.

Verified configuration frontier: DOL 337/396, code 1851888/2173968
(85.18%), data 692768/717328 (96.58%); full project 376/926.
These are source-selected build results, not claims that remaining fallback
owners are reconstructed. All 61 starting residual owners were inventoried;
96.57% data did not imply all 61 were small.

## Source causes and useful tooling

QrPreMult required the packed triangular traversal's one-based column, including
the first matrix advance, plus the pointer/counter lifetime ordering. Changing
only a late register operand would not repair the traversal. The final source
uses ordinary pointer locals and a post-increment loop condition; no assembly,
padding or fake owners were added.

ProcessDCTLift required output-count creation before input-end creation and
normalization through the saved output-end pointer rather than the loop cursor.
InitDCTLift required computing input span before potentially aliasing size
stores, the target's float-precision integer conversions before double math,
a full-width signed lift-period local, and the reconstructed coefficient/index
declaration chronology. The stages improved 85.97 -> 88.41 -> 98.98 -> 99.45 ->
99.61 -> 100; this was not a Cartesian spelling matrix.

`recovery_causal_groups.py --producers N` now optionally follows block-local
physical definitions to load/field roots. DCTLift's later pointer mismatches
trace back to fields +0x28/+0x2c. This independently exposes the dependency that
the retained field-order correction repairs. It does not invent source owners
or claim a predicted C spelling is correct. Calls, unsupported instructions and
control-flow joins remain explicit boundaries. The Stationarity loop-carried
min/max cycle still needs additional evidence; the new output does not solve it.

Final proof exposed a second tool gap: GC1.2.5n SDA21 records use instruction+2,
while DTK's reconstructed ELF records use instruction-start. Raw relocation
offset comparison therefore remains nonexact for those entries. The existing
receipt now separately reports narrowly checked DTK instruction-application
equivalence, retaining raw offsets and differences. The source-selected MWLD
link and exact retail bytes independently prove application for this batch.
This diagnostic alone never authorizes a mismatch or replaces link proof.

Local evidence: `build/dol-batch/batch-object-proof.json`, per-owner physical
receipts and baseline/champion objdiff files; proof workspace
`build/dol-small-batch-observation.json`. Use normal GC1.2.5n O4,p GSSDK flags;
the GC1.3 context diagnostic regressed and is not retained.

## Partial gains and next batch

- ctxdata's local static const virtual table fixes FillContextV2VirtualTable and
  its read-only ownership. Keep locally; its three pointer helpers still need
  reconstruction. It is not marked Matching or included in source promotion.
- NewMore's five functions become instruction-exact with object-local
  `-str nopool`; extab and extabindex also match. Retain the compiler-option
  correction, but keep the owner NonMatching. Linking exposes real external
  ownership seams: lbl_8024536C is the exception vtable; lbl_802166D8 is a
  separate all-zero object consumed by kerent and currently assigned to this
  split. Do not add fake zero padding or source label transplants.
- Undersampler int/u32 narrowing/reuse probes, DelayBlock zero spelling,
  Stationarity scope/direct-load forms and VQ comma initialization were neutral
  or worse. Their live sources are restored. None exhausts its function.
- CombinerProcess preparation for the next batch retains 87.40 -> 97.55%
  with target size restored to 840 bytes: consume the first input sample once
  and delay the band-dependent pointer advance until after the scalar header
  writes. Its sibling initialization loop experiment regressed and was restored.
  Current binding is `build/dol-batch/c16/combiner.json`; this partial owner is
  not selected in the present source promotion.

The public agent gate passed, including the full workflow suite and live-input
review. Both affected tooling test modules were additionally rerun after the
final edits: 38 tests, one opt-in fixture skipped, no failures.

Promote the two recovered source files together and the reviewed configuration
changes together; never merge the investigation branch into main. Tooling and
this notebook stay private to the recovery workspace.
