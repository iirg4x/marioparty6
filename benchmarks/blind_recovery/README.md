# Blind source-recovery benchmark

This directory stores **benchmark metadata and replayable artifacts**, not normal
recovery context.

The benchmark intentionally separates four results:

1. **Assembly equivalence** — did the candidate reproduce the supplied target or
   surrogate assembly?
2. **Retained-source fidelity** — after the blind phase ended, how closely did
   the candidate resemble the withheld recovered C?
3. **Organicity** — does the candidate look like ordinary maintainable C, or does
   it contain compiler controls, synthetic storage, opaque identifiers, or
   suspicious redundant structure?
4. **Reproducibility** — can another reviewer replay the score from the preserved
   evidence, frozen candidate, assembly files, source commit, and hashes?

A candidate can score perfectly on source fidelity while inheriting questionable
structure already present in the retained source. Source similarity is therefore
never used as the organicity score.

## Existing cases

The two July 25, 2026 proof-of-concept trials are retained as
`legacy-reported`. Their summary metrics are useful, but their raw evidence,
frozen candidate, compared assembly, and prompt were not preserved. They do not
count as independently replayable benchmark proof.

```sh
python tools/blind_recovery.py audit
```

The default audit passes with explicit warnings for legacy cases. A strict audit
fails until every listed case is reproducible:

```sh
python tools/blind_recovery.py audit --strict --replay
```

## Organicity review

Score a complete file or one function:

```sh
python tools/blind_recovery.py organicity \
  src/gssdk_lib/asrpho/common/blocks/stacker.c \
  --function ProcessStacker
```

The automated review looks for high-risk compiler controls and lower-confidence
source-quality debt, including:

- pragmas, forced inline/no-inline controls, inline assembly and dead branches;
- unexplained `volatile` or `register` use;
- cast ladders and foreign include-guard manipulation;
- opaque raw/blob/tail/padding storage;
- address-derived and `unk_*`/`reserved*` identifiers;
- a narrow class of guaranteed post-loop conditions.

The score is a review aid, not proof of authenticity. Legitimate old source can
trigger a finding, and aesthetically clean code can still be historically wrong.
Every finding remains subject to target, consumer, sibling and compiler evidence.

## Reproducible holdout flow

The evaluator prepares the trial in a local ignored directory. The worker should
receive only `packet/` and the candidate template—not `private/reference.c`.

```sh
python tools/blind_recovery.py prepare \
  --id board-example \
  --source src/board/example.c \
  --function fn_80000000 \
  --evidence build/blind-evidence/board-example.md \
  --target-assembly build/blind-evidence/board-example-target.s
```

The worker writes a candidate. Before the retained source is revealed, freeze it:

```sh
python tools/blind_recovery.py freeze \
  --run-dir build/blind-recovery/<run> \
  --candidate build/blind-recovery/<run>/candidate.c
```

Compile the frozen candidate with the same compiler configuration, then score it:

```sh
python tools/blind_recovery.py score-run \
  --run-dir build/blind-recovery/<run> \
  --candidate-assembly build/blind-evidence/board-example-candidate.s
```

The scorer records assembly similarity, source-token similarity, candidate and
reference organicity, candidate-only debt, and retained-source debt inherited by
the candidate.

Archive only after `candidate.s`, `result.json`, and the report exist:

```sh
python tools/blind_recovery.py archive \
  --run-dir build/blind-recovery/<run> \
  --destination benchmarks/blind_recovery/cases/board-example
```

Then add the new `case.json` path to `manifest.json`.

## Replay requirements

A case marked `reproducible` must preserve:

```text
evidence.md
candidate.c
target.s
candidate.s
result.json
report.md
case.json
```

The retained C body is not committed as a duplicate. `case.json` records the
source path, source commit and SHA-256. Replay extracts the reference from Git,
verifies the hash, reruns every deterministic score, and compares it with the
stored result:

```sh
python tools/blind_recovery.py replay \
  benchmarks/blind_recovery/cases/<case>/case.json
```

## Retail benchmark standard

A conclusive workflow comparison should include at least six exact GP6E01
functions across three difficulty bands and multiple owners. One arm should use
the structured workflow and one an equivalent unstructured baseline. Record:

- input/output tokens and wall time;
- number of compiler attempts and objdiff progression;
- exact assembly result;
- source-token similarity after reveal;
- organicity findings before reveal;
- unsupported constructs and consumer regressions;
- exact evidence packet and frozen candidate;
- toolchain, source commit and target-object identity.

Until those retail cases exist, the workflow remains
**infrastructure-tested and source-holdout-tested, not retail blind-recovery
validated**.
