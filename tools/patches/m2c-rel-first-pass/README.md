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
`C:/Users/Anony/.codex/tools/m2c-rel-20260914-eabi3/m2c.py`.
Use this path for new REL preparation and reconstruction. The original installed
m2c and the already-running all-REL baseline were deliberately left unchanged.

To recreate it in a **new** directory:

```text
rtk proxy C:/Python313/python.exe tools/patches/m2c-rel-first-pass/install.py --source C:/Users/Anony/.codex/tools/m2c --destination C:/Users/Anony/.codex/tools/m2c-rel-20260914-eabi3
```

The installer preserves the source installation, rejects baseline drift, applies
the portable patch, and verifies all 30 changed output files. It never modifies
game source, configures a build, or publishes matching status.

For a new batch use `tools/rel_first_compile_prepare.py --help`, supplying the
verified link directory, real flags JSON, this `--m2c`, `--ppc-abi gekko-eabi`,
and a fresh output path. The default remains `legacy` for older translators.
Run the prepared manifest with the now-canonical existing runner:

```text
rtk proxy python tools/rel_first_compile_batch.py build/<batch>/manifest.json --workers 4
```

The runner preserves bounded scratch use and resumable census results, binds
translator implementation inputs once per run, and does not promote raw scores.
Use `tools/rel_first_compile_freeze.py` for a drained continuation. Neither tool
changes an active manifest or grants source promotion. Keep compact champions
and useful diagnostics; do not retain full per-function objdiff JSON history.

Unknown heap layouts, larger stack-array bounds, missing project-specific
interfaces, loops and inline structure remain genuine reconstruction work.
Compilation and raw similarity are not strict/data/physical/source-link proof.

## Follow-up: actual call interfaces and stack arguments

- The preparer discovers unique **external** header providers through existing
  `decompctx`. It never imports another REL's same-named `fn_1_*` declaration.
  Conflicting bodies/macros stay out; only exact builtin-only declarations can
  supplement translator context without altering source includes. Ambiguity
  remains explicit. Bulk header lookup skips unrelated function-shape analysis.
- The opt-in Gekko profile uses independent r3-r10/f1-f8 argument banks and
  real outgoing stack slots beginning at +8. A ninth float is not f9. Unsupported
  non-word spills and absent/stale writes are errors, not guessed arguments.
- Stable outgoing scalar copies are consumed directly when mutation, alias,
  escape and whole-function stack-use checks permit it. Real aggregate owners
  and ambiguous snapshots remain intact. Partial writes and interior-address
  escapes preserve the original captured value, not a later reread.
- Unused save/restore address bookkeeping no longer creates a fake `arg_sp0`.
  Real incoming loads and address-passed parameters still register normally.

Final installed tests: **497/497**, including **84 unit tests**. No additional
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
