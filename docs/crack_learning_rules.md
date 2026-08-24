# CRACK_REPORT learning rules

`tools/crack_learning_rules.py` turns four reviewed function-level lessons into
deterministic, read-only diagnoses. It composes the installed causal objdiff
reducer and emits self-hashed `crack_learning_diagnosis/v1` JSON with
`authority_advanced:false`.

The output is not a source generator or retention receipt. Every matched rule
includes its confidence, physical evidence, natural source class, and proof
limitations. Unmatched rules remain visible with a rejection reason so a
partial resemblance cannot silently become advice.

## Usage

From the repository root:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/candidate.strict.json \
  --function ev_CapBobleOMExec
```

The aggregate-copy rule additionally requires an explicitly named exact donor
from the same object report:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/candidate.strict.json \
  --function mbev_CapBomheiMove \
  --same-tu-donor mbev_CapBobleMove
```

Being named is not enough: the donor is accepted only when both report sides
declare 100% and both contain the same bounded self-copy/final-consumer shape.
The report-level join is the translation-unit boundary; no cross-report donor
claim is accepted.

Python callers can use:

```python
from tools.crack_learning_rules import diagnose_document

result = diagnose_document(
    objdiff_document,
    focus_symbol="mbev_CapBomheiMove",
    same_tu_donor_symbols=("mbev_CapBobleMove",),
)
```

## Installed-functionality audit

The explicit else-return recognizer already belongs to
`tools/mismatch_cluster_audit.py`. This module calls that reducer and consumes
its exact `explicit_else_return_epilogue` hypothesis; it does not duplicate the
CFG detector. The existing reducer also remains the owner of generic stack,
aggregate, branch, ABI, and relocation clustering.

The three new joins are narrower than those generic classifications:

| Rule | Required evidence join | Natural source class |
|---|---|---|
| explicit else-return CFG | Installed reducer's conditional-to-second-exit, adjacent shared-epilogue branches, and candidate direct-epilogue signature | Explicit else-return control flow |
| assignment/condition saved-GPR cycle | Equal function size; identical operations, relative branches, relocations, and non-register operands; a closed cycle of at least three nonvolatile GPRs; and a call result copied to the cycled register immediately before comparison and conditional branch | Assignment in its consuming condition |
| switch-case FPR lifetimes | Indirect switch dispatch; larger target frame corroborated by the reducer's uniform stack-home delta; larger target function; and at least three target-only call-result copies into nonvolatile FPRs | Used floating-point result locals scoped to individual switch cases |
| aggregate self-copy at final consumer | A target-only three-or-more-component aggregate load/store self-copy, no later call after its bounded final consumers, and an explicitly named same-report donor that is 100% on both sides with the same copy signature | Used aggregate self-assignment at the final-consumer boundary |

These joins preserve the reviewed boundaries from
`ev_CapTeresaFadeMatHook`, `ev_CapMiracleCoinTrade`,
`ev_CapBobleOMExec`, and `mbev_CapBomheiMove`. The names identify acceptance
fixtures, not symbol-specific allowlists: another function must reproduce the
same physical signature.

## False-positive gates

The focused fixtures reject the tempting incomplete variants:

- an almost-identical branch tail whose two target exits do not share one
  epilogue;
- a saved-GPR permutation without a call-result assignment immediately
  consumed by a condition;
- FPR captures and a frame delta without indirect switch dispatch;
- a final-consumer aggregate self-copy whose named donor is not exact.

Other structural differences also close a rule. The assignment/condition rule
rejects inserts, deletes, opcode or relocation changes, inconsistent mappings,
volatile-register differences, and two-register swaps. The switch rule rejects
an uncorroborated or excessively large frame delta. The aggregate rule rejects
early copies, unequal load/store offset multisets, non-final consumers, unnamed
donors, and donor signature drift.

## Output and authority boundary

The document binds canonical objdiff input, donor names, this implementation,
and the installed reducer by SHA-256. `diagnosis_sha256` hashes the complete
document excluding that field. Rule order and JSON serialization are stable.

No diagnosis establishes semantic names, original spelling, or source
provenance. It never authorizes a source edit, candidate retention, promotion,
or recovery-state change. Strict text/data, physical relocation, section,
consumer, protected-sibling, linked-container, and checksum proof remain
separate gates.
