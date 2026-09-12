# Small DOL batch, 2026-09-12

## FFT owner closure and speed gaps (latest)

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
batches, using Qwen-only parallel support. Current fetched main `a1aa433` has
337/396 Matching DOL owners (59 remaining); Median is additionally verified
locally and pending promotion. Aim for 3-5 ready owners per batch, without
holding a verified batch indefinitely for a difficult unrelated residual.
Partial gains remain in the protected local champion, not counted as main
closures. Finish only when main's full DOL owner census and source-selected
retail link pass. No fixed crack-rate guarantee is made.

Next batch uses two genuine Qwen slots on SlidingHisto_NewItem and CreateWindow;
the primary continues independent reconstruction. Current packets and queue:
`build/qwen-dol-next-batch-20260912/`. The GenderFilter constructor's two direct
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
