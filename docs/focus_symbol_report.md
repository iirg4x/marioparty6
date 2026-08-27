# Compact focus-symbol reports

`tools/focus_symbol_report.py` removes whole-owner JSON from the causal-analysis
hot path. It reads the canonical strict/data reports once and writes a small,
deterministic artifact containing:

- every normalized target/candidate strict instruction for one function;
- every strict/data diff row and a digest of the unabridged rows;
- every objdiff relocation annotation, its referenced symbol descriptor, and
  referenced object payloads needed for typed-pool reduction;
- section sizes and match percentages without whole-section payloads; and
- exact sibling identities plus canonical digests of every sibling metric.

Relocation annotations and referenced pool payloads are stored once in the
strict channel; the data channel carries a strict-channel reference instead of
duplicating them. Raw strict/data rows retain separate digests, while a second
instruction-payload digest proves the two channels describe the same code even
when their diff annotations differ.

Objdiff relocation annotations are diagnostic, not physical proof. Pass an
independently generated physical-relocation receipt to embed and cross-check
physical rows. If no receipt is supplied, the artifact says `UNKNOWN`; it never
silently upgrades report annotations into physical authority.

## P0 workflow

Extract a baseline artifact once, then one artifact per candidate:

```text
python tools/focus_symbol_report.py extract STRICT.json DATA.json Function \
  --expect-strict-report-sha256 STRICT_SHA \
  --expect-data-report-sha256 DATA_SHA \
  --physical-relocation-receipt PHYSICAL.json \
  --expect-physical-relocation-receipt-sha256 PHYSICAL_SHA \
  --require-physical --output focus.json
```

Run the protected-sibling gate using only the compact artifacts:

```text
python tools/focus_symbol_report.py gate baseline.focus.json candidate.focus.json \
  --expect-baseline-artifact-sha256 BASELINE_FILE_SHA \
  --expect-candidate-artifact-sha256 CANDIDATE_FILE_SHA \
  --require-pass --output sibling-gate.json
```

The gate is identical to the owner rule: every exact baseline sibling, excluding
the focus function, must remain exact. Newly exact siblings are allowed and
reported. Internal hashes are canonical and inputs are read once, hashed, and
parsed with duplicate-key rejection.

The extractor is diagnostic only. It emits no C, retains no candidate, changes
no source, and advances no integration or promotion authority.

## Bound acceptance

On the exact `ConfigPadClose` proof, the canonical strict/data inputs were
11,173,070 and 11,144,021 bytes. The resulting artifact was 208,397 bytes and
contained all 72/72 objdiff annotations, the independent 72/72 physical receipt,
zero focus rows, and 39 protected exact sibling identities. A baseline artifact
was 177,611 bytes; the 1,506-byte gate passed with 38 to 39 protected exact
siblings and zero losses in both channels. Five warm, hash-bound extractions had
a 166.035 ms median and 182.956 ms maximum on the acceptance host.
