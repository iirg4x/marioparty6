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

## Voicing: three function gains retained

The owner now has **4/6 strict/data-exact functions**, up from 1/6. Its original
2320-byte `.text` is now the target size, **2168 bytes**. No exact sibling was
lost. This is a retained partial owner, not a new Matching owner or main push.

| Function | Before | Retained strict/data | Target/source bytes |
| --- | ---: | ---: | ---: |
| Voicing_AddSignal | 95.277780% | 100% | 216/216 |
| Voicing_MaintainNoiseEner | 96.818184% | 100% | 220/220 |
| Voicing_GetMaxVoicing | 88.858490% | 99.481130% | 848/848 |
| Voicing_Reset | 80% | 100% | 160/160 |
| InitVoicing | 66.205300% | 99.801320% | 604/604 |
| Voicing_Free | 100% | 100% | 120/120 |

Reset's real slot store must precede publishing the advanced ring cursor.
Correcting that source order made Reset exact and stopped its wrong inlined
body from bloating InitVoicing. The initializer also needs the actual `floorf`
and `ceilf` float interfaces: the prior math header expanded floorf through
double floor and did not declare ceilf. Typed calls, target field-publication
order, last-use period-to-sample conversion, and live allocator-result chains
removed 128 excess InitVoicing bytes. AddSignal's outcome initialization belongs
before its two buffer captures.

The repeated backwards ring-energy traversals were an **inline helper source
boundary**, not three unrelated register problems. The ordinary
`static inline Voicing_LogEnergy(block, lag)` reconstruction made
MaintainNoiseEner exact, removed GetMaxVoicing's last extra instruction, and
fixed the return-value/guard boundary. No forced-inline pragma, ASM, padding,
fake local, or compiler-option change was added. The helper name is descriptive
reconstruction, not a claim of recovered original naming. The remaining
GetMaxVoicing mismatch is a saved-FPR allocation/reuse problem; InitVoicing has
three multiplication operand rows. Do not repeat the neutral isolated operand
swap or shared-zero test, or the regressing separate period-conversion helper.

Retained source SHA-256:
`a35102b37deefdee356425780b73086ae589587aa44006c6278c97744c17db0f`.
Object: `81f0e67059233708e41ce406cabb1d7583ad8b9ca1c558a672434e449b6bec5b`.
Strict/data: `1aa7f5f50cfae9044274d265cd7850568124774aacfc8aec7dcdeb752f243743`.
Proof: `build/small-first-20260913/voicing-inline-energy-helper/frontier-proof.json`.
Independent compilation of the retained live source reproduced that object.

AddSignal, Reset and Free have raw exact physical receipts. MaintainNoiseEner's
three relocations agree under the already implemented, explicitly recorded
DTK 0.9.2 instruction-site convention: two SDA21 records address the instruction
versus its halfword. Raw offset identity is **not** claimed. The `.sdata2`
payload is byte-identical for 60 bytes; the target has a four-byte alignment
tail, confirmed by the existing section-tail diagnostic. No fake data producer
was added. Final source-selected link/promotion remains pending owner closure.
Batch count remains **1/10**, local **348/396**, main **347/396**.

## Cursor producer context and Qwen validation reliability

`recovery_causal_groups.producer_slice` now understands `subi` and numeric
`lwzu/lfsu/lfdu` updates. It distinguishes the loaded value from the updated
cursor and binds each use to the pre-update base. Invalid/symbolic update
forms and unknown CFG entries remain explicit UNKNOWN. Existing
`decision_packet` accepts optional, bounded `producer_sites` context; legacy
output is unchanged. On current Smoother evidence it resolves rows 118/123's
old/new cursor relationship and rows 120/128's actual load producers. This is
better input for the named source decision, **not a newly discovered source
gain**. The full prior packet already included the setup statements.
Actual-use receipt: `build/dol-small-next-20260913/smoother-producer-context-replay.json`
(SHA-256 `cc034be32bae4025825c669d627721b993e7bbb07164d181514c402b72c2059e`).
It verifies 12 target/candidate dependency links with a 16,315-byte packet.

The installed Qwen runner no longer wastes a naturally completed inference
solely because the validator was updated while it ran. For validator-only
drift it validates the original exact prompt and final answer with the current
validator, pinning/rechecking all hashes. Prompt/packet drift, incompatible
validation, mid-validation mutation, partial completion, and exit-zero empty
or rejected validator receipts still fail closed. There is no HTTP retry,
model restart, output cap, or reasoning reduction. Old rejected job receipts
are not rewritten. Primary review remains required: syntactic validity does
not prove a factual finding.

Installed runner SHA-256:
`2f70d9916f9698c1d23ddcfd04ff1e2e589c4c21071e8df41f6b2ff10d9d96a0`
at `C:/Users/Anony/.codex/tools/qwen-support/run-job.ps1`; usage is documented in
the adjacent README. The 26 fake-HTTP replay tests pass, including one-request
validator-only drift, cancellation, four real concurrent slots, and malformed
validation statuses. The causal/header/section-tail suites pass 93 tests (one
platform skip), for **119 tests total, one skipped**. These tests do not run a
model or owner compiler.

This round's two real Qwen jobs finished naturally but were rejected by the
old validator-drift behavior. Their raw answers were reviewed without retry:
Pitch's proposed cached single rate conversion contradicts the two target
conversions and is not compiled; Smoother confirmed the existing column-base
recomputation but supplied no new winning source change. They are not credited
with the Voicing gains. The runner fix addresses wasted inference, not model
translation accuracy by itself.

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
