# CRACK_REPORT learning rules

`tools/crack_learning_rules.py` turns reviewed function- and owner-level lessons into
deterministic, read-only diagnoses. It composes the installed causal objdiff
reducer and emits self-hashed `crack_learning_diagnosis/v11` JSON with
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

When source and candidate bytes are already fixed but a target metadata repair
changes only relocation label/addend attribution, audit object ownership before
editing C:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/player-biriq.corrected.strict.json \
  --function PlayerBiriQOMExec \
  --metadata-owner-context work/player-biriq.metadata-owner-context.json
```

The closed `metadata_owner_coherence_context/v1` binds the corrected canonical
objdiff and before/after target metadata, requires the source and candidate
object to stay unchanged, enumerates every removed interior byte label within
nonoverlapping typed data objects, and requires equal relocation row counts,
zero effective-target changes, unchanged payload sections, zero protected loss,
and exact linked retail proof. A match ranks a target metadata object-extent
audit before any source-shape experiment. It never authorizes metadata edits:
the split and strict/data/physical-relocation/section/linked-retail proof must be
rerun independently.

This rule is fail-closed. It rejects a stale report hash, a focus function not
sealed by the correction, changed function bytes, overlapping or incomplete
object extents, a missing interior byte label, a relocation row-count change,
any effective-target change, or a rebinding count that differs from the removed
labels. The Player BiriQ acceptance case merges three four-byte `.sdata` objects
and removes exactly nine interior labels while preserving all 2,249 physical
relocation rows and the exact linked image; both `PlayerBiriQOMExec` and
`mbPlayerBiriQSet` use the same correction and must not be implemented as two
source-learning rules.

When typed pool-owner rows are disjoint from live-range and comparison rows,
compose the installed pool decoder evidence before compiling another cell:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/glow-baseline.strict.json \
  --function mbev_CapEffGlowOMExec \
  --pool-live-range-context work/glow.pool-live-range-context.json
```

When an otherwise exact float guard differs only in field/zero `lfs` order,
bind the two rows, the exact `fcmpu`/branch consumers, the object-identical
explicit-comparison control, and an exact truthiness precedent:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/starman-near-exact.strict.json \
  --function mbev_CapStarManOMExec \
  --float-truthiness-context work/starman.float-truthiness-context.json
```

The resulting cell ranks `if (field)` first and suppresses both explicit
`field != 0.0f` operand orders. It remains diagnostic-only.

The closed `pool_live_range_interaction_context/v1` binds the canonical objdiff,
typed-pool decoder receipt and same-TU owner receipt; partitions every residual
into disjoint live-range, two-row float-comparison, and SDA21 pool-owner groups;
and records the measured size-exact precursor plus the first exact combined
cell. The rule verifies value-equivalent relocation aliases itself and schedules
one natural combined cell. It never guesses a literal owner or duplicates the
pool decoder or interaction planner.

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

The planner normally orders control, single-axis, then interaction cells. When
independent target evidence has already closed every axis and the shortest path
is the fully composed natural source, add an ordered `priority_selections`
array to the request. Each entry must name every axis exactly once and may use
only `natural` levels. A prioritized cell is rejected if it is constrained,
blocked, already measured, or a duplicate topology; the default order is
unchanged when the field is absent. For example, the closed
PlayerMoveIdleCreate evidence prioritizes its allocation chain, complete owner
cycle, and narrow-consumer normalization together:

```json
"priority_selections": [
  {
    "allocation_chain": "chained",
    "owner_chronology": "target",
    "narrow_consumer": "int"
  }
]
```

Use `recommended_execution_order` as the authoritative compile order, then
record the resulting source, object, attestation, and reports in the matching
workbench before editing another cell. Priority changes scheduling only; it
does not generate source, relax evidence gates, or advance retention authority.

When one swapped owner is a function parameter and the other is an allocation
result that the target explicitly preserves, use the parameter-aware context
instead of the generic allocator factorial:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/eject-c001.strict.json \
  --function mbev_CapPlayerMoveEjectCreate \
  --parameter-allocation-context work/eject-c001.parameter-allocation-context.json \
  > work/eject-c001.learning.json
```

`parameter_allocation_consumer_chain_context/v1` seals exact size/frame/data/
CFG/physical-relocation/sibling gates, the parameter and allocation-result
colors, the exact allocation call and immediately following `mr` capture, and
the ordered field-store/typed-pointer-copy rows. The rule verifies a complete
two-saved-GPR swap but never proposes redeclaring the parameter. If the target
captures `r3` into the allocation-result owner, it also suppresses direct
producer fusion and ranks only the natural right-associative consumer chain.
The owner and field identifiers come from the authenticated context rather than
from a hard-coded function template. This lets an exact same-TU precedent such
as PlayerMoveEject transfer to BonusCoin's `process->property` consumer while
keeping the physical allocation/capture/consumer proof mandatory for the new
function.

Minimal context:

```json
{
  "schema": "parameter_allocation_consumer_chain_context/v1",
  "proofs": {
    "objdiff_canonical_sha256": "<64 lowercase hex>",
    "function_size_exact": true,
    "stack_frame_exact": true,
    "data_values_exact": true,
    "physical_relocations_exact": true,
    "cfg_calls_exact": true,
    "protected_siblings_preserved": true,
    "strict_report_sha256": "<64 lowercase hex>",
    "data_report_sha256": "<64 lowercase hex>",
    "physical_relocation_receipt_sha256": "<64 lowercase hex>",
    "trace_receipt_sha256": "<64 lowercase hex>",
    "source_boundary_receipt_sha256": "<64 lowercase hex>",
    "same_tu_donor_receipt_sha256": "<64 lowercase hex>"
  },
  "owners": {
    "parameter": {
      "name": "playerNo",
      "target_register": "r29",
      "candidate_register": "r28",
      "evidence_sha256": "<64 lowercase hex>"
    },
    "allocation_result": {
      "name": "workData",
      "target_register": "r28",
      "candidate_register": "r29",
      "evidence_sha256": "<64 lowercase hex>"
    }
  },
  "producer": {
    "call_name": "HuMemDirectMallocNum",
    "call_row": 24,
    "capture_row": 25,
    "return_register": "r3",
    "preserve_explicit_identity": true,
    "evidence_sha256": "<64 lowercase hex>"
  },
  "consumer_chain": {
    "typed_pointer": "workP",
    "field_owner": "obj",
    "field_name": "data",
    "allocation_result": "workData",
    "evaluation_order": ["field_store", "typed_pointer_copy"],
    "consumer_rows": [26, 27],
    "evidence_sha256": "<64 lowercase hex>"
  }
}
```

For an exact-size register-only cycle caused by repeated complete member-wise
copies of one aggregate parameter, supply a closed aggregate-use context:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/explodekiller-baseline.strict.json \
  --function mbev_CapEffExplodeKillerAdd \
  --aggregate-use-context work/explodekiller.aggregate-use-context.json \
  > work/explodekiller.learning.json
```

`aggregate_use_multiplicity_context/v1` binds the canonical report, exact
size/frame/data/CFG/physical-relocation/sibling gates, the complete saved-GPR
cycle, the aggregate type and ordered fields, and every complete copy group.
The rule ranks only `destination = *source` for those sealed groups. It never
removes or rewrites separately authenticated consumers; BoostAdd's independent
`particleWorkP->alpha = color->a` read is preserved while only its full color
copy is reconstructed. Input aliases and parameter declaration-order shaping
remain suppressed. A prior same-owner exact crack is ranking evidence only;
the new function still needs its own closed physical and source-use receipts.

Minimal source-use portion (all proof receipts are also required):

```json
{
  "schema": "aggregate_use_multiplicity_context/v1",
  "proofs": {
    "objdiff_canonical_sha256": "<64 lowercase hex>",
    "function_size_exact": true,
    "stack_frame_exact": true,
    "data_values_exact": true,
    "physical_relocations_exact": true,
    "cfg_calls_exact": true,
    "protected_siblings_preserved": true,
    "strict_report_sha256": "<64 lowercase hex>",
    "data_report_sha256": "<64 lowercase hex>",
    "physical_relocation_receipt_sha256": "<64 lowercase hex>",
    "source_use_receipt_sha256": "<64 lowercase hex>",
    "trace_receipt_sha256": "<64 lowercase hex>",
    "exact_precedent_receipt_sha256": "<64 lowercase hex>"
  },
  "owners": [
    {"name":"pos","target_register":"r31","candidate_register":"r30","evidence_sha256":"<64 lowercase hex>"},
    {"name":"vel","target_register":"r30","candidate_register":"r29","evidence_sha256":"<64 lowercase hex>"},
    {"name":"color","target_register":"r29","candidate_register":"r31","evidence_sha256":"<64 lowercase hex>"}
  ],
  "aggregate_parameter": {
    "name":"color", "type":"GXColor", "fields":["r","g","b","a"],
    "target_register":"r29", "candidate_register":"r31",
    "evidence_sha256":"<64 lowercase hex>"
  },
  "copy_groups": [
    {"destination":"color1","destination_type":"GXColor","source":"color","fields":["r","g","b","a"],"consumer":"mbev_CapEffExplodeAdd","evidence_sha256":"<64 lowercase hex>"}
  ],
  "independent_consumers": [],
  "rejected_axes": []
}
```

When aggregate reconstruction leaves exactly one typed two-owner saved-GPR
swap, use a separate post-aggregate context rather than overloading the
aggregate-copy rule:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/electricadd002.strict.json \
  --function mbev_CapEffElectricAdd \
  --aggregate-followup-context work/electricadd.aggregate-followup-context.json \
  > work/electricadd.learning.json
```

`aggregate_two_owner_followup_context/v1` seals the exact size/frame/data/CFG/
physical-relocation/sibling gates, both typed owner colors, the aggregate
expression already applied, the bounded declaration order, and a separately
measured expression-fusion regression. The detector accepts only one complete
two-register swap. If fusion changed size/topology and regressed strictness, it
schedules exactly the declaration-only, split-expression cell and suppresses
both fused cells. This prevents the ElectricAdd failure mode where combining a
good declaration axis with a rejected fusion axis grew the function.

For a target that names a saved pointer to an address-taken local while the
candidate passes `&local` directly, provide the physical owner seam explicitly:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/glowkinokoalt-baseline.strict.json \
  --function mbev_CapEffGlowKinokoAddAlt \
  --address-taken-context work/glowkinokoalt.address-taken-context.json \
  > work/glowkinokoalt.learning.json
```

`address_taken_local_pointer_context/v1` binds the report, exact CFG/data/
physical-relocation/sibling receipts, the measured size delta, local aggregate
stack offset, incoming pointer colors, saved local-address color, typed argument
register/callee, and target-only parameter home. The detector independently
requires the candidate's direct `addi argument,r1,offset`, the target's saved
`addi` plus `mr` into that same argument, the incoming-owner move on each side,
one larger target frame, and one unique target-only home. It then ranks exactly
one live typed pointer at the consumer boundary and suppresses dead pointer
storage, declaration-only edits, and artificial lifetime extension.

When three small residual groups already exist as exact same-TU and caller
contracts, bind them in one closed retrieval context instead of rediscovering
each source shape independently:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/electricmodelset-baseline.strict.json \
  --function mbev_CapEffElectricModelSet \
  --same-tu-shape-context work/electricmodelset.same-tu-shape-context.json \
  > work/electricmodelset.learning.json
```

`same_tu_exact_sibling_shape_context/v1` seals exact data/CFG/physical-
relocation/sibling proof, exact strict/data status for the named same-TU donor,
and an authenticated caller contract. The detector then independently requires
the target-only `li/srawi/srwi/subfc/adde` fixed-array tail, a candidate-only
`extsh` whose result and the target argument register feed the same store, and
one target f32 load feeding three reverse-order stores where the candidate used
three loads. It rejects any residual outside those groups and emits exactly one
combined cell: the donor expression, the wide callee view while preserving the
narrow producer, and the right-associative zero chain. It never treats a donor
name, a guessed prototype, or a literal-load resemblance as sufficient proof.

For two call-valued mask tests whose target branches converge on one shared
Boolean result, bind the exact call and branch destinations before testing
source spellings:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/masulink-baseline.strict.json \
  --function mbev_CapMasuLinkNextGet \
  --short-circuit-context work/masulink.short-circuit-context.json \
  > work/masulink.learning.json
```

`short_circuit_boolean_call_order_context/v1` seals the canonical report,
exact data/physical-relocation/sibling receipts, both target call pairs, the
shared `bne`/`beq` true/false destinations, the baseline's duplicated true
assignments, and one later exact-topology four-owner GPR cycle. The detector
requires the target to call each branch getter before its masu getter while the
C expression writes the masu getter first, matching the pinned MWCC frontend's
right-to-left operand evaluation. It schedules the explicit shared `if/else`
cell first and permits exactly one sealed declaration-chronology follow-up only
after topology is exact. Direct Boolean assignment, call-order guessing, dead
Boolean temporaries, and early declaration permutations remain suppressed.

When an adjacent exact sibling has the same dependency graph and complete
Boolean/call-order transformation, transfer that semantic source before fresh
permutation and solve only independently visible residual boundaries:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/masurandom-baseline.strict.json \
  --function mbev_CapMasuLinkNextRandomGet \
  --exact-sibling-transfer-context work/masurandom.exact-sibling-transfer.json \
  > work/masurandom.learning.json
```

`dependency_equivalent_exact_sibling_transfer_context/v1` requires strict/data
exactness for the distinct sibling, a dependency-graph receipt, the same sealed
target and candidate call-order inversion, the shared Boolean destinations,
and every target-only adjacent `extsh`/s16-consumer pair. A separate capacity
receipt must bind the live array macro and byte extent. Only when all joins
agree does the detector schedule one combined cell: the sibling's semantic
Boolean source, the independently proved `int` owner, and the authenticated
array capacity. It suppresses fresh CFG permutations, declaration-order probes,
narrow owner guesses, and capacity guessing.

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

The reciprocal-source-shape rule likewise requires a closed evidence context:

```sh
rtk python tools/crack_learning_rules.py \
  --report build/GP6E01/reports/explode-small001.strict.json \
  --function mbev_CapEffExplodeOMExec \
  --reciprocal-context work/explode-small001.reciprocal-context.json \
  > work/explode-small001.learning.json
```

`reciprocal_source_shape_context/v1` seals exact size/data/CFG/physical-
relocation/sibling gates, a typed-literal receipt, the zero-based rows for two
invariant constants plus the swapped variable/reciprocal loads and exact
`fmuls`, and a measured compiler-neutral commuted-multiply control. The rule
independently recomputes the f32 reciprocal bits, accepts only a power-of-two
denominator, verifies that no physical residual exists outside the sealed
window, and requires the commuted control objects to be byte-identical. It then
ranks exactly one natural `/ N.0f` cell and suppresses further commutative
multiply permutations.

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

Minimal reciprocal context shape:

```json
{
  "schema": "reciprocal_source_shape_context/v1",
  "proofs": {
    "objdiff_canonical_sha256": "<64 lowercase hex>",
    "function_size_exact": true,
    "data_values_exact": true,
    "physical_relocations_exact": true,
    "cfg_calls_exact": true,
    "all_non_window_rows_exact": true,
    "protected_siblings_preserved": true,
    "strict_report_sha256": "<64 lowercase hex>",
    "data_report_sha256": "<64 lowercase hex>",
    "physical_relocation_receipt_sha256": "<64 lowercase hex>",
    "typed_constant_receipt_sha256": "<64 lowercase hex>",
    "neutral_observation_receipt_sha256": "<64 lowercase hex>"
  },
  "window": {
    "invariant_constant_rows": [95, 96],
    "target_variable_row": 97,
    "candidate_variable_row": 98,
    "target_reciprocal_row": 98,
    "candidate_reciprocal_row": 97,
    "multiply_row": 99,
    "denominator": 16,
    "reciprocal_f32_bits": "3d800000"
  },
  "neutral_observation": {
    "axis": "commuted_multiply",
    "baseline_object_sha256": "<64 lowercase hex>",
    "candidate_object_sha256": "<same 64 lowercase hex>"
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
`parameter_allocation_context=`, `aggregate_use_context=`,
`aggregate_followup_context=`, `address_taken_context=`,
`same_tu_shape_context=`, `short_circuit_context=`,
`exact_sibling_transfer_context=`, `capacity_context=`, `branch_context=`, or
`reciprocal_context=`, `pool_live_range_context=`, or
`float_truthiness_context=`. The same closed validation used by the CLI is applied
to Python callers.

## Installed-functionality audit

The explicit else-return recognizer already belongs to
`tools/mismatch_cluster_audit.py`. This module calls that reducer and consumes
its exact `explicit_else_return_epilogue` hypothesis; it does not duplicate the
CFG detector. The existing reducer also remains the owner of generic stack,
aggregate, branch, ABI, and relocation clustering.

The seventeen joins are narrower than those generic classifications:

| Rule | Required evidence join | Natural source class |
|---|---|---|
| explicit else-return CFG | Installed reducer's conditional-to-second-exit, adjacent shared-epilogue branches, and candidate direct-epilogue signature | Explicit else-return control flow |
| loop branch destination | Equal size/frame; exactly one relocation-identical conditional branch residual; sealed target/candidate relative destinations classified as target loop exit versus candidate loop increment; exact data, physical relocations, and siblings | Explicit else-break for an authenticated zero terminator |
| assignment/condition saved-GPR cycle | Equal function size; identical operations, relative branches, relocations, and non-register operands; a closed cycle of at least three nonvolatile GPRs; and a call result copied to the cycled register immediately before comparison and conditional branch | Assignment in its consuming condition |
| allocator two-register swap interaction | Equal function size and measurable frame; identical operations, relative branches, relocations, immediates, data values, physical relocations, and protected siblings; exactly one closed two-nonvolatile-GPR swap; exact VarInfo owner-to-register mapping; and one authenticated producer/consumer identity boundary | A bounded 2x2 interaction of natural declaration chronology and natural producer/consumer expression fusion |
| parameter/allocation consumer chain | Equal size/frame; a complete parameter versus allocation-result saved-GPR swap; exact data/CFG/physical relocations/siblings; an authenticated allocation call followed immediately by a saved-owner `mr`; and adjacent field-store then typed-pointer-copy consumers | Preserve the explicit allocation-result identity and fuse only its consumers as a right-associative assignment |
| aggregate-use multiplicity | Equal size/frame; exact operations/CFG/data/physical relocations/siblings; one complete two-or-more saved-GPR ownership cycle; a sealed live aggregate parameter; and one or more complete ordered same-type member-copy groups | Replace only each complete member-wise group with a natural aggregate assignment while preserving unrelated same-owner consumers |
| aggregate two-owner follow-up | Aggregate reconstruction already applied; equal size/frame and exact CFG/data/physical relocations/siblings; exactly one sealed typed two-saved-GPR swap; and a measured expression-fusion cell that changes size/topology and regresses strictness | Keep split producer/consumer expressions and compile only the authenticated declaration-order cell; never combine it with the rejected fusion axis |
| address-taken local pointer consumer | Exact CFG/data/physical relocations/siblings; a bounded positive size delta and larger target frame; matching local stack address; target saved-address materialization/copy versus candidate direct argument materialization; authenticated incoming-owner colors; and one target-only parameter home | Introduce one live typed pointer to the already live local aggregate immediately before the typed consumer |
| same-TU exact-sibling source shapes | Exact data/CFG/physical relocations/siblings; exact same-TU donor and caller receipts; one target-only fixed-array Boolean lowering; one candidate-only callee `extsh` feeding the same consumer store; one-load reverse-order aggregate zero stores; and no residual outside those groups | Compile one combined cell using the exact donor expression, authenticated wide callee view, and right-associative zero chain |
| short-circuit Boolean call order | Larger baseline candidate; two sealed target call pairs in branch-getter then masu-getter order; target `bne`/`beq` destinations converging on one true/false assignment pair; duplicated candidate true assignments; pinned MWCC frontend receipt; and one later exact-topology closed four-owner GPR cycle | Explicit shared `if/else` with source-commuted AND operands, followed only by the sealed declaration chronology after topology is exact |
| dependency-equivalent exact-sibling transfer | Distinct strict/data-exact sibling; authenticated equivalent dependency graph and capacity; the same sealed target/candidate call-order inversion and shared Boolean destinations; and three target-only adjacent `extsh`/s16-consumer pairs normalizing one owner | Transfer the sibling's semantic Boolean source and combine it only with the independently proved `int` owner and live array capacity in one cell |
| typed pool/live-range interaction | Larger-target baseline; exact data/CFG/physical relocations/siblings; disjoint sealed live-range, two-row `lfs` comparison, and value-equivalent SDA21 owner groups; authenticated same-TU named f32 owner; and measured size-exact precursor/exact cells | Reuse the live temporaries and preincrement, use natural float truthiness, and bind the authenticated pool owner in one combined cell |
| float truthiness comparison ranking | Equal size/frame and exact CFG/data/physical relocations/siblings; exactly two swapped field/zero `lfs` rows feeding an exact adjacent `fcmpu`/conditional branch; an object-identical commuted explicit-zero control; and an authenticated exact truthiness precedent | Compile `if (field)` first and suppress both explicit zero-comparison operand orders |
| stack extent/interface capacity | Equal function size; sealed candidate and target extents for one live array; positive whole-element delta; exact data/CFG/physical relocations/siblings; and Graphify-bound producer maxima that all equal the computed capacity | Live array capacity implied independently by target extent and producer contract |
| reciprocal source shape | Equal function size; exact data/CFG/physical relocations/siblings; one typed power-of-two reciprocal; variable and reciprocal f32 loads swapped around an exact `fmuls`; no residual outside the sealed window; and an object-identical commuted-multiply control | Natural division by the exact denominator, with further commutative permutations suppressed |
| switch-case FPR lifetimes | Indirect switch dispatch; larger target frame corroborated by the reducer's uniform stack-home delta; larger target function; and at least three target-only call-result copies into nonvolatile FPRs | Used floating-point result locals scoped to individual switch cases |
| aggregate self-copy at final consumer | A target-only three-or-more-component aggregate load/store self-copy, no later call after its bounded final consumers, and an explicitly named same-report donor that is 100% on both sides with the same copy signature | Used aggregate self-assignment at the final-consumer boundary |

These joins preserve the reviewed boundaries from
`ev_CapTeresaFadeMatHook`, `ev_CapMiracleCoinTrade`,
`mbev_CapKuribo`, `mbev_CapPlayerMoveEjectCreate`, `mbev_CapKokamekku`,
`mbev_CapEffExplodeOMExec`, `mbev_CapEffGlowKinokoAddAlt`,
`mbev_CapEffElectricModelSet`, `mbev_CapMasuLinkNextGet`,
`mbev_CapMasuLinkNextRandomGet`,
`mbev_CapEffGlowOMExec`,
`mbev_CapStarManOMExec`,
`ev_CapBobleOMExec`, and
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
- a parameter/allocation claim without an immediate target `r3` capture, with
  a different owner mapping, with non-adjacent or reversed consumer rows, or
  with any structural residual beyond the complete saved-GPR swap;
- a post-aggregate two-owner claim without an exact physical swap, without the
  already-applied aggregate receipt, with a fusion observation that preserved
  size/topology, or with a declaration order naming anything beyond the two
  sealed typed owners;
- an address-taken pointer claim with a non-unique consumer, no larger target
  frame, a mismatched aggregate offset, no saved-address-to-argument copy,
  absent incoming-owner colors, or a candidate that already has the target
  parameter home;
- a same-TU source-shape claim with an inexact donor/caller receipt, a different
  array bound or donor expression, a missing Boolean-lowering instruction, no
  candidate-only `extsh`, nonmatching consumer homes, more than one target zero
  load, or any residual outside the three sealed groups;
- a short-circuit Boolean claim with reversed or missing target calls, source
  operands that do not encode the pinned frontend's right-to-left call order,
  branch destinations that do not converge on one true/false pair, absent
  duplicated candidate true assignments, a false frontend proof, or a topology
  observation that is not one closed four-owner GPR cycle;
- an exact-sibling transfer whose donor is the focus function or is not
  strict/data exact, whose dependency graph or capacity is unauthenticated,
  whose donor expressions differ from the sealed mask tests, whose baseline
  call/branch topology drifts, or whose three target-only `extsh` rows are not
  adjacent to the named s16 consumers and sourced from one owner;
- a typed pool/live-range claim whose row groups overlap, whose comparison is
  not exactly two non-relocated `lfs` rows, whose pool relocations are not
  value-equivalent SDA21 owner aliases, whose named owner is absent from the
  target rows, whose precursor retains anything beyond comparison/pool rows,
  or whose report has a residual outside the sealed groups;
- a float-truthiness claim whose two `lfs` accesses are not exact reversals,
  whose `fcmpu`/branch consumers drift, whose explicit comparison control is
  not object-identical, whose precedent is unauthenticated, or whose report has
  any residual outside the two sealed loads;
- an unaligned, zero/negative, or internally inconsistent stack extent; a used
  prefix larger than the candidate capacity; or producer maxima that disagree
  with the computed target capacity;
- a loop-destination claim with multiple residual rows, a non-conditional or
  relocated branch, wrong relative targets, an inexact frame, or destination
  classes other than candidate increment versus target exit;
- a reciprocal claim with a non-power-of-two denominator, wrong f32 bits,
  mismatched relocation types, a non-exact `fmuls`, any residual outside the
  sealed window, or a commuted control whose objects differ;
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
allocator, parameter/allocation, typed-pool/live-range, capacity, branch, or
reciprocal evidence by SHA-256; the allocator path also binds the installed
interaction planner, and the typed-pool path binds the installed pool decoder.
Rule order and JSON serialization are stable.

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
