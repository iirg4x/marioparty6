# Target-emitted overwritten-computation diagnosis

`tools/target_emitted_overwritten_computation.py` implements the
`target_emitted_overwritten_computation` rule in
`crack_learning_diagnosis/v29`.

The rule is intentionally narrower than a dead-assignment detector. It accepts
only the authenticated `main:board/capspecial` / `ev_CapMiracleSprUpdate`
program point from CRACK_REPORT SHA-256
`41d8257182fac0f040b6888f5b3845ea05d5cde557d7e9b531d75080a0bf2bcd`.
The baseline must contain the exact target-only instruction chain
`lfd, lfs, fmuls, fmul, lfd, fdiv, bl cos, frsp` at rows 608–615, with the
sealed preceding and following instructions. The exact report must remain
2780/2780, strict/data exact, and 172/172 physical relocations.

When the baseline contract matches, the rule ranks one source cell:

```c
scale = cos((M_PI * (90.0f * time)) / 180.0);
```

The recommendation is diagnostic only. It requires the bound owner-retained
record and policy-correction record. Exact bytes alone are insufficient for
admissibility. The rule explicitly refuses to create a blanket waiver for dead
or unused assignments, and it forbids invented calls/constants/locals,
synthetic CFG, assembly, padding, register shaping, broad searches, automatic
retention, and promotion.

Telemetry is bound fail-closed: this campaign has incomplete interval coverage,
is excluded from measured crack/hour, and permits no imputation.

Use the integrated CLI with both the objdiff report and authenticated context:

```text
python tools/crack_learning_rules.py \
  --report strict-full.json \
  --function ev_CapMiracleSprUpdate \
  --target-emitted-overwritten-context context.json
```

The output binds canonical hashes for the report and context, the rule
implementation SHA-256, `authority_advanced=false`, and either the single
bounded source cell or an explicit fail-closed reason.
