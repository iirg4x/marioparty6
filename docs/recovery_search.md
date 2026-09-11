# Bounded source reconstruction search

Use `tools/recovery_search_batch.py` for a small reviewed family of natural-C
solutions. It compiles isolated candidates, evaluates the whole owner, retains
compact next-mismatch evidence, and ranks only candidates admitted by the
existing strict/data, sibling and physical-relocation checks. It does not edit
live source, sign permits, promote source, or claim that a prediction is proof.

## Generated search and automatic private retention

`recovery_guided_search.py` connects source generation, cached measurement,
whole-owner evaluation, and independent retention. A reviewed
`recovery_guided_search/v1` plan binds the current source/index, actual residual
row numbers, supporting evidence, and bounded source constraints. It does not
require hand-written candidate files or a new approval for each generated cell.

Installed families are `snapshot_point_use`, `loop_predicate_guards`,
`sequence_result_consumer`, `indexed_base_snapshot`, `sequence_scalar_argument`,
`shared_initializer_producer`, and `canonical_void_contracts`. The source-shape
families use parsed, exact spans; the contract family uses the
actual-call/canonical-interface generator described below. The existing two-stage
contract/sequence composition is also supported.
Unknown families, stale inputs and unbound target rows are rejected. These are
narrow supported transformations, not a claim to solve arbitrary allocator
mismatches. Missing source constraints remain an explicit missing capability.

Known constraint: explicit `(void)Call(...)` is not an approved GC2.6
return-allocation repair. The pinned native path obtains the callee's formal
return type and allocates a basic nonvoid result of width <=4 at `0x5288FD`
without consulting a parent-discard flag. The speculative discard family was
removed; it must not consume candidate compilations for this cause.

Use `--plan`, `--out-dir`, the compiler/evaluator arguments below,
`--max-candidates` (1–8), and `--timeout`. Requests run in earliest-mismatch
order; duplicate sources are skipped, infrastructure errors stop execution,
and the first admitted gain ends the search. `--retain` additionally requires
the plan's `allow_retention: true`. `recovery_retain.py` then independently
recompiles and proves the gain before updating private source and the current
index. A pending journal preserves a proved gain across interrupted publication;
`--resume-pending` finishes that publication without another compile. This does
not commit, promote to main, or claim linked exactness.

## Inputs and execution

Supply a current `recovery_frontier.py snapshot` index with source/object compile
receipt, and a `recovery_search_batch/v1` manifest:

- `root_reviewed: true`, one `causal_family`, current index file SHA-256 and live
  source SHA-256;
- one to eight candidates, each with `id`, source path/hash, focus `functions`,
  and hash-bound `evidence` paths;
- only evidence-supported source alternatives, no deliberate failure controls,
  fake owners, arbitrary declaration matrices or added matching assembly.

Invoke the script with `--root`, `--index`, `--scratch`, `--source-relpath`,
`--object-relpath`, `--compiler-script`, `--manifest`, `--out`, `--objdiff`, and
`--readelf`. `--workers 2` parallelizes proof, not source ownership or compiler
use. Compilation uses the existing private compiler lock. Context drift stops
the batch. A compiler failure does not consume a function forever.

## Decision feedback

`recovery_causal_groups.py` groups observed register relationships, moves,
instruction/CFG changes, and relocation differences. It distinguishes the first
machine mismatch from the unknown source cause; its groups are not independent
source defects. These summaries are also kept in evaluator results after raw
reports are cleaned. The standalone CLI takes `--strict`, `--function`, and
optionally `--before`.

Group closure is based on the complete, bound target-site census, not a bucket
ID disappearing. A register relation changing while its instructions still
differ is reported as reclassification. The comparison separately counts
resolved and newly mismatching target sites; incomplete or different target
bindings yield unknown closure. On the retained Kettou 314-to-291-row repair,
this identifies two genuinely closed groups covering 23 sites and three
reclassified buckets, rather than incorrectly claiming five closed groups.

`recovery_extsh_decision.py` captures pinned MWCC conversion-selector inputs and
branch outcomes using the existing native debugger lifecycle. Run `--help` for
current index/request/capture-adapter arguments. It checks compiler hook bytes,
restores debug registers, and requires the emitted object to equal the current
baseline. Missing source joins remain explicit. A trace is not a source patch or
proof of retail equality.

### Native expression-to-instruction join

Pinned GC2.6 capture has opt-in `--expression-origins` on the ordinary
`capsule_same_session_capture.py` prepare/preflight/capture/validate commands.
It tracks 34 verified expression handlers and 87 return sites, checking each
runtime frame before associating an active expression with a created PCode
node. No active frame means a missing edge, never an inferred owner. Defaults
are unchanged; use the existing `--partial-evidence-dir` to preserve useful
diagnostic evidence when an unrelated final ownership join remains incomplete.

`recovery_expression_join.py --envelope ... --object ... --source ...
--function ... --original-id ...` performs the source-expression/PCode/alias/
color/machine-word join centrally. Default output is bounded to 32 rewrites and
32 unions; `--details` is explicit. It verifies every captured function word
against the supplied object and rejects mixed-session evidence.

The live unchanged-source Koopa test reproduced object `84a892dc...39c40` and
all 667 words. It joined the initial `mulli` at instruction 7 to the active
expression at source coordinate 5133, its V32-to-3 rewrite and GPR3 color; the
final union joins a distinct call expression at coordinate 5373. There were
414 active-handler origins and 176 explicitly missing contexts. This closes a
previously missing candidate-side causal link; it does not recover target
source identities or constitute another function crack. V4's tracker identity
was independently checked before/after; future profiles additionally seal the
tracker hash through the capture implementation.

`recovery_alias_cause.py --root ... --index ... --envelope ... --function ...`
starts from the first current register-only mismatch, instead of requiring an
operator to find a virtual-register ID. Its supported destination forms join
the candidate instruction to the actual PCode rewrite, alias union, expression,
and recorded temporary lifecycle. Structural differences and unsupported operand
roles return a specific missing edge. This command does not generate a C patch.

The newer V5 unchanged-source run reproduced the same object and all 667 words.
It observed the GPR counter reset **259 to 32** after the final team-mode
`mbPlayerCapsuleRemove` expression, then a native return allocation of V32 by
the actual `mbPlayerCapsuleMaxGet` expression and its later V32-to-3 union.
The first address calculation had already used V32. The reader now derives
this whole chain automatically. A reduction of at least three pre-reset slots
is a conditional counter-budget alternative, **not** a proved safe source edit
or match prediction; changing alias eligibility is a separate possibility.

Compiler source coordinates are enclosing CodeGen lines, not child-expression
columns: coordinate 5373 is a closing brace. The source resolver identifies
the enclosing loop and uses the captured callee name to locate the unique
capacity-query candidate at line 5363. It never treats the brace as a callsite
or native multiplication as proof of a literal C `*` (array scaling can produce
it). Default preflight output is compact; `--full-output` retains the full hook
list, which is always present in the immutable request.

Donkey's current selector/predecessor run also reproduced the baseline object.
`recovery_extsh_decision.py --constraints-capture ... --capture-sha256 ...
--sequence ...` reads that trace without another compile. Event 7 has an
unsigned-extraction predecessor (`0x67`) into the same V54; the sole reuse
blocker is that signed-short reuse accepts `0x65`/`0x64`, not `0x67`.
The source-kind-derived EBX flag and predecessor are now captured in one event,
not inferred by combining different sessions. This narrows the missing
frontend representation constraint, but does not license an ABI change or
claim a new crack.

## Memory and continuation

`recovery_search_memory.py` suppresses exact source/context repeats before
compilation and object repeats before repeated proof. It checks immutable
result and compile-receipt bindings. It has no function-lifetime tombstones.
Entries are bounded to 128 and 512 KiB; changing compiler, headers, proof tools,
implementation, focus or baseline invalidates reuse.

One narrower exception avoids rediscovery after tooling-only changes: a verified
same-source/same-compiler-context receipt whose raw object equals the current
baseline object can suppress a known neutral compile. This reuses object
identity only, never old scores, proof admission or retention permission.

Actual evaluator causal feedback is retained in the compact memory entry:
closed/new/reclassified groups, unresolved rows and structural hazards. The
compact form is limited to 4 KiB, without copying full reports. A cached clean
gain remains eligible for independent retention; cache suppression must not
silently make that gain unreachable.

After the owner reviews and retains a genuine gain, refresh the current index
and advance the memory frontier. Subsequent candidates are based on that new
source; never restart from the initial baseline or reuse its old proof as proof
against the retained gain. The live source remains owned by the lane/root.
Automatic retention advances this cache frontier too; a cache-maintenance
failure is a warning and cannot roll back the proved source gain.

## Source-producing contract repair

`recovery_call_contract_repair.py` composes one function-entry declaration
repair from every native actual-call return observation and an ordered set of
owner-reviewed canonical void signatures. Each signature is bound to an exact
existing header/definition span and hash. It never infers `void` from an unused
return value, substitutes an enclosing expression for the actual call, or
creates a declaration search matrix. Run `--help` for the frozen-source,
capture, contract, current-index and output arguments.

The output is candidate C, selection evidence, and a guarded batch manifest—not
just a diagnosis. The ordinary batch evaluator still decides measured gain and
the owner retains it. Historical-source replay is explicitly non-runnable
against a changed live frontier.

Check declaration visibility at **every callsite**, including calls before a
later block-local declaration. Do not globally blacklist a genuine contract
correction because it failed on an older source body: changed, evidence-backed
owner reconstruction can make the composition materially different. Exact
source/context duplicates remain suppressed.

## Earlier retained CapSpecial checkpoint

Kettou's complete canonical declaration repair, composed with the retained
typed guide/counter owners, reduced **291 strict/data rows to zero** at
6,532/6,532 bytes. One candidate compile and an independent proof compile
reproduced object `84a892dc609dcc65fd453bc5516a1aed1ca48c0b965111c79aa22727fad39c40`.
CapSpecial advanced from 40/44 to **41/44 instruction-exact**, with all other
43 function bodies unchanged. The contract-repair generator reproduces the
retained candidate source `667628184083184faf2387f646279eb65c2ff87782629191438dbabb5a03290a`.

The 395 symbolic relocation records agree. Fourteen absolute relocation
differences remain, all calls to `ev_CapKettouMesGet` at the same inherited
+12-byte TU displacement. This is a retained instruction closure, not final
owner/source-linked binary exactness. The earlier Teresa experiments and
Donkey selector capture produced no retained gain and are not counted as
cracks.

## CapSpecial closure gaps closed, 2026-09-10

CapSpecial subsequently reached44/44 strict/data/physical exact functions and
137/137 source-selected retail DOL/REL outputs, and landed Matching on main at
`bf77696ce47c4914e39fe726972c837065cdedde`. The improvements below extend the
existing readers/generator/evaluator rather than introduce another search engine.

### Shared producers, including locally matching code

`recovery_source_shapes.analyze_shared_initializers(source_bytes, function, ...)`
now reports assignment-chain producers, ordered recipients, real declarations,
types, exact statement spans, and missing safety facts. A locally matching
instruction region is not excluded: different frontend identities can emit
the same immediate stores yet constrain later allocation differently.

After reviewing macro/type context, select one target-supported existing
producer with `target_producer`: source SHA, statement start/end/SHA, producer,
rationale, and `observed` artifact SHA/location/fact. This is a reviewed source
hypothesis, not recovered target source identity. Evidence descriptors in the
guided plan still bind the actual input artifacts. A supplied SHA-shaped string
alone is not a native event or proof of original source.

Pass that census as `constraints.shared_initializer_analysis` and select family
`shared_initializer_producer`. The generator repeats its safety checks and emits
at most one candidate. It preserves all existing recipients and their relative
order, moving only the reviewed producer to the innermost assignment. No
declaration permutations, new locals, or storage changes are generated. This
bounded version supports zero-valued chains of same-type automatic integral
scalars/fixed array elements. Unknown macro/type context, volatile/const storage,
shadowing, address escape, indirect or side-effecting lvalues, and ambiguous
bounds are rejected. Unsupported shapes remain ordinary owner reconstruction
work, not a function tombstone.

KoopaCoin's pre-win working source was
`4f6607ce1d17d2d9b9fb547cd8a9556952c77745f53c124c6f66444aa7cb25ac`.
The generator now emits the verified winning TU
`dc959f9357dcf9c776c33c8c1ce93f6500022d57776fb2e7b7436024df3b5fdf`:

```c
/* before */ loseCount = teamLose[0] = teamLose[1] = FALSE;
/* after  */ teamLose[0] = teamLose[1] = loseCount = 0;
```

The reviewed facts are `BOOL` equivalent to `int`, `FALSE` equal to zero, and
retail0x801BF780..788 setting r27 to zero then storing it at0x2c/0x28. Both
initialization regions were locally instruction-exact before this change.
The regression fixture `tools/tests/fixtures/capspecial_koopacoin_prewin.c`
contains only the actual pre-win function (plus terminal LF): SHA
`8d706e8315dde58f761e3592b40fb10ebe904be2b053bac598cdd38813444837`.
The test ranks the losing-count site ahead of the unrelated team-count chain
and emits the independently verified winning function SHA
`c9485726f777fb5192aa0a5fac852c2ce1d3629a9f39bb84070d4cbe2c7545f0`.
Renamed and non-Koopa cases guard against identifier-specific behavior.

### Traces return source questions, not just virtual-register IDs

`recovery_expression_join`, `recovery_alias_cause`, and `recovery_temp_epochs`
now include bounded `source_regions`: observed line/callee facts, inferred AST
enclosures, shared-initializer review locations, and explicit unknown edges.
They distinguish a closing-brace CodeGen coordinate from a callsite and refuse
ambiguous same-line statements. They neither invent target virtual IDs nor
claim that an inferred region caused the observed alias.

Replay of the existing Koopa lifecycle envelope
`6f91c21d45dca16d94aa9996f3ab6aaf16659f948e01e388e013e5e90b9dc822`
binds all667 machine words to object `84a892dc...39c40` and frozen source
`667628184083184faf2387f646279eb65c2ff87782629191438dbabb5a03290a`.
The reader returns the entry declaration, the capacity loop containing the
closing coordinate, and both real shared-initializer sites, including the
later winning site. The trace-to-initializer causal edge and post-win counter
sequence remain unproved; no new capture is necessary to use these observations.

### A working reconstruction is not the protected champion

Guided plans and batch manifests accept an optional `working_source` descriptor:
`path`, `sha256`, `champion_source_sha256`, `baseline_index_sha256`, a bounded
`lineage` explanation, and1..8 hash-bound `evidence` descriptors. The existing
top-level source/index fields continue to bind the protected champion.
Generation constraints bind the working bytes; evaluation, sibling protection,
and retention compare against the champion. Both source deltas survive selection,
evaluation, retention, and interrupted-publication recovery. A rejected working
reconstruction never becomes a gain merely because the next edit is small.
Displayed diffs are compact; full delta counts and hashes are preserved.

Koopa's champion was `08aae32a...c973a20`, not its canonical working source.
Calling the winning edit "one line" relative to that champion would have hidden
the already reconstructed accessors and storage. Tests cover distinct working
and champion sources, composed gain, no gain, stale input rejection, cached
measurement reuse and retained-journal lineage. Contract generation from a
different working source is explicitly unsupported until its own capture/object
context is supplied; it never silently borrows the champion's trace.

### Hypothesis memory does not ban functions

Batch candidates may include `hypothesis_binding` with `family`, `scope`, the
generation `source_sha256`, `participants`, `boundary`, `causal_evidence_ids`,
and optional `producer`/`dependencies`. `SearchMemory.lookup_hypothesis` returns
related measured hypotheses as warnings, not equivalence claims. Only the
existing validated source/compile-context or object measurements justify actual
duplicate suppression. Changed reconstruction, producer, coupled dependencies,
or nested-versus-flat loop boundaries remain eligible. Lower-scoring structural
findings remain compact diagnostic evidence, never a retained gain. The existing
128-entry/512-KiB limits remain unchanged.

### Donkey: diagnose parameter conversion before allocation

The existing call-contract tool has a read-only mode:

```text
rtk proxy python tools/recovery_call_contract_repair.py declarations --root ROOT --request REQUEST.json
```

The request has `caller` and `provider` signature descriptors (path, file SHA,
start_byte, end_byte), plus optional reviewed scalar typedef mappings. It
distinguishes `void f()` from `void f(void)` and typed parameters, reports the
GC2.6/2.7 int32/short16 default-promotion conversion domain, and marks unsupported
type syntax UNKNOWN. Include/scope visibility is caller-selected evidence, not
automatically proved. No native formal edge or source patch is synthesized.

Actual Donkey replay of the approved caller source `481b169a...a2937` and
Charman `a423478e...9cee9e9` identifies `s16` to `int` default promotion and
**INCOMPATIBLE** caller/provider types for the non-prototype caller. It requires
a source exception, not register shaping. The ordinary canonical-void repair
generator now refuses non-prototype insertion. Donkey's explicit user-approved
exception is local to that recovery; it is not blanket ABI permission.

These are known-case ranking/diagnostic regressions plus transfer-shape tests.
They do not establish an automatic inverse compiler, original Hudson source,
or a measured crack/hour improvement. The next unresolved owner must demonstrate
that the new ranking produces a useful candidate or retained gain in practice.

Closeout validation: the full tools suite passed2,264 tests (18 skipped).
After independent review corrections, the final affected suite passed221 tests.
Those corrections reject multi-operand destination slices, non-captured alias
unions, duplicate event identities, invalid variadic declarations, and failed
compiles masquerading as reusable measurements. Verified semantic-object aliases
remain reusable. The real Koopa667-word replay and Donkey declaration CLI also
pass; no new owner compile/capture or math-source change was made for this work.

## Dice/Math closeout: retain useful measurements (2026-09-11)

Use this central tooling checkout with `--root` pointing at the active owner;
do not copy an older owner's `tools/` directory over it. KoopaCoin's shared
producer analysis, dual working/champion lineage, and hypothesis-aware memory
above already exist. They are not new work or a prerequisite to every compile.

### A row can improve before it becomes exact

`recovery_evaluate.py` now compares the bound strict/data instruction streams
at stable target addresses. Its compact `code_quality` findings identify
corrected opcodes and operand positions, as well as lost constraints. An
unchanged row count no longer hides an evidenced improvement. Unchanged
unresolved insert/delete regions are allowed to remain unresolved; changed or
ambiguous regions cannot certify this additional monotonic-gain path.

The historical Math `ObjectCullHook` input-read correction changed three
`lfs` field operands while keeping 111 strict/data rows and 728 bytes. Its score
rose from 93.33517 to 93.35165. Replaying the saved, hash-checked objects and
reports now returns `improved` instead of `no_gain`, with three closed operand
positions in each report view and no losses. These are the same three code
changes, not six independent gains. Index identities:

- Before: `2a55dc5e9903794ae4c8a95c686c118d13fcd4d6c64d1770e1d3f38a28d3b50f`.
- After: `d97b787d0c98cf50a92a050c0e880de2658063b9838bf69ca578118a61f01dd7`.

A percentage alone is still insufficient. Exact siblings, exact size,
relocations, compiler/source binding, typed-data review, and source fidelity
remain protected. Structural changes that trip those gates can expose a useful
quality finding without replacing the champion. This is a known-gain replay,
not a new crack, new source inference, or measured throughput improvement.

### Use the already-developed optimized compiler observer centrally

Dice's verified GC/2.6 `mwcc_win32_varinfo.py` additions are now in this central
tool set: optimized GPR/FPR selection, frontend temporary/range-split origins,
and LI/LIS common-subexpression eligibility/reuse. They supplement, not replace,
the existing O0 local-assignment observer. Example option sets:

```text
--regalloc --regalloc-class fpr
--regalloc --regalloc-class gpr
--regalloc --regalloc-class gpr --frontend --cse
```

Invoke `python tools/mwcc_win32_varinfo.py --help` for the complete interface.
Always supply the active `--compiler`, `--cwd`, `--target`, `--output`, and the
complete compiler arguments after `--`, including the source/output and actual
optimization flags. The legacy default command is a Telop example, not the
current owner's recipe. `--assign` and `--regalloc` select different stages;
frontend requires regalloc, and CSE requires its GPR class. The observer is
Windows/WOW64 and pinned-GC/2.6 only; it does not establish GC/2.7 support.

Compiler fingerprints, optional hook byte seals, register-class identities,
range-split caller checks, bounded observations, and explicit incomplete joins
remain enforced. Output describes the supplied compilation, never retail
virtual IDs or the source edit to choose. The integration uses the existing
Dice dependency chain through `206f470`; no new native capture was needed.

Dice's final Zorome repair itself needed no new source generator: distinguish
the real read-only pitch input, magnitude output, and later color/alpha owner.
An authorized native operation's input/output contract is not an in-place
clobber. Keep that reconstruction question visible rather than expanding a
declaration-order matrix or automatically extending its native authorization.

### Clean promotion hooks must not require tools inside public branches

Install the managed hooks from the central tooling checkout. The wrapper pins
that tooling root and the shared Git directory; it does not try to execute a
nonexistent `tools/agent.py` in a clean `recovery/*` or `project/*` worktree.
Verified public promotions use the existing manifest, queue/source proof,
allowlisted blob and branch-boundary checks. Source exceptions are checked
against the verified source commit's policy, not whichever tool branch happens
to invoke Git. Supporting progress is checked against the prospective staged
tree or pushed commit, so generating its required sidecars is not blocked by
their absence in the preceding commit.

An ordinary public promotion does not rerun the unrelated AI tooling test
suite. AI workspace pushes retain their startup, metadata, benchmark, and test
checks; public main retains its progress and clean-boundary checks. Unknown or
mixed AI/public pushes are rejected. This does not waive object, consumer,
source-selected link, or retail proof, and is not a branch-name-only bypass.

Read-only replay against the real completed Dice branches passed:
`recovery/board-dice` at `d1cc86575876968888549cc82bb995a2f2b9094a`
and `project/board-dice-matching` at
`d5ba03e2a017c8f362b5cf064037a369eea139fd`.
The two existing-proof audits took about nine seconds locally, without a new
compile or push. The old hook had rerun the full unrelated suite for 525 and
463 seconds on those promotions. This measures avoided verification work,
not faster source discovery.

The installed wrapper also passed from the actual clean Dice worktree. An
abandoned registered worktree with a broken old MSYS `.git` path is ignored as
an untrusted manifest source; it cannot veto an unrelated valid promotion.
Missing selected proof still rejects the promotion. Pre-commit preserves Git's
alternate staging index, while pre-push clears inherited local Git variables.

Integration validation: 2,310 non-hook regression tests passed (18 skipped),
with hook tests run separately. After the final observer edge corrections,
all 194 affected observer/capture tests passed (one skipped). The saved Math
gain, existing Koopa/Donkey regressions, real Dice promotion branches, and
installed-wrapper smoke check passed without a new owner compile or capture.
