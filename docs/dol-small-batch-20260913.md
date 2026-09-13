# Small DOL batch — 2026-09-13

## m621: typed call context and terminal-return guidance (17/66 selected)

The current retained m621 frontier is **17/66 source-selected application
functions**, up from 13. The four round callbacks (`fn_1_5F0`, `fn_1_70C`,
`fn_1_778`, `fn_1_A5C`) add **1,372 code bytes**, for **2,428 including startup**.
Independent strict/physical comparison, linked section bytes, the retail REL,
and all **137 checksums** pass. Forty-nine application functions remain original
fallback. This is a verified partial module, not a whole-minigame/main closure.
Current proof: `build/minigame-recovery-20260913/m621/entry-proof/receipt.json`.

The round group's first raw draft was not exact. It exposed three repeatable
first-pass input problems, corrected together on the next compile:

- `fn_1_5F0` needs an `int` loop counter; the store narrows only the player-number
  field. The existing target loop-width cue distinguishes this from the real
  short counters in `fn_1_778`. An inferred field width is not a local type.
- Local calls need verified prototypes too. m2c inferred an argument to
  `fn_1_27A0` from the surviving global-address register, although the callee
  overwrites it before use. It similarly invented a third argument to
  `fn_1_5328`. The recovered contracts are `void(void)` and two integer
  parameters, respectively; they were checked against their callees.
- Three final conditional branches reach the immediately following epilogue.
  Ordinary explicit returns inside those conditional regions reproduce them.
  Removing such returns as redundant loses a target instruction under this MWCC
  configuration.

`decompctx.discover_call_context` now reports local missing prototypes separately
from external API/header discovery. It never synthesizes argument counts.
`target_integer_shapes` adds a narrowly checked terminal-branch review cue with
the actual branch/epilogue/conditional-predecessor rows. This does not prove
return versus goto, original names, or guarantee first-compile exactness.
The actual MWCC-preprocessed context now includes the real `game/mg/score.h`
contracts and the recovered module records. The preparation path runs this
preflight before m2c instead of merely having an unused diagnostic available.

The first draft of the camera/player-state group is retained as working source,
not selected Matching. Four of those callbacks are raw instruction-exact; the
other five now differ only in equal-valued literal ownership. Reconstructing the
helper's mixed float/integer parameter order fixed two caller schedules together;
`from = to = pos` fixed the ray endpoint copy chain. Neither is a universal
template. Camera `fn_1_CEC` retains the target's unused typed `obj->data` capture
as disclosed source-shape debt; no new fake store or assembly was introduced.

Focused verification: **67 tests, 66 passed and one optional replay skipped**,
including the existing Qwen runner's safe error diagnostics. Three prior broad
support jobs exhausted the shared KV cache (131,075 active tokens versus
131,072 total), not their individual 65K limits. New narrow questions run two
at once with xhigh and uncapped generation unchanged; no model restart/retry or
partial answer admission. Error receipts now distinguish this cause without
persisting arbitrary server text. This reliability fix earns no matching credit.
Replay: `build/minigame-recovery-20260913/m621/entry-proof/context-pass.json`.

## m621: earlier first-compile entry batch and shared-pool dependency guidance

At private commit `e9ec074`, the first canonical m621 compile had **15/16 application functions at raw
objdiff 100%**. The 13-function entry unit has no instruction differences and
passed independent physical relocation comparison, a source-selected link,
retail-identical m621 REL, and all **137 project checksums**. This retains 1,056
source-selected code bytes including startup, with **13/66 application functions
selected** at that checkpoint. The newer 17/66 frontier above supersedes its
partial source-selection counts; neither is a complete minigame/main advance.

The first draft used the real preprocessed API contracts, ordinary typed vector
initializers at their target creation point, and the actual earlier object
creator definition. The constant-size creator call auto-inlined naturally in
`fn_1_2E0`; no copied inline assembly, manual inline expansion, storage filler,
or compile-control directives were needed. The thirteen entry bodies were not
changed after their first successful compile. The verified source retains
address-derived function names where original names are unknown.

The sixteenth draft function, spatial audio `fn_1_420`, already has the correct
372-byte instruction sequence. The existing typed pool decoder resolves all
**36 reported rows** to equal literal bytes and owner identities, with zero
semantic/contract or unknown-value rows. Selecting its pool into a partial link
exposed a real shared dependency rather than an arithmetic mismatch. The audio
draft is preserved as `NonMatching`, not falsely counted as source-selected.

The existing `pool_reloc_summary` census now includes bounded
`partial_unit_dependencies`, calculated before output truncation. On the actual
first-compile object, it identifies two shared pool owners and five target
consumer functions absent as definitions from that object:

- `40.0f`: `fn_1_13EC`, `fn_1_215C`.
- `360.0f`: `fn_1_3084`, `fn_1_3E18`, `fn_1_47B4`.

That is an object-subset dependency observation, not evidence that those
functions have no source, proof of their original TU, or a guarantee that five
functions alone close the link. Missing or ambiguous owner/function mappings
remain unknown; different bytes never become value-equivalent. The useful
first-pass action is to inspect the source-selected split/link family before
selecting a pooled provider, leaving the already-correct audio body untouched.
It must not introduce label aliases or another source authorization gate.

Focused validation and the real replay are recorded in
`build/minigame-recovery-20260913/m621/entry-proof/first-pass-tooling.json`.
This change prevents a specific unnecessary body-edit/link detour; no universal
first-compile match guarantee is claimed. m635 remains at the independently
verified 52/52 application frontier below; the four-byte BSS ownership question
has not been resolved by inventing storage.

## m635: 52/52 application functions and first-pass shared-value guidance

The current retained local frontier is **52/52 application functions**, up from
33/52. All 20,224 code bytes, including startup/runtime, are source-selected;
all initialized `.rodata`/`.data` bytes are reconstructed. Independent function
relocations, linked section bytes, the retail-identical m635 REL, and all 137
project checksums pass. The remaining fallback is **332 bytes of `.bss`**:
this is not yet whole-module source closure or a main-progress advancement.
The current receipt is
`build/minigame-recovery-20260913/m635/entry-proof/receipt.json`.

The causal corrections were structural and shared-value changes, not register
permutations:

- Restored one graphics translation unit in target definition order. Earlier
  helpers are visible to the later callers under ordinary automatic inlining;
  their calls, inline bodies, and real literal producers now agree together.
  The 12 obsolete fragment files were removed; their earlier bodies remain in
  Git. No C-file includes, inline directives, literal seeders, or assembly were
  added. The graphics unit's 952 initialized data bytes matched on its first
  composed compile. Its leading 16-byte weak `sqrtf` constant contribution is
  deduplicated by the normal source-selected link; full linked `.rodata` is exact.
- `fn_1_2954` had one three-owner GPR cycle. Declaration/scope alternatives were
  neutral. Reusing the actual field-store result,
  `team = lbl_1_bss_BC[i].teamNo = GwPlayerConf[i].grpNo`, closed all ten code
  differences while preserving the 916-byte body. The local and both fields
  are independently declared `s16`; the indexed bases and local index are
  stable across the two original statements. This repeats the shared-producer
  lesson, not a rule to chain every assignment.
- The missing `fn_1_2F08` setter matched on its first compile. New `fn_1_33FC`
  matched instruction shape on its first compile, with literal ownership closed
  by the composed TU. `fn_1_30C8` and its automatic expansion in `fn_1_3738`
  needed one coupled local-declaration correction for the two observed captures;
  it fixed both standalone and inline stack homes. These are narrower claims
  than saying all 19 newly selected functions were discovered in one compile.
- The two observed producers in `fn_1_30C8` are a player pointer calculation
  and `CharMotionMaxTimeGet` result. The target stores both and does not directly
  reload them; the same producers recur in the inline caller. Their recovered
  assignments remain explicitly reviewed source-shape debt, not proof of the
  original spelling or permission to create fake storage. The Boolean return
  and third player argument of `fn_1_3B7C` are established by the standalone and
  inline call sites; the third parameter is unused by that callee.

Two existing tools now carry these lessons into the next draft:

1. `decompctx.target_integer_shapes` adds bounded `stack_capture_reviews` before
   compilation. It flags computed-pointer/call-result stores with no observed
   direct reload so m2c simplification can be reviewed against the real target.
   It does **not** infer an unused local: a callee may consume an outgoing stack
   argument. Branches, alias/escape uncertainty, overlapping stack accesses,
   unsupported instructions, or an incomplete frame/return keep the result
   unknown. Actual standalone/inline target packets retain that distinction.
2. `recovery_source_shapes.sequence_result_consumer` adds optional
   `mode=chained_field_capture`; default comma behavior is unchanged. It composes
   one reviewed shared-value pair and requires exact source/statement bindings,
   matching conversion types, stable indexed field bases, and explicit target
   rationale. Side effects, escaped or shadowed locals/indexes, changed types,
   stale bytes, and a local-dependent lvalue are rejected. Immutable replay from
   `17f1a17:src/REL/m635dll/players.c` produces the current `fn_1_2954` token
   stream. This is verified composition of a known gain, not automatic discovery.

Before the first compile of a similar family: use the real preprocessed headers,
review target loop/capture widths and producer stores, reconstruct helper
visibility/TU data ownership, and compose coupled creation/consumer changes when
the evidence connects them. Compile the coherent family, not artificially
isolated functions with a different inline/pool context. No new approval gate
or universal first-compile guarantee is introduced.

Validation: eight affected tool suites pass **165 tests, two skipped**; the
explicit immutable chained-capture replay also passes. Tool hashes, target cues,
and logs are bound in `entry-proof/first-pass-tooling.json`. The full workflow
suite was not rerun for these bounded changes. The unresolved BSS question is
four bytes between the 92-byte work record's last known byte and the next
object; preserve that uncertainty instead of adding invented struct padding.

## Previous m635 frontier: first-pass corrections and 33 functions

The retained local frontier is now **33/52 application functions**, up from
26/52. Seven additional functions are source-selected: `fn_1_2F40`,
`fn_1_448C`, `fn_1_2268`, `fn_1_27E4`, `fn_1_280C`, `fn_1_28A8`, and
`fn_1_2904`. Six were zero-row instruction matches on their first compile;
`fn_1_28A8` needed its real sprite-pointer snapshot restored after m2c folded
the address directly into the field access. This is measured target-guided
reconstruction, not a guarantee or a claim that a generator discovered all C.

The current-main-based isolated source-selected build verifies all 33 function
bodies and their independent physical relocations, all allocated section bytes
and BSS extent, and all 137 retail checksum entries. The selected code is
9,924/20,224 bytes including startup/runtime. Nineteen application functions
and 10,300 original code bytes still use fallback: **m635 is not closed and
main has not been advanced by this partial proof**. Compact receipt remains
`build/minigame-recovery-20260913/m635/entry-proof/receipt.json`.

The existing target preflight was corrected and extended, without another
workflow or permission gate:

- A saved halfword narrowed again before arithmetic/comparison is reported
  separately from an already-promoted int capture. Lexical capture scans stop
  at real branch destinations, branches, indirect calls and register restores;
  decorative GNU instruction labels are not mistaken for CFG joins.
- Back-edge containment records nested loops and their independent comparison
  widths. Actual `fn_1_2954` has four int-counter loops, including three nested
  player/motion initialization loops. A short field or call argument does not
  narrow the surrounding loop. The fresh sprite initializer's five loops are
  likewise int despite m2c's short suggestion for one reused counter.
- Consecutive two-dimensional word-array address formation records the row
  stride owner before the element stride owner. In `fn_1_195C`, both resource
  tables are `[night][team]` with 8/4-byte strides, not `[team][night]`. The
  two dimensions both being length two had concealed the transposition.
  The rule validates the same-symbol relocation pair and register chain; it
  does not invent source names, declarations, or table dimensions beyond the
  observed row stride.
- The pool decoder now retains aligned external data references as unresolved
  when definition bytes are unavailable. On the current scene report, 114
  literal-owner differences have identical values; the other 47 references
  have matching instruction/relocation contracts but external definition
  bytes remain unknown. They are no longer mislabeled missing consumers or
  counted as demonstrated semantic mismatches. Changed operands, symbols,
  relocation types/addends, and genuinely absent consumers remain failures.
  This is diagnostic classification, not a waiver of linked data proof.
- Pool census/chronology details now honor the existing group/row limits and
  report omissions while preserving total counts. The real 2/2-limited scene
  report shrank from 163,732 to 13,620 JSON characters (91.7%), excluding the
  CLI file digest. Existing SDA-specific classification is unchanged.

Acceptance: actual-target preflight covers the narrowed position capture,
all four player loops/two nesting relationships, both resource table strides,
and prospective sprite/motion loops. The affected eight-tool test group passes
142 tests, two skipped. No full workflow suite was run for these bounded edits.
The first sprite batch matched four functions immediately; its larger
initializer matched instruction shape/size with only literal-owner rows.
The sprite update's live button-member capture before the switch and its empty
state-zero case were missed by m2c and still required primary source review.
Do not mistake successful width guidance for full ownership/CFG reconstruction.

Working source retained separately as NonMatching now covers the camera,
scene/player initialization and movement, sprite animation, motion helpers,
model animations and moving model. Most are instruction/size-aligned with
remaining shared literal-pool ownership. Player initialization still has a
three-owner GPR cycle. Preserve their real original graphics-TU producer order
when composing the remaining functions; do not seed/export float labels to
make independent fragments look exact. Temporary partial splits must not
interleave code ranges from two object files: that creates a DTK link-order
cycle. The retained split boundaries are contiguous and source-selected.

Layout disclosure: `M635Sprite` represents the target's four 16-byte records.
Group/state/timer/member/scale consumers establish offsets 0/2/4/6/8. The last
four bytes are explicitly unknown byte storage, not an invented semantic field
or local register-shaping pad. The exact record extent, index stride and linked
BSS are recorded in `m635/sprite-layout-proof.json`; the source-quality exception
is limited to that real unaccessed tail. No inline assembly was introduced.

Qwen support did not finish the prior two jobs: the old runner/server were
gone, not silently still thinking. Restart attempts at 4/2/1 slots all failed
the existing 768-MiB GPU-headroom check and terminated their own server. No
partial answer was admitted and no resource safety setting was weakened.
The primary continued reconstruction without waiting for unavailable support.

## Previous m635 frontier: target-guided widths and 26 functions

The local m635 source frontier now contains 26 of 52 application functions:
10 entry callbacks, 13 logic functions, and three position helpers. The position
helpers (`fn_1_32E0`, `fn_1_3340`, `fn_1_33F8`) were 3/3 raw-instruction exact
on their first compile. Source-selected code is 8,916/20,224 bytes including
startup/runtime; 11,308 bytes still use original-object fallback. This is not a
whole-module closure or a main-progress claim. Proof is kept compactly at
`build/minigame-recovery-20260913/m635/entry-proof/receipt.json`.

The existing tools now carry the reusable corrections into the next draft:

- `decompctx.target_integer_shapes` runs on the target before candidate
  compilation. The existing call-context report and GNU-to-m2c adapter include
  its results automatically. It separates a loop's comparison width from
  narrowing performed only for a callee argument; records signed/unsigned
  halfword loads, already-extended saved-value captures, final return transfers,
  stack-array bases and immediate masks. These are instruction-backed cues,
  not inferred original declarations or a prerequisite permission gate.
- `recovery_source_shapes` now accepts `target_integer_width`. Given the
  reviewed real source-owner/target-register association, exact assembly hash
  and existing scalar typedef context, it emits one composed local-type repair.
  It supports bounded zero-based unit-step loops and a signed-halfword load
  promoted into a real int local. It rejects escaped, shadowed, modified,
  ambiguous, stale, or unsupported owners. No generated padding, new local,
  register directive, syntax matrix, compile or retention authority is added.
- Feed the actual preprocessed header context to m2c, then review these target
  cues before compiling the function batch. A halfword field does not make all
  locals receiving it short; a short API parameter does not make its loop
  counter short. Likewise `lhz`/`lhzx` constrain memory interpretation but do
  not alone prove a complete C type. Resolve masks, array identities and live
  snapshots through their consumers instead of universal declaration-order
  rules. Keep the target's natural TU/pool boundaries.

Measured retrospective replay: reconstructing the four earlier width mistakes
in `fn_1_D84` and `fn_1_F44`, the generator restored all four declarations in
one emitted batch compile. All 13 logic functions were instruction-exact. This
is a known-case regression replay, **not** four newly discovered gains or a
blind first-pass benchmark. The source-owner mappings were primary-reviewed;
the engine chose width from target comparisons/uses, without function-name
rules. Receipt: `m635/first-pass-width-replay/receipt.json` under the same build
directory. The initial failed draft was not archived verbatim; this replay
explicitly records a reconstructed width-failure fixture.

Prospective use: the new position helper has int loop counters but a short
captured player index (the target explicitly narrows it again). The preflight
correctly distinguished it from F44's int captures. Astra wrote the natural C,
with bounded Qwen ABI support; the tool checked the widths, not the entire
algorithm. First compile matched all three helpers. This is evidence of useful
first-pass guidance, not a guarantee that similar functions always match.

Other logic corrections remain source reasoning, not claimed automatic fixes:
unsigned button-index fields and return type; a typed complemented button mask;
live pad/player/character/button stack arrays; the captured night flag; the
team-array base captured before the model-reset call; and the literal-producer
boundary at `fn_1_1774`. The width tool does not solve arbitrary allocator or
translation-unit layout mismatches.

Verification also exposed avoidable memory pressure in the existing evidence
reader: `read(limit + 1)` reserved the 32/64 MiB ceiling even for tiny files.
It now sizes the read from the opened file, rejects oversize inputs before
allocation, and rejects growth/shrinkage during the read. Size-limit and
allocation regressions are tested. The local full-link helper also bounds DTK's
Rayon worker count after the machine refused its default thread-pool allocation;
this changes neither compiler flags nor source evidence.

## m651 gameplay batch: verified gains without blocking on entry BSS

Delivered through PRs 43 and 44 at main
`30dd8a6c3064e13eb98d15e41c0690aea571253f`. Exact promoted blobs were checked
against main; public progress is 400/936 objects, 22.34% code, and 43.40% data.
Both merged topic branches were deleted with exact-head leases. The clean main
checkout fast-forwarded successfully. This delivery adds three recovered owners,
not a whole-module closure. The paired proof and delivery receipt are under
`build/promotion/m651-gameplay-20260913/`.

The next m670 support batch gives four concurrent local Qwen/xhigh workers 16
small functions: sequence helpers (4), callbacks (5), motion (3), and tail
helpers (4). Each immutable packet is approximately 9-11 KB with selected m2c
bodies, relevant instructions, and API context. Manifest:
`build/minigame-recovery-20260913/m670/qwen/reconstruction-4way-v1.json`.
These requests are running, not validated gains. Do not resubmit the completed
m670 entry packet. Compile useful answers as they arrive rather than awaiting
batch consensus; primary owns larger state machines and integration.

Primary context repair is retained separately in `m670/typed/`: include the
actual `game/mg/actman.h` and source-backed mic-provider declarations before
re-running m2c. The first generic context omitted those interfaces, creating
stale-register ghost arguments and array-of-pointer guesses. All 28 application
functions decompile without errors with the repaired context. Use that typed
context to reconcile the currently running answers, without overwriting their
sealed prompts or treating inferred structures as canonical layouts.

The minigame-first pass reconstructed all 38 application functions. The complete
scratch source link has zero application instruction differences. The final
paired CPU-delay residual closed in both creation and update through the normal
signed jitter expression `(rand8() % 2 - 1) + delayTable[difficulty]`; moving the
subtraction to the table or outside the sum had not reproduced the target.

The publishable batch selects `players.c` (24/24 application functions),
`prolog.c`, and the existing compiler `runtime.c`. Together these account for
18,204 of the module's 19,228 code bytes. A fresh main-based project build selects
those three source objects and passes all 137 retail checksums; the 33,308-byte
m651 REL is retail-identical, SHA-256
`a873512645afe699544ff7a07a498939747585f5af94dd001b58395356f3fd48`.
Receipt: `build/minigame-recovery-20260913/m651/project-proof/receipt.json`.

This is **not** a whole-module source closure. `m651.c` stays NonMatching and its
14 instruction-exact functions remain local. Its BSS is eight bytes short. After
correcting the compiler's reverse global-definition allocation order, all 696
remaining effective-relocation differences are the same eight-byte displacement
into gameplay BSS. Changing the output section's alignment does not fix it;
main-TU pool-data mode regresses code and does not provide the missing storage.
The unused initialized data at offset 8 and the unreferenced BSS extent still
need ownership evidence. No padding, fake BSS array, or altered linker address is
being promoted to make this file appear complete.

The speedup was shared reconstruction: one actual type context for all m2c
bodies, typed model/motion arrays, natural per-function local tables and original
definition order, named live call results, and an early real module link. That
link resolves pooled-string annotation differences without source label hacks.
The five-element typed model/motion arrays are supported by loop extents and
Hu3D API consumers, not opaque tail storage. Unknown scalar fields retain offset
names. The initializer in `fn_1_13D8` includes a target-backed unconsumed zero
store; its original semantic name is unknown and is disclosed as `unused`.

Qwen was support only and did not choose retained source. Large grouped prompts
hit their context limit or spent many minutes without an answer. A reused batch
manifest also caused a status-writer collision; later requests used a unique
manifest and two small independent questions. Neither answer discovered the
winning expression. Keep tokenized prompt headroom, unique immutable job paths,
and small missing-decision packets; do not wait for support on already solved
functions. These local observations do not warrant another mandatory tool gate.

## m616dll closed from source

The first selected numbered minigame now rebuilds to the exact 19,788-byte
retail REL. Both SHA-256 values are
`75c0261ec53a59ec57fc0daa7460c1963d8649e928a446bae47f4220fb84a231`.
Canonical source comprises the startup file, four application translation
units and the existing compiler runtime. All 22 application functions and the
runtime/startup paths are verified by the real CodeWarrior/DTK source-selected
link. Receipt: `build/minigame-recovery-20260913/m616/source-link/receipt.json`.
This is a local closure, not a claim that main has already been updated.

The batch-sized gain came from source context, not isolated register edits:
independent literal producers and naturally aligned string regions identified
four application files. Reconstructing those boundaries, retaining real table
ownership, and using the already-established REL `-pooldata off` setting took
the combined object from 11 to 21 score-exact application functions while
making all initialized section bytes match. GC1.3.2 and GC2.6 generated the
same application instructions in the tested context; use the existing REL
default, not a claim that the original compiler version was uniquely inferred.

Initialization then closed through its real value producer:
`character = work.characterNos[i] = GwPlayerConf[i].charNo`, with the int
character owner and actual vector declaration/lifetime layout. Correcting
width/layout alone improved some constraints but left a larger allocator
cascade; the shared-state capture removed it without fake storage, callbacks,
assembly or register controls. The three remaining objdiff color-byte rows
were DTK's one-byte symbol-size annotation for the four-byte GXColor. The
HuVecF symbol is 12 bytes plus natural next-file alignment, not a 16-byte fake
vector. Both are corrected in the split metadata; the complete REL was already
byte-identical before those annotation repairs.

For following minigames: reconstruct source-file/literal ownership and reuse
the existing runtime before tuning individual function allocation. Qwen's
completed first helper answers were not credited with this crack; raw-offset
and variadic-argument guesses were rejected against actual source consumers.
Next-target support continues in parallel, without making it an approval gate.
Older milestone sections below remain chronological, not current status.

## Active scope: minigame throughput, not a DOL percentage prerequisite

The user switched immediately to minigames. Standalone DOL recovery is paused;
recover a DOL dependency only when an active minigame needs it. Two already
verified local MusyX closures remain preserved, not yet published.

The current-main selections over the available DTK split inventory contain 136
REL modules, 113 with unmatched code, including 82 numbered modules. The 597
unmatched code-bearing object instances are not 597 separate DLLs; automatic
section splits are not authenticated original C translation units. The bound
operational census is `build/minigame-recovery-20260913/backlog-census.json`.
It is a ranking input, not a regenerated public progress or closure proof.

First ten small real numbered targets, by remaining code: m616dll, m651dll,
m670dll, m635dll, m621dll, m657Dll, m629Dll, m612dll, m640dll, m659Dll. Begin
source/type support for later targets while the primary integrates the active
one; do not wait for a worker consensus or open new owner chats. Two Qwen jobs
for the first two application functions of m651dll/m670dll are running under
`build/minigame-recovery-20260913/next-small-support`; their inputs are bounded
target assembly, m2c hypotheses and current header declarations. They do not
own canonical source. Reuse existing runtime, API, compile/diff and final-link
capabilities. Batch by common source/compiler context, not blind source edits
or owner-count inflation. Completion remains source-selected retail REL proof.

Immediate measured use on m616: fix the missing no-input switch arm, preserve
the loaded pad value before masking, and materialize the live motion table
after the preceding motion call. `fn_1_4C0` improves 95.39939 -> 97.44932 with
unchanged default compiler settings; all eleven prior 100% symbols survive.
The existing `-pooldata off` context used by several already-matched RELs is a
separate diagnostic, not a silently adopted production flag. It removes the
candidate-only cached section-base chains across three functions: fn_1_160
94.20465 -> 99.76744, fn_1_1590 90.384964 -> 98.64812, and the corrected
fn_1_4C0 97.44932 -> 99.924355 (2644/2644 bytes, only ten data/string/jumptable
attribution rows). Evidence is the default and `pooldata-off/latest-compile.json`
under `build/minigame-recovery-20260913/m616`. No new whole REL closure or
Matching flag is claimed. Next resolve actual TU/pool ownership before local
register changes. A bounded initializer declaration/width trial restored the
vector homes but worsened that diagnostic and was not retained; it is not a
reason to exhaust the function.

Main starts at `086a3a84fac9d928c9176df3624582fd28befd45`, **347/396**
Matching DOL owners. This is the next ten-owner batch, not ten new closures.
Current locally verified frontier: **350/396**, **3/10** batch owners.

## Highest-payoff SDK closures: Stream and DSP control

Main `c0995049eff7a1eb9008497892b269c10c9e6a16` contains the delivered
NewMore/ctxfuncs/Vq1500 batch: 350/396 DOL owners, 86.43% code. The next
clean-main-based source-selected build closes two more owners: Stream 18/18
and DSP control 15/15. Its 137 retail checksums pass; verified local progress
is 352/396, 87.77% code (1,908,148/2,173,968 bytes), 98.84% data
(709,008/717,328 bytes). These two owners are not yet promoted to main.

The change in selection matters: optimize credible complete-owner paths and
verified bytes per active hour, not proximity to 100% or the smallest object.
Do not spend another long run cycling tiny saved-register tails while larger
SDK owners have a single shared source cause. Keep their partial champions.
Missing speech-recognition owners still require m2c/target reconstruction;
SDK donor availability is not a prerequisite. Group those by real shared
interfaces and dependencies, and use Qwen for disjoint bounded source
questions while the primary chooses and measures changes. No consensus gate,
new lane, flag tournament, or general tool project is implied.

Current inventory after these closures: 44 owners / 265,820 code bytes remain.
The speech library owns 39 / 222,100 bytes; 15 of those have no source file,
covering 147,324 bytes (55% of all remaining code). The other five owners are
GeckoException, MIC, printf, msmsys, and msmstream (43,720 bytes). Their closure
would reach about 89.78%, not 100%; the missing-source reconstruction track
cannot be deferred behind endless near-match polishing. Reused/current
baselines are in `build/dol-batch/current-dol-fast-path.json` and
`build/small-remaining-baseline-20260913/*/sdk-selection-eb7/`. MIC is 36/41
score-exact, msmsys 19/23, msmstream 24/28. Those are selection facts, not new
closures or promises that the remaining source causes are trivial.

Stream's one reconstruction restored SDK sample-cursor versus ADPCM byte-offset
lifetimes in `streamHandle`; it closed all 44 register-cycle rows. Its
remaining pool-owner rows closed by including the real Dolphin math header,
which avoids the unrelated weak Newton constants from the generic math header.
All 18 instruction bodies and 349 relocation applications match. Three unused
SDK functions are naturally stripped. The `.sbss` two-byte and `.sdata2`
four-byte tails are ordinary linker alignment, not source padding.

- Source `ccccf7ac0bd422f98cc1ef19db4643a4cff976536f9cc40980db847d38e4d14b`
- Production object `84931cdf5dc8a8a4598327b0cfa130c387749de7e95a6d9accc7cf3392114f7d`
- Proof `build/small-first-20260913/stream-dolphin-math-context/live-proof.json`

DSP control's first hypothesis removed only the later SDK format-6 extension
absent from every retail dispatch/loop path, and restored the actual eight
`salMalloc` call consumers. It removes the entire 156-byte excess: 15/15
functions, 13,780 instruction bytes, 604 relocation applications. Six local
SDK helper names are authenticated by equal raw bodies and linked destinations,
not guessed from addresses. The real low-pass parameter block and unsigned
DSP command-list type complete its headers; the actual existing two-hook
allocator and byte-voice callback interfaces stay unchanged. The necessary
`salStartDsp` unsigned-pointer definition agrees with the SDK and changes no
linked bytes. All affected Matching consumers remain retail-identical.

- Source `1afff6574da19d7ea7a6e20f1b42e60fc6f9fad6d6bc2249d4d1002235e0969c`
- Production object `832956d56f5bba5f7191b69beebc69ce14753035ef89f68a62c3d93b6ba0d5ff`
- Proof `build/small-first-20260913/dsp-target-streaming-formats/live-proof.json`

The SDK donor is `AxioDL/musyx` at
`adc8df9a959f1e37f71bdf3155e229f9f87ad166`; source shape is still checked
against this game's target. Existing Stream inline depth is SDK-backed;
neither closure added ASM, fake owners, register controls, or changed flags.
No new production recovery tool was needed. Completed Qwen SDK jobs supported
the same causes; primary reconstruction and measurement made the decisions.
Qwen now analyzes two separate MSL formatting causes in
`build/qwen-printf-support-20260913`; do not restart running jobs by elapsed time.

## Delivery checkpoint and source presentation

At the user's roughly twelve-hour checkpoint, only three owners were closed.
The ten-owner target was not met. Byte-small/high-percentage register tails
consumed disproportionate effort; file size alone is not a reliable closure
cost estimate. Prefer concrete full-owner reconstruction/SDK opportunities,
keep partial champions, and reassess low-yield tails instead of expanding
instrumentation indefinitely. A verified subset is being prepared for delivery;
no main progress is claimed before the clean promotion and retail checks pass.

Public-source cleanup names the observed GS context status domain, expresses
the GCD tag with its characters, and uses the real default VQ input length.
Unneeded VQ alignment members were removed; ordinary C alignment preserves
the exact field offsets. Existing opaque context intervals remain explicitly
unresolved and are covered only by their target-consumer evidence, not invented
field semantics. Numeric extent presentation does not resize them.

Fresh actual-compiler rebuilds reproduce both previously exact objects:
ctxfuncs source `b075db43c8ec7c8518e4a0f3f3949cefd8614f3a98046de3a34e52e7a238f944`
-> object `5cf28b795285b65215a189d792bd47deffc8425e5e2b1ba36a9b8ecd7e6011d7`;
vq1500 source `553b8901e4c45234c6d48f8a0057638bf7425a94aa73908d1f7ef1e96863ece8`
-> object `830c8c5f0e5a15ad7f66a9b22eb363d8154470a38773620ddf6350b1f92b52d2`.
`include/gssdk/gsapi.h` changes only equivalent integer array extent spellings
from the already verified header. Transition receipt:
`build/dol-batch/three-public-presentation/verified.json`.

## FFTMod: four additional exact functions retained

FFTMod improved from **1/6 to 5/6** strict/data and raw instruction matches.
CalcPowerSpec (1056 bytes), CalcAmplitudeSpec (796), ProcessFFTMod (400),
InitFFTMod (632), and the protected ControlFFTMod (164) are exact. The entire
72-byte `.sdata2` payload is exact. A fresh live-source rebuild reproduced the
candidate object. No owner closure or linked FFTMod proof is claimed yet:
ConstructFFTMod remains 156/156 bytes with only its incoming context copy
`mr` versus `addi ...,0` differing (three aligned objdiff rows).

The source causes were concrete: cursor advancement around the first spectrum
sample, f32 logarithm conversion before f64 scaling, the real `floorf` provider
and SDK math-header visibility, FFT-length snapshot lifetime, corrected
FFT-length/frequency/sample-rate formula, and the target's two-arm clamp value
boundaries. A typed FFTMod initialization receiver removes an unnecessary
base-to-derived copy. Existing SDK callback casts preserve the pointer ABI;
no new shared prototype, compiler flags, assembly, or padding was introduced.

Retained source `9bc09143dee3ca39055bdc815fb57101ea6e3590bf8b70848f31f5cbf0c9872d`;
object `d079dc75c1301aa2fae0e4cca979d12f6491685a1e8c56d785fb394a7e4e34b0`;
strict/data `f7f158d7288de38f2bfed9ef5581cdcc4db72ee8a6751a733568fb7f1521ebc2`.
Proof: `build/small-first-20260913/fft-typed-init-receiver/frontier-proof.json`.
All 57 relocation applications agree, including the explicitly recorded DTK
0.9.2 SDA21 instruction/halfword convention; raw physical-offset identity is
not claimed for those 26 mappings. Final linked application remains pending.

## Two concrete Qwen compatibility gaps fixed

The existing recovery prompt now records the observed GC/1.2.5n rejection of
nonconstant local aggregate initialization and suggests ordinary field
assignments without changing compiler flags. This is a compiler-specific
caution, not a universal C90 restriction or source-admission rule. The real
FFTMod initializer attempt supplied the acceptance case; it did not compile
and was not retained. The guidance/fact compatibility suite passes 69 tests
(one optional skip).

The installed Qwen runner preserves the successful launch-time prompt receipt,
its validator identity, and immutable prompt/packet hashes. On natural
completion it checks the answer and packet with a stable current validator;
it no longer rerenders an already validated prompt with a changed template.
Incompatible answer/packet validation or any prompt/packet/receipt mutation
still rejects. Thirty-one fake-HTTP tests pass without inference or retries.
Runner: `C:/Users/Anony/.codex/tools/qwen-support/run-job.ps1`, SHA-256
`ffde5c6872e13baa26fee82206d66bcf10b2878a2689ccec35a8573c701ae9b2`.

Actual Exev job `exev-indexed-source-boundary` completed naturally in 771.489s
but the old runner rejected it because the renderer changed during inference.
Its preserved answer independently validates with the current checker. The
old rejection receipt is untouched; no inference was repeated and no source
gain is credited to that proposal without measurement. This fixes wasted
completed work, not source-cause accuracy by itself.

## Active Exev dependencies and Qwen rewrite checks repaired

The existing `recovery_causal_groups.py` producer slicer now models validated
non-update indexed GPR/FPR loads and stores. It records base/index dependencies
without inventing memory identity; RA=`r0` is a zero base while RB=`r0` is a
register input. Stores do not define their value register. Validated `mtlr`
reads its GPR without discarding unrelated definitions; indirect call targets
remain unproved, and call preservation still requires the explicit ABI model.
Malformed operands and update-indexed forms remain UNKNOWN.

Read-only replay on Exev report
`12df75035c9e7900a4553f65a2a8dfec5777476429e986f8e11f1288acf519fd`
now retains the dependencies at rows **21, 22, 73 and 82**, previously dropped
as unsupported. Both streams converge in four unique-CFG passes. In particular,
the cache bases at73/82 bind to definition13; the transition-table loads bind
their base to18. Conflicting loop-carried indices and unknown call results
remain UNKNOWN. This repairs input visibility, not source-cause inference.

The same existing tool's Qwen answer checker now adds a narrow advisory
store-count review for straight-line indexed-to-advancing-pointer rewrites.
It catches the actual completed Combiner initialization answer's **8 -> 7**
stores. Comments do not count; unsupported control flow, aliases or literals
remain UNKNOWN. A matching count is not semantic-equivalence proof, and the
checker does not grant retention authority. The primary corrected the omitted
eighth store before compiling; that corrected rewrite was object-neutral.
Neither this fix nor the Exev replay is counted as a new crack.

Affected regression suites: **165 tests, 23 optional skips, pass**; the explicit
actual-Exev replay also passes. Both Qwen jobs in
`build/qwen-exev-comb-small-20260913/` are complete. Exev's suggested endpoint
assignment swap does not explain the observed dataflow and was not compiled.
The retained Combiner source gain below remains unchanged.

## Combiner: retained source gain using the repaired comparison

`CombinerProcess` improved **97.547620 -> 98.190475%** strict/data with
**840/840 bytes** and its exact 32-byte frame preserved. The real history-row
selection index is reused for the subsequent band traversals, in the signed
domain required by their comparisons. This keeps the first loop's zero-init
after history selection and restores the input-header advance chronology.
The target-anchored comparison reports **4 resolved, 27 persisting, 0 introduced**
observations. The primary selected the source change; the tool measured its
effect rather than discovering or proving the original spelling.

The live rebuild reproduces source
`e47efc7e90c620912d5932b264fcee38e4a6b2941e916cdc31c4a1a98282495d`, object
`0f4d6e8eb43dd818970fe5e4d1b90e9116b989f3a5377d60b38316b5ab83dd74`, and
strict/data report `300e39c9fe036561216887c2a46294cf9dd6529d22d519fb5f61f2aefe7d55ed`.
All three other function bodies, allocated data, and every candidate physical
relocation inventory are unchanged. Both previously exact siblings remain
raw-exact. Process's existing physical mismatch remains; no owner closure or
new linked proof is claimed. Compact proof:
`build/small-first-20260913/comb-shared-history-band-index/frontier-proof.json`.
The organicity checker reports 100/100; source review independently confirms
real sequential index uses, without padding, flags, ABI changes, or fake locals.

A distinct direct derivative-cell cursor composition closed the remaining
large cursor cycle, but folded two input advances and shrank 840 to 836 bytes.
Its 7-row diagnostic is **not retained** despite the higher scalar score
(report `3d7bb843...`, object `e3bcc260...`). Advancing that cursor in a separate
statement regressed further; a real energy-reader helper left the 836-byte
body unchanged. Keep only this compact constraint: the productive pointer
consumer and the input-header value boundary must be reconciled together.
Exev's entry-context snapshot also regressed and did not replace its champion.

The earlier Smoother Qwen job `06b18baf...` completed; its proposed `valuesEnd`
substitution contradicts the target's explicit rows/base construction and was
rejected before compilation. Independent Qwen support for Combiner initialization
(`3c54f13c...`) and Exev transition traversal (`f00b7e40...`) subsequently
completed under `build/qwen-exev-comb-small-20260913/`, with the results recorded
above. Neither was a gate on primary reconstruction. Batch remains 3/10;
main remains 347/396.

## Success-path producers and scheduling comparisons

The existing slicer now offers `--producer-flow-model unique-cfg` alongside
`--producers` and the optional `--producer-call-model ppc-eabi`. It propagates
only unanimous physical definitions over recognized edges after convergence.
Conflicting definitions, unsupported effects/edges, unresolved destinations,
forward/cyclic dependencies and bounded-analysis exhaustion stay UNKNOWN.
Default block-local output is unchanged; no source identity is inferred.

Actual undersampler report `0049279c...` now connects quotient25 to use37:
target `r5`, candidate `r4`. Branch28 reaches the success block34; the error
arm's logging call exits past that block. Both streams converge in two passes.
This closes the earlier artificial CFG-entry UNKNOWN, not the source mismatch.

`compare_function_constraints` also no longer aborts when two inserted
instructions share one target boundary. It reports that whole boundary as
unresolved and compares the remaining target sites. The actual Smoothing
`smoother-input-end-recurrence` replay yields 12 resolved, 30 persisting and
4 introduced observations, plus one unresolved two-instruction boundary.
Its score regression **97.887850 -> 95.481310%** and unchanged open frame
remain explicit: those resolved observations are not a retained gain.
Ambiguous neutral replay produces no invented resolutions. Detail limits
retain full counts while bounding groups/rows. Fact prompts now describe the
selected CFG/EABI scope instead of contradicting the supplied dependencies;
legacy no-model prompt text is unchanged. **138 tests pass, 22 skips**.

Current source work is not credited as closure: the Qwen factor snapshot
compiled to the unchanged undersampler report; the callback proposal to cache
the allocation extent contradicts target rows93-98's post-call reload and
recomputation and was rejected before compilation. Sliding bin-helper/clear
boundaries were neutral; sample normalization and a separate Smoother element
counter regressed and were not retained. Exact siblings/champions are untouched.
Qwen's current Smoother structural question is packet `06b18baf...`, under
`build/qwen-smoother-source-boundary-20260913/`; its task is not a tooling gate.

## Earlier producer gaps repaired

The existing `recovery_causal_groups.py` slicer was dropping the actual queue
`clrrwi` and undersampler `divw` producers as unsupported opcodes. It now
validates rotate/mask aliases and integer divides (including record/overflow
spellings), preserves their register dependencies, and rejects malformed forms.
On queue report `c53b9a10...`, target rows **18 -> 20 -> 22 -> 23** remain linked;
row24 also traces its word-count input to18. This reveals the shared size value
instead of treating two independent UNKNOWNs as source owners.

An explicit `--producer-call-model ppc-eabi --producers LIMIT` option preserves
known ordinary nonvolatile GPR/FPR14-31 definitions across recognized calls.
The default remains conservative. The assumption is hash-bound in the packet;
callee conformance is **not** claimed proven. Reserved registers, memory,
CR/XER and unknown CFG joins are not inferred. Actual `InitUndersampler` report
`0049279c...` now traces division25's input to conversion10 across calls15/20;
the quotient use37 remains UNKNOWN at CFG entry34. Thus a normal call no longer
hides that known input when the appropriate ABI model is explicitly selected.
**124 affected tests pass, 22 optional skips**; both active reports were checked
read-only. No compiler flags, source code or Matching gates change in this fix.

Completed Qwen jobs `89007` and `84148` supplied no additional retained gain:
the queue provider helper regressed an exact sibling; the port snapshot grew
the frame without resolving the owner cycle; the constructor's profile-context
consumer removed12 required bytes. The packed-u16 record proposal was rejected
before compilation for questionable pointer arithmetic and insufficient new
evidence. Callback typed-context and cursor-loop probes also regressed; their
compact constraints are under `build/small-first-20260913`. None bans a function.

The repaired producer output is in the next two actual Qwen packets under
`build/qwen-repaired-producers-20260913`: undersampler `661c0810...` (27,004 prompt
bytes), callback local extent `70e948a8...` (33,039 bytes). Both are running in
session52654, with current source hashes and selected measured constraints.
Inference limits are unchanged. The retained queue source remains `d11a4493...`;
local/main totals remain350/347. Tool coverage is not counted as a crack.

## Vq1500 closed; queue gain retained; GC1.3 scratch mapping repaired

`vq1500` is **9/9 strict/data and raw-instruction exact**, 1,264 text bytes,
with all eight previously exact siblings unchanged. `GetLabel`'s six-register
operand differences closed by giving the fine-search enumeration its own
`fineIndex`, instead of reusing the coarse-search index. The two real search
phases keep independent index lifetimes; no arithmetic, ABI, fake storage,
assembly, pragma, or compiler flag changes were needed. Source
`319fced430e52b2b01d0420b2a11099f08ca960f6dd479b60fa34968abc727ee`, object
`830c8c5f0e5a15ad7f66a9b22eb363d8154470a38773620ddf6350b1f92b52d2`, strict/data
`797976233d9794389caa7c4e1ac41f33d7f5dc2a836dac61b5f846d06850c290`.

The 25 relocation applications are verified, not falsely described as raw
offset equality: two SDA21 records in `GetLabel` use the MWCC halfword site
versus DTK's instruction-start site, with identical instruction/target values.
The sole 4-byte float pool is unchanged; target `.sdata2` has four additional
zero bytes named `gap_11_802C2E44_sdata2`. The real source-selected MWLD link
proves both equivalences: retail-identical DOL `172ae27a...`, **137/137**
checksums. Full bounded proof:
`build/small-first-20260913/vq-fine-search-index-lifetime/closure-proof.json`.
The Matching gate is retained locally; main is still **347/396**, pending the
ten-owner batch. Indexed fine lookup grew code; moving both cursor/counter into
the for-loop changed scheduling. Neither failure rejected the distinct
fine-index lifetime that actually closed the owner.

Qwen's corrected known-measurement packet completed with a new live
`firstNewElement` snapshot for the reader update and return payload. The primary
placed it in a C90 block (the proposed mixed declaration is unsupported by the
actual compiler). This improves `qEnQueueOne` **92.878784 -> 96.818184**, preserving
264 bytes, all 15 other raw bodies, data and 14 exact/physical siblings.
Only the late-versus-early allocator word-count shift remains (three aligned
rows); the unrelated reader function remains 98.965515. Live source
`d11a449308a00816987313e43c8a43f37b83ef86f6c033306417cd6eff373075`, object
`841c3faa333876c8ef9ff10cddc8b6e22de8c2fb97afb8caab4b0fcc6a3ac1f7`, strict/data
`c53b9a106794d508d9d21ea3f5e73332690bc5eaeb1b6e02c83a01e9050c79eb`.
`build/small-first-20260913/mqueue-first-new-element-block/frontier-proof.json`
binds the live-source reproduction. This is a retained partial, not queue owner
closure. No Qwen inference remains pending.

Actual configure exposed the missing GC/1.3 scratch mapping. `tools/project.py`
now maps it to `mwcc_242_53`, authenticated by the installed compiler's
2.4.2 build53 version and decomp.me's compiler registry, never by aliasing to
GC/1.3.2. Three focused generated-config tests pass (library/object overrides,
DOL/REL, unchanged1.3.2 and unsupported-version behavior); actual proof-workspace
configuration emits valid scratch mappings for all 30 GC/1.3 units without the
warning. This repairs access to scratch comparison for those owners, not a
claim that the mapping itself matched code.

The active Qwen declaration failure also revealed a reusable prompt gap.
The existing source-hypothesis renderer now explicitly honors the supplied
dialect, uses block-entry declarations when mixed declarations are unsupported
or unestablished, allows legitimate nested-block snapshots, and forbids changing
language flags to make a proposal compile. Fact-mode and historical packet
bytes remain unchanged; the actual saved queue packet was rendered read-only
in the regression test. This is guidance plus a primary syntax review, not a
claim that text prompting guarantees valid code.

The next two Qwen jobs are independent, current-source-bound support for the
remaining allocator scheduling and reader lifetime causes. They include actual
measured counterexamples and the new dialect guidance, under
`build/qwen-mqueue-last-two-snapshot-20260913`. Word packet `e22728fd...`
(27,071 prompt bytes), reader packet `08a63922...` (25,318 bytes). No inference
limit changed. Primary's unconditional index-before-word-count cell compiled
object-identical to the retained snapshot and was not applied; it is included
as a measured constraint rather than another proposed winning cell.

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
# Current priority: minigames immediately

The user subsequently superseded the 90% threshold: start minigame closures
now; match another DOL owner only when it is needed by the selected minigame.
The two verified MusyX closures below remain preserved locally. No further
standalone DOL experiments or speech reconstruction are scheduled. The two
speech Qwen jobs were interrupted when changing scope. Four merged public
GitHub branches were deleted after verifying their exact heads are ancestors
of main c099504; all their content remains on main. Unmerged tooling and the
active recovery branch were preserved.

### Minigame m616dll: first current-target reconstruction

Selected the smallest nonempty numbered module in the current retail inventory:
m616dll, 12,252 text bytes including compiler runtime. Empty placeholders are
not recovery gains. Target PLF SHA256 is
`48aaa5c45888ba7333c5a3cb7cf7feacba9628a3768cb1b976393f234a560261`.
The application has 22 functions from fn_1_A0 through fn_1_2580. The following
fn_1_2598 is an unsigned floating-conversion runtime signature, not a gameplay
function to reconstruct with hand-written assembly.

The first canonical C reconstruction and 460-byte work layout are in
`src/REL/m616dll/m616.c` and `include/REL/m616dll.h`. They are deliberately not
configured Matching and have not been promoted. Existing historical prefix and
helper fragments remain untouched until the final canonical source ownership
is proved; do not configure them together with duplicate canonical bodies.

Actual current-target compilation reports 11/22 functions at 100% objdiff:
fn_1_A0, fn_1_104, fn_1_140, fn_1_4BC, fn_1_135C, fn_1_1360,
fn_1_1364, fn_1_139C, fn_1_20D8, fn_1_2104, fn_1_2580.
Four of these had preexisting unconfigured source fragments; seven are newly
reconstructed bodies (680 target bytes, including three real empty callbacks).
These are object-symbol observations, not new whole-module or main progress.
There is no source-selected REL proof yet. Ordinary natural C recovered the
500-byte timing callback without a source-shape probe.

Main fn_1_4C0 improved 87.97125 -> 95.39939 by correcting the decompiler-inferred
short loop counter to the actual full-width counter and reusing its genuine
player iteration across phases. The winner sequence fn_1_F78 is 99.948715,
CPU selection fn_1_23B0 99.913795; both have only a string relocation pair left
in the current comparison. Init fn_1_1590 is 90.384964. No asm/pragma/volatile/
register-control changes were introduced. GC1.3.2 is the configured REL default;
the same 18-function intermediate on GC2.6 was instruction-identical and did
not settle the compiler identity. Keep that uncertainty explicit.

Next cause, not a syntax search: reconstruct source-file/data ownership.
Retail has distinct equal-zero producers at rodata+0x10, +0xA0, +0xD0 around
the main/init/helper regions, plus initialized tables after their strings and
the position global before the main work global. The current single-TU first
pass pools those differently and introduces a cached string-section base in
fn_1_160/main/init. Investigate the actual TU boundaries/definition chronology
before declaration/register changes or compiler-flag overrides. A three-region
main/init/utility partition is a hypothesis, not an authenticated split yet.
The local initializer vector at retail rodata+0x90 has emitted initialization
but no observed later use; preserve this bounded target finding rather than
inventing a consumer or calling an invented padding array authentic source.

Tool use: m2c with canonical compiler-preprocessed headers successfully supplied
the starting bodies. Its switch parser initially failed because DTK jump-table
entries use `fn_1_4C0+offset`; resolving these exact targets to local instruction
labels in generated assembly (no binary/source mutation) produced the complete
switch. Current compact evidence and reusable local compile/context commands:
`build/minigame-recovery-20260913/m616/`,
`build/model-support-test/compile-m616.py`, and
`build/model-support-test/m616-m2c-context.py`. These are generated experiment
support, not newly shipped tooling. Qwen's two bounded read-only jobs are under
that folder's `qwen-batch.json`; the primary did not wait for consensus and
reconstructed the owner while they ran. Check their actual terminal output
before reuse; elapsed time is not an answer or failure.

Added the missing Hu3DMotionTimingHookReset declaration to game/hu3d.h from its
existing identical definition in game/hsfmotion.c:578. No DOL implementation
changed. Affected-consumer/link verification remains part of integration.

GitHub cleanup is complete: the four merged project/recovery branches for
`dol-context-vq-runtime-20260913` and `dol-small-ten-20260913` were deleted with
exact-head leases. Fresh ls-remote shows only main, the active recovery branch,
recovery-context-workflow and recovery-tool-review-fixes. The last two are
unmerged tooling, not safe-to-discard merged branches. Open PR census was empty.

## Superseded target: DOL 90%, then minigames

The user's latest instruction on September 13 supersedes the earlier 100% DOL
priority: reach at least 90% verified DOL code on main, then switch to minigame
owner closures. Do not continue polishing DOL tails beyond that threshold.
Current verified local progress is 1,908,148 / 2,173,968 code bytes (87.77%),
352 / 396 owners, leaving 48,424 bytes to the threshold. Main remains 350 / 396
until the Stream/DSP batch is published. The five remaining non-speech owners
contain 43,720 bytes; adding the 5,272-byte speech context-conversion owner would
cross 90%. This is a selection path, not a promise that these owners are easy.
Prefer another demonstrably faster complete-owner path if evidence changes.

The unchanged-source Qwen floating-format packet failed at its 65,536-token
context boundary: 46,567 prompt tokens and 18,969 generated tokens, no final
answer. No answer was admitted. Two smaller reconstruction packets now use
m2c output and only their relevant target functions (11.6 KB / 18.1 KB), while
the primary continues independently. A GCC unroll pragma from the completed
integer-formatting answer was rejected; its useful loop-location evidence was
retained. The natural post-increment loop probe and MSM resume-read consumer
probe were both object-neutral; neither changed live source.

## Shared minigame initialization context: reusable repair

REL `_prolog`/`_epilog` and the selected compiler runtime are already verified
in the delivered m616/m651 modules. No constructor-runtime defect was found;
m651's unresolved eight-byte entry BSS extent remains a separate source-ownership
problem. Do not add padding or call it a runtime fix.

The active m670 initializer instead exposed missing shared API context. Extend
the existing `tools/decompctx.py`, rather than creating another recovery engine:

```text
python tools/decompctx.py build/.../context.i --calls-from build/.../target.s --provider src/game/mic.c -o build/.../call-context.json
```

Feed **actual compiler-preprocessed context** to this optional read-only audit.
It lists uncovered direct calls, actual header/provider declaration locations
and hashes, and all header alternatives. Include suggestions are not automatic
ABI approval. Macro-generated and old non-prototype declarations remain explicit;
unsupported cases do not become guessed signatures. A narrow additional cue
flags a context-declared `void` local helper whose standalone target explicitly
copies a saved-register result to r3 in its final call-free return tail. Review
the return and inline callers; the cue does not infer a C return type.

Live use identified **21 missing external API declarations** in m670's initial
context. `game/mg/actman.h` provides the real actor/player construction types.
New `include/game/mic.h` supplies eight declarations and the response callback
typedef from the current `src/game/mic.c` definitions. Corrected context covers
80 external calls instead of 59. The remaining 18 are fourteen compiler save/
restore helpers and four legacy empty-parameter declarations, explicitly not
new unknown gameplay prototypes. The local Qwen packet generator now takes
call declarations from the preprocessed context, including transitive headers,
instead of scanning only its directly listed header files. No model setting or
output limit was changed, and completed packet identities were not rewritten.

Independent provider check: compiling the current mic provider with its local
callback typedef replaced by the public header is **object-byte-identical** to
the unchanged provider: `3a3f9d870e633258c65993f7ac039edb82bcbef149b7a1d13142cc582a3bdfe6`.
The live provider itself was not modified. Proof:
`build/minigame-recovery-20260913/m670/mic-header-check/result.json`.

Applied source results, not merely audit output:

- m670 CPU initialization `fn_1_3B30`: **172/172 bytes, zero objdiff rows**
  using the actual MGPLAYER pointer and shared record layout.
- Normalization `fn_1_1460`: the erroneous `void` context was flagged; target
  has a common Boolean result. Reconstructing it and the target unsigned random
  conversions brings the helper to **344/344 bytes** with matching instruction
  operands; 27 reported constant-pool attribution rows remain in this isolated
  TU. It is not declared fully matching.
- Main construction `fn_1_1658`: preserving the live light-creation result and
  restoring the right-associated collision-model publication chain raises
  **95.254340 -> 98.641440%**, reducing **390 -> 196** reported rows. Size is
  **3220 versus 3224** target bytes. The source evidence, not a register-number
  permutation, selected these creation/consumer changes. Header discovery alone
  did not choose the C repair.

Current working source/object:
`b9c2e0bb3917c83ad13d4cc53fad2b8e394194e8b0ae566aea0dd18c2cbf4c6a` /
`5b5102267d55931ca0f946657476e3ec1380f73b3c64454d753c55b50c45d393`.
Compact proof and working reconstruction:
`build/minigame-recovery-20260913/m670/constructor-context-result.json` and
`constructor-check.c`. Preserve them as the active reconstruction, not a new
history tree. Shared work-layout unknown slots and original TU/data ownership
still require integration proof; **no m670 module closure or main progress is
claimed**. Twenty focused context tests run successfully (one explicit-local replay skipped).

## m670 motion/collision owner batch

The next selected batch is `m670dll/prolog.c`, `actor.c`, and the existing
compiler-runtime implementation. This is **three source owners, not three
minigames**. The five actor functions recover 1,844 gameplay bytes. Unfinished
initialization, sequence, pillar, and CPU code remain original-object inputs;
their useful local reconstructions are retained separately.

The actor owner is bounded by target evidence: code 0x22F0..0x2A24, constants
0x130..0x148, strings 0x268..0x288. The next source region begins with fresh
eight-byte data alignment at the shout string and restarts its literal pool.
No helper was split out simply to create another closure count. Target data
alignment supplies the seven trailing string bytes and four constant bytes;
the C source contains no synthetic padding or added data producers.

Primary reconstruction closed the five functions together from typed m2c and
the actual target. Important coupled fixes were the full-width player-index
interface and its automatic inline consumers, the explicit one-case state
switch and live player snapshot, and the face-normal/position aggregate homes.
The target really passes `modelId < 0` to `Hu3DMotionShiftIDGet`; changing it to
a comparison of the call result would alter retail behavior. The target also
contains the otherwise-unused player-number copy and object snapshot retained
in the source. Those are disclosed target-emitted operations, not new padding
or fabricated register owners. Unobserved work fields remain honestly unknown;
the header does not allocate or seed them.

Verification: a detached checkout from main 30dd8a6 built the three source
objects into the actual m670 PLF, with all other regions still original.
The linked strict objdiff has zero differing symbols, and **all 137 retail
checksums pass**. The complete m670 REL is byte-identical, SHA-256
`f2cbf290f7bed55ee1531b7a866fe9f7f7a4adb4e10cd768a413a871afc6346f`.
Raw split-object reports still expose anonymous-pool names, coalesced weak
sqrt constants, and section-tail alignment; these are not presented as raw
object-file equality. The source-selected linked proof resolves every physical
byte and relocation. Evidence is under
`build/minigame-recovery-20260913/m670/project-proof/receipt.json` and
`linked-strict.json`.

Four bounded Qwen fact packets ran in parallel without holding the primary's
compile path. All returned a model-server error after roughly sixteen minutes,
with no final answer; no generated claim from that batch was admitted. The
primary completed this actor group independently. Do not repeatedly submit the
same failed batch or count waiting/analysis as a recovery gain.

## m670 pillar and CPU batch

The next two complete source regions are `pillar.c` (five functions, 4,364 code
bytes) and `cpu.c` (two functions, 2,144 bytes). All seven functions were
reconstructed from typed m2c plus target dataflow, not historical donor bodies.
The actual source-selected PLF has zero differing symbols, the m670 REL is
retail-identical, and all 137 project checksums pass. The prior actor, startup,
and runtime selection remains exact. Total selected m670 code becomes 11,140 /
19,924 bytes; this is still not a full minigame closure.

The useful source decisions were structural: recover four-element candidate
position/index arrays, reuse the insertion index for the nearest-choice pass,
keep difficulty branches with their separate live conditional results, and
preserve the old/new timer snapshot before testing a negative-to-nonnegative
transition. m2c incorrectly rendered that last comparison as an impossible
test of one current value. In the pillar rise phase, snapshotting height before
its compound increment reproduced the target live scalar producer. Correcting
the work-state signedness removed seven incorrect unsigned CPU comparisons.

Target-emitted redundancy is disclosed, not silently normalized: the CPU
difficulty timer is overwritten after the switch (the random calls still
occur); pillar state four computes a completion flag but advances without using
it; state five retains an unused floating frame snapshot. These operations are
observed directly in the target, not fabricated storage. Four-element CPU
arrays are supported by actual frame homes, four-way selection, and the five
pattern tables containing each of six values exactly four times.

One small Qwen factual check completed while the primary independently built
the CPU controller. It confirmed array addressing and register reuse but could
not prove the unnamed zero constant without data. The primary had the data and
full source context. The second question was cancelled when the complete
pillar source-selected link made that narrower investigation obsolete.

Proof: `build/minigame-recovery-20260913/m670/gameplay-proof/receipt.json` and
`linked-strict.json`. These are final linked comparisons, not claims of raw
object-file equality. Unfinished constructor work separately gained exact
3,224-byte size after the microphone handle became unsigned (target clrlwi
at creation and lhz at consumption); its remaining register cycle is not
part of this selected batch.

Reporting correction: the prior public 540-to-548 REL object denominator was
not a valid census. Only m670 changed in the actual DTK configs, from four to
ten units (+6). Use fixed code/data byte denominators and the registry's 82
numbered minigame modules; m616 is fully source-selected, leaving 81. Splitting
a fallback span creates build objects, not newly discovered minigames.

## m670 initialization and sequence batch

Sixteen additional application functions (8,784 bytes) now pass the actual
source-selected retail link: three in initialization and thirteen sequence,
microphone-response, panning, and result functions. Four new source owners are
selected: init, pattern, sequence, and seqparam. m670 now selects all 19,924
code bytes from source, with all 137 project checksums passing. This is not
yet a full module-source closure: the original 32-byte data record at .data
0x28 and the shared BSS region from 0x10 remain fallback inputs.

Constructor closure came from the live chained creator assignment
`players[i] = player = MgPlayerCreate(...)`. It preserved the returned player
and global consumer together and resolved the j/player saved-register cycle.
Declaration/scope changes alone had been neutral. The 160-byte positional
sound helper required conversion of screen.x to int before division by five.
Five pattern arrays and their pointer table occupy their own normally aligned
data owner; that TU boundary supplies the target five-byte gap after the
context filename without manual string padding or a dead producer.

The result function needed source lifetime reconstruction, not individual
register substitutions: survivorCount and shadowPlayer belong to the outer
function scope, while count belongs to the winner-collection branch. The
three live name arrays precede the result position aggregate. Those changes
closed both register cycles and the array/vector home exchange. Three static
sequence counters are emitted in reverse declaration order; preserving their
target order closed twelve actual REL relocation bytes after code already
looked exact. Static linkage is supported by their exclusively local users.

Source-shape debt is explicit: fn_1_5AC contains the target's unused integer
initialized to one. Its store is visible in retail, not newly introduced
padding. The raw microphone callback argument uses the existing u16-pointer
API and a typed response view authenticated by the same-game provider's
status/confidence/count/value-pointer fields. No inline assembly is added.

Qwen independently reviewed bounded sequence and storage facts while the
primary reconstructed and compiled. It did not choose the source campaign.
The four unobserved bytes after the 24 used pillar-delay entries do not prove
a 25-element array; the 32-byte data record's sole pointer store does not
prove the remaining field types. Do not invent these merely to claim a
whole-minigame closure. All code gains are retained independently.

Proof: `build/minigame-recovery-20260913/m670/sequence-proof/`, particularly
`receipt.json`, `linked-strict.json`, and `batch-delivery-proof.json`.
The final REL SHA-256 remains
`f2cbf290f7bed55ee1531b7a866fe9f7f7a4adb4e10cd768a413a871afc6346f`.

## m670 full source-selected storage closure

The final link now has no original-object fallback. All 19,924 code bytes,
all initialized data, and the complete 1,156-byte BSS section come from the
nine selected source owners. The 28 application functions, their normalized
physical relocation streams, all four main sections, and the final retail
REL are exact; all 137 project checksums pass. The additional storage recovered
in this step is 32 initialized bytes and 1,140 BSS bytes.

The constructor owns the existing M670WORK allocation. The pillar controller
owns the two existing arrays and height/speed scalars. A real GC1.3.2 storage
compile established natural four-byte alignment, reverse BSS definition order,
and the absence of a compiler-created extra word after a 24-int array. The
known 936-byte work object and two 96-byte arrays were first linked separately;
this recovered 1,128 bytes without changing the unexplained region. That
intermediate also passed all retail checksums.

The final declarations are deliberately an inferred storage reconstruction,
not proof of unique original types. The contiguous timer allocation is
represented as 25 ints, of which only 24 have observed users. The initialized
32-byte record is represented by eight null character pointers; only index 1
has an observed pointer store. These are the existing live allocations, not
new scalar padding, an opaque byte tail, or new runtime operations. The unused
slot types and original declared capacities remain source-shape debt: the
binary cannot distinguish every compatible original declaration. The earlier
warning against claiming those types as recovered fact still applies. No
semantic names or sentinel role are invented for the unused storage.

A bounded relocation census of every same-game microphone-listener module
found no further user or matching record that establishes a more specific
type. It is therefore not used as donor proof. All original-data fallback
inputs are gone, but this binary/source-selection result must not be presented
as recovery of original unused-field semantics.

Proof: `build/minigame-recovery-20260913/m670/storage-proof/`, including
`linked-strict.json` and `batch-delivery-proof.json`. The actual source-selected
link rule is included in the latter. Main promotion independently rebuilds
the exact source and split blobs from clean main before publishing progress.

## m651 full source-selected closure

All 38 application functions now use the reconstructed source link, including
the fourteen entry/sequence functions previously left behind an original-object
fallback. The entry instructions were already exact. The remaining discrepancy
was the extent of the live zero-initialized counter allocation: its target span
is twelve bytes before the next owner's storage, while a scalar short left an
eight-byte downstream shift after ordinary linker alignment.

The counter is represented as six shorts, with only element zero consumed by
the observed increment and comparison. This preserves the existing allocation
and its actual s16 access type without new runtime operations. The unused
capacity is an inferred storage interpretation, not evidence of six original
counter semantics or a uniquely recovered declaration. The unused initialized
entry-table tail also retains its separately documented grouping uncertainty.
Neither a byte-identical link nor an address-derived label establishes original
names or unused-field semantics.

The complete source-selected standalone REL is byte-identical, SHA-256
`a873512645afe699544ff7a07a498939747585f5af94dd001b58395356f3fd48`.
The final proof additionally checks all 38 application functions, normalized
physical relocation streams, all sections, no original-object linker inputs,
and the project's 137 retail checksums from a current-main-based checkout.
The newly selected entry contributes 1,024 code bytes and 128 data/BSS bytes;
the whole module selects all 19,228 code bytes. Compact proof is under
`build/minigame-recovery-20260913/m651/complete-proof/`. Source/type uncertainty
is separate from the verified binary and source-selection results.
