# Blind recovery benchmark

## Current status

The workflow has two **reported source-holdout trials**, but no committed
independently replayable retail trial yet.

The earlier trials reported first-attempt token-identical C and identical
surrogate PowerPC assembly. Their raw evidence packet, exact frozen candidate,
compared assembly, prompt and execution transcript were not preserved. They are
therefore classified as `legacy-reported`, not as reproducible benchmark proof.

Run:

```sh
python tools/blind_recovery.py audit
```

The audit passes while warning about those legacy cases. A strict replay audit
will fail until the manifest contains only fully reproducible cases:

```sh
python tools/blind_recovery.py audit --strict --replay
```

## What is scored now

Blind recovery is no longer represented by one success number. Every new case
records four independent dimensions.

### 1. Assembly equivalence

The normalized target and candidate assembly are compared independently from the
source text. This may be retail Metrowerks output or an explicitly labelled
surrogate compiler result.

### 2. Retained-source fidelity

After the candidate is frozen and the blind phase ends, its C token sequence is
compared with the retained recovered source at a recorded Git commit.

This measures how closely the candidate reproduced the retained C. It does not
prove that the retained C was itself original or organic.

### 3. Organicity

The candidate and retained source are scored separately for visible source debt.
The automated review flags:

- pragmas, forced inline/no-inline controls and inline assembly;
- dead preprocessor branches and foreign include-guard manipulation;
- unexplained `volatile` or `register` use;
- cast ladders;
- opaque raw/blob/tail/padding arrays;
- address-derived and `unk_*`/`reserved*` identifiers;
- a narrow, high-confidence class of guaranteed post-loop conditions.

The result distinguishes:

```text
candidate-only debt
retained-source debt inherited by the candidate
retained-source debt removed by the candidate
```

The score is a review aid, not proof of historical authenticity. Old organic C
can contain redundant or unusual constructs, while clean modern C can still be
historically wrong.

### 4. Reproducibility

A case counts as reproducible only when it preserves:

```text
raw evidence packet
frozen candidate C
exact target assembly used for comparison
exact candidate assembly used for comparison
machine-readable result
human-readable report
source path, source commit and reference SHA-256
candidate SHA-256 and freeze time
blindness assertions
```

Replay extracts the retained function from the recorded source commit, verifies
its hash, reruns deterministic scoring and compares the result with the committed
record.

## Review of the two legacy trials

### `ProcessStacker`

Owner:

```text
src/gssdk_lib/asrpho/common/blocks/stacker.c
```

The function body looks strongly organic. It performs an ordinary first-input
scan, early return, output allocation, conditional copies and pointer advance.
It contains no visible compiler-control scaffolding.

The surrounding `Stacker` structure still contains `reserved28`, which is
honest layout debt rather than recovered semantics. That debt is outside the
blind function body and must not be confused with the function-body score.

Reported legacy result:

| Metric | Reported result |
| --- | ---: |
| Candidate attempts | 1 |
| Normalized surrogate assembly | Identical |
| Source-token similarity | 1.000000 |
| Reproducible from committed artifacts | No |

### `ProcessLookAhead`

Owner:

```text
src/gssdk_lib/asrpho/common/blocks/flfxblks/lkahead.c
```

Most of the function is ordinary circular-history and delayed-output C. One
source-shape question remains:

```c
while (block->queued != 0) {
    ...
    block->queued--;
}
if (block->queued == 0) {
    block->flushActive = 0;
}
```

The post-loop condition is guaranteed by the loop. It may be authentic defensive
source, but token-identically reproducing it does not authenticate it. The new
organicity checker reports this as `guaranteed-post-loop-condition` and requires
human target/sibling/compiler review rather than silently calling it perfect.

The surrounding structure also contains `reserved29`, which remains semantic
layout debt outside the function body.

Reported legacy result:

| Metric | Reported result |
| --- | ---: |
| Candidate attempts | 1 |
| Normalized surrogate assembly | Identical |
| Source-token similarity | 1.000000 |
| Reproducible from committed artifacts | No |

## Reproducible local protocol

The evaluator creates a sealed run under ignored `build/blind-recovery/`:

```sh
python tools/blind_recovery.py prepare \
  --id board-example \
  --source src/board/example.c \
  --function fn_80000000 \
  --evidence build/blind-evidence/board-example.md \
  --target-assembly build/blind-evidence/board-example-target.s
```

The worker receives only the packet and candidate template. The retained body is
stored under the run’s private directory and must not be mounted into the worker
worktree or prompt.

Before revealing or scoring against the retained source, freeze the candidate:

```sh
python tools/blind_recovery.py freeze \
  --run-dir build/blind-recovery/<run> \
  --candidate build/blind-recovery/<run>/candidate.c
```

Compile the frozen candidate with the same toolchain and flags. Then score:

```sh
python tools/blind_recovery.py score-run \
  --run-dir build/blind-recovery/<run> \
  --candidate-assembly build/blind-evidence/board-example-candidate.s
```

Archive the complete replayable case:

```sh
python tools/blind_recovery.py archive \
  --run-dir build/blind-recovery/<run> \
  --destination benchmarks/blind_recovery/cases/board-example
```

Add the resulting `case.json` to
`benchmarks/blind_recovery/manifest.json`, then replay it:

```sh
python tools/blind_recovery.py replay \
  benchmarks/blind_recovery/cases/board-example/case.json
```

## Required retail comparison

The conclusive benchmark must run on the local PC with the private GP6E01 target
objects and pinned compiler.

Use at least six exact functions across three difficulty bands and multiple
owners. Give one arm the structured workflow packet and another an equivalent
unstructured baseline. Preserve and compare:

- exact evidence packets;
- input/output tokens and wall time;
- compiler attempts and objdiff progression;
- retail assembly result;
- frozen candidate before reveal;
- source-token fidelity after reveal;
- organicity findings before and after reveal;
- unsupported constructs and affected-consumer regressions;
- target-object identity, toolchain, source commit and hashes.

Until those cases exist, the correct description remains:

> **Infrastructure-tested and source-holdout-tested, not retail blind-recovery validated.**
