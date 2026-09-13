# Small DOL batch — 2026-09-13

Main starts at `086a3a84fac9d928c9176df3624582fd28befd45`, **347/396**
Matching DOL owners. This is the next ten-owner batch, not ten new closures.
Current locally verified frontier: **349/396**, **2/10** batch owners.

## Constraint-comparison gap closed; champion remains 14/16

`recovery_causal_groups.py --function NAME --baseline-strict OLD --strict NEW`
now compares mismatches by target instruction address/identity, not candidate
row numbers or percentage alone. It reports resolved, persisting and introduced
observations while preserving score, size, frame and sibling-regression gates.
Ambiguous target/alignment identities reject comparison. Output is bounded and
advisory; it neither invents source causes nor authorizes retention.

Active acceptance: `mqueue-control-shared-loop-index` versus
`mqueue-batch-slot-cursor` resolves **eight** node/slot use sites, preserves two
nonexact address sites, and introduces two address/boundary observations.
The worse **98.965515 -> 96.27586** score remains a regression, despite equal
232-byte size/40-byte frame. This exposes a useful constraint for reconstruction,
not a newly discovered gain. No source change from this round is retained.

Compact new constraints: capturing the reader index before node/slot construction
still grows the frame to48; countdown only changes the zero-count branch; the
typed reader-operation helper, node-validation assignment and next-node snapshot
are object-identical to the champion. Moving the allocation word count into
`u16` at creation loses the exact enqueue sibling and shrinks both bodies.
Their bound outputs are under `build/small-first-20260913/mqueue-*`; none of
these observations exhausts the function or prohibits a distinct causal change.

The Qwen scheduling answer repeated the earlier direct typed-output proposal.
No repeat compile was spent. Its caller had omitted the existing selected-known-
measurements facility; the corrected packet now binds three relevant actual
source/object/report observations (including the output-lifetime regression),
with no new model tool or inference limit. Packet `c592f387b0721d08b7aed15d0ec89f99df633b0a2301d98ad8444aa29e992431`
is 25,910 prompt bytes. The new support run is pending; no accuracy improvement
or recovered function is claimed from preparing it.

## Current queue frontier: 14/16, all fourteen raw/physical exact

Two more functions are retained: `qEnQueue` **356 bytes, 4/4 relocations** and
`qQueueControl` **800 bytes, 4/4 relocations**, both strict/data/raw exact.
The control fix removes the earlier four-byte provider shift; independent
checks now prove **all fourteen instruction-exact functions physically exact**,
not just the two new bodies. All previous exact siblings survive and allocated
data is unchanged. `qEnQueueOne` additionally improves **80.78788 -> 92.878784**
and restores its retail 264 bytes. It and `qCheckDeQueueOne` (98.965515, 232 bytes)
are the two remaining instruction-nonexact functions. This is still not owner
closure or a main promotion; the ten-owner batch remains **2/10**.

The source causes were connected, not a register matrix:

- The allocator writes through a real generic output interface into a local
  typed allocation result before the caller consumes the node. This explains
  the retail result stores and alias-sensitive free-list reload. The retained
  source uses the old-C `(void **)&allocation` interface, not a forced stack
  slot, volatile, register qualifier, padding or assembly. It is a reconstructed
  implementation-specific allocator boundary, not a claim of portable generic
  pointer-to-pointer conversion or recovered original local names.
- The tail-search cursor and the saved append tail are separate live values.
  Restoring the explicit null result and traversal/result boundary supplied the
  missing twelve bytes. Once operations, homes and CFG matched, the actual
  index-before-byte-cursor declaration order closed the final saved-owner cycle.
- Control's apparent zero/CSE problem came from a redundant loop-index identity.
  Reusing the existing index across mutually exclusive switch scans matched
  without coupling a numeric counter to a null pointer. Qwen had returned
  insufficient evidence for this case; that did not exhaust the function.

The proposed `u16` allocator prototype improved caller scheduling but changed
the already-Matching provider (276 -> 272 bytes), so it was rejected. Capturing
the actual allocator before the caller's existing `u16` argument conversion
produced the same winning caller object **without any header/provider change**.
The other Qwen proposal duplicated a slot/index expression and grew the closed
dequeue frame 40 -> 48; the frame-aware comparison catches that regression.
Neither that proposal nor its neutral declaration-only refinement was retained.

Live source `f771bc792accabc4516158f14db00df743ccf61a698527683ba62a423c5292da`,
object `6e9089cb2a0d9a222da29d33ee9163d5a77284ddbbcbe00f2c4bd02ac0f4fabc`,
strict/data `878c1c8844bfc620e899c3ff1d88266b1f923639add9539c8fbfbce706b37ff8`.
The live-source compiler run reproduced the isolated object. Proof and all
fourteen relocation counts are in
`build/small-first-20260913/mqueue-control-shared-loop-index/frontier-proof.json`
and `exact-function-physical-counts.json`. The earlier allocation checkpoint's
one local `qUpdateReadPtrsAndIncNbrOfEnqueues` target shift (1968 -> 1972) is now
closed by Control's size fix. Full owner linking remains a final closure gate.

## Previous queue frontier: 12/16; frame and model-direction gaps fixed

`qUpdateReadPtrsAndIncNbrOfEnqueues` is newly exact: **316 retail bytes,
strict/data 100%, 1/1 physical relocations**, with all eleven exact siblings
preserved. The real head snapshot is shared by count traversal and removal;
an independent scan cursor counts nodes, and the live removal cursor advances
before freeing its old node. This closes the remaining lifetime/frame cascade
without synthetic storage. The stronger composition is retained, not the
higher-scoring intermediate that enlarged the already-exact frame.

Further retained gains: `qQueueControl` **61.06 -> 98.42** and
`qCheckDeQueueOne` **94.655174 -> 98.965515**. Control needed the true full-width
operation argument, case-specific domains, common success return and correctly
guarded reader activation. Batch dequeue needed direct consumption of the
reader slot, without altering the separately exact public dequeue helper.
Four instruction-nonexact queue functions remain; **this is not owner closure**.

All six direct header consumers were checked. TinyOS's full-width forwarding
and shared result path improve `ConnectInQtoBlock` **71.666664 -> 79.95238** and
`tosCallControlFunc` **91.875 -> 96.29808** (420 -> 416 retail bytes). The other
five consumer objects are unchanged, including the corrected local declarations
in Acne and SpecSub. No exact sibling was lost.

Current queue source `386f7062dc622cab41d306106650ad47390593b477076d76a21a497e57550286`,
header `f3b47c6ea766582b0a48574cbc689102de58bfc3c3c5b9aa37cc8fea274f9b5d`,
object `fa29f31215499d5f1ae022048377b7c8a639d0bd241103dc4b65c067bb060bcc`,
strict/data `005d5cb5618dc5b707cd17ef8192ff93016e49505fb418c5426fe95d963a962c`.
TinyOS source `d829529f3ee6829faba84df0331311ff28e382c75d36f2d1ff81350723ac5de0`,
object `7a896605e9dbe88d8f10022afa693eef9b673fba47975eaac5f9137f39ee98e1`.
Live-source rebuilds reproduced the isolated hashes. Aggregate proof:
`build/small-first-20260913/mqueue-batch-reader-slot/control-frontier-proof.json`.

Two additional improvements extend the existing causal tool, not new tools:

- Frontier comparison now detects closed **stack-frame** regressions. The actual
  count-loop intermediate improved 95.81013 -> 98.03797 at unchanged 316 bytes,
  but grew frame 40 -> 48 against target 40. The tool flags that mixed result;
  the retained exact candidate preserves frame 40. Unknown prologues stay unknown.
- New source-hypothesis packets bind explicit immutable-target/editable-candidate
  direction facts for differing aligned rows. Validation recomputes them and
  rejects rehashed swapped facts; late differences remain inside the bounded
  excerpt. Legacy packet/prompt identities are preserved. This addresses the
  completed Qwen allocation reply's reversed interpretation of a target-only
  stack store. That proposal was rejected without a compile. Direction metadata
  is not a guarantee that the model's causal proposal is correct.

Affected verification: **133 tests run, OK, 2 optional skips**. Both later Qwen
jobs completed; neither supplied a retained new source cell. Astra reconstructed
and verified these gains directly. Main/batch counts remain **347/396 and 2/10**.

## Previous queue frontier: 11/16, two exercised tool fixes

The current retained `mqueue.c` advances **5/16 -> 11/16 strict/data instruction
matches**. Six newly exact raw function bodies also pass independent physical
relocation checks: `qDeQueueOne` (80 bytes, no relocations), `qCheckInputQueues`
(256, none), `qFreeUnusedElemements` (292, 1/1), `qQueueReset` (204, 1/1),
`qQueueReDimension` (196, 2/2), and `qQueueConstruct` (140, 2/2).
All five prior exact bodies survive; eight other raw bodies and allocated data
are unchanged. Two partials improve: `qCheckDeQueueOne` **87.22414 -> 94.655174**,
and `qUpdateReadPtrsAndIncNbrOfEnqueues` **93.46835 -> 95.81013**, with the latter
restored to the retail 316 bytes. **Five instruction-nonexact functions remain;
this is not an owner closure or main promotion.**

The causes were normal pointer traversal instead of array-index loop spelling,
advancing a live cursor before freeing its old node, a shared free-list slot
owner, and the original byte-domain constructor arguments plus a one-element
trailing reader array. The explicit nonzero-size/profile fallback in ReDimension
preserves the target's independent result lifetime. No assembly, forced
register, padding, flag change, or dead temporary was introduced.

Live source SHA-256 `bacae1e46c1529fcde963cb95de05fd3db69f9a6c42ba5e5fc2b57ed872ba1e6`;
live header `64ff786b9849362c72970745fd84ee34d2109ed9ee1822a5b754da6e37d4c2bb`;
object `03b89c2f60859e55472f614032d1c13c69cb7f45b2a5171804f5631a5f897d4e`;
strict/data `cf094df0caa2097af32c0fef1cd83e46b35dc45f2763ff2a27a673e073c63194`.
The formatted live source independently reproduces the isolated object's hash.
Proof: `build/small-first-20260913/mqueue-byte-owner-construction/frontier-proof.json`
and its `production-binding.json`.

All six direct header consumers were built against old/new declarations. Five
objects are byte-identical. TinyOS's one affected caller needed the same real
byte queue-index domain: the composed change improves `ConstructQueue`
**82.32667 -> 82.69334**, preserves its 580-byte size and all 24 other function
scores, and reproduces object `55e4289a62f07f156786438653ee1e22bd5357cc3b112172f41c14a16aba814f`
from live source `233ed06b4d4fc90766da1dfd186c5ab94213937279e5755872f289a7f7016098`.
The intermediate header-only caller regression was not retained.

Two existing tools were extended and used on this work:

- `recovery_causal_groups.compare_match_frontiers(before, after)` / CLI
  `--owner-summary --baseline-strict BEFORE --strict AFTER` flags mixed gains,
  lost score-exact siblings, and closed-size regressions. It exposed the first
  free-cursor family's two new exact functions alongside an Update regression;
  the retained composition uses the stronger prior Update body. Scores remain
  advisory, not raw/physical proof or automatic retention authority.
- `compile_recovery_candidate.include_context`, `preflight_context`, and
  `compile_candidate` accept explicit `header_overlays` descriptors with logical
  `name`, resolved `candidate`/`reference` paths and both SHA-256 values. Default
  stale-header rejection remains strict; duplicate, unused, wrongly resolved,
  or drifting overlays fail. This removed the actual precompile blocker on the
  queue tail experiment without mutating the live header; subsequent byte-domain
  reconstruction closed the constructor. Selected and reference headers remain
  recorded separately. It is a Python API, not a blanket CLI skip-preflight flag.

Combined affected verification: **124 tests pass, 2 existing optional skips**.
Both Qwen support jobs completed: its free-cursor hypothesis helped; its trailing
array observation was conditional. Astra selected and checked the typed slot,
byte argument domains, caller repair and retained composition. Intermediate
regressions remain compact source constraints, not function bans.

## Earlier queue gain and measured-constraint tooling

`mqueue.c` advances **3/16 -> 5/16 strict/data instruction matches**.
`qQueueJumpBack` (268 bytes) and `qQueueJumpBackOne` (192 bytes) both use
one common success return, with the actual operation guarded by `count != 0`.
This closes the first branch mismatch and removes each candidate-only
eight-byte early-return sequence. One composed compile, no control matrix;
all fourteen other bodies and allocated data are byte-unchanged. The three
previous exact siblings survive. Source `2e5241907d31d49bf04078bb489122121a05fc83c5b5466a166d0fd91fab2a1a`,
object `d481431787c4fb23625f446b89fd86de097f74e977499495f77a3e6bea0aa7d8`;
strict/data `990f42f8064f5c2edbb245a97ed5f1c1c85c8eff0d1b9a1ecede86b0cc4a4ccb`.
Proof: `build/small-first-20260913/mqueue-common-jump-success/frontier-proof.json`.

This is **not owner closure**. JumpBackOne has no relocations and is fully
function-exact. JumpBack's single call has the correct site/type/symbol but
`qFreeUnusedElemements` still lies at candidate `.text+2924` versus retail
`+2984`; its physical/link closure awaits the remaining queue reconstruction.
That checkpoint left eleven instruction-nonexact functions; the current 11/16
frontier is above. Main and the ten-owner count do not advance for partials.

Two concrete tooling gaps were fixed and exercised here:

- `recovery_causal_groups.decision_packet(..., known_measurements=...)`
  now carries up to eight explicitly selected compact observations outside
  the 800-character question, inside the packet hash and shared byte budget.
  The existing CLI accepts `--decision-question`, `--decision-rows`,
  `--decision-mode source-hypothesis`, and `--known-measurements SELECTED.json`
  on its existing `--strict/--function/--support-source/--source-lines` path.
  Existing packets/prompts remain compatible. Observations are advisory,
  not cached compiler proof or function/family bans; different helper or
  coupled changes remain available. Select relevant measurements before the
  next Qwen request, rather than squeezing them out of its short question.
- The active scratch compiler now captures and checks wrapper/compiler bytes
  and the target hash before/after execution. Full argv stays in its compact
  retained summary, not repeated in console output. The actual queue gain
  was compiled through these fresh bindings. Old summaries are not upgraded
  into newly authenticated receipts.

Real acceptance: existing function fingerprints identify the byte-identical
ctxdata `group-property-count` / `ctx-word-property-extent` source recurrence.
Both selected count/header measurements now appear in a **12,913-byte** Qwen
prompt (`build/known-measurements-acceptance-20260913/decision.json`, packet
`2a2f06891c31a289be02170ba36c7ff65c69f92c3d0d37b44235a536ac3e4db6`).
The older compiler/include binding is incomplete and its raw object differs,
so no cached proof is claimed. This acceptance used no new inference/compile.
The active support builders pass these selected observations through; existing
dispatched packets were not overwritten. Ninety affected tooling tests pass
(22 existing optional skips), plus the bounded scratch-driver fixtures.

Compact non-gain constraints: ctxdata cached outer-record + nested-region
composition restored Syntax's 80 bytes but regressed WordProp; genfilt's real
byte-return input-query helper restored 124 bytes but regressed scheduling;
Combiner's eight-row initialization loop was unrolled to 296 bytes but added
eight frame bytes and left the cursor cycle. None replaced a live champion.
These observations constrain those exact hypotheses, not their entire owners.

## Context functions: closed, 15/15 exact

The small `gssdk_lib/gsapi/ctxfuncs.c` owner originally had ten missing source
functions. All fifteen bodies are now reconstructed; **15/15** pass strict,
data and independent raw physical checks. No exact sibling was lost. The
complete source `.text` is the retail **2500 bytes** and all **52 relocation
occurrences** agree. The owner is now locally source-selected Matching. A real
MWCC/MWLD rebuild passes all **137 retail checksums** (DOL and 136 RELs); the
production object reproduces the isolated candidate hash. It is closure 2/10
in the pending batch, not a main push.

New exact functions: `SessionDataExport`, `SessionDataImport`,
`ContextActivateParams`, `ContextDeActivate`, `ContextGetParam`,
`ContextSetParam`, `ContextSetWrdData`, `ContextUnLoad`, `ContextSetGcdData`, and
`ContextGetAction`, and `ContextActivate`. The four previously exact bodies
remain exact. No padding or fake local was added to fix frames.

Live source `483b8ff2fe4a08ae3ac9b043f1b2ad55da6f74b9b1bb57be29190e2dc7d5a35b`;
object `5cf28b795285b65215a189d792bd47deffc8425e5e2b1ba36a9b8ecd7e6011d7`;
target `aa14666733e8e64cdd22786dc385e75659a45f7100733ef0c1670917bdc01211`;
strict/data `3a7c0e38f6391fa4206577b6237285a921df2dd856a1f4d56f6e20b60be07784`.
Proof: `build/small-first-20260913/gsctx-import-result-boundary/frontier-proof.json`.
DOL SHA-256 `172ae27aa6fcc9074de286b07ae4ac9f9152175fa913fffc3f5b8365e139ffec`.
Main remains 347/396 until the source-only batch promotion.

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
did not improve GetAction; the then-live 13/15 champion was preserved. The optional
export's assignment-condition spelling was object-neutral. Qwen's delayed Mel
block lifetime was tested in valid C89 scope and was also object-neutral.

GetAction then closed with one new, primary-reviewed Qwen hypothesis: give only
the case-3 first-match scan its own live `limit = table->count`, retaining the
original function-scope `count` for case 4. Splitting both scan bounds had already
failed; that did not test this asymmetric lifetime relationship. This single
compile made **312/312 bytes raw exact**, strict/data 100%, without changing any
of the other fourteen bodies or the owner's 52 physical relocation occurrences.
All thirteen previously exact siblings survive. The new local is the real loop
extent, not a register-only alias. Qwen supplied bounded support; the primary
distinguished the hypothesis from the earlier failure and verified retention.

Activate's remaining 13 frame/home rows then closed by preserving the unsigned
`SessionDataImport` return as a block-local `u32 sessionStatus` before assigning
the caller's signed `result`. This is the real SDK-to-API result boundary, not
an added array, alignment directive or unused local. It restores the 48-byte
frame and session home at 0x18 while retaining all 80 target instructions and
all fourteen exact siblings. An earlier unsigned-import ternary normalization
changed the frame but regressed its standalone callee and register owners; it
was not retained. The caller conversion boundary solved the cause without that
callee edit. No original variable name or compiler virtual ID is claimed.

Both final closures used the existing compile/objdiff/physical tools. The
updated hypothesis guidance contributed one useful Qwen proposal; the primary
independently chose the import boundary. These outcomes do not justify a new
matching engine or copying either source shape without its real consumers.

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

## Keep compiler results usable, and hypotheses genuinely testable

Both active scratch drivers (`small-remaining-baseline.py` and
`small-first-cell.py` under `build/model-support-test/`) now use the existing
`tools.bounded_process` implementation for compiler and objdiff calls. Baseline
compilation has a 120-second deadline; candidate compilation and objdiff have
60-second deadlines, with concurrently drained, bounded diagnostic output and
process-tree termination. This does not cap Qwen inference or change compiler
flags. Verified source/header/object bindings are atomically saved **before**
report generation and score summarization, so a rendering failure cannot erase
a usable compile. Report-only recovery never compiles or retroactively invents
missing historical bindings.

Fifteen driver/scenario fixtures and existing read-only replay checks pass,
including compiler/objdiff hangs, report/census failure, and source/header drift.
A real unchanged-source baseline reproduced retained langdata source `a7223284`
and object `d3000e9f`, with a current binding at
`build/small-remaining-baseline-20260913/gssdk_lib/asrpho/common/ctxdata/langdata/retained-data-snapshot/binding.json`.
The updated candidate driver also ran a distinct field-snapshot composition
successfully and preserved its binding. That source cell regressed to 83.91428%
at 140 bytes; it is not retained, and the 84.77143% champion remains intact.

The completed Qwen codebook reply cited `source_causality_proven=false` as part
of its reason for supplying no hypothesis. Hypothesis-mode guidance now
explicitly distinguishes **unproved** from **forbidden**: actual used typed
locals, aggregate snapshots, and coupled source boundaries may be proposed
from supplied types/consumers, with uncertainty. A compiler ownership trace is
not a prerequisite for an ordinary experiment. Fake/dead storage, numeric
register shaping, and invented ABI remain prohibited. Validation, legitimate
`insufficient` replies, and legacy fact/target-only prompts are unchanged.
The saved reply remains valid and is not rewritten or credited with the
independently found source gain. Subsequent actual use of the updated guidance
produced the case-3-only GetAction hypothesis above, and primary review plus one
compile verified a new exact function. That is one demonstrated useful proposal,
not proof of a general model quality improvement or an automated matching engine.

The affected causal/Qwen/bounded-process/compiler suites pass **122 tests,
22 skipped**. These reliability and prompting fixes are not new owner closures.

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
