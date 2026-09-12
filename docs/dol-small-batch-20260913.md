# Small DOL batch — 2026-09-13

Main starts at `086a3a84fac9d928c9176df3624582fd28befd45`, **347/396**
Matching DOL owners. This is the next ten-owner batch, not ten new closures.
Current locally verified frontier: **348/396**, **1/10** batch owners.

## NewMore: closed by the real adjacent provider

`Runtime.PPCEABI.H/NewMore.cp` is now source-selected and exact: five functions,
276 instruction bytes, 22 effective physical relocations, strict/data 100%.
Its source was already correct under the retained GC/2.6 exception/RTTI flags
and `-str nopool`; no function or artificial padding was added.

The original split incorrectly assigned the following `ptmf.c` object's
`__ptmf_null` to NewMore. NewMore's 90 string bytes align to 96; the real
12-byte const PTMF null object plus four bytes of normal alignment explains
the original 112-byte combined range. Correct ownership is:

- NewMore `.rodata`: `0x80216678..0x802166D8`.
- PTMF `.rodata`: `0x802166D8..0x802166E8`; `__ptmf_null` size 12.

The exception vtable is now identified by its real C++ symbol. Five string
extents and the bad-alloc RTTI extent exclude ordinary linker alignment;
target bytes and addresses are unchanged. The existing kerent export thunk
still has exactly the same branch bytes. Its imported label is now
`__ptmf_null`. MWCC requires a function-kind declaration for that existing ASM
branch operand; the source explicitly documents that the real provider is
data, not a callable C function. This preserves an inherited compatibility
thunk; it does not introduce a new ASM matching implementation.

Independent proof also preserves the PTMF and kerent instruction bodies. The
real source-selected Ninja/MWLD link passes all **137** configured retail
checksums (DOL plus 136 RELs). DOL SHA-256 remains
`172ae27aa6fcc9074de286b07ae4ac9f9152175fa913fffc3f5b8365e139ffec`.

Compact proof: `build/dol-small-next-20260913/newmore-provider/live-proof.json`;
physical rows: the adjacent `live-physical.json`. Source hash:
`147061638875e6bed3ffe582a20ac7bed847ba653554909b6567f7fe05bfdf79`;
candidate object `6f115f477d0cbc50510796ebfa290ca5d7a8cf17ed8062386be2b20dfe0455b5`;
original target `3c3d6d861ac59a091b8f210d94cbf01fb0c5c4b5cf673eb99b1ef24a12203a2d`.

## Closed tooling gap

The existing `tools/pool_reloc_summary.py` now accepts:

```text
section-tail TARGET_OBJECT CANDIDATE_OBJECT --provider ADJACENT_SOURCE_OBJECT
```

It distinguishes an alignment-only tail from a byte-compatible real named
provider and recommends inspecting the split boundary. It binds the ELF
hashes, sizes, actual object symbol, alignment hypothesis and compared bytes.
Ambiguous providers, unsafe alignment, wrong bytes, missing sections and
relocations in compared data return UNKNOWN/ambiguous rather than an owner
claim. Zero-byte coincidence alone is explicitly not ownership proof. It
neither edits a split nor advances matching authority.

The live original NewMore replay identifies `90 + 6 + __ptmf_null(12) + 4`.
This automates the diagnosis of the known closure; it did not discover a
second gain. Twenty-one focused tests pass, including legacy CLI behavior,
malformed ELF diagnostics and separate tool-code/result hash checks.

## Continuing small-owner work

Fresh unchanged-source objects reproduced GenderFilter 3/4 exact (constructor
79%) and Undersampler 3/4 exact (initializer 99.6875%). Two Qwen read-only
support jobs ran concurrently with the NewMore work. Neither supplied a new
retained source gain. An isolated constructor-profile-helper reconstruction
regressed and is not retained; the actual source remains unchanged. The
existing delayed/direct quotient and helper/count controls remain measured
constraints, not reasons to freeze either owner. Do not repeat those cells.

Small-owner scheduling must consider the remaining source cause, not just
file size or a high percentage. Keep the full current machine slice and the
compact rejected source classes in future support packets; do not ask a fresh
model to rediscover a known neutral conversion or helper boundary.

## Actual-header selection and compact compiler output

The reusable proof checkout had an older `gssdk/triggerlr.h` declaring
`SlidingHisto_Init` unsigned, while the live source/header correctly use a
signed result. The unchanged-source scratch compile failed because its include
search selected that old header. Source was not repaired to fit the stale copy.

`tools/compile_recovery_candidate.py` now accepts the optional
`--reference-include-root include` (Python: `reference_include_roots=[...]`).
It checks every intended-root file against the **first explicit search match**,
rejects stale/missing/conflicting headers before launch, and rechecks mutations
before publishing. It does not rewrite flags or claim implicit/system/source-local
dependency tracing. Scratch callers now put the live include root before the
proof root; generated headers and compiler paths still come from the proof tree.

The live check rejects the stale proof-first root and accepts all 310 intended
headers with the current root first. SlidingHistory then compiles successfully
at the unchanged 6/8 exact frontier. Artifact:
`build/dol-small-next-20260913/header-root-live-check.json`.

Use `--summary` with preflight/compile to avoid printing the full header census.
The compact result retains binding hashes/counts (and the full receipt path for
compile); full API/default CLI output is unchanged. Thirty-one focused tests
pass, including stale shadowing, lower-priority duplicates, mutation/alias
rejection, and bounded summaries with 312 headers. The 21 section-tail tests
also pass. These are input/reliability fixes, not owner closures.

## PitchWindow partial gain

`InitPitchWindow` retains **99.788140 -> 99.830505%** strict/data at 472/472
bytes. Reusing the real single-precision duration for its last sample-count
product makes the final double count write directly to the target saved owner.
The four residual rows remain a smaller volatile temporary conflict; this is
not an exact function. All other function bytes, the three exact siblings,
allocated data and all 14 effective relocation placements/targets are unchanged.
Inherited physical/link differences remain unclosed; no new link is claimed.

Source `4bd7b76196456c3a808dc874976a61b05e4b43edadbbdd942ffadf054120b9e4`;
object `bd17068039e6a5ceaac0ff1fb8916417e74aba2474d4944d2968d867071013f8`.
Proof: `build/small-first-20260913/pitch-single-product-reuse/frontier-proof.json`.
A separate fresh compile of the retained live source reproduced that object.
The batch remains **1/10 owners**, local **348/396**, main **347/396**.

Bounded constraints from this round: Exev context-lifecycle reuse and Sliding
relative-bin-first were object-neutral. Shared Sliding bin helpers changed
labels but not residuals/frame. GC1.2.5e damaged exact siblings in the three
checked unchanged-source owners; retain GC1.2.5n, not a global compiler switch.
Pitch double-extent reuse fixed the saved-owner transition but introduced a
nonretail `frsp`; preserving the real f32 product produced the retained gain.
Moving length publication later regressed and is not retained.

The two Qwen jobs in `build/qwen-exev-sliding-20260913/` used legacy text
packets. Do not repeat that broad dispatch: subsequent questions must use the
existing `decision_packet`/`render_decision_prompt` plus manifest validator,
one missing fact and current paired rows, as documented by the installed Qwen
runner. Unvalidated prose is support only, never retention proof.
