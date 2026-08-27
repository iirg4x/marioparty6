# D-form aggregate/scalar-owner composer

`tools/dform_scalar_owner_composer.py` turns a sealed four-stage matching-decomp campaign into a bounded two-cell plan. It is intended for functions where the target proves all of these independent causes at once:

- a truthful standard-library or threshold boundary;
- a typed `HuVecF` copy lowered as `psq_l`, `lfs`, `psq_st`, `stfs`;
- reuse of a real scalar parameter plus one live scalar owner;
- authenticated typed-pool ownership; and
- one final multiplication-operand-order seam.

The tool is diagnostic. It emits no source patch and never authorizes retention, promotion, or authority advancement.

## Required evidence

The input context uses schema `dform_scalar_owner_composer_context/v1` and hash-binds:

1. a completed `CRACK_REPORT/v1`;
2. four immutable `focus_symbol_report/v1` artifacts:
   `structural`, `dform`, `owner_pool`, and `exact`;
3. source and object SHA-256 identities for every stage; and
4. an independent physical-relocation summary.

Every semantic axis must cite the completed report SHA-256. The exact source shape remains an owner/orchestrator decision; the context only records already authenticated evidence.

The analyzer fails closed unless all of the following hold:

- each focus artifact passes its internal self-hash and external file-hash binding;
- all artifacts name the same function and keep authority fields false;
- strict and data metrics match both target and candidate compact-row counts;
- residual counts decrease monotonically and only the structural stage has a size deficit;
- target instruction content and protected exact-sibling identities remain stable;
- the target D-form sequence is absent from the structural candidate but exact in all later stages;
- every owner/pool-stage relocation mismatch lies inside the sealed final row set;
- the exact stage has zero strict/data rows; and
- the independent physical receipt reports equal counts and zero differences.

Raw target/candidate symbol-table indices and absolute branch addresses are not compared directly. Zero-row objdiff proof owns those semantics, while the composer separately verifies opcode order and the independent physical relocation receipt.

## Output

A successful diagnosis uses schema `dform_scalar_owner_composer/v1`, status `READY`, and exactly two ordered cells:

1. `compose_abs_dform_scalar_pool` — combine the independently authenticated threshold, D-form helper, scalar owners, and typed-pool owners before compiling.
2. `commute_final_multiply_operands` — run only the sealed target/control expression pair when the first cell leaves exactly the expected arithmetic rows.

Partial structure-only, D-form-only, scalar-only, pool-only, declaration-permutation, tracer, and unsealed operand probes are suppressed.

## CLI

```powershell
python tools/dform_scalar_owner_composer.py `
  context.json `
  structural.focus.json `
  dform.focus.json `
  owner_pool.focus.json `
  exact.focus.json `
  physical-relocations.json `
  CRACK_REPORT_v1.md `
  --expect-context-sha256 <sha256> `
  --output diagnosis.json `
  --pretty `
  --require-ready
```

Exit code `2` means an evidence gate failed. No fallback or partial recommendation is emitted.

## `mbPauseGuideMoveSet` acceptance replay

The bound report SHA-256 is `399f14e0f39756ea1d9767521adfff37d6c911f238873d460bea809198e56081`.

The compact replay proves:

| Stage | Candidate bytes | Strict rows | Data rows |
|---|---:|---:|---:|
| structural | 980/1012 | 102 | 89 |
| D-form | 1012/1012 | 55 | 38 |
| scalar-owner/pool | 1012/1012 | 5 | 5 |
| exact | 1012/1012 | 0 | 0 |

The sealed D-form rows are 27–30 and contain the target `psq_l/lfs/psq_st/stfs` copy. The final seam is rows 99, 100, 106, 107, and 108; all remaining relocation annotations are within that set. The independent physical receipt is 53/53 with zero differences, and 47 protected exact siblings are unchanged.

The successful diagnosis self-hash is `8f6aea9f704b91104383545c8c1876dc983fc5c7a565b85550b37d4c58446355`; its file SHA-256 is `b9729bbbdaf4276ca9f6083f47d4062bddc763a08e546f197ce14c918b65573e`.

## Tests

```powershell
python -m unittest tools.tests.test_dform_scalar_owner_composer -v
```

Tests cover the successful two-cell plan, deterministic output, CLI file-hash binding, malformed context, artifact tampering, missing D-form lowering, final-row drift, protected-sibling loss, and physical-relocation differences.
