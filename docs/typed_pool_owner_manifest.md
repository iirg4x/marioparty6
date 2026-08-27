# Typed pool owner manifest and first-pass triage

Installed implementation checkpoint:
`bef1a6e2e113cdf7d564450b78c8fa9c3348de6e`.

Owning files:

- `tools/typed_pool_owner_manifest.py`
- `tools/crack_first_pass.py`
- `tools/tests/test_typed_pool_owner_manifest.py`
- `tools/tests/test_crack_first_pass.py`

These read-only tools remove the manual grouping step for one common MWCC
residual. They never edit source or launch a compiler. A match means only that
one named-owner binding cell is justified for owner review and compilation.

## P0 first-pass command

Run this command immediately after producing strict and data objdiff reports:

```text
rtk C:\Python313\python.exe tools\crack_first_pass.py ^
  STRICT.json DATA.json FUNCTION ^
  --target-object TARGET.o ^
  --candidate-object CANDIDATE.o ^
  --expect-strict-report-sha256 STRICT_SHA256 ^
  --expect-data-report-sha256 DATA_SHA256 ^
  --expect-target-object-sha256 TARGET_OBJECT_SHA256 ^
  --expect-candidate-object-sha256 CANDIDATE_OBJECT_SHA256 ^
  --output build\FUNCTION.first-pass.json
```

The `crack_first_pass/v1` result selects exactly one of these routes:

1. `typed_pool_owner_manifest`: one owner-binding candidate, no trace;
2. `typed_pool_decoder`: pool-only semantic/type/chronology reduction, at most
   three evidence-composed candidates and no trace;
3. `causal_reducer`: structural/ABI/aggregate reduction, at most three
   candidates and at most one late trace; or
4. `causal_reducer_then_typed_pool_decoder`: close structure before pool rows.

Every route has an analysis deadline of five minutes, no more than three
actions, and `authority_advanced:false`.

## Direct manifest command

Use the specialist directly when a pool-owner-only residual is already
suspected:

```text
rtk C:\Python313\python.exe tools\typed_pool_owner_manifest.py ^
  STRICT.json DATA.json FUNCTION ^
  --target-object TARGET.o ^
  --candidate-object CANDIDATE.o ^
  --expect-strict-report-sha256 STRICT_SHA256 ^
  --expect-data-report-sha256 DATA_SHA256 ^
  --expect-target-object-sha256 TARGET_OBJECT_SHA256 ^
  --expect-candidate-object-sha256 CANDIDATE_OBJECT_SHA256 ^
  --output build\FUNCTION.typed-owner-manifest.json ^
  --require-match
```

`--require-match` exits nonzero if any closed gate fails. Without it, a
well-formed but ineligible input emits a deterministic `status:"blocked"`
receipt and its blocker list.

## Schemas and hashes

The manifest contains a closed `typed_pool_owner_manifest_binding/v1` object
with the four artifact paths and verified SHA-256 values. Its top-level schema
is `typed_pool_owner_manifest/v1` and its self-hash is `manifest_sha256`,
computed over canonical JSON before that field is added. The first-pass schema
is `crack_first_pass/v1`; `triage_sha256` uses the same rule.

A matched owner entry records:

- target name, owner class, section, offset, width, bytes, decoded type/value;
- the compiler-anonymous candidate owner;
- every exact function-relative row and target/candidate instruction address;
- target/candidate instruction text, opcode, destination, relocation type and
  addend; and
- `source_site_status:instruction_site_bound_source_span_unresolved`.

The single candidate cell says to bind all listed owners at only those
semantic consumers. It deliberately sets `source_patch_emitted:false` and
`owner_source_site_confirmation_required:true` because objdiff does not prove
the original C declaration or source span.

## Authenticated inputs and trust boundary

The caller supplies an authenticated retail target object, the exact candidate
object, and their strict/data reports. The tool hashes all four files and
rejects a mismatch against the expected values before decoding. Objdiff target
pairing and the target object's named owners authenticate the machine-level
owner identity. The owner lane remains responsible for confirming each row has
a truthful semantic source consumer.

The tools do **not** prove:

- an original C declaration, linkage spelling, `const` qualifier, or source
  span;
- physical relocation equality after the candidate is compiled;
- protected-sibling preservation, source admissibility, retention, owner
  closure, linked output, or promotion; or
- that an invented label, seeder, padding object, or declaration permutation is
  acceptable.

Both outputs set `retention_authorized:false`,
`promotion_authorized:false`, and `authority_advanced:false`.

## Closed match contract

A manifest matches only when all conditions hold:

1. strict target/candidate function sizes are equal;
2. the data report is size-equal and 100 percent for the function;
3. at least one strict residual exists and every strict residual row is paired
   by the typed-pool decoder;
4. every row is `owner_identity_mismatch` with no difference outside target
   owner name/offset;
5. target and candidate use identical typed bytes, width, consumer type,
   `.sdata2` section, relocation type and addend;
6. the relocation is `R_PPC_EMB_SDA21`;
7. target owner class is `named_label` or `named_object`, while the candidate is
   `compiler_anonymous`;
8. both instructions are supported SDA21 loads with the same opcode and
   destination; and
9. each target owner has one unambiguous typed contract and candidate owner.

Any value, type, section, addend, relocation, consumer, instruction, function
size, data, non-pool, unresolved, or ambiguity drift blocks the manifest.
Numeric searches, seeders, invented labels, CFG edits, declaration-order
permutations, register shaping, and tracing are explicitly suppressed.

## ConfigPadClose acceptance replay

The real c109 baseline is bound by:

- strict report `91698007465222ff5976694d5071a621173cae430c2d9c5634e9fb18faf32975`;
- data report `97af8d660dc5d6d316ad22e7fea1fc4e8562b7f6d52a3659790d72e052634e7a`;
- retail target object `dde815bf17d9acaf93453a1d862368cc21d4220bd3ff3ab9bce66bd1ec618762`;
- candidate object `312bdc9e28970fdc33f9cf2970c067b4509fd0db2f88705e35f68cb4f3ceddd8`.

The replay emits exactly 11/11 owner-only rows, seven named target owners, one
candidate cell, candidate budget one, and trace budget zero. The receipt file
SHA-256 values are:

- manifest file: `427eb759c8610edc19f2e55cd3a8056ea91e9f07ec8843823e5c79b1d8b6f232`;
- manifest internal: `41eea1907af4d84737afae21a5cb13416e5ebf609a2694bc8a2e38741a0bb7c3`;
- first-pass file: `5f2534b30f8e6bf3813071b9dea2eb87b99e37f7815cd0e153a54e45d0c408c0`;
- first-pass internal: `0eab340f783280d0f4b0b908740154b2a56bd93e802ecbbab2f6b7144892383c`.

Focused verification is 29 tests passing, including real decoder regression,
closed binding, hash mismatch, non-pool, data, literal value, relocation,
target-owner, consumer-register, CLI, and four route cases.

## Speed claim

On ConfigPadClose the two real replay commands completed together in under four
seconds of observed command wall time. The completed crack report estimated
two to three minutes for the manual decode-to-candidate preparation that this
tool replaces: roughly a 30x--45x acceleration for this **specific triage and
manifest stage**. This is not a universal 10x claim for whole-function
recovery. The larger payoff comes when first-pass routing prevents unrelated
declaration, CFG, numeric, or trace experiments in longer campaigns.

Compatibility is additive: the installed `typed_pool_owner_decoder/v1` schema
is consumed without modification, and existing `match pools` behavior is
unchanged.
