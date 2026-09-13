# Small DOL batch — 2026-09-13

Main starts at `086a3a84fac9d928c9176df3624582fd28befd45`, **347/396**
Matching DOL owners. This is the next ten-owner batch, not ten new closures.
Current locally verified frontier: **348/396**, **1/10** batch owners.

## Context functions: 4/15 to 13/15 exact

The small `gssdk_lib/gsapi/ctxfuncs.c` owner originally had ten missing source
functions. All fifteen bodies are now reconstructed; **13/15** pass strict,
data and independent raw physical checks. No exact sibling was lost. The
complete source `.text` is the retail **2500 bytes** and all **52 relocation
occurrences** agree. Two functions remain nonexact, so this is a retained
partial owner, not a Matching promotion or a new DOL-link claim.

New exact functions: `SessionDataExport`, `SessionDataImport`,
`ContextActivateParams`, `ContextDeActivate`, `ContextGetParam`,
`ContextSetParam`, `ContextSetWrdData`, `ContextUnLoad`, and `ContextSetGcdData`.
The four previously exact bodies remain exact. Remaining strict/data scores:
`ContextActivate` **99.8375%** and `ContextGetAction` **99.42308%**.
Activate has an eight-byte frame difference; GetAction has an eight-row index/result-base
cycle in its first-match scan. No padding or fake local was added to fix frames.

Live source `fda0c328c3e41f8ed316d502484ad1eff256e85b87c530f3a9981b66d31a92ca`;
object `ba05e59dea29297af031a07dbb92ed05f51b1ee99fdd7b1fe9642330fbaeeaeb`;
target `aa14666733e8e64cdd22786dc385e75659a45f7100733ef0c1670917bdc01211`;
strict/data `f23881ef6b99a051ebb1c696d085045c69cf9cdd88946fe2a15af375a7e065e7`.
Proof: `build/small-first-20260913/gsctx-chunk-field-snapshot/frontier-proof.json`.
The owner is still unpromoted; main and batch counts above are unchanged.

The productive path was target assembly plus m2c reconstruction, actual callee
interfaces and consumer layouts, then ordinary indexed C. m2c's inferred
argument counts were not accepted blindly: the real `Wrd*` source and reaching
argument registers resolve the missing second/third arguments. The private
record/field names are descriptive reconstructions, not recovered original
names. Runtime and loaded-context layouts are separate; no shared ABI guess,
ASM, compiler-option change or artificial storage is involved.

`ContextActivateParams` demonstrates the useful next source question. Retail
iterates input IDs 0..17, but the actual 18-entry `TranslateParamTable` contains
one 255 sentinel, so only 17 output entries exist. Correcting output capacity
and the inclusive loop limit fixed the frame/domain. Replacing the explicit
output cursor with `parameters[count]` let the compiler create its genuine
induction temporary and closed the last seven register rows. GetAction likewise
improved 87.333336 -> 97.051285 -> 99.42308 by snapshotting the repeatedly used
count and using indexed source/result arrays. A last-match loop needs the
snapshot because output can alias the table; an early-exit scan does not prove
the same source owner. Local declaration, signed-index and initialization-order
variants that produced the same object are recorded as neutral constraints,
not reasons to exhaust or freeze the function.

`ContextSetGcdData` then closed in two cheap, causally connected compiles.
Its real on-disk chunk header is an eight-byte `{type, size}` record. A local
whole-header copy explained the missing frame extent but added four nonretail
stack instructions (368 -> 384 bytes); that regression was not retained.
Assigning the same two live header fields separately let MWCC forward both
values while preserving the record's stack extent: **368/368 bytes, 100%
strict/data, 3/3 raw physical relocations**, all twelve exact siblings preserved.
No padding, unused field, fabricated aggregate, or compiler switch was added.
This is a bounded reconstruction lesson: a frame-only residual can be caused
by a genuine scalar-forwarded record, not necessarily a missing scalar local.
It is not a rule to wrap arbitrary locals in structs. The eight-byte data record
and both field consumers independently justify this particular source shape.

Subsequent first-match early return, direct action-pointer consumption,
countdown scan, shared indexed scan position, and case-local indexed extents
did not improve GetAction; the live 13/15 champion remains intact. The optional
export's assignment-condition spelling was object-neutral. Qwen's delayed Mel
block lifetime was tested in valid C89 scope and was also object-neutral.

Source-fidelity caveat: the target `SessionDataExport` writes the allocated
header before its null check. The reconstruction preserves that legacy order;
this is not a claim that the original allocation-failure path is memory-safe.

## Missing-function tools repaired and used

- Existing `tools/decompctx.py` now exports pure `adapt_target_function` for
  GNU target disassembly when pinned DTK's ELF disassembler fails. It checks
  ELF32 big-endian PowerPC identity, function bytes/counts, branch destinations,
  complete relocation census, REL24 link-bit and immediate/memory forms. It
  does not infer signatures, invoke a compiler, edit source or prove a match.
  Existing include-flattener/CLI behavior is unchanged. The scratch caller now
  uses this implementation, explicit target/tool hashes and subprocess deadlines
  rather than a duplicated parser or optimization-disabled assertions.
- The adapter produced usable m2c input for all ten missing functions after
  the DTK panic. The promoted converter was exercised on current Params/Gcd
  targets; both m2c invocations succeeded. All 13 adapter/context tests pass,
  including all fifteen actual ctxfuncs targets, the old 352-instruction callback
  and a real SDA21 target. That replay validates the converter, not a new crack.
- Existing `recovery_causal_groups.decision_packet` accepts opt-in
  `allow_missing_candidate=True` for factual reconstruction support. It emits
  real target rows, explicit null/unavailable candidate evidence, and marks the
  source excerpt as context, never a fabricated function body. Source-replacement
  mode is rejected when the candidate is absent.
- Real Qwen use revealed a second gap: without addresses it swapped the two
  switch arms despite citing valid row numbers. New target-only packets bind
  each instruction address and direct-branch destination to included row IDs.
  Actual replay proves `0x4c0 -> row35` and `0x510 -> row55`; rehashed swapped
  mappings and partial maps fail validation. Older in-flight packets retain
  byte-identical prompts and factual validation, so a validator update does not
  discard ongoing inference. New packets always include the address facts.

The two target-only Qwen jobs run in parallel with primary reconstruction;
neither is a compile/consensus gate. The incorrect action-layout answer was
rejected by primary review. Their schema validity is not semantic proof and
none of the eight new exact functions is credited to an unverified model claim.
Affected tooling suite: **119 tests run, 23 skipped** (platform/opt-in cases);
the explicit local adapter replay separately passes **13/13**. The gaps are
fixed in existing tools, not a new recovery engine.

## Duplicate-symbol census repaired on the next small owner

The unchanged `langdata` compile succeeded, but its score census stopped on
two distinct target functions both named `_langGetNbrSpeechUnit`. The existing
`summarize_match_scores` now preserves each report-local symbol index, checks
reciprocal pair indices, and distinguishes separate extents from same-extent
aliases. Unique-name output is unchanged. An unpaired or null-scored symbol
stays unresolved; neither names nor identical bytes transfer an exact score.

Read-only replay of the existing report yields **76 target function symbols:
66 score-exact, nine mismatches, one unpaired**. Candidate
`_langGetNbrWarpFactors` remains candidate-only. The second target extent at
`0xEC` has the same twelve bytes as that candidate, but this is not original
symbol-name evidence or a new crack. No object was rebuilt just to recover the
census; the absent old compile binding remains explicitly unavailable.
All pre-existing object/report hashes are preserved. The causal/Qwen suite
passes **80 tests, 22 skipped**, including an actual langdata report replay.
This removes a real analysis blocker without adding another tool or model run.

The repaired census was then used for a retained source gain:
`_cdbGetCodeBookSize` **81.42857 -> 84.77143% strict/data**, **136 -> 140
bytes** (the retail size). A real `LanguageDataV2 *data` snapshot shared by both
flag tests restores the target's second flags load. There is no volatile,
synthetic write, ABI change, or new helper. All **75 other function bodies**
and allocated data payloads are byte-identical to the previous candidate;
all **66 score-exact siblings** survive. The changed function has no
relocations and its physical check is exact. This is a partial gain, not a
new exact function, owner closure, or source-linked DOL claim.

Retained source `a7223284d02bd39778f4cccdff6edea140b2106e264e75043b8a48d19c71e530`;
object `d3000e9fb88f200590d40afde62996f778524859ead831391f259aef23acf275`;
strict `b78646696902987215c1703190127733a0a700ed01d25b52b3aa6ec2316606ce`;
data `05d32b6dd10ba208ce55d7bcdf1b01d60064319148db7ba888244c7868fb144f`.
Proof: `build/small-first-20260913/lang-codebook-data-snapshot/frontier-proof.json`.
Replacing the subsequent incremental size calculation with one expression
over-collapsed it to 124 bytes and was rejected. Reconstructing a probability
pointer boundary or duplicating the tone traversal also regressed and was not
retained. These findings constrain those source hypotheses, not the functions.

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

## Mel: source reconstruction gains and remaining causes

The retained live owner still has **2/5 exact functions**; no new owner closure
is counted. Both exact siblings survive. Improvements are retained in the
working branch, not promoted to main:

| Function | Original strict/data | Retained strict/data | Target/source bytes |
| --- | ---: | ---: | ---: |
| ProcessMel | 93.5% | 93.5% | 440/440 |
| ControlMel | 72.53488% | 99.30232% | 344/344 |
| MelInitBands | 93.87539% | 99.43302% | 1284/1284 |
| InitMel | 100% | 100% | 108/108 |
| ConstructMel | 100% | 100% | 68/68 |

Control's repeated teardown is an ordinary inline helper with a live cached
context. The command-1 caller retains its independent guard. Reading the actual
fourth argument's float payload directly removes the extra copied local home;
logical status inversion removes the nonretail negate. Its only remaining
instruction difference is the final byte-to-word return extension. The actual
callback interface remains u32: changing the function to u8 was rejected by the
compiler, not hidden with a function-pointer cast or shared-header change.

MelInitBands needed unsigned bin-count arithmetic, the real profile narrowing,
direct band-ordinal consumers, and one selected triangular-weight store.
Indexed center-frequency stores remove an artificial extra cursor, closing 29
more rows. Owner `.text` shrank from 2296 to the target **2244 bytes**;
`.sdata2` shrank from 104 to the target **96 byte-identical bytes**. The compiler
stays pinned to GC/1.2.5n. No forced inline, ASM, register directive, padding,
or invented storage was added. Descriptive helper/local names remain
reconstructions, not recovered original identities.

Live source SHA-256:
`276b306f9d15742b8207505565f6c2032e1b7b3dbb2ab3849f3043d9f5dbc2df`.
Object: `d6534eb282c920b9a2faba1abcbeee6b8f582d872addc270bd6bda9c29b6598c`.
Strict/data: `ae336f553d56003095517eab1b66c5685e41d937d76a4911c3f6c50d4d020bbc`.
Proof: `build/small-first-20260913/mel-indexed-centers/frontier-proof.json`.
A fresh compile of live source reproduced that object at
`build/small-remaining-baseline-20260913/gssdk_lib/asrpho/common/blocks/flblocks/mel/retained-indexed-centers/`.
Control's 10 relocations and both exact siblings' receipts are raw exact.
ProcessMel and MelInitBands retain physical differences; no new source-linked
DOL proof is claimed. Batch stays **1/10**, local **348/396**, main **347/396**.

Remaining useful constraints: MelInitBands' 52 ARG rows include an 8-byte frame
delta and FPR/volatile-owner cycles. A direct two-arm limit assignment fixes
the frame but adds a move; default-limit assignment removes a target branch.
Neither replaced the champion. Putting the ternary directly in the loop
condition regresses; ordinary inline-minimum and frequency-for forms are
instruction-neutral. ProcessMel's signed index is object-neutral, indexed
output regresses, and an accumulation helper closes only three rows without
solving the owner cycle. These constrain those measured hypotheses, not the
whole functions or distinct coupled source causes.

## Concrete support proposals and recoverable score summaries

The existing `recovery_causal_groups.decision_packet` now has an opt-in
`decision_mode="source-hypothesis"`. Default factual packets/prompts remain
byte-compatible. A hypothesis must cite supplied rows, state uncertainty, and
provide one `source_change={before, after}`: the before text must occur exactly
once in the sealed excerpt and the after text must be nonempty and different.
It may express coupled statements; it is not an automatically applied patch,
compiler instruction, or retention decision. The primary must still reject
ABI guesses, fake operations, known-neutral repeats, and unsupported claims.

This closes a real prompt defect observed on Mel: factual-only support demanded
unknowable original-source identity; the first permissive hypothesis replay
then merely redescribed the existing u8 return and loop index. Those two real
answers are not source gains. The tightened prompt asks for a concrete new
replacement or `insufficient`, rather than accepting prose as a next source
decision. Installed Qwen receipt compatibility is preserved (`valid_finding`
with explicit hypothesis/review/no-authority fields); schema validity does not
prove the proposal will match. New real replay:
`build/qwen-mel-source-change-20260913/`.

`summarize_match_scores` in that same tracked module is now shared by the
baseline and candidate scratch drivers. Missing target/candidate functions,
null/alias scores, and duplicate names cannot crash halfway through a census
or turn missing source into an exact result. Score exactness is explicitly
separate from physical/instruction proof. Both drivers' `--report-only` mode
reads retained objects/reports without compiling or rewriting historical
bindings. It recovers the ctxfuncs census (15 functions, 4 score-exact, 11
residuals), including ten absent source functions, from the existing artifacts.
Unavailable old bindings stay unavailable; no provenance is fabricated.

Affected causal/header/pool tests: **96 run, one skipped**. Both-driver fixtures
also verify object/report/binding preservation on successful recovery,
malformed JSON, and mismatched object bindings. No compiler or model is run by
those fixtures. Tool results and retained source gains are counted separately.

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
