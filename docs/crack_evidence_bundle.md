# Crack evidence bundle

`tools/crack_evidence_bundle.py` is the only production compile/evidence adapter
used by the crack harness. It runs twice in one disposable detached worktree:
baseline before the source overlay and candidate after it. The exact CLI is:

```text
python tools/crack_evidence_bundle.py --root WORKTREE --context OUT/approval-context.json --out OUT
```

The phase and sealed owner/function/unit/source/target/context bindings come from
`CRACK_HARNESS_*`. The tool refuses stale phase outputs, source/context drift,
ambiguous objdiff units, target-object drift, output escape, unsupported ELF
layout, and unresolved relocation targets.

One central `mp6_crack_toolchain/v1` manifest (schema:
`tools/MP6_CRACK_TOOLCHAIN_V1.schema.json`) hash-binds objdiff 3.8.0, DTK,
sjiswrap, the compiler and binutils trees, and authenticated retail inputs. The
approval `toolchain_key` is the manifest's canonical self-hash. The manifest
also pins Ninja; the adapter has no PATH/CLI build-runner override. Detached
runs reuse those read-only tools and copy retail inputs into ephemeral storage
for the complete phase, deleting those inputs unconditionally after proof or
failure. They never download or retain a worktree-local toolchain.

Baseline emits `target.o`, `baseline-strict.json`, and `baseline-data.json`.
Candidate preserves those bytes and adds `candidate.o`, candidate strict/data
reports, and `physical.json`. Reports come from the pinned objdiff executable.
Physical rows are independently decoded from big-endian PowerPC ELF relocation
and symbol tables after the pinned GNU readelf accepts both objects. All phase
artifacts and tool identities are bound by compact self-digested receipts.
