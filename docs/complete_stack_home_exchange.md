# Complete stack-home exchange diagnosis

`tools/complete_stack_home_exchange.py` is a fail-closed first-pass reducer for
an exact-size function whose non-pool residual is entirely an r1-relative stack
home remap. It is meant to prevent long declaration/scope permutation campaigns
when the target already proves a missing or differently sized aggregate owner
family.

The tool does not generate source. A match only ranks one bounded action:
enumerate the observed physical extents, bind them to live typed source owners
with Graphify and one exact same-game donor, and compile the composed
aggregate/capacity cell before any lexical permutation. Remaining pool rows are
then handed to the typed-pool decoder.

## Match contract

A diagnosis matches only when all of these are true:

- target and candidate function sizes and instruction counts are exact;
- target and candidate data-function sizes are exact;
- every non-pool residual row is `DIFF_ARG_MISMATCH` on both sides;
- every such pair has the same instruction address, size, opcode, and non-stack
  operands;
- the only instruction difference is one word-aligned r1-relative displacement
  in a supported load, store, or `addi` address materialization;
- candidate-to-target stack offsets form a consistent bijection;
- at least eight rows and two homes are present, including stores, loads, and an
  address consumer.

Any unsupported row, size drift, operand drift, multiple mapping, or incomplete
consumer family blocks the diagnosis. A blocked result falls back to the generic
causal reducer and retains its single late-trace allowance.

A matched result suppresses declaration-order permutations, scope permutations,
dead/fake locals, padding, register shaping, and trace-led searching. It ranks
one evidence-backed natural-C cell that owns the earliest mismatch, sets a total
candidate budget of one and a trace budget of zero, then requires a fresh
residual before another cell. Donor evidence is optional support, not a compile
prerequisite. Source retention, promotion, and authority remain false.

## Usage

```text
python tools/complete_stack_home_exchange.py STRICT.json DATA.json FUNCTION \
  --target-object TARGET.o \
  --candidate-object CANDIDATE.o \
  --expect-strict-report-sha256 STRICT_SHA256 \
  --expect-data-report-sha256 DATA_SHA256 \
  --expect-target-object-sha256 TARGET_SHA256 \
  --expect-candidate-object-sha256 CANDIDATE_SHA256 \
  --output diagnosis.json \
  --require-match
```

`tools/crack_first_pass.py` runs this diagnosis automatically after the narrower
typed-owner manifest gate. A match routes to
`authenticated_aggregate_donor_home_exchange_then_typed_pool` before the generic
causal reducer.

## Hanachan h075 acceptance

The authenticated `mbev_CapHanachan` h075 replay binds:

- strict report SHA-256
  `519df2738708a4cca1e3c58f911c51c6c287fca79e2300a9b859d90b268bd367`;
- data report SHA-256
  `ea7feede6e2802f0f8ebd65a136101b28eb5a42115b5ed0b736f007cbfed2ea8`;
- target object SHA-256
  `544ad14982f23269527bafef3f14eb8cc1d00cde53f91cac0e077f99aed0fa4e`;
- candidate object SHA-256
  `767f49b3846ef29c36af368fc9e0d37196b83b323b7f05bd812bb8998b149bed`.

The classifier accounts for all 47 residual rows as 46 stack-home rows plus one
pool handoff row, finds 18 consistent home mappings across `stw`, `lwz`, `lfs`,
`stfs`, and `addi`, emits one donor-composition cell, and disables tracing.

Acceptance receipt:

- diagnosis file SHA-256
  `29baea6c17a6f12f2068d482a034ff019ff274b717b798562d940188bbb1f890`;
- diagnosis internal SHA-256
  `50c08f88b6e78ec6e387b5b01c0ec3e66986e4f618179561f170e51f21a8b41f`;
- first-pass file SHA-256
  `1b0733cfef6f4202d4fd26d5fb0ae41851fd378cb7b2b1d9c1fe626dd17b220c`;
- first-pass internal SHA-256
  `c48dda326f1eb9f6202f542f92fa681452bc5affe08fb924a3c8bce0529b8697`.

In the completed campaign, this evidence class led to the exact same-game
aggregate family (`Mtx hookMtx`, `int motFile[8]`, and `HuVecF setupPos[5]`) and
then one semantic pool-owner seam. The report's counterfactual is three to five
candidates instead of 83, saving roughly four to five active hours. The tool
makes that first classification deterministic; it does not claim the whole
function will always close automatically.
