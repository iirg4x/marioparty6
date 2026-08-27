# Frontend temporary-birth-order reducer

`tools/frontend_temporary_birth_order.py` is a diagnostic-only reducer for a
small, closed MWCC stack-home permutation. It is intended for the late stage
where function size, CFG, calls, data values, and every non-home instruction
are already closed.

The tool consumes:

- one hash-bound `focus_symbol_report/v1` artifact;
- one closed `frontend_temporary_birth_order_context/v1` manifest;
- two or more compiler call-result temporaries;
- exactly one live typed address consumed once at an existing call boundary;
- sealed current and proposed frontend birth ranks.

It accepts only paired `r1` memory rows whose opcode, value register, and base
are identical and whose stack offset is the sole difference. Candidate and
target homes must form one bijection. Both the observed candidate homes and the
proposed target homes must follow the same MWCC model: later frontend births
receive lower stack homes.

On a match it emits one source *class*, not source text: preserve the required
aggregate copy and materialize its typed address inside the existing final call
argument at the consumer boundary. The exact expression spelling remains
function-family evidence. The reducer never emits a patch, retains a candidate,
or advances relocation, source, closure, or promotion authority.

## Usage

```sh
python tools/frontend_temporary_birth_order.py \
  build/focus.json build/temporary-birth-context.json \
  --output build/temporary-birth-diagnosis.json \
  --require-match
```

Malformed or unbound input returns status 2. Valid but incomplete evidence
returns `status: blocked`; `--require-match` also makes that status 2. Output is
canonical, self-hashed, and written atomically.

## KillerMove replay

The `mbev_CapKillerMove` h105 replay contains six rows and three homes. The
candidate order is final dust address `0x10`, initial `sqrtf` `0x0c`, route
`sqrtf` `0x08`. Moving only the live address into the final typed consumer makes
its proposed frontend birth latest, predicting retail `0x08`, `0x10`, and
`0x0c` respectively. The reducer schedules one compile cell, suppresses the
recorded h116-h121 explicit-local controls, and assigns trace budget zero.

Physical relocation evidence may be `exact` or `unknown`. Unknown evidence is
preserved as a warning and never upgraded. Whole-owner linked provenance is a
separate obligation handled by `tools/source_linked_owner_closure.py`; this
reducer deliberately does not duplicate or weaken that gate.
