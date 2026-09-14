# REL first-pass translator fixes

These are changes to the existing m2c translator, not an alternate decompiler or
an exactness gate. The patch is against the installed working version at upstream
`4b2bb9110321a97df458566d8a9d55290aba443a`, including its earlier local fixes.
`manifest.json` checks those actual inputs; a pristine upstream checkout alone is
not this baseline. The upstream project is <https://github.com/matt-kempster/m2c>.

## Fixed causes

- Preserve context-provided plain `char`, distinct from `signed char` under
  MWCC's unsigned-char mode. Do not change all byte variables to `char`.
- Express actual byte offsets through void/incomplete object pointers without
  inventing a record layout. Cover indexed stores and typed offset-zero loads.
- Declare the real owning stack aggregate for nested element accesses and retain
  live read-only stack declarations. Do not invent initializers or array bounds.
- Preserve each PowerPC comparison operand's float/double precision. Do not turn
  numerical promotion into a bit reinterpretation; MIPS comparison inference is
  unchanged.
- Allow an empty, inference-created stride record to adopt a compatible known
  consumer record. Never overwrite a context-defined identity or observed fields.
- The batch preparer now includes the existing `fdlibm.h` provider before actual
  MWCC preprocessing. This removes phantom arguments caused by absent math
  prototypes. The freezer also creates empty rewritten include roots.

## Measured result

The fixed installed executable was tested against the same frozen headers,
assembly, target objects and compiler flags as the old executable. The sample
contains 48 functions across 30 modules, selected deterministically before their
new results. It is targeted by diagnostic category, not representative of the
whole game's recovery percentage.

| Sample category | Before compiling | After compiling |
| --- | ---: | ---: |
| Character API failures | 0/8 | 7/8 |
| Pointer-expression failures | 0/8 | 7/8 |
| Stack declaration failures | 0/8 | 2/8 |
| Unknown aggregate failures | 0/8 | 0/8 |
| Precision cases | 8/8 | 8/8 |
| Existing exact controls | 8/8 | 8/8 |
| Total | **16/48** | **32/48** |

All eight precision scores increased, with no new code-size regression. Examples:
`m613Dll:fn_1_10DB4` 91.323530 -> 96.911766%;
`m675Dll:fn_1_2064` 93.070175 -> 95.877190%;
`m627dll:fn_1_3590` 98.169014 -> 98.556335%.
All eight exact controls retained identical object hashes. No new raw-100
function or owner closure is claimed from this sample.

Additional bounded acceptance cases: the known-consumer aggregate fix made
`m635dll:fn_1_3340` compile at 73.413040%; the actual math-provider context made
`safdll:fn_1_33FC` compile at 74.724140%. Both remain reconstruction candidates,
not recovered source. These two tests are separate from the 48-case sample.

The complete translator suite passes **478 tests**, including 65 unit tests.
Fifteen reviewed output goldens changed: fourteen for plain-char identity and
one for corrected floating comparisons. Preparation/freezing have six additional
focused tests. `validation.json` retains compact paired hashes and metrics.

One development replay had five unexplained assembly-parser failures, including
an exact control. The unchanged final installed replay completed all 48 m2c
invocations successfully; cache/no-cache and three hash-seed checks also passed.
No parser fix is claimed. Capture `--stacktrace` on recurrence; do not classify a
translator crash as a source mismatch or silently discard it.

The subsequent 167-case affected-function replay completed with **23 -> 125
compiling**: 35 character, 36 pointer and 31 stack-declaration failures became
compilable. Of 23 already-compiling precision cases, 21 scores improved, one
decreased and one was unchanged. Four drafts reached raw 100%, but all still
contain unresolved-field/unknown-type macros and receive **no closure credit**.
This is the `build/rel-gap-fixes/affected-batch-v1` replay, not the 48-case
holdout or the later call-interface sample below.

## Install and use

The current verified installation is
`C:/Users/Anony/.codex/tools/m2c-rel-20260914-eabi8/m2c.py`.
Use this path for new REL preparation and reconstruction. The original installed
m2c and the already-running all-REL baseline were deliberately left unchanged.

The portable package includes the terminal-switch, late-pointer and call-copy
boundary fixes below. The eabi8 installation verifies the same package hashes. To install in a
**new directory**:

```text
rtk proxy C:/Python313/python.exe tools/patches/m2c-rel-first-pass/install.py --source C:/Users/Anony/.codex/tools/m2c --destination C:/Users/Anony/.codex/tools/m2c-rel-20260914-eabi8-new
```

The installer preserves the source installation, rejects baseline drift, applies
the portable patch, and verifies all 36 changed output files. It never modifies
game source, configures a build, or publishes matching status. Destinations may
be nested inside another checkout: patch application disables parent repository
discovery and inherited Git repository variables without creating a repository.
Automatic Git newline conversion is disabled before the package's explicit
newline restoration and hash verification.

For a new batch use `tools/rel_first_compile_prepare.py --help`, supplying the
verified link directory, real flags JSON, this `--m2c`, `--ppc-abi gekko-eabi`,
and a fresh output path. The default remains `legacy` for older translators.
Run the prepared manifest with the now-canonical existing runner:

```text
rtk proxy python tools/rel_first_compile_batch.py build/<batch>/manifest.json --workers 4
```

The runner preserves bounded scratch use and resumable census results, binds
translator implementation inputs once per run, and does not promote raw scores.
It requests m2c stack traces so the bounded failure diagnostic preserves parser
causes; an intermittent parser failure is still unresolved, not silently retried
or credited as a source result.
Use `tools/rel_first_compile_freeze.py` for a drained continuation. Neither tool
changes an active manifest or grants source promotion. Keep compact champions
and useful diagnostics; do not retain full per-function objdiff JSON history.

Unknown heap layouts, larger stack-array bounds, missing project-specific
interfaces, loops and inline structure remain genuine reconstruction work.
Compilation and raw similarity are not strict/data/physical/source-link proof.

## Explicit grouped first compiles

The existing batch runner accepts optional per-module groups, for example:

```json
"translation_groups": [{"name": "gameplay", "functions": ["fn_1_420", "fn_1_594"]}]
```

Each group is one ordered multi-function m2c invocation, one compiler invocation,
and one objdiff invocation. Members keep separate function metrics but share the
source/object hashes; storage is retained once. Groups must be explicitly chosen
from known module functions, with no duplicate/overlapping membership. Ungrouped
functions are unchanged. Resume skips a complete group and reruns an incomplete
group without appending duplicate completed rows. Complete a partial group with
`--resume` before freezing a new phase; freezing must not change its TU context.
No TU boundaries or compiler flags are guessed, and no new closure credit follows.

Local Qwen supplied the implementation and tests; primary review simplified the
retention path, fixed resumed storage accounting, and protected the freezer.
The real m621 two-function acceptance used exactly three processes and shared
artifacts with independent results. The 52-function gameplay attempt used one
translation and one compile, but still fails on an incomplete inferred table
type. Thus grouped execution is verified, **not** a claim that unedited m2c now
reproduces the 52-function recovered source or its 432-byte literal pool.
Compact live results: `build/rel-gap-fixes/grouped-first-acceptance/`.

The first narrow-call Qwen proposal was neutral: both paired current m621
`fn_1_2E68` drafts remained 428/404 bytes and 20 differences. The revised,
measured fix below is now included; the neutral attempt is not credited as a gain.

## Active-recovery follow-up: narrow call copies versus full-width owners

In m621 `fn_1_2E68`, a full-width loop counter feeds arithmetic and comparison;
only a copied outgoing call argument is sign-extended to short. Type inference
incorrectly let that call copy narrow the counter itself. Merely disabling
unification at cast creation was insufficient: `Cast.use()` and formatting
reintroduced the same constraint later.

The PPC instruction-use graph now recognizes an `extsh`/`extsb` destination
whose complete observed uses are outgoing call inputs. Its existing cast keeps
the narrow call type without unifying the producer during creation, use or
formatting. Arithmetic/comparison uses and context-owned narrow storage retain
normal inference; this does not rewrite the shared MIPS/ARM cast path. A missing
or mixed use set does not qualify. Local Qwen supplied the revised implementation
and focused tests; the primary integrated and compiled it against actual headers.

Paired current m621 compilation improves **428 -> 412 bytes** against a 404-byte
target, **20 -> 12 rows**, and **85.544556 -> 90.247530% raw**. The installed
package reproduces the scratch result's source/object/report hashes. This is a
translation gain on an already recovered minigame, not a new closure. Both arms
apply the same context-prototype spelling correction (`int`, not MP6's `s32`
alias for `long`); that independent formatting gap remains unresolved.

The fresh eabi7 copy passes **531/531 tests**: 413 end-to-end and 118 unit tests,
without golden rewrites. The four new cases cover call-only short/byte copies,
genuinely narrow arithmetic, and context-defined short storage. Original m2c,
eabi6 and active batch manifests remain unchanged. Compact paired bindings are
in `narrow-call-validation.json`; this fix is also included in eabi8.

## Active-recovery follow-up: flat terminal switches with a shared return

m612's two direction helpers have one physical `blr`, but the generated flat
switch placed `return;` in every case. MWCC emitted an extra instruction in each
function. The Gekko MWCC renderer now uses `break;` for that narrowly identified
terminal void-switch shape. It counts original instruction locations, not
duplicated CFG return nodes. Value-returning functions, multiple physical returns,
nested control flow, nonterminal switches and legacy mode retain existing output.

With identical actual headers and GC1.3.2 flags, **unedited generated C** changes
`m612dll:fn_1_5E90` from 100/96 bytes and one row to **96/96, zero rows**;
`fn_1_5EF0` changes from 108/104 and one row to **104/104, zero rows**. The
installed eabi8 reproduces both scratch source/object hashes. This automates the
primary's verified reconstruction lesson; it did not discover that lesson by
itself. Both natural helpers are retained in `src/REL/m612dll/movement.c`.
The module is still incomplete; no whole-module or main progress is credited.

All **536 translator tests pass** (413 end-to-end, 123 unit) without golden
rewrites. Five new cases cover the positive shape and its exclusion gates.
Package hashes and the paired compiler result are in
`terminal-switch-validation.json`. New preparation uses eabi8; previous
installations and already-running manifests remain unchanged.

## Follow-up: actual call interfaces and stack arguments

- The preparer discovers unique **external** header providers through existing
  `decompctx`. It never imports another REL's same-named `fn_1_*` declaration.
  Conflicting bodies/macros stay out; only exact builtin-only declarations can
  supplement both translator context and the private compiler prefix after a
  same-flags preprocessing compatibility check. Ambiguity remains explicit.
  Bulk header lookup skips unrelated function-shape analysis.
- The opt-in Gekko profile uses independent r3-r10/f1-f8 argument banks and
  real outgoing stack slots beginning at +8. A ninth float is not f9. Unsupported
  non-word spills and absent/stale writes are errors, not guessed arguments.
- Stable outgoing scalar copies are consumed directly when mutation, alias,
  escape and whole-function stack-use checks permit it. Real aggregate owners
  and ambiguous snapshots remain intact. Partial writes and interior-address
  escapes preserve the original captured value, not a later reread.
- Unused save/restore address bookkeeping no longer creates a fake `arg_sp0`.
  Real incoming loads and address-passed parameters still register normally.
- An explicit cleared CR1 variadic-call flag excludes inferred floating varargs.
  Known fixed floating parameters remain intact; unknown/set flags, intervening
  clobbers and block joins retain conservative inference. This removes phantom
  floating arguments from the actual m657 `OSReport` calls without guessing from
  format strings.

Final installed tests: **501/501**, including **88 unit tests**. No additional
golden rewrites were needed for this follow-up.

Actual paired external-header sample: **0/12 -> 11/12 compile**, with identical
translator, target and flags. `ttwarsdll:fn_1_D88` reaches 99.41406% raw; there
are **zero new raw-100 functions or owner closures** in this sample. Seven
successful drafts still contain placeholder constructs and require reconstruction.
The selection, paired hashes and scores are in `call-abi-validation.json`.

The m657 camera acceptance also exposed the limit: correct stack-argument
translation shrinks fn_1_258 from 468 to 452 bytes, but its raw score changes
77.564354 -> 76.722770%, not an improvement. fn_1_438 now compiles at only
18.787878%. These ABI fixes remove incorrect translation; they do not solve
missing record layouts, loops, stack arrays or source lifetimes. Preserve that
distinction when deciding the next reconstruction task.

The four-worker follow-up selected six additional external-call failures outside
the 12-case sample, using their existing baseline census rather than compiling
controls again. **Five now compile**, with `miraclebookdll:fn_1_4FD4` at 96.41157%
raw; no raw-100 result or owner closure. The remaining failure is an unsized local
stack array, not missing call context.

The CR1 compiler acceptance uses the same translator with only that filter
disabled/enabled: `m657Dll:fn_1_47EC` changes from an illegal pointer-to-float
variadic conversion to compiling at 64.320755% raw (216 versus 212 target bytes).
It is a translation-correctness improvement, not an exact function. An unrelated
inferred integer vararg and incomplete layouts remain visible.

## Active-recovery follow-up: stack extents and compiler-visible declarations

The next measured failure was not fixed by adding another declaration: m2c
inferred an unsized indexed local and treated its second element as a separate
scalar. The existing preliminary translation now uses its natural-loop/phi
graph to recover the minimum observed extent of a zero-start, unit-step,
constant-bounded store loop. D28 becomes `s32 sp10[2]`; former `sp14` consumers
refer to `sp10[1]`. Context-owned fields, escapes, unknown/nonunit bounds, narrow
overflow and overlapping/incompatible storage remain unresolved. Neighboring
stack addresses alone never establish an array.

With identical actual headers, target and GC1.3.2 flags, m657 D28 changes from
MWCC's incomplete-array error to **856/872 bytes, 87.7156% raw**. This fixes a
compile barrier, not the entire source shape. Primary reconstruction separately
reaches **872/872, 99.6789%** with natural result/character/player arrays and the
external `abs` interface; do not credit that manual reconstruction to m2c.

Two actual-header gaps are also fixed in the existing preparer:

- Builtin-only declaration fallback now reaches the compiler prefix as well as
  m2c context. Historical saf `exp` proved why: m2c omitted its redeclaration,
  leaving the compiler without `double exp(double)`. Equivalent extern/spacing
  duplicates are accepted; conflicting prototypes and provider bodies are not.
- When the target explicitly calls external `abs`, an active private-prefix
  `abs` macro is disabled and the exact existing MSL prototype is retained.
  The m657 preprocessing replay changes two `__abs` expansions back to two
  `abs` calls. Other macros and shared headers are untouched.

Installed eabi5 passes **515 tests** (413 end-to-end and 102 unit), including
14 new array-bound cases; preparer/freezer passes13. One earlier default-parallel
installed run returned514/515 with its failure text truncated; the captured
four-worker run passed515/515 without edits. No intermittent-parser fix is
claimed. Keep bounded captured diagnostics rather than masking such failures.

The companion `pool_reloc_summary.py` change distinguishes proven split-symbol
annotation equivalence from actual literal/type changes. It does not alter
objdiff scores, physical receipts, source selection or promotion requirements.
At that follow-up, the m657 reconstruction had23 zero-row functions among30
implemented symbols; it remained incomplete with no new whole-owner closure.

## Active-recovery follow-up: an empty terminal switch case

The PPC/MWCC irregular-switch recovery now retains one compiler-evidenced empty
terminal case. The rule requires exactly three contiguous nonempty lower cases,
the same selector, and an otherwise empty `cmpwi`/unconditional-branch tail to
the common void return. Incoming bounds below the dead exclusive upper bound
must contain exactly one value. It does not infer labels from plain gotos,
multiple-value ranges, noncontiguous cases, different selectors, side effects,
or another compiler profile. Explicit equality dispatch for case 4 stays case 4.

For `m657Dll:fn_1_1F6C`, unedited old/new drafts compiled with identical actual
headers, target and GC1.3.2 flags both occupy **200/172 bytes**. Retaining
`case 3: break` improves **29 to 23 mismatch rows**, or **77.441864% to
82.37209% raw**. This is a partial translation gain, not an exact function.
The separately reconstructed source reaches **172/172 bytes and zero rows**;
that manual result is not credited to the generated draft. A single isolated
case-4 control produced 176 bytes and one mismatch row, retaining the extra
equality branch that is absent from the target. These constraints support the
narrow rule, not unique original-source spelling for arbitrary switches.

Installed eabi6 passes **523/523 tests** (413 end-to-end and 110 unit), including
all eight portable switch regressions. The unit test requires no project files
or private compiler; compiler replay receipts remain under ignored `build/`.
The installer verifies all 34 cumulative changed files against the original
pinned baseline. `active-gap-validation.json` binds the package, captured test
log and paired compiler evidence. An earlier scratch run had one unrelated ABI
runtime diagnostic failure; its focused 16 tests and the captured full rerun
passed without ABI changes. No intermittent-runtime fix or new closure is claimed.

## Bounded follow-up: a pointer type learned after address creation

The actual eabi6 residual `m649Dll:fn_1_688C` still emitted `arg0 + 16`
and `arg0 + 40` despite declaring `arg0` as `void *`. At `add_imm`, the
base was not yet known to be a pointer, so it bypassed the existing pointer
handler. Later consumer inference changed its type without revisiting that
routing decision. This is distinct from the already-fixed indexed-store path.

Constant non-partial additions on an initially unknown base now retain their
machine byte displacement until formatting. Only a final void/incomplete object
pointer triggers byte-address lowering; complete typed pointers and integer
expressions retain the existing behavior. Default diagnostic output, skipped
casts, zero offsets and partial relocation immediates are unchanged. No record
fields or sizes are invented. The two calls now pass `(Point3d *) ((u8 *) arg0 +
16)` and the corresponding 40-byte address.

With the same frozen context, target and compiler flags, regenerated eabi6
fails MWCC with `illegal type`; the new draft compiles at **88/88 bytes,
2 mismatch rows, 94.545456% raw**. `M2C_FIELD` remains and no recovery closure
is claimed. The frozen flags also contain a missing, unused `include-2` directory;
the before diagnostic records that warning. This is one paired function, not a
new result for the original 167-case first batch or the solved stack-array cases.

The new copy at `C:/Users/Anony/.codex/tools/m2c-rel-20260914-late-pointer-check`
passed installer verification of all 34 cumulative output hashes and **23 focused
pointer tests**, including four new late-typing tests. Original m2c and eabi6 were
not modified; no default rollout or full-suite result is claimed. An initial
installation inside this project failed hash verification because Git applied
no files from that nested checkout directory; the rejected copy remains under
ignored `build/rel-gap-fixes/late-pointer-package-check`. The installer now fixes
that independent limitation; fresh external and nested installations under both
MSYS and native Windows Git verified the same 34 hashes. Focused tests include
hostile inherited repository variables and `autocrlf=true`.
Compact evidence is in `late-pointer-validation.json`; full paired bindings
remain under ignored `build/rel-gap-fixes/late-pointer-acceptance/results.json`.
