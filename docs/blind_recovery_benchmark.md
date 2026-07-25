# Blind recovery benchmark

## Purpose

This benchmark tests whether a recovered C implementation can be withheld and
reconstructed from binary-facing evidence without reading the retained source or
its historical recovery report.

It is a controlled **source-holdout benchmark**. It validates recovery reasoning,
context size, and source-fidelity scoring. It is not a substitute for a retail
Metrowerks/object benchmark because the test environment did not contain the
private GP6E01 inputs or the pinned proprietary compiler.

## Isolation rules

For each trial:

1. Select an already recovered exact C function.
2. Fetch the source as opaque base64 and decode it directly into a temporary
   workspace without displaying the body.
3. Do not read the corresponding wave report.
4. Extract only the function signature programmatically.
5. Compile the withheld source to PowerPC assembly with Clang 17 targeting
   `powerpc-unknown-eabi`, `-mcpu=750`, `-m32`, and `-O2`.
6. Give the recovery attempt only:
   - the function signature;
   - required public type layouts and prototypes;
   - generated PowerPC assembly.
7. Write one natural C candidate without reading the source.
8. Compile the candidate with the same surrogate compiler and compare normalized
   assembly.
9. Reveal the withheld source only after the candidate is frozen, then compare
   source tokens and structure.

The benchmark did not use the source body, a decompiler output, a wave report, or
a knowledge card containing the answer.

## Trial 1: `ProcessLookAhead`

Owner:

```text
src/gssdk_lib/asrpho/common/blocks/flfxblks/lkahead.c
```

Withheld function:

```c
static void ProcessLookAhead(
    TosBaseBlock *baseBlock, void **input, s32 inputCount);
```

Evidence packet:

- public `TosBaseBlock` and queue declarations;
- 84 lines of generated PowerPC assembly;
- no retained C body;
- no Wave 34 recovery notes.

Results:

| Metric | Result |
| --- | ---: |
| Candidate attempts | 1 |
| Normalized PowerPC assembly | Identical |
| Reference tokens | 204 |
| Candidate tokens | 204 |
| Token-sequence similarity | 1.000000 |
| Semantic/control-flow differences | None |
| Textual differences | Formatting only |
| Approximate evidence tokens | 412 |
| Approximate candidate tokens | 325 |

The recovered candidate reproduced the history-ring update, wraparound,
look-ahead queue delay, threshold output, and flush behavior. After revealing the
source, the token sequence was identical; only indentation and signature wrapping
differed.

## Trial 2: `ProcessStacker`

Owner:

```text
src/gssdk_lib/asrpho/common/blocks/stacker.c
```

Withheld function:

```c
static void ProcessStacker(
    TosBaseBlock *baseBlock, void **inputs, s32 inputCount);
```

Evidence packet:

- public `TosBaseBlock`, queue, and `memcpy` declarations;
- 82 lines of generated PowerPC assembly;
- no retained C body;
- no Wave 34 recovery notes.

Results:

| Metric | Result |
| --- | ---: |
| Candidate attempts | 1 |
| Normalized PowerPC assembly | Identical |
| Reference tokens | 140 |
| Candidate tokens | 140 |
| Token-sequence similarity | 1.000000 |
| Semantic/control-flow differences | None |
| Textual differences | Indentation only |
| Approximate evidence tokens | 573 |
| Approximate candidate tokens | 200 |

The candidate recovered the first-available-input scan, early return, output
queue allocation, per-input enable mask, element-size copies, and output pointer
advance. After the source was revealed, the token sequence was identical.

## Result

Both blind source holdouts recovered the retained C token-for-token on the first
candidate and emitted identical surrogate PowerPC assembly.

This is positive evidence that focused signatures, public layouts, and compact
assembly can support highly faithful recovery without loading historical wave
text. It also demonstrates that a useful target packet can stay below roughly
600 estimated tokens for small-to-medium functions.

## What this benchmark does not prove

It does **not** yet prove:

- exact output under the pinned Metrowerks compiler;
- success against retail GP6E01 assembly rather than assembly generated from the
  withheld source;
- performance on large game-state functions;
- the benefit of knowledge cards on a target with a known compiler trap;
- comparative token cost against the unstructured `main` workflow;
- independent Claude and Codex recovery quality across a statistically useful
  sample.

## Required retail benchmark

The next benchmark must run on the local PC with the private target objects and
pinned compiler:

1. Choose at least six exact functions across three difficulty bands and multiple
   owners.
2. Remove their C bodies in disposable worktrees while retaining target objects.
3. Hide source history, relevant wave reports, and answer-revealing owner cards.
4. Give one worker the structured workflow packet and another an equivalent
   unstructured baseline packet.
5. Record attempts, wall time, input/output tokens, objdiff score after each
   attempt, exact-match result, source-token similarity, unsupported constructs,
   and affected-consumer regressions.
6. Reveal and score the retained source only after each final candidate is
   committed.

Until that retail benchmark is complete, the workflow should be described as
**infrastructure-tested and source-holdout-tested**, not retail blind-recovery
validated.
