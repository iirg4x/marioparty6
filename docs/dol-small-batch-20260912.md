# Small DOL batch, 2026-09-12

## Ten-owner batch merged; observed preflight gaps fixed (2026-09-13)

Main `086a3a84fac9d928c9176df3624582fd28befd45` includes source PR #37 and
supporting/progress PR #38: **347/396 DOL owners Matching, 49 remaining**.
The batch contains 99 functions across ten owners. The separate clean
main-derived supporting build passed all137 checksum checks and reproduced
the retail DOL; progress consistency tests and public CI passed. Earlier
"pending" snapshots below are historical, not current promotion status.

Two existing-tool changes address actual time lost in this batch:

- `compile_recovery_candidate.py` now records ordered explicit compiler
  include roots and response-file hashes, checks them before/after compilation,
  and accepts repeatable `--reference-header INCLUDE=FILE` bindings. A current
  scratch/include no longer hides a stale external include directory when the
  relevant header is reference-bound. The real hardware invocation passes with
  the corrected DSPvoice/snd headers; an isolated overlay of the original proof
  headers is rejected before compilation. The scratch runner now uses that
  check. This is explicit-path evidence, not a complete preprocessor trace:
  opaque scripts and implicit/source-local lookup remain explicitly identified.
- `crack_evidence_bundle.py` checks every configured block-form DTK DOL/REL
  input against the sealed retail tree before staging or launching commands.
  A .gitkeep directory or main.dol without m699Dll.rel is diagnosed directly,
  rather than discovered after configuration. Complete central inputs pass.

Both affected suites passed **46 tests**, including all15 combinations of
header/reference/response-file inputs against object/receipt/log destinations;
aliases reject before any input is overwritten. The live non-compile regression
receipt is `build/dol-batch/preflight-gap-live-check-20260913.json`.
These checks prevent known wasted setup/source probes; they do not claim a
new owner gain or automatic source reconstruction. No new standalone tool,
candidate-approval gate, or raw-history requirement was added.

## Hardware closed: ten-owner batch ready (2026-09-13)

Final source review: named allocator constants replace raw hexadecimal spelling,
and DelayBlock's two redundant one-byte padding members were removed in favor
of natural C alignment. Both complete production objects remain byte-identical.
Window's existing halfword at bytes56..57 remains explicitly unresolved: target
stores at52/54/58/60 and the80-byte allocation authenticate the gap, not its
semantic identity. The SDK's volatile ITD table and this one real unknown field
have narrowly documented source-quality entries; no general gate is disabled.
Evidence: `build/dol-batch/presentation-cleanup-20260913.json`.

`musyx/runtime/hardware.c` is source-selected Matching: **44/44 raw instruction
bodies**, strict/data 100% after authenticated symbol-name reconciliation,
**178/178 physical sites/types/resolved targets**, and the retail-identical DOL.
All **137 container checksums pass**; the proof workspace is **347/396** DOL
owners. Pending batch: **10/10**, not yet a main promotion.

The decisive cause was SDK interface drift, not a register puzzle. The old
proof headers lacked the 2.0.1+ DSPvoice filter member and the floating-return
sndSqrt/sndCos declarations. Correct headers remove fourteen unwanted conversion
instructions and restore the 248-byte voice stride. The SDK's volatile const
ITD lookup restores the target's two actual table reads, closing the last four
bytes. Its ordinary static helper spelling is code-neutral. No new assembly,
padding, fake local or register hint was introduced.

Donor: clean AxioDL/musyx `adc8df9a959f1e37f71bdf3155e229f9f87ad166`;
hardware blob `8251f3bded9ab42c00f00060d11346b00014e1e3`, dspvoice header
`d2401bc9b1df83fa3f249d396bb704cf58f1d2be`, snd header
`b73a20fdf8c914a0cbc04d8674f61428b5beb90e`. Qwen independently identified the
stale filter interface; the primary completed the source and link corrections.

The exact relocation sites authenticate the SDK global names. `salHooks` is
the genuine eight-byte two-callback object; its old second-word label becomes
the +4 field rather than a fake separate global. Canonical target metadata
changes no allocated target bytes. Two final sdata alignment bytes and sixteen
unreferenced weak-sqrtf constant bytes are handled by the real linker, not
source padding or a header suppression hack.

Source SHA-256: `8c497e96fd162b3ed1303298eb27233e64d86c56e5bd1cc94333802fdd83a06e`.
Object SHA-256: `bec8491939c426a5b6bc9d6a996701e4cdf65c9b9b6930d5acc140326367fa0e`.
Original target: `e914981e1e09f2016f0f860dcb69b61cae43fc4af69c9d0d4e47132c6738629f`.
DOL: `172ae27aa6fcc9074de286b07ae4ac9f9152175fa913fffc3f5b8365e139ffec`.
Evidence: `build/small-remaining-baseline-20260913/musyx/runtime/hardware/sdk-table-qualification/live-*`.

## StdReverb closed: ninth verified owner (2026-09-13)

`musyx/runtime/StdReverb/reverb.c` is source-selected `Matching`: **5/5
instruction bodies**, strict/data 100% after authenticated alias mapping,
**49/49 physical relocation sites/types/resolved targets**, and a retail-identical
DOL. All **137 container checksums pass**; the proof workspace is **346/396**
DOL owners. The pending batch is **9/10**; main has not been advanced yet.

The actual cause was a missing SDK version conditional. The three crosstalk
instructions that load/apply `value0_6` belong only to MUSYX <=2.0.0; this
target uses 2.0.4. Restoring both existing guards changes DoCrossTalk from
400 to 388 bytes and restores the 0.3-before-0.6 constant producer order.
The clean MP4 MUSYX donor is AxioDL/musyx commit
`adc8df9a959f1e37f71bdf3155e229f9f87ad166`, file blob
`8e50bc904e8dd789630c1dabc1c17fb0ef6045d0`, same relative source path.

Source-fidelity disclosure: this owner already contains the authentic SDK
native-assembly kernels DoCrossTalk and HandleReverb. This change restores
version selection; it is **not an all-C rewrite** and adds no instruction,
padding, or register hint. The target's address names fn_800EC408/fn_800EC58C
map to those SDK names at identical linked addresses. A names-only disposable
target view leaves original allocated bytes untouched. Unreferenced
ReverbHIModify and weak sqrtf constants are stripped naturally; no synthetic
data or math-header workaround was introduced.

Source SHA-256: `c5a5e1b26d9cbbc5ad1279ed742e3f7a87fd764cbbe503e2aaca303e59fe27e1`.
Object SHA-256: `0b2a09de9e197b46ba1f8c5a508f99ee04567efdb223e81aaa95efe828adb20e`.
Target SHA-256: `18b56068fa1f4330626e3fd8fee707944cad7333c6b3f16e9b071c1db30dbff2`.
DOL SHA-256: `172ae27aa6fcc9074de286b07ae4ac9f9152175fa913fffc3f5b8365e139ffec`.
Evidence: `build/small-remaining-baseline-20260913/musyx/runtime/StdReverb/reverb/sdk-201-guard/live-*`.

Small-owner scheduling lesson: estimate the remaining causal work, not just
file size or match percentage. A version-selection defect closed this whole
owner in one source candidate; several tiny 99% register tails remain harder.

## Allocator closed: eighth verified owner (2026-09-13)

`MSL_C.PPCEABI.bare.H/alloc.c` is now source-selected `Matching`: **11/11
instruction bodies**, **50/50 relocation sites/types/resolved link targets**,
strict/data 100% after explicit authenticated symbol-name reconciliation,
and a byte-identical retail DOL. All **137 container checksums pass**.
Proof workspace DOL status is **345/396**. Public main is not advanced by
this local proof; the pending batch is now **8/10**: Median, FFT, Window,
Matrix, Stationarity, DelayBlock, NMWException, and Allocator.

The last constructor needed a byte-buffer cursor and a live byte successor,
computed before initializing the header. Typed-node iteration had encouraged
eight-pointer precomputation or field reloads; two byte cursors plus the
correct calculation/store order restore the actual eight-way compiler unroll
without manual unrolling, register hints, or assembly. The body is **296/296
bytes with zero diff rows**, preserving the other ten functions.

The first real link then exposed definition-order differences. With GC/1.3
deferred inlining, public definitions `malloc`, `free`, `calloc` and helper
definitions `Block_subBlock`, `Block_link`, `SubBlock_merge_next` emit in the
required reverse order. Moving unchanged definitions restored the retail
layout and closed every DOL byte. No linker ordering override was added.

The original target remains untouched. Six placeholder function names and
three data names are bound to the same source-selected linked addresses; a
disposable names-only target view supplies strict comparison. Original raw
function bytes and physical relocation offsets/types are checked separately.
The linker strips unused helper bodies and retains the target guard alignment
(four source bytes within the target's eight-byte split); no source padding
was added. The inherited opaque `MemPool` storage remains naming/type detail,
not an invented new layout. Semantic helper names are recovered descriptions,
not a claim of surviving retail local names.

Source SHA-256: `bd7e9570402cdb99acf51bfad07ffc5f197c7e73e463718b6294df16f1c45a02`.
Production object: `26e8a807c2ed8ed5890117c031f4d743b6911266d5b024bc5a32fb56d8bc5926`.
DOL: `172ae27aa6fcc9074de286b07ae4ac9f9152175fa913fffc3f5b8365e139ffec`.
Proofs: `build/small-profile-msl-20260912/alloc/live-proof.json`,
`live-physical.json`, `live-strict.json`, and `live-data.json`.
Qwen's bounded constructor question was cancelled after the primary solved
and verified it; no waiting or consensus gate was introduced.

## Small-owner allocator frontier: 10/11 instruction-exact (2026-09-13)

Prioritize estimated remaining closure work, not merely source-file size:
small near-complete owners and bounded SDK profile corrections precede the
large LangData tail. The verified pending batch remains **7/10 owners**.

`MSL_C.PPCEABI.bare.H/alloc.c` now has **10/11 raw instruction-byte matches
and data-mode 100% functions** with the object-local **GC/1.3** compiler.
The owner remains `NonMatching`: only `SubBlock_merge_next` and
`Block_subBlock` currently also pass strict and raw physical comparison;
eight instruction-exact functions still require target/source symbol, data,
and final source-selected link reconciliation. This is retained progress,
not an eleventh or eighth owner closure.

The compiler correction resolves widespread GC/2.6 scheduling differences;
GC/1.3.2 does not reproduce this target. Natural source repairs remove the
non-retail empty-block store, separate the allocator's real early guards,
join its two allocation results through one live result, and restore the
`calloc` multiply order. `allocate_from_fixed_pools` improved from
97.831856% strict to 99.867256% (data 100%, raw 452-byte equality): removing
the unnecessary local table pointer closed the broad register cycle; placing
the allocation result declaration before its maximum-count peer closed the
remaining two-owner cycle. The three strict rows now concern helper names,
not differing instructions. All nine earlier byte-exact functions survive.

`FixBlock_construct` is the only remaining code residual: 296/296 bytes,
68.756760%. Following its initialized next field retains the target frame
and size but emits reloads where retail forwards the pointer. Direct cursor
progression, a previous-entry local, and integer-address spelling recover
the unwanted eight-pointer precomputation/frame and are not retained. This
is an unroller/source-boundary question, not a reason to permute registers.

Retained source SHA-256:
`588b31862bd32905f231141f172ec3e4886bdf8786a9f12dba13cae1765e7e4e`.
Candidate object:
`d3d2efda1963272d34b970d06779d5a915e93c7a606e745e8f1b270310b087ac`.
Target object:
`34fe30352c290b352cb0d44b44f0c1b1d6063d30c6e6451e797e170a3ae00db3`.
Compact proof: `build/small-profile-msl-20260912/alloc/retained-frontier.json`.
The existing opaque pool storage/initialization guard, source ordering, and
symbol names still need final provenance/link review; no padding, assembly,
shared-header edit, or linker proof is claimed for this partial.

## Small-owner priority: Exev initializer retained, 4/6 exact

Smallest credible owner closures take priority over the 76-function LangData
tail. The pending fully verified batch remains **7/10 owners**; function gains
below do not increment that count.

`InitExtraEventDP` is now **76/76 bytes, strict/data 100%, physical relocations
exact**, preserving the three previously exact siblings. The live C stores the
explicitly narrowed profile result in a live `u32 inputSize`, then assigns it to
the input port. A `u16` result local was neutral; the promoted result boundary
restores the target normalization/load/store/return order. No ABI, constant,
header, or compiler change is involved.

Exev now has **4/6 exact functions**; all six function relocation inventories
are target-exact. `DynProgExtraEventsProcess` remains 95.740740% and
`ControlExtraEventDP` 97.567566%, both at exact size. No owner closure or new
source-selected link is claimed. Source SHA-256:
`b077d7223565a2e55fbe687559d9214fe2051f36898a72c596432bb95fa12ed1`.
Proof: `build/small-exev-current-20260912/exev_dp/retained-initializer-proof.json`.
The current-base state-release/context-birth probes were neutral, a larger
context helper stayed outlined, and phase-iterator reuse regressed. None was
retained. Qwen independently confirmed the score-cache cross-call lifetime;
it did not identify a new structural difference or discover a gain.

## NMWException closed: seven verified owners in the pending batch

The unchanged `Runtime.PPCEABI.H/NMWException.cpp` is **8/8 strict/data exact**
with its object-local compiler corrected from GC/2.6 to **GC/1.3.2**.
`__construct_array` changes from 252 to the retail 248 bytes and 93.854836%
to 100%; the other seven functions stay exact. All five code relocations are
physically exact, and `.text`, `.sdata`, `extab`, and `extabindex` are 100%.
The production build uses that source object, not the target fallback.
The DOL is retail-identical, SHA-256
`172ae27aa6fcc9074de286b07ae4ac9f9152175fa913fffc3f5b8365e139ffec`,
and all 137 container checksum checks pass. Proof workspace DOL status is
**344/396**; public main has not been advanced by this local proof.

The pending batch is **7/10**: Median, FFT, Window, Matrix, Stationarity,
DelayBlock, and NMWException. LangData partials remain committed but paused in
favor of smaller owner closures. No source change was needed for NMWException;
the build-profile correction will use the separate supporting-change promotion.
Evidence: `build/small-runtime-20260912/nmw/live-proof.json`,
`live-physical.json`, `live-strict.json`, and `live-data.json`.

Source SHA-256 is `1e2501aa30ba251af1565a2f8d0449389815b0d4ecf30ee7d567a90a1d0ce403`;
production object SHA-256 is
`7824a19eb725dae0f7f49770e38ae10714737f8679014d42d32e683505943a94`.
GC/2.7 reproduces the old mismatch; moving pointer creation before the guard
object is neutral. This supersedes the old wave24 source-shape blocker:
check an SDK object's compiler provenance before treating an isolated allocator
or optimizer difference as missing source. A profile correction requires full
owner/section/link proof; do not apply GC/1.3.2 indiscriminately to other owners.

Current bounded CtxData accessor-chain probes and Undersampler delayed/direct
quotient probes did not improve their champions; no source was retained.

## Language dispatch table retained: 67/76 instruction-exact

The current language-data owner now defines all **76 functions**: **67 are
strict/data exact; 66 also have exact physical relocations**. This turn adds
`FillLanguageVirtualTable` at **44/44 bytes, 3/3 physical relocations** and
reconstructs the typed 316-byte table: all 75 function-pointer slots and four
null slots agree with the retail dispatch identities. The two identically
named target getters are checked by their distinct byte ranges; slot 0x4c
selects WarpFactors, while slot 0x50 selects SpeechUnit. This corrects the
descriptive header names without editing the target input. No external source
calls these now-private accessors directly.

`_langGetpExtraEventContext` retains **87.034485 -> 95.836205**, shrinking
496 to the target **464 bytes**. The tone pointer uses its existing final-offset
predecessor, restoring a real inline boundary in this consumer. Separating the
16-bit product from the promoted aligned count restores the missing truncation
instruction; no fake storage or register hint is used. `_cdbGetCodeBookSize`
also retains **81.0 -> 81.42857**, unchanged at 136 versus target140 bytes,
through its ordinary conditional probability-column value. All prior exact
functions and already-exact physical channels survive. Four independent old/new
header consumer compiles (`dpgenuw`, `dpscruw`, `vq1500`, `gender`) produce
byte-identical objects. The live-source rebuild reproduces the selected
allocated sections, strict/data metrics, and independently checked relocations.

Live source `bc19d49c39920d4b988741d96de722aac818e50749ddc752f50f8020f30404b3`;
header `1d85951a445d96f801c258d76d860c2af8b8a19b62594e6228de80e9ad3e2465`;
object `00594ae8cf6d33d22605d3219cc07b0a84d59217993b4711767477e09a2a8636`;
strict `551b03cf33dbdc38a6b5df8ead509a7081ca97a29918ff525cdedbbc40063f42`;
data `57e5c609e94c17380cb4e7ea62453a4b0b54f15f797e599b6d4fe103f3010333`.
Compact proof: `build/language-table-20260912/frozen/langdata/retained-frontier.json`
and `build/language-table-20260912/consumer-proof/result.json`.
Nine functions, provider-address alignment, and the source-selected link remain;
this is **not** a seventh closed owner. The verified pending batch stays **6/10**.

Speed corrections applied rather than waiting for more broad worker analyses:
the existing private batch preparer now uses up to four validated Qwen slots,
not its stale two-client default. Reuse the existing decision-first packet mode
for a named missing relationship; primary-only freezes/compiles remain independent.
The broad language-layout jobs completed but invented extra call arguments from
stale volatile registers; one also reversed `subf`/lost a returned pointer.
These findings were not source authority and did not discover the gains above.
The maintained decision prompt now explicitly preserves these PPC dataflow
rules. This is a tested prompting guard, not a claim of automatic correctness
or measured new model throughput; highest uncapped reasoning is unchanged.

Compact constraints for continuation: typed training-pointer/getter composition
is object-neutral; direct m2c arithmetic flattens away target accumulation
boundaries, and signed local dimensions do not repair that. Do not repeat these
cells. The constructor/table and independent header checks were compiled in
parallel; proof reused the objects instead of rerunning discovery.

## Grouped language-data reconstruction: 29 new exact functions

`langdata.c` now retains **37 newly implemented functions**. Of these,
**29 are strict/data instruction-exact and 28 also have raw-exact physical
relocations**. The remaining instruction-exact function,
`_langGetpPhenUserWordTraining`, still has a local-provider address shift.
Eight new functions have partial matches; the existing
`_langGetpErgodicPenalty` improves from 77.04 to 78.04. Every other pre-existing
function's bytes are unchanged, including both getters affected by a duplicate
target symbol name. No owner Matching gate or main progress changed: the
verified pending batch remains six owners, not seven.

The first shared-layout compile produced 24 new exact leaf functions. The
next coherent section-chain reconstruction added the speech-unit, translation,
training and tone traversal bodies together. Two useful source causes then
closed five more functions: ordinary incremental section-size accumulation
preserved the target arithmetic boundaries; consuming the existing lexicon
accessor preserved its inline boundary. Removing an unnecessary named offset
from the translation pointer expression also closed its frame and the inlined
training consumer. These were related source-layout repairs, not a syntax
matrix or per-register search. No assembly, padding, forced inlining, or shared
header change was used. New local layout names are descriptive reconstructions.

Speed practice applied: batch independent getters with one shared layout;
reconstruct related section consumers together; reuse completed compiler
objects/reports for proof; let two narrow Qwen layout questions run without
blocking primary compilation. A duplicate target name must not force a rebuild
or silently count two matches: the proof checks the two getter byte ranges
separately and leaves the symbol-configuration correction explicit. These
results demonstrate retained function output, not a promised owner/hour rate.

Evidence: `build/language-accessors-20260912/langdata/family-champion-*`,
`frontier-proof.json`, `live-source-proof.json`, and non-leaf physical receipts.
The post-patch live-source rebuild reproduced the proved code/data sections,
strict/data results and independently rechecked physical receipts. Its whole
object differs only in the non-loadable symbol/string tables; whole-object
byte identity is not claimed.
Live source SHA-256
`90eacc5b4c54f334acd4360bbbc80d7a1675f834a7102da67991e3e176097991`;
live-source object `f34299f10b033d9c7364ab2db38a5aed8aed5129730b745e317bae05c3705de2`;
strict `ba87ccf7086c9c9cc1325381a0f4245e79e4fb5443f0901db7ba4503ad7c0b60`;
data `d23790ae7139177343726755403994d4c2d7051eec43747dbdfde04316bf0242`.
The target's second `_langGetNbrSpeechUnit` at `.text+0xec` loads the warp-factor
count and byte-matches `_langGetNbrWarpFactors`; the target input was not edited.
Virtual-table reconstruction, remaining source causes, and real source-selected
link proof are still required before this owner can be promoted.

## Reconstruction-first throughput: missing result callback recovered

`asrspi_cbResult` is now implemented in ordinary C and retained at
**97.75284% strict/data**, from an undefined source stub. The reconstructed
function is **1404/1408 bytes**, with **34/34 relocation occurrences**;
relocation placement is not yet exact. All six pre-existing exact callbacks
remain strict/data/physical exact. This is a substantive partial recovery,
not a seventh verified owner or a main promotion.

GNU PowerPC disassembly plus m2c supplied the control-flow starting point.
The pinned DTK object-disassembler failed on this split object, so the
existing GNU disassembler was used, with named relocations preserved for
m2c. Untyped m2c output omitted allocator arguments and incorrectly made
the allocation-size loops appear empty: the reconstruction therefore binds
function signatures and real result/detail layouts to the target loads,
stores, calls, and strides instead of treating raw decompiler output as C.

The first compiler-tested implementation was already **96.11648%**, at
1408 bytes. Correctly expressing the linked-list successor as the next
typed entry removed an unnecessary integer multiply; ordinary initialization
and loop boundaries improved the retained result to 97.75284%. Two Qwen
layout questions run independently of primary reconstruction and compilation;
no consensus or permit waits are imposed. The unresolved 56-byte frame
difference, source lifetimes, and one copy remain genuine reconstruction
questions: no padding, forced-register qualifiers, assembly, or invented
storage was added to hide them. Descriptive field/type names are inferred,
not claimed to be original SDK names.

The shared `gsapi.h` update replaces only opaque reserved ranges with the
observed fields. Independent old/new-header compiles of `extaudio.c` and
`ctxfuncs.c` produce byte-identical objects. Callback source SHA-256:
`1c6f0e33278f69eb150626c9027bfd45f96630e26767943aa5e6e0a54603d406`;
header `8f345a808524f48ba55e4ed2c3adf121496f9c2312fa8d486bfa160e93b72b61`;
object `80b07eeb21110e38478d38c5b606bc8ebe8b2ecce4e9b0d9e39d6c009b2c9772`.
Evidence: `build/callback-current-20260912/callbacks/retained-*`,
`frontier-proof.json`, and the named target-physical receipts.

Practical speed rule: after a closed-register cause becomes underdetermined,
prefer a missing body whose operations and data layout can be reconstructed
directly. Use m2c with real signatures/types, compile that reconstruction
immediately, and retain the gain while support questions continue. This run
demonstrates new source recovery; it does not establish a universal rate or
claim that local token generation is free of GPU/time costs.

## Reused causal lesson: two PitchWindow functions gained, faster triage

PitchWindow now retains **3/5 strict/data exact functions**, up from 1/5.
`ProcessPitchWindow` closes **93.51219 -> 100%, 320 -> 328 bytes** by
capturing the real window pointer before the queue call, as the target does.
`ControlPitchWindow` closes **99.016396 -> 100%, 244 bytes** by expressing
its existing advancing history-fill loop as `FillPitchHistory(block, value)`.
This directly reuses the DelayBlock value-parameter lesson; it is not a
register alias, forced inline directive, or changed loop behavior.
Both functions have **3/3 effective target relocation applications**:
Process uses the separately checked DTK instruction-site convention; Control
is raw-exact. The pre-existing exact constructor remains instruction-exact.

`InitPitchWindow` improves **96.65254 -> 99.78814%, 476 -> 472 bytes**.
The ordinary chained `windowLength = block->windowLength = ...` keeps one
conversion result rather than two stack conversions. Four FPR argument rows
remain. `CreateWindow` is unchanged at 81.52747%; this is **not a seventh owner
closure** and there is no new main/promotion count. Initializer/local-function
physical closure still requires the remaining owner work and real link.
The retained source SHA-256 is
`e878f4dbdbcb3a699d33c1580c63beb2cc21ac81cea051ed7efb085f03dbf554`;
object `e002ef5da4dfbb8d5ab0c898dfc56c506312dc18836e87a8c71d8bc39500ff21`.
Evidence: `build/qwen-next-owner-causes-20260912/pitchwin/fill-and-length-owners-*`
and its named target-physical receipts. Live source equals the frozen candidate.

CtxData separately retains **81.86667 -> 82.2%** for `_contextGetpWordProp`,
60 bytes, zero relocations, and all **19 exact siblings** preserved. Its
aligned-halfword count is a real arithmetic operation; helper name inferred.
The typed-section and byte-base rewrites did not improve it and are not live.
Evidence: `build/qwen-tail-structure-20260912/ctxdata/word-alignment-champion-*`.

The existing causal-group tool now recognizes a same-block `li` producer
copied by target `addi ...,0`/`mr` where the candidate independently emits `li`.
It directs review to a real consumed argument/inline boundary, never asserts
original source identity or automatically inserts a helper. Calls, joins,
unknown/redefined producers, unequal constants, and PPC `addi` zero-base
semantics fail closed. It reproduces the DelayBlock/PitchWindow clue and
recognizes its disappearance in both exact results; these are measured-case
replays, not newly discovered gains. **106 related tests pass, one skipped**.
The current cached-owner scan found no additional eligible copy pattern, so
this is not advertised as unlocking every remaining owner.

Four independent Qwen decisions ran while primary source work continued.
Once PitchWindow Control closed, its now-obsolete question was cancelled;
the other three completed without interruption. Their conservative findings
did not produce the retained PitchWindow changes. Primary-only freezes now
skip Qwen prompt generation entirely in the existing local preparation script,
so a local compile is no longer blocked by a support-prompt byte budget.

## DelayBlock closure: a real fill parameter explains the last copy

DelayBlock is **4/4 strict/data exact**, with all three protected siblings
preserved. `ControlDelayBlock`'s sole remaining replacement (`li r4,0`
versus target `addi r4,r5,0`) closed when the existing fill loop became
the ordinary `FillDelayBlock(block, value)` C operation, called with zero.
The compiler automatically inlines it. This is a real consumed parameter,
not a zero alias or register directive; no assembly or fabricated storage.
The target-backed loop repeatedly writes the current write pointer without
advancing it. That inherited behavior is intentionally unchanged; this is
not a memset of the whole buffer. The helper name is inferred, not original.

The first structural fill-parameter probe matched at 228 bytes; the formatted
source reproduced it. Normal Ninja/MWLD passes **137/137 retail checksums**
and direct DOL equality. All **19/19 relocation applications** are proven:
15 raw-identical entries plus four constructor HA/LO entries addressing the
same exact local functions after normal MWLD strips the unused 36-byte
standalone helper. Their final linked halfwords were independently checked.
No constant/data sections exist in this object. This is linked equivalence,
not an assertion that every raw object byte/offset is identical.

Source SHA-256 `150764f785409abdeaaf8ca616cdaa2f2f6a705cd756bab4e552d7d765893708`;
normal object `987cd649f7b105d023c3d8205dec9285ea8c18b8f95899e670a589f811f9dd35`;
strict/data `b7cd57b964e1a659438c90637849606bc8a2feb5f75133f4c668e6abb926fef2`.
Evidence: `build/qwen-parallel-20260912/delaybl/closure-*`,
`normal-physical.json`, and `final-link-proof.json`.
The source-selected DOL is still SHA-256
`172ae27aa6fcc9074de286b07ae4ac9f9152175fa913fffc3f5b8365e139ffec`.

The batch is **6/10 verified owners**: Median, FFT, Window, Matrix,
Stationarity, DelayBlock. Proof workspace DOL is **343/396**; public main
is still **337/396**. Do not push fewer than ten or count partials as owners.
Three independent Qwen questions were running while this closure was made;
no worker answer or approval was awaited. The lesson is to recognize the
semantic parameter/automatic-inline boundary before repeating constant or
register spellings, not to wrap arbitrary assignments in helpers.

## Smoothing retained gain and next throughput correction

Smoothing retains **96.556076 -> 97.88785** strict/data at the same
**856 bytes**. Both exact siblings and every baseline physical relocation
are unchanged. This is still **2/3**, not a sixth owner closure. The change
forms the actual active input span in one expression before the matrix end
is constructed; it does not add storage or change the algorithm.
Live source SHA-256 is
`7be8d5ca3e0f28424b4cb4f39cc5e5e6fe4bb9d183ca0bcc0965680bf0e4854d`;
object `d2ba2713d42cdf743569cb7cd4714d376bc1f9ed7985d256c8ddf560fbff8219`.
Evidence: `build/qwen-final-tail-decisions-20260912/smoothing/active-input-span-{strict,data,verification}.json`.

The old broad Stationarity support request finished only after the primary
had closed its owner (2094.873 seconds); it supplied no closure acceleration.
Do not dispatch another question for that completed owner. Use a single
decision question and paired evidence for new support, not a whole-function
rewrite. Genfilt and Undersampler now use the existing validated decision
packet path while primary compilation proceeds independently.

The existing local Qwen runner now supports an exact per-job `.cancel`
marker: superseded work releases its request without killing the server or
other jobs, and cannot publish a partial answer as success. Fifteen replay
tests pass, including cancelling one of two concurrent requests while the
other completes. This adds no approval round or model-generation limit.
Keep the marker action next to the actual question-resolution/owner-closure
decision so obsolete jobs do not consume another half-hour unnoticed.

The broad Smoothing answer proposed replacing a column-relative pointer by
`matrixEnd - rows + column`. This is not equivalent after the first iteration:
matrixEnd stays fixed while column decreases. Reject it without a compile.
Pointer recurrence and loop invariants must be checked before treating a
plausible local scheduling explanation as a source candidate. The retained
gain above was independently reconstructed, not generated by that answer.

## Stationarity closure: recover the range operation before register spelling

Stationarity is now **4/4 strict/data exact** with all three exact siblings
preserved. `ProcessStationarity` keeps its retail 424-byte extent; the last
seven argument rows closed when the history min/max scan became an ordinary
`StationarityRange` C helper. The actual source distinction is the isolated
range computation and its returned difference, not a maximum-variable rename.
The helper name is an inferred semantic name, not a recovered original symbol.
No assembly, storage padding, compiler hint, or external interface change.

Normal source-selected Ninja/MWLD passed all **137 retail checksums**, including
direct DOL byte equality. All **21/21 relocation applications** are accounted
for: raw GC1.2.5n SDA21 sites use the separately checked DTK instruction-site
convention; six constructor HA/LO references preserve the same local functions
after MWLD strips the unused 64-byte standalone helper. Their linked halfwords
were checked against the final function addresses. The three live constants
are byte-exact; the target's extra four-byte zero split gap is ordinary linked
alignment, not source padding. These are linked equivalences, not raw object
identity claims.

Source SHA-256 `3b80ca70c2a1c2bf9b4bb5879ae6f6150500b4f10e7f67b1a3fdbe311794e6d5`;
normal object `2c09ed9b65ed3a8cdcfa6e527143672d506135ee9f82576c06d96d3bb2ffd4af`.
Evidence: `build/qwen-final-tail-decisions-20260912/statio/final-link-proof.json`,
`normal-physical.json`, and `normal-ninja-{strict,data}.json`.
The real DOL remains SHA-256
`172ae27aa6fcc9074de286b07ae4ac9f9152175fa913fffc3f5b8365e139ffec`.
The local ten-owner batch is now **5/10** (Median, FFT, Window, Matrix,
Stationarity), or **342/396** in the proof workspace. Public main is still
**337/396**; no smaller promotion batch was pushed.

Throughput lesson: Matrix's column-address helper and Stationarity's range
helper both closed their final mismatch families in the first structural
probe after earlier local-variable approaches. This supports prioritizing
meaningful helper/automatic-inline boundaries in similar residuals, not a
universal helper template. Four Qwen support questions continued independently;
the primary did not wait for the Stationarity answer before testing the fix.
Compilation and final link are not the demonstrated bottleneck. Reduce repeated
source hypotheses and worker wait time; do not inflate worker count, context,
or proof volume and call that throughput.

## Latest DpGenUw frontier: source identity across phases

`DynProgUserWords` now retains **94.39237 -> 98.36512** strict/data,
**1456 -> 1468 bytes**, equal to the retail function size. The other eleven
functions remain strict/data 100%; this is still **11/12**, not an owner
closure. Main remains 337/396; Stationarity above advances the batch to **5/10**.
The active source and immutable `formatted.c` are byte-identical, SHA-256
`8e2decd58bc553d44cb1be81c9e5041e39caa6c2f61c26af992562b6a593a669`.
Candidate object SHA-256:
`fa493462e9c8267ac363869a45a7f2767d45ce1da080c8d6df37b79a957d9101`.
Evidence is under `build/dol-dpgenuw-boundaries-20260912/recurrence/`, with
`formatted-{strict,data,verification}.json` as the current proof. The old
`recurrence/binding.json` binds the pre-integration source and must not be
reused for a new probe without refreshing its live-source binding.

Retained causal sequence:

- `state-selection`: the repeated minimum/maximum word-state scans became
  small shared C helpers. This made the complete initial minimum scan exact
  and restored the two later result-pointer copies: 95.96458%, 132 rows.
- `reused-path-node`: one genuine working backtrace pointer is reused while
  constructing the transition and final paths. The former separate inlined
  producer identities had incorrectly coalesced the first result with its
  retained transition snapshot. Explicit reuse restored both missing copies
  and the r20-r31 save set: 96.711174%, 1464 bytes.
- `phase-creation`: after that source-identity correction, the remaining
  saved-register cycle mapped directly to the now-complete live scalar
  creation order. Ordering those real declarations by their evidenced phases
  gave 97.378746%; no dummy owner, operand matrix, or register directive.
- `silence-state`: a distinct live silence-state pointer, rather than reusing
  the generic traversal cursor, restored the last missing address-result
  copy and the exact 1468-byte size. Its ordering relative to the selected
  entry pointer closes their volatile-register cycle.
- `silence-score-domain`: the second silence-state score is a real shared
  signed-16-bit field snapshot consumed by both comparisons. Widening this
  owner to s32 added an unwanted copy; its actual field domain retains the
  gain. The unused AddUserWordBacktrace reconstruction was removed and the
  retained source was formatted/recompiled without a score change.

Current remainder: 67 aligned rows (64 ARG, one insert, one delete, one
replace), including frame 0x68 versus target 0x58; a repeated r4/r5 cycle
rooted in finalBacktrace; second previous-backtrace r20 versus target r23;
and the remaining silence distribution/score load chronology. Independent
physical changes in DynProgUserWords and three callers remain unclosed;
they include local function placement from reconstructed helpers. No
source-selected linked exactness or promotion is claimed for DpGenUw.

Compact rejected constraints: a second allocation-helper layer stayed
outlined and regressed to 88.68%; a shared caller alias alone, state-versus-
previous helper input, reference-count wrapper, and chained score assignment
were instruction-neutral on their bound bases. A state getter alone lost
an instruction. Merging the final-path assignment after the branch regressed
to 94.75%. None of these results bans a different source cause. The typed
working-node reuse, not generic aliases or another getter, supplied the gain.

Two local Qwen jobs ran concurrently while the primary compiled. They returned
bounded observations about state-pointer chronology and caller-owned path
snapshots, not a verified winning patch. No consensus or worker wait gate was
introduced. For speed, prioritize reconstruction of shared versus distinct
live values before repeated equivalent helper/alias spellings, and retain
the actual compiler result rather than a confident source prediction.

## Matrix closure and current throughput decision

Matrix is now **12/12 strict/data exact**, with **25/25 effective relocation
applications** and all eleven exact siblings preserved. Normal source-selected
Ninja/MWLD passes all 137 retail checksums and direct DOL byte equality. The
verified local batch is **4/10 owners ready** (Median, FFT, Window, Matrix),
**341/396** DOL owners locally; public main remains **337/396**.

imtxDeleteCol's last 12 frame/register rows closed in one compile by recovering
a live column-address helper and consuming its result in both memcpy pointers.
This is an ordinary inferred C abstraction, not an extracted original name.
The normal object contains its unused 24-byte standalone copy; MWLD strips it,
as independently confirmed by the retail-identical source-selected link. All
twelve target functions are exact; no fake storage, hints, or assembly was used.
Normal and isolated objects differ in STT_FILE filename metadata only, with
identical code/constants/relocation bytes/compiler metadata. Raw GC1.2.5n SDA21
offsets remain separately reported from DTK instruction-application equivalence.

Source SHA256 `9a37b383be0d1c057f49abdd8ccb50dd28a6b049500e0ae95c7fd847affe4ad0`;
normal object `e5c396278640402eaf438ca255d97a2672cc08e4873a5f2de054002db8ae7556`.
Evidence: `build/qwen-owner-next-20260912/mtx/final-link-proof.json`,
`normal-physical.json`, and `normal-ninja-{strict,data}.json`.

Three other functions reached strict/data exact during the same round:
InitializeArchitecture (signed loop bounds and low-byte callback status),
GenderFixedGender (one Boolean result reused across ResetGender), and qQueueInitEx
(the real ternary/merged store). InitializeArchitecture has independently exact
2/2 target relocations. Gender/MQueue still inherit unresolved local provider
address shifts; neither is a closed owner. ProcessMel retains 91.681816 -> 93.5
by putting the real output initialization before its following loads. All exact
siblings survive. Evidence: the respective `init-domains-*` / `consumer-order-*`
files under `build/dol-gssdk-current-inventory-20260912/`.

The four-slot Qwen batch completed all six questions, but several answers arrived
after Astra had already compiled their clear fixes (roughly nine minutes for
the initial batch). More workers alone did not discover these gains. Dispatch
Qwen on unresolved structural decisions while compiling clear type/control/
consumer fixes immediately. Favor completion of whole owners; reuse bound
evidence and batch final integration. Never make worker consensus or answer
arrival a compile gate, and do not claim this as a controlled throughput benchmark.

InitViterbi then closed **65.95098 -> 100**, 440 -> target408 bytes, in two
compiles. Moving the state cursor increment to the loop update produced the
target ten-way unrolling; keeping the distribution-index increment as its own
statement after the actual load enabled strength reduction and eliminated the
remaining cascade. The intermediate postincrement-in-subscript form recovered
unrolling but added index shifts, so it was not retained. Qwen's completed
recurrence analysis correctly distinguished ten-way from eight-way-plus-remainder;
Astra chose the source change. This is reconstruction of the induction, not a
request for every spelling of the same arithmetic expression.

AllocateBacktrace then closed **91.84252 -> 100**, 500 -> target508 bytes, in
one compile. The target reloads the mutable chunk-size field after allocation;
the old code cached it across the call. A local traversal bound created after
the call, plus the actual u16 chunk-index postincrement, restores that chronology
and all 70 differing rows. DpGenUw is now **9/12 instruction-exact**; the remaining
SendResultUserWords, DynProgUserWords, ControlDpGenUw prevent owner promotion.
No previously exact function was lost. Evidence:
`build/dol-viterbi-induction-20260912/{dpgenuw,allocate}/`.

The subsequent command-handler reconstruction closes ControlDpGenUw
**10.130435 -> 97.690216 -> 100**, 884 -> 744 -> target736 bytes. Target case
order, a shared cleanup helper defined before FreeBacktrace's body, the real
ergodic-phenome callback at slot0x9c, and a full-width helper status consumed as
a low byte restore the target outline/inline/return boundaries. These helper
names are semantic reconstructions, not claims of original symbol provenance.

SendResultUserWords closes **72.732025 -> 98.95425 -> 100**, 520 -> target612
bytes. Preserve the heap-context snapshot; reload the final backtrace after
allocation; consume each descending distribution index in the actual store;
then restore the count/frame and result/silence owner chronology. Moving all
those declarations to the top was worse and is not retained. Both helpers are
ordinary C; all existing exact functions survive.

DpGenUw is now **11/12 instruction-exact**. Its last function, DynProgUserWords,
retains **69.133514 -> 85.77929 -> 94.39237**, 1600 ->1496 ->1456 bytes against
target1468. Evidence distinguishes u16 bounded state scans from full-width
update/distribution traversals; a cached distribution cursor, consumed reference
decrement, post-score backtrace snapshot, shared normalization producer, and
actual distribution-identity comparison repair structural/dataflow causes.
An extra shared-backtrace/getter variant regressed and was not retained. The
remaining frame/value-boundary and register differences are unresolved; this
owner stays NonMatching and is not a fifth batch closure.

Current source SHA256
`544cd34edf62ca8be305308b67dcf675a2695558cc5357a230a98d1e31b3d229`,
object `16591fb94d7c8c9ebe1f3ff195b0bc09b951d6c45d6e2bcb207dbbb0545ef838`.
Evidence: `build/dol-viterbi-induction-20260912/`
`control/typed-consumers-*`, `result/traversal-owners-*`,
`recurrence/phase-traversals-*`. Physical/local call positions still depend on
the final provider layout and helper stripping; final linked owner proof remains
required. All four structural Qwen questions completed. They corroborated case
order and loop facts; Smoothing's proposed helper boundary remained explicitly
underdetermined, not a prohibition on further primary reconstruction.

## Window owner closure (latest)

Window is **5/5 strict/data exact**, 3068 code bytes and **31/31 effective
relocations**. GC1.2.5n raw SDA21 offsets are kept distinct from DTK instruction
application equivalence. Normal Ninja/MWLD with Median, FFT and Window selected
from source passes all 137 retail checksums and direct DOL byte equality. The
local proof count is now **340/396**; main remains 337/396 pending the ten-owner
batch. Source SHA256: `fa71d99535244b2a863f67389c3ad35fc02c6e92f96bdd7cc143bae7fdc0d347`.

InitWindow needed f32 integer conversion before the existing double-angle
formula, the chained window-length store/result, and the post-store frame-length
field consumer. WindowFlush needed the unoffset history snapshot across the
queue call and subtraction afterward. ProcessWindow needed one live pointer
snapshot consumed by both the zero store and memcpy; this removed its extra
reload. With operation/size topology closed, existing pointer declaration
chronology closed ProcessWindow. The final Flush cycle needed distinct live
previous-frame-copy and windowing-read cursors. Reordering the late assignments
alone was neutral; it was not retained. No fake locals, hints or assembly.

Evidence: `build/dol-window-final-20260912/window/owner-proof.json`,
`final-link-proof.json`, `normal-physical.json`; the retained intermediate
source/object pairs are in `build/dol-window-{flow,closure}-20260912/window/`.

The initial compact Qwen packets hid the late extra-load cause behind early
register/branch differences. Both returned hypotheses were rejected on actual
call/value semantics before compilation. The existing support-excerpt tool now
includes bounded first added/deleted-operation contexts and category/size facts.
The baseline ProcessWindow replay includes rows128-132 and the adjacent memcpy
consumer; it explains an already solved cause, not a new tool-discovered gain.
31 focused tests pass (one private-fixture skip). The preceding full public
agent gate against 9acec84 passed; private retail proof remains separate.

## FFT owner closure and speed gaps

`fft_maye` now closes both functions in one coherent unsigned/f32-domain cell:
`fht` 85.01944 -> 100 (1884 -> 1852 bytes), `realfft` 53.844036 -> 100
(352 -> 436 bytes). Signed lengths/indices produced arithmetic shifts and signed
comparisons. Unsuffixed SQRT2 and half literals introduced double evaluation,
extra rounding and the wrong automatic unrolling. The unsigned caller lengths,
target compare/shift forms and single-precision arithmetic justify the changes;
no register hints, assembly, invented storage or literal-value changes were used.

Both strict/data channels are exact. Target relocation applications are exact:
fht 9/9 and realfft 2/2; raw GC1.2.5n SDA21 +2 representation remains separately
reported. The only header consumer, fftmod, is object-behavior/physical unchanged.
Normal Ninja/MWLD with FFT and Median source selected passes all 137 checksums.
The proof checkout reports DOL 339/396 (57 remaining), 1855312/2173968 code and
692920/717328 data. This is a verified local batch, not yet a main count.

Evidence: `build/dol-fft-domain-20260912/fft/owner-proof.json`, domain strict/data
reports and per-function physical receipts; `consumer/domain-verification.json`.

The existing `tools/recovery_causal_groups.py` now adds:

- Direction-specific signedness/evaluation-precision evidence. Repeated compares,
  shifts and single/double arithmetic are surfaced before opaque register tails.
  Loads alone and mixed directions remain weak; no exact source variable is inferred.
- `--owner-summary`: whole-object exact/remaining counts and domain-review priority,
  without claiming a completed owner from instruction matching alone.
- `--support-source FILE --source-lines START:END`: one bounded source question,
  first mismatch plus domain examples, hashes and explicit omitted-row counts.
  Oversized excerpts are rejected with narrowing guidance, not silently clipped.

The tool replay identifies both FFT domains and reports 0/2 -> 2/2. It encodes
the source decision independently made during this closure; it did not discover
the already-authored winning candidate. The actual realfft support excerpt is
6232 bytes. The active packet builder uses this API for bounded jobs and refuses
whole prompts above 28000 bytes; local Qwen reasoning/output limits remain uncapped.
The previous TriggerLR job exhausted its 65536 context (no final answer) after
1170 seconds. Two new narrow xhigh support jobs completed naturally in 119 and
102 seconds. These are different tasks, not a controlled speedup benchmark.

The README now separates public build/progress information from recovery-internal
policy. No static current-percentage claim is added to either README.

## Active completion goal

Recover every remaining main DOL owner and promote verified source closures in
batches, using Qwen-only parallel support. Current fetched main `5c7bca5` has
337/396 Matching DOL owners (59 remaining); FFT, Median, Window, Matrix, Stationarity and DelayBlock are verified
locally and pending promotion. User requirement: promote exactly 10 verified owners per
batch (except the final remainder). Current batch is 5/10 ready. Do not push a
smaller intermediate source/status batch or count function partials as owners.
Pivot among eligible owners when a particular residual stalls; retain completed
owners safely while filling the same batch.
Partial gains remain in the protected local champion, not counted as main
closures. Finish only when main's full DOL owner census and source-selected
retail link pass. No fixed crack-rate guarantee is made.

Current local Qwen support uses two genuine slots on ConstructGenderFilter and
imtxDeleteCol while the primary continues reconstruction. Packets and queue:
`build/qwen-owner-next-20260912/`. Earlier SlidingHisto_NewItem/CreateWindow
packets are retained measurements, not active jobs. The GenderFilter constructor's two direct
hypotheses did not supply a gain: nonconstant narrowed aggregate initialization
is rejected by GC1.2.5n, and a separate full-width profile-result owner regresses
79.0 -> 53.19355 and 124 -> 128 bytes. Original source and its three exact
siblings remain intact. The next source question is the profile-construction/
call-expression boundary, not a repeated register-order guess. Compact measured
evidence: `build/dol-current-primary-20260912/genfilt/verification.json`.

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

## Larger-batch continuation

The first two owners and their configuration/progress updates are merged in
public PRs 34 and 35; main is `a1aa433`, DOL 337/396. Do not count pending
champions as already promoted.

Median is additionally closed at 4/4 functions, 1136 code bytes, 13/13 physical
relocations, strict/data 100. The one substantive candidate snapshots the
median traversal bound before the loop and consumes the ring-index increment
in its comparison. This enables the target's eight-way loop unrolling and
removes the ring-index reload. It closes ProcessMedian from 78.95 to 100 without
changing its three exact siblings. Decimal 24 preserves the existing input-size
domain; it is not a newly inferred semantic name.

Normal Ninja reproduces `c30/median.o` exactly, SHA256
`1ab00fea31260597737849c1dcbf895823e3c2c59793a478530b939ed151868b`;
source SHA256 `1e31147597336285bd591c044d317fd6a15f63315ab5089cbfabc2cf500271cc`.
The real three-owner source-selected build and direct comparison pass all 137
retail outputs. The verified local frontier is now DOL 338/396, code
1853024/2173968 (85.24%), data unchanged at 96.58%; full project 377/926.
Median remains retained for the next public batch, not yet in main.

Smoothing now retains 75.15 -> 95.48 with exact target size 856 and both exact
siblings preserved (`c30/smoother.json`). The source cause was not a collection
of independent register rows: preserve the beta snapshot, decrement the active
index before its store, snapshot the matrix end, preserve the input end/column
relation, save the diagonal scalar across elimination, and chain the final
dimension restoration. The column lifetime shares beta's register after its
last use. Its declaration boundary plus the spline/working-matrix identities
close the saved-register cycle. The remaining frame and volatile-loop owner
differences are unresolved; do not mark this owner Matching.

### Maximum-reasoning local support trial

Qwen3.8-27B Q4_K_M supplied a further retained Smoothing gain: 95.481310 ->
96.556076 strict/data, 36 -> 31 mismatch rows, still 856 bytes. It moved the
independent matrixEnd assignment between initial inputValue creation and the
final inputValue adjustment. This changes actual value-creation chronology;
it is ordinary C, without new locals or forced registers. Both exact siblings
remain exact and all per-function physical relocations are unchanged relative
to the baseline (45 total, including Smoothing's 9). Existing SDA21 raw
offset/application distinctions are unchanged. This is partial, not a new
Matching owner or public promotion.

Live source SHA256 `9d1339e9f38b230440a8c4d805d4826bc24028338678317c9fad00b55f3c0c2f`;
object `ae0414d3e40e7a8a4511a5e407fd3d33f709e8f62a66d25fe91a66ba4c5a2a3d`;
strict/data report `08bc3e961ea9a07e692109f4639879041bf6f3001212cfcc424a6a477fff72c8`.
Current champion evidence is `build/model-support-test/qwen-max/`, replacing
`c30/smoother.json` as the newest source-bound frontier.

The user requested highest reasoning without a token-cost cap. Native Ollama
`think=max`, `num_predict=-1`, 65,536 context returned a complete answer after
808.168 seconds and 31,158 generated tokens. One candidate compile produced
the gain. Earlier 8K/32K truncated thinking runs had no answer; the completed
non-thinking Qwen and Luna/max candidates both regressed. Keep those distinct:
one useful max-reasoning sample does not establish a universal model ranking.
Compact comparison: `build/model-support-test/RESULTS.md`. No global Codex
model substitution or new lane was configured.

### Qwen-only support, 2026-09-12

The user's latest support-model instruction supersedes the Luna tier for this
batch: use local Qwen at highest reasoning with no fixed output-token cap;
do not spawn Luna. Astra remains the sole live-source integrator and reviews
each proposed source cause before compiling. Qwen receives independent frozen
source/object/diff packets, not concurrent write access or a consensus role.

Ollama 0.34.0 serializes the qwen35 architecture even when parallelism is set
to two. The initial four-job queue therefore was not parallel inference:
Smoothing and ControlDelayBlock answered sequentially; Combiner and
Undersampler received no tokens before their queue-wait timeout. Those two
are infrastructure failures, not tested source hypotheses. They are assigned
to the shared-weight llama.cpp backend for actual concurrent inference.

The second completed Smoothing recommendation swapped independent input and
coefficient pointer creation. One isolated compile regressed 96.556076 to
96.369156 with exact size/siblings and unchanged baseline relocations; the
96.556076 champion remains untouched. Evidence:
`build/qwen-parallel-20260912/smoothing/verification.json`. ControlDelayBlock's
answer proposes an extra zero-valued local solely to induce an addi producer;
it has no stronger source-cause evidence and is not automatically compiled.

The standalone backend passed two simultaneous isolated requests, then real
Combiner and Undersampler jobs starting together at 2026-09-12T09:21:49Z.
They completed naturally in 754.415 and 504.935 seconds, respectively. Two
64K slots shared fully GPU-offloaded weights, using about 21.3 GiB of the
24 GiB GPU. Both used xhigh reasoning with no fixed output-token cap. Answers
and scalar metrics are retained; reasoning text is not stored.

Compiler validation did not yield a further gain: Undersampler stayed
99.6875% with exact siblings/unchanged physical relocations; Combiner's proposed
declaration after a statement is not accepted by the pinned C89 compiler.
Neither source was retained. Future packets explicitly require C89 declaration
placement. Parallel inference is verified, but it is not itself a cracked owner.

Context BeginOfWords becomes instruction-exact using the existing typed
accessor composition, retaining the static virtual-table gain. WordProp and
Syntax's flattened-source champions are preserved; subsequent cursor/grouping
probes were neutral or worse. Current `c32/ctxdata.json` binds the composed
champion. It remains an incomplete owner.

Throughput rule for this batch: compare the actual live source, not a stale
main-derived cached object. A single initial current-source sweep exposed that
the existing SlidingHisto_Init gain was absent from the old proof baseline;
that is recovered historical progress, not a new crack. Prefer a coherent
structural reconstruction such as Median's bound snapshot over repeatedly
probing isolated high-score register colors. Parallel support is bounded by a
specific source question, not an exhaustive inventory or a consensus gate.

### Retained continuation, 2026-09-12 10:49 UTC

Two additional functions are strict/data exact locally, not two completed
owners: `SlidingHisto_LowerQuantile` (200 bytes) and `mtxCompress` (196 bytes).
Independent physical receipts bind all five relocations in each function to
the correct effective targets. Their three SDA21 entries retain the documented
GC1.2.5n instruction+2 convention; DTK instruction-application equivalence is
true, raw equality is false. Final owner-selected link proof is still pending.

| Function | Earlier strict/data | Retained strict/data | Target/source bytes |
| --- | ---: | ---: | ---: |
| SlidingHisto_NewItem | 93.45 | 95.21667 | 240/240 |
| SlidingHisto_LowerQuantile | 94.90 | 100 | 200/200 |
| CreateWindow | 76.7033 | 81.52747 | 364/364 |
| mtxCompress | 94.79592 | 100 | 196/196 |
| QrDeleteCol | 94.11855 | 99.175255 | 776/776 |

NewItem consumes the history-write increment directly in its wrap comparison;
the redundant field reload disappears. LowerQuantile creates the bin pointer
before its count and directly consumes the pointer difference in the final
float expression. Slidhist is now 6/8 strict/data exact. The source is
`5f2886aaf033420b8f6397b96f06ab1f26e91f35a8f81347472e929dd2a06116`;
the composed object is `9c3f237346e8dd4922b2db3831e0f76e6a024de003d8c8f1a27db1fc617ec35a`.
Evidence: `build/dol-current-quantile-20260912/slidhist/` and the preceding
`build/qwen-dol-next-batch-20260912/slidhist/`.

CreateWindow initializes the real sample counter before calculating halfLength.
It preserves the target's two distinct sampleRate conversions; the Qwen proposal
to cache and reuse one conversion was not compiled because it contradicted
that target relationship. Source `463f889af70b79bf7a09e1021dc31e8ad880ddc089380197a80b34865f4b17ad`,
object `d573fbb39ab87c14e4ad7b66f0aab38596f0c7a5d03675a40f5eed337fd7f3a5`.
Evidence: `build/qwen-dol-next-batch-20260912/pitchwin/`.

mtxCompress needed actual indexed traversal over a saved values base, not the
hand-strength-reduced pointer loop. Counter creation before elementCount then
restored the saved-owner relationship. Selecting the existing Dolphin math
header removes the other header's two weak static sqrtf constants; it yields
the target's 48-byte constant section rather than 64 bytes. No shared header,
math primitive body, compiler flag, or pragma was changed.

QrDeleteCol snapshots the destination before the coefficient calculation,
consumes the first diagonal decrement in the lower-value load, and reuses the
live diagonal cursor and upper/lower pair for the two rotation phases. This
removes the extra reload/cursor instructions and restores target size. Its
remaining 26 rows are a four-pointer register cycle. All nine originally exact
mtx siblings remain exact; the owner is now 10/12 strict/data exact. Source
`fba8d4edf56c6c89c945d5ac3df46a5c6c837ee7fffbc37e75fe84934f34f886`,
object `1c984f4f82c5e8db804ac41142735c9ea22ab14ff1bd4b0f7e7780b0753dcb4a`.
Champion evidence: `build/dol-current-mtx-20260912/mtx/pair-step-*`.

Bound constraints, not function bans: moving all matrix traversal declarations
to function scope regresses allocation; moving end calculations inside the
column loop changes the loop topology and regresses. imtxDeleteCol's named
offset/destination is neutral; parameter-offset reuse regresses one operand.
Neither is retained. SlidingHisto_Clear inlining is object-neutral for
PutSession; extra named min/max/center floats do not improve NewItem. VQ indexed
and loop-latch indices advances regress; Stationarity's fresh maximum local is
neutral. Keep the respective champions instead of those probes.

A single current-source GSSDK inventory now exists at
`build/dol-gssdk-current-inventory-20260912/inventory.json`. It separates absent
source, compile-context failures, exact siblings and structural residuals.
It is selection evidence, not newly recovered progress. Two further Qwen jobs
in `build/qwen-dol-structural-followup-20260912/` run concurrently while the
primary works; no Luna workers or new user-owned lanes were started.

#### Subsequent exact QR closure

QrDeleteCol subsequently reached **100 strict/data, 776 bytes**, with every
other function unchanged. Moving only the two reusable traversal cursors
`source`/`diagonal` to function scope closes the four-pointer cycle; the
per-column destination stays local. This is the important distinction from the
regressing all-outer-declarations probe. mtx is now **11/12 exact**, with
imtxDeleteCol at 98.67647 remaining. The composed live source is
`3ccc87d3759aeb50d8af939bf6927629b8e1cb65e6a560f6385c76e538365120`,
object `2a0e7e353a701dffcb834cf2aec2c4b976adc4d61a2cd7556093860cabb9412e`.
Evidence: `build/dol-current-mtx-followup-20260912/mtx/`.
Named row/offset snapshots for imtxDeleteCol remain neutral, not retained.

The next structural owner, exev_dp, retains its full-width InitViterbi index:
71.17242 -> 74.793106 and 120 -> target 116 bytes. The target has no 16-bit
mask on this loop's comparison; the other process loop's masks remain intact.
ControlExtraEventDP also improves 53.63964 -> 54.585587 from the corrected
inlined copy. Its unwanted inlining is still unresolved; no pragma is added.
The exact Process/Construct instructions remain exact; their local relocation
offsets move with the four-byte size correction and still require final linked
proof. Source `fc36aeb4c3fe6ca0a36b9387adf3d4d8b7ea3013f4cc01be2ff73f09652cfbcc`,
object `9c937181d7e0635dfd5eb0b477f7700905c9af67450b8512e44ded1ae1f3dbdc`.
Evidence: `build/dol-current-exev-20260912/exev_dp/verification.json`.
Reusing a zero index through the first-state assignment regresses size and the
control function; switching the full-width index to signed is neutral. Neither
replaces the retained unsigned source.

### Current continuation and token discipline

The later, distinct peeled-count reconstruction keeps the first state-ID read
at constant index zero, then advances the real count after that state. It makes
InitViterbi 116/116 strict/data exact and naturally restores the outlined call
in ControlExtraEventDP. The prior failing state-cursor probe indexed that first
read; it did not test this source shape. Control's case order, positive result
branch, and context snapshots then improve it to 97.567566 at 444/444 bytes.
DynProgExtraEventsProcess is 95.74074 at 432/432 after the state-ID callback
argument, explicit transition bounds, rolling score reuse, and cache-test
assignment are reconstructed. All prior exact functions remain exact. Current
source is 6b8abed938c734d5c9d2ad0651579db3a7d1773aea8790f3a89cbae619d877d6;
object 2ce16b207eab7bd8acc3a0694259283d65f9f953c9f250f7fc3c04be68f9b060.
Evidence: `build/qwen-dol-bounds-20260912/exev_control/control-topology-*`.
This is 3/6 functions exact, not a completed owner or main promotion.

User-requested token optimization applies immediately:
- Read bounded source ranges and the first causal mismatch group; do not repeat
  whole source/report/instruction-policy dumps within an intact context.
- Reuse hash-bound evidence and report changed functions, exact losses, and
  physical changes only. Full verification remains in its local artifact.
- Keep independent research on the two local Qwen GPU slots; no Luna or paid
  support fan-out. Astra integrates and compiler-tests credible answers.
- Consume completed support answers once. Do not poll unchanged work repeatedly
  or turn speculative suggestions into long approval/report exchanges.
- Keep commentary short; communicate verified gains, closures, and actionable
  blockers. Never hide regressions to save tokens or weaken final link proof.

### TriggerLR type/control reconstruction

ProcessTriggerLR improves **86.15888 -> 99.85981**, retaining target size
428 bytes and independently exact **7/7 physical relocations**. The source
reconstructs the two distinct control-call branches, consumes the incremented
lookback count directly, and snapshots the real input pointer before its call.
The remaining two differences are conditional-branch topology, not register
ownership. A common call-result local incorrectly merged a target test and
shortened the function; that variant is not retained.

Five 32-bit mode/flag fields in `triggerlr.h` are signed, as demonstrated by
the target's signed comparisons: triggerEventMode, holdInputQueuesAfterTrigger,
speechActive, processMode, speechState. Width, offsets and total layout are
unchanged. A frozen private include overlay tested all four consumers;
voicing, subsamp and slidhist have unchanged function/physical results, including
all existing exact siblings. FindSpeech improves to 85.80272 and Control to
66.75 as additional consequences. The conflicting redundant `_tosControl`
declaration was removed; its canonical included prototype is unchanged.

Retained evidence:
`build/qwen-dol-causal-next-20260912/trigger_find/process-input-*` and
`build/dol-trigger-header-20260912/{trigglr,voicing,subsamp,slidhist}/`.
InitViterbi also has an independent exact 0/0 physical receipt. These are local
partial-owner gains, not a new Matching owner or a main promotion.

Qwen's named matrix-offset proposal regressed imtxDeleteCol and was rejected.
Its later InitViterbi definition-move suggestion was stale and not compiled:
the primary had already solved that inlining boundary by the real peeled loop.
Two fresh bounded Qwen questions cover the remaining Exev control owner cycle
and TriggerLR FindSpeech structure; neither is a source-authority gate.

The next InitTriggerLR reconstruction retains **87.9375 -> 98.370834**:
target input initialization order, a live common error-result lifetime, and
both interpolation values computed before their destination stores. Size moves
952 -> 964 against target 960; this remains a partial, with a smaller absolute
size discrepancy and no exact sibling losses. Its evidence is
`build/dol-trigger-followup-20260912/trigglr/init-status-*`. The Process gain is
unchanged. Chaining the equal input-size assignments was neutral; a conditional
call expression merged a required Process test and regressed, so neither is
retained. No flags, padding, assembly, or interface changes were introduced.

### TriggerLR retained continuation, 2026-09-12

FindSpeech is now **98.512474**, with the exact 1764-byte function size and
16/16 instruction-application-equivalent relocations (raw SDA21 placement is
reported separately). The reconstruction fixes the event argument order,
lookback-limit consumer, Boolean accumulation, ring bounds, and delayed third
median input. Its saved-FPR cycle, median selection and frame remain open.
ControlTriggerLR improves **65.55597 -> 99.406715**, retaining 1072 bytes:
target switch order/common return, parameter-consumer chronology, distinct
nullable session paths, session-size accumulation and destructor context
snapshot restore the control flow. Its 27/27 relocation inventory is not yet
exact, and interpolation/frame differences remain. Init stays 98.370834;
Process stays 99.85981; the exact constructor is preserved.

Seven additional real 32-bit flags now use signed comparisons without layout
changes. All three other header consumers (voicing, subsamp, slidhist) compiled
with the frozen overlay have unchanged function and physical results. Evidence:
`build/qwen-dol-trigger-interpolation-20260912/trigglr/median-late-*`,
`build/dol-trigger-reset-20260912/overlay/`, and
`build/dol-trigger-header-20260912/*/reset-flag-verification.json`.

The typed WordProp accessor also retains **66.933334 -> 81.86667**, 60/60
bytes, 0/0 exact relocations, and all 19 exact ctxdata siblings. Evidence:
`build/dol-current-context-accessors-20260912/ctxdata/typed-layout-*`.
Qwen's subsequent staged byte-layout proposal regressed to 80.333336 and was
not retained. Median helper outlining and early result reuse likewise did not
replace the protected source. These are partial owner gains, not new Matching
owners or main promotions.

ProcessTriggerLR subsequently became **100% strict/data, 428/428 bytes, 7/7
raw/effective physical relocations**. A real common error exit (`goto done`)
preserves the two separate call-result tests and corrects the remaining branch
orientation; the previously rejected shared result variable had merged them.
Evidence: `build/dol-trigger-terminal-20260912/trigglr/process-exit-*`.
This raises TriggerLR to 2/5 exact functions, not whole-owner closure.

The next shared interpolation reconstruction separates histogram interpolation
from the final unit conversion and reuses its accumulator. Control improves to
**99.81343**, 1072/1072, exact 0x50 frame, eight register-only rows. Init
improves to **98.975**, still 964 versus target 960. The intermediate explicit
range variables regressed but exposed the frame cause; they were not retained.
The status return of SlidingHisto_Init is now s32, supported by its signed retail
caller test and its 0/1 implementation. Provider, voicing and subsamp remain
object/physical unchanged; all exact siblings survive. Header width/ABI is
unchanged. Reversing the noise-floor product alone was neutral and does not
account for this gain. Evidence: `build/dol-trigger-coupled-20260912/trigglr/`
`interpolation-accumulator-*`, `build/dol-trigger-status-20260912/overlay/`, and
`build/dol-trigger-header-20260912/*/status-return-verification.json`.
Init and Control still fail full target physical equivalence, despite matching
43/43 and 27/27 inventories; no final link or new owner promotion is claimed.

### DpGenUwProcess and bounded continuation

DpGenUwProcess improves **95.60227 -> 100 strict/data**, 348 -> target352 bytes.
The target first-frame lookup reads the block's silence distribution, not the
selected state's distribution. The one DynProgUserWords consumer tests a signed
status; correcting that static return type preserves width and provider code.
The closed loop's counter/cursor declaration and increment chronology complete
the remaining two rows. The owner is now **6/12 instruction-exact**, five prior
exact functions preserved; physical target inventory is10/10 but effective
relocation identity is not yet exact, so this is not a closed/promotable owner.
Live source equals `build/dol-dpgenuw-owner-20260912/dpgenuw/loop-order.c`;
its strict/data, verification and target physical receipt are adjacent.

SlidingHisto_NewItem retains **95.21667 -> 95.38333** with unchanged size,
physical applications and all six exact siblings. The direct bin-center
expression restores the subtraction destination but not the remaining schedule
or frame. Evidence: `build/dol-slidhist-owner-20260912/slidhist/bin-center-*`.

Bounded negative constraints: a separate constructed-block return local in
GenderFilter is neutral; parameter-to-element-offset reuse in imtxDeleteCol
regresses; moving Stationarity's existing scan-value declaration to function
entry is neutral; direct composition of existing ContextData tail accessors
regresses and still merges the target's duplicated count load. None replaces
the champion or exhausts its function. Artifacts are under
`build/qwen-owner-next-20260912/` and `build/dol-small-tail-20260912/`.
# Four concurrent Qwen workers — 2026-09-12

User-requested expansion is live: four actual simultaneously processing slots,
not four queued clients. The installed llama-server shares one model copy and
the existing 131072-token q8 KV pool across four slots, with unchanged 65536
per-request context and highest uncapped reasoning. All 66 layers are on GPU.
Observed during four real jobs: GPU 22453/24564 MiB, utilization 93%, system RAM
available 2071 MiB. Aggregate long-context contention remains possible; neither
context exhaustion nor a truncated response counts as a completed finding.

Independent current-source questions: InitializeArchitecture, GenderFixedGender,
qQueueInitEx, ProcessMel; InitViterbi (DpGenUw) and ControlExtraEventDP queue next.
Evidence is reused without baseline recompiles under
`build/qwen-four-workers-20260912/`. Astra remains the source integrator; these
questions do not delegate owner decisions or count as new cracks.

Existing installed launch/batch wrappers now accept 1-4 workers and verify real
backend capacity, unchanged per-request context and startup GPU headroom. A
cached-only batch does not query or start inference. The Qwen suite passed
12/12, including a four-party HTTP barrier proving concurrent dispatch and a
too-small-backend rejection before inference. There is no speedup claim until
useful outputs/retained gains are measured.

# Qwen decision-first validation and DpGenUw gain — 2026-09-12

The support workflow now uses one factual source/compiler question instead of
asking a worker to invent a complete matching patch. The existing causal-group
tool builds a source/report-bound packet with selected arithmetic rows and the
whole function's call/branch census. Its finding checker validates JSON shape,
packet/function identity and cited rows; factual correctness remains a primary
review, not an automatic source-admission decision. Highest Qwen reasoning and
uncapped generation are unchanged. The installed run-job/run-batch wrappers at
`C:/Users/Anony/.codex/tools/qwen-support/` validate prompts before inference,
keep malformed/stale/truncated answers separate, reject duplicate prompts, and
reuse identically bound completed findings without overwriting their evidence.

Real current-artifact trial: Matrix's prior broad rewrite took 1067.739 seconds
and proposed a nonexistent loop/count fix. The decision-first job took 64.308
seconds and correctly found one memcpy/no loop and no unique source cause.
This is a different, narrower question at the same reasoning setting, not a
universal model speedup claim. InitBacktrace's pointer recurrence finding took
383.522 seconds and was correct. A repeat manifest reused both answers in
0.669 seconds with no inference. Focused tests including installed-runner fake
HTTP replays: 47 tests, one private fixture skipped.

In parallel, primary reconstruction changed InitBacktrace's indexed links into
a walking pointer, moved the pointer increment into the loop update, and put
the existing chunk counter declaration before the loop's invariant owners.
The sequence was 39.852940%/404 bytes -> 71.794120%/300 -> a countdown diagnostic
95.661766%/276 -> ascending cursor-update 99.044120%/272 -> final 100%/272.
The countdown diagnostic was not retained. The final source is ordinary C,
uses every existing owner, and preserves the link traversal and final sentinel.
Six exact siblings survive; DpGenUw is now 7/12 instruction-exact, not a closed
owner. InitBacktrace's independent target physical receipt passes. Provider
size movement changes three downstream local-call relocations; whole-owner
and source-selected linked proof remain pending, and no main Matching credit
was added. Qwen corroborated the target recurrence after the primary had
already compiled the structural improvement; it did not discover the gain.

Evidence: `build/qwen-decision-workflow-20260912/backtrace/chunk-owner-*`,
`InitBacktrace-target-physical.json`, and both `answer/` directories.
Source SHA256 `378221d36ee0055eddc3a8a55a3764f3d7426712d43b5c105957fb201c23c6d5`;
candidate object `82e66984bd0644b3d729cc75a3520a23943cc343fc3bcdc3b3ba53622b603776`.

The user-requested public README rewrite was independently committed on a clean
main-derived documentation branch and merged by PR #36, CI passing. Main merge
`5c7bca500f64f67544bb3dd08ed64f8a7204c79c`; published README blob
`3809209f5c31ca58c764b1b2acb653d586903ce0` equals the reviewed public draft.
The hook's README-only branch path rejects mixed source/config and private
workspace content; it does not relax recovered-source promotion. No tooling
crossed into main. Ten-owner source batching remains in force (3/10 ready).
