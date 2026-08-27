# Owner-flow matcher

`tools/owner_flow_matcher.py` turns an exact-size, argument-only focus residual
into named source-owner flows before any declaration or scope probes.

The tool consumes:

1. a hash-bound `focus_symbol_report/v1` artifact; and
2. a closed `owner_flow_context/v1` manifest that binds source declarations,
   definitions, uses, call boundaries, object identities, protected siblings,
   and any independent physical-relocation receipt.

It classifies every strict diff row as one of:

- candidate-to-target `r1` stack-home flow;
- contiguous register-result flow;
- typed SDA21 pool-owner identity;
- semantic immediate;
- branch destination; or
- unsupported, which blocks the diagnosis.

It then solves a minimum-cost bipartite assignment between the stack/register
components and the semantic owners in the context. A result is accepted only
when the assignment is unique and every strict row is accounted for. Stack
edges are converted into complete owner cycles. The output ranks at most two
bounded cells: one natural declaration/lifetime cell for all closed cycles,
then one composed semantic closure cell. It never emits C or authorizes
retention or promotion.

## ConfigExec acceptance replay

The authenticated c196 focus artifact is 3,772/3,772 bytes with 17 strict and
10 data rows. The matcher deterministically separates:

- seven SDA21 rows into five target pool owners;
- five rows into the `doneF -> oldValue -> value` three-home cycle;
- one flag immediate;
- one branch destination; and
- three rows into the live `vibrateF` Boolean result flow.

All 17 rows are covered, the compile budget is two, and the trace budget is
zero. The source context records that `doneF` is target-observed write-only;
the tool does not upgrade that evidence into a broader semantic claim.

## Usage

```sh
python tools/owner_flow_matcher.py \
  build/ConfigExec-c196-focus.json \
  build/ConfigExec-c196-owner-flow-context.json \
  --expect-focus-file-sha256 <sha256> \
  --expect-context-file-sha256 <sha256> \
  --output build/ConfigExec-c196-owner-flow-diagnosis.json \
  --require-match
```

Generated output is written atomically. Missing hashes, changed function size,
non-argument rows, ambiguous assignments, incomplete semantic groups, or any
unclassified row fail closed. A missing physical-relocation receipt is carried
as `UNKNOWN`; it never becomes implicit authority.
