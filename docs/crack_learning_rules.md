# CRACK_REPORT learning rules

`tools/crack_learning_rules.py` turns seven reviewed function-level lessons into
deterministic, read-only diagnoses. It composes the installed causal objdiff
reducer and emits self-hashed `crack_learning_diagnosis/v3` JSON with
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

The allocator two-register-swap rule requires a separately authenticated
context file. It is intentionally unavailable from objdiff rows alone:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/kuribo-v128.strict.json \
  --function mbev_CapKuribo \
  --allocator-context work/kuribo-v128.allocator-context.json \
  > work/kuribo-v128.learning.json
```

The context is a closed `allocator_two_register_swap_context/v1` object. Every
proof Boolean must be literally `true`, every receipt must be a lowercase
SHA-256 digest, there must be exactly two distinct VarInfo owners, and the
producer/consumer boundary must be explicit:

```json
{
  "schema": "allocator_two_register_swap_context/v1",
  "proofs": {
    "objdiff_canonical_sha256": "<64 lowercase hex from the tool input>",
    "data_values_exact": true,
    "physical_relocations_exact": true,
    "cfg_calls_exact": true,
    "stack_frame_exact": true,
    "protected_siblings_preserved": true,
    "strict_report_sha256": "<64 lowercase hex>",
    "data_report_sha256": "<64 lowercase hex>",
    "physical_relocation_receipt_sha256": "<64 lowercase hex>",
    "varinfo_receipt_sha256": "<64 lowercase hex>",
    "source_boundary_receipt_sha256": "<64 lowercase hex>"
  },
  "owners": [
    {
      "name": "playerNo",
      "usage_class": 13,
      "target_register": "r27",
      "candidate_register": "r26",
      "lifetime_role": "long_lived",
      "evidence_sha256": "<64 lowercase hex>"
    },
    {
      "name": "diceValue",
      "usage_class": 14,
      "target_register": "r26",
      "candidate_register": "r27",
      "lifetime_role": "producer_consumer_boundary",
      "evidence_sha256": "<64 lowercase hex>"
    }
  ],
  "boundary": {
    "producer": "mbDiceExec return value",
    "consumer": "kuriboCoinTbl indexed load",
    "transformations": ["subtract one", "table lookup"],
    "evidence_sha256": "<64 lowercase hex>"
  },
  "observations": []
}
```

`observations` uses the exact observation schema from
`candidate_interaction_request/v1`. The two selection keys are
`declaration_chronology` (`existing` or `long-lived-first`) and
`value_identity_boundary` (`split` or `fused`). Supplying measured control and
single-axis cells lets the emitted request schedule only the missing combined
cell. Save `evaluations[].evidence.interaction_request` as JSON and run:

```sh
rtk python tools/candidate_interaction_planner.py \
  work/kuribo-v128.interaction-request.json
```

The Kokamekku-derived capacity and loop-destination rules are also unavailable
from objdiff rows alone. Supply either or both closed evidence contexts:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/kokamekku-v76.strict.json \
  --function mbev_CapKokamekku \
  --capacity-context work/kokamekku.stack-capacity-context.json \
  --branch-context work/kokamekku.loop-branch-context.json \
  > work/kokamekku.learning.json
```

`stack_extent_interface_capacity_context/v1` seals exact size/data/CFG/
relocation/sibling gates, the candidate and target byte extents of one live
array, its element size and used prefix, authenticated producer maxima with
Graphify source locations, and a bounded list of real declaration positions.
The detector computes the missing bytes and elements itself. It matches only
when the candidate extent equals `candidate_capacity * element_size`, the
target-only extent is a positive whole-element delta, and every producer
contract converges on the computed target capacity.

`loop_branch_destination_context/v1` seals exact size/frame/data/relocation/
sibling gates plus one zero-based residual row. It records the independently
classified `loop_increment` and `loop_exit` destinations and their exact
function-relative targets. The detector verifies that the report has exactly
that one relocation-identical conditional-branch residual before ranking one
natural `else { break; }` cell.

Minimal capacity context shape:

```json
{
  "schema": "stack_extent_interface_capacity_context/v1",
  "proofs": {
    "objdiff_canonical_sha256": "<64 lowercase hex>",
    "function_size_exact": true,
    "data_values_exact": true,
    "physical_relocations_exact": true,
    "cfg_calls_exact": true,
    "all_non_extent_structure_exact": true,
    "protected_siblings_preserved": true,
    "strict_report_sha256": "<64 lowercase hex>",
    "data_report_sha256": "<64 lowercase hex>",
    "physical_relocation_receipt_sha256": "<64 lowercase hex>",
    "stack_extent_receipt_sha256": "<64 lowercase hex>",
    "interface_contract_receipt_sha256": "<64 lowercase hex>"
  },
  "array": {
    "name": "capsuleObjId",
    "element_size": 4,
    "candidate_capacity": 3,
    "used_prefix_elements": 3,
    "candidate_extent_bytes": 12,
    "target_extent_bytes": 20
  },
  "producer_contracts": [
    {
      "provider": "mbPlayerCapsuleMaxGet",
      "source_location": "game/src/board/player.c:L3838-L3841",
      "maximum": 5,
      "evidence_sha256": "<64 lowercase hex>"
    }
  ],
  "declaration_positions": ["before_moveDir", "after_moveDir"]
}
```

Minimal branch context shape (relative targets are decimal byte deltas from
the branch instruction address):

```json
{
  "schema": "loop_branch_destination_context/v1",
  "proofs": {
    "objdiff_canonical_sha256": "<64 lowercase hex>",
    "function_size_exact": true,
    "stack_frame_exact": true,
    "data_values_exact": true,
    "physical_relocations_exact": true,
    "all_non_branch_rows_exact": true,
    "protected_siblings_preserved": true,
    "strict_report_sha256": "<64 lowercase hex>",
    "data_report_sha256": "<64 lowercase hex>",
    "physical_relocation_receipt_sha256": "<64 lowercase hex>",
    "branch_destination_receipt_sha256": "<64 lowercase hex>"
  },
  "branch": {
    "row_index": 123,
    "guard_class": "zero_terminator",
    "target_destination": "loop_exit",
    "candidate_destination": "loop_increment",
    "target_relative_target": 44,
    "candidate_relative_target": 24
  }
}
```

Python callers can use:

```python
from tools.crack_learning_rules import diagnose_document

result = diagnose_document(
    objdiff_document,
    focus_symbol="mbev_CapBomheiMove",
    same_tu_donor_symbols=("mbev_CapBobleMove",),
)
```

For the context-bound rules, pass parsed objects as `allocator_context=`,
`capacity_context=`, or `branch_context=`. The same closed validation used by
the CLI is applied to Python callers.

## Installed-functionality audit

The explicit else-return recognizer already belongs to
`tools/mismatch_cluster_audit.py`. This module calls that reducer and consumes
its exact `explicit_else_return_epilogue` hypothesis; it does not duplicate the
CFG detector. The existing reducer also remains the owner of generic stack,
aggregate, branch, ABI, and relocation clustering.

The four additional joins are narrower than those generic classifications:

| Rule | Required evidence join | Natural source class |
|---|---|---|
| explicit else-return CFG | Installed reducer's conditional-to-second-exit, adjacent shared-epilogue branches, and candidate direct-epilogue signature | Explicit else-return control flow |
| loop branch destination | Equal size/frame; exactly one relocation-identical conditional branch residual; sealed target/candidate relative destinations classified as target loop exit versus candidate loop increment; exact data, physical relocations, and siblings | Explicit else-break for an authenticated zero terminator |
| assignment/condition saved-GPR cycle | Equal function size; identical operations, relative branches, relocations, and non-register operands; a closed cycle of at least three nonvolatile GPRs; and a call result copied to the cycled register immediately before comparison and conditional branch | Assignment in its consuming condition |
| allocator two-register swap interaction | Equal function size and measurable frame; identical operations, relative branches, relocations, immediates, data values, physical relocations, and protected siblings; exactly one closed two-nonvolatile-GPR swap; exact VarInfo owner-to-register mapping; and one authenticated producer/consumer identity boundary | A bounded 2x2 interaction of natural declaration chronology and natural producer/consumer expression fusion |
| stack extent/interface capacity | Equal function size; sealed candidate and target extents for one live array; positive whole-element delta; exact data/CFG/physical relocations/siblings; and Graphify-bound producer maxima that all equal the computed capacity | Live array capacity implied independently by target extent and producer contract |
| switch-case FPR lifetimes | Indirect switch dispatch; larger target frame corroborated by the reducer's uniform stack-home delta; larger target function; and at least three target-only call-result copies into nonvolatile FPRs | Used floating-point result locals scoped to individual switch cases |
| aggregate self-copy at final consumer | A target-only three-or-more-component aggregate load/store self-copy, no later call after its bounded final consumers, and an explicitly named same-report donor that is 100% on both sides with the same copy signature | Used aggregate self-assignment at the final-consumer boundary |

These joins preserve the reviewed boundaries from
`ev_CapTeresaFadeMatHook`, `ev_CapMiracleCoinTrade`,
`mbev_CapKuribo`, `mbev_CapKokamekku`, `ev_CapBobleOMExec`, and
`mbev_CapBomheiMove`. The names identify acceptance
fixtures, not symbol-specific allowlists: another function must reproduce the
same physical signature.

## False-positive gates

The focused fixtures reject the tempting incomplete variants:

- an almost-identical branch tail whose two target exits do not share one
  epilogue;
- a saved-GPR permutation without a call-result assignment immediately
  consumed by a condition;
- a two-register swap without exact data/relocation/frame/sibling receipts,
  without exactly matching VarInfo owner colors, or with an unsealed source
  boundary;
- an unaligned, zero/negative, or internally inconsistent stack extent; a used
  prefix larger than the candidate capacity; or producer maxima that disagree
  with the computed target capacity;
- a loop-destination claim with multiple residual rows, a non-conditional or
  relocated branch, wrong relative targets, an inexact frame, or destination
  classes other than candidate increment versus target exit;
- FPR captures and a frame delta without indirect switch dispatch;
- a final-consumer aggregate self-copy whose named donor is not exact.

Other structural differences also close a rule. The assignment/condition rule
rejects inserts, deletes, opcode or relocation changes, inconsistent mappings,
volatile-register differences, and two-register swaps. The allocator rule
rejects inserts, deletes, non-register differences, partial/three-way cycles,
volatile registers, owner/register disagreement, unknown context fields, false
proof gates, report/context hash disagreement, and malformed or incomplete
interaction observations. The switch rule rejects
an uncorroborated or excessively large frame delta. The aggregate rule rejects
early copies, unequal load/store offset multisets, non-final consumers, unnamed
donors, and donor signature drift.

## Output and authority boundary

The document binds canonical objdiff input, donor names, this implementation,
and the installed reducer by SHA-256. `diagnosis_sha256` hashes the complete
document excluding that field. Context-bound paths additionally bind canonical
allocator, capacity, or branch evidence by SHA-256; the allocator path also
binds the installed interaction planner. Rule order and JSON serialization are
stable.

No diagnosis establishes semantic names, original spelling, or source
provenance. It never authorizes a source edit, candidate retention, promotion,
or recovery-state change. Strict text/data, physical relocation, section,
consumer, protected-sibling, linked-container, and checksum proof remain
separate gates.

## Required cracking-lane workflow

Use Graphify first for source locations, producer/consumer contracts, and
same-game provenance. Use Graft next for installed tool symbols and dependency
paths. Only then perform a narrow named-file verification. Do not run broad
recursive searches over `.codex`, all worktrees, or unrelated build roots.

The detector is a cracking aid, not a Manager intake format. Working lanes make
their own admissibility and retain/reject decisions, run the complete proof
gates, and send the Manager only a completed `CRACK_REPORT/v1`. Do not send
source-policy requests, tooling requests, status packets, or acknowledgment
requests as substitutes for finished cracking work. Each completed report must
include measured active seconds and must fail closed on telemetry completeness;
an incomplete interval is reported and excluded from measured crack/hour rather
than imputed.
