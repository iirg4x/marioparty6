# Stack-object/lifetime reducer

`tools/stack_object_lifetime_reducer.py` is the installed invocation path. It
is a deterministic, read-only bridge from authenticated objdiff/source/VarInfo
evidence to natural-C stack lifetime axes. It does not compile, edit source,
launch a process, write a recovery artifact, submit a candidate, or advance
authority.

The tool has two commands:

```text
python tools/stack_object_lifetime_reducer.py bind \
  --observation-id jango-v7 \
  --function mbev_CapJango \
  --strict-report <strict.json> \
  --source <capthrow.c> \
  --varinfo-report <mbev_CapJango.varinfo.json> \
  --name hookMtx --name motionId --name motFile

python tools/stack_object_lifetime_reducer.py reduce <request.json>
```

`bind` prints `mwcc_stack_object_lifetime_bound/v1`. Every input is described
by its canonical absolute path, byte size, and SHA-256. The bound contains:

- target/source frame sizes, mnemonic/relocation-shape hashes, and a strict
  difference class (`home_only` or `semantic_or_topology`);
- exact declaration type, dimensions, extent, scope depth, and declaration
  hash for each requested existing object;
- every interval from the preceding declaration/use to the next semantic
  identifier use, with byte and line/column bounds plus an interval SHA-256;
- pointer-free VarInfo final state and assignment chronology; and
- a deterministic `bound_sha256` with `authority_advanced=false`.

The four-player `GW_PLAYER_MAX` array bound is resolved as a versioned ABI
constant while retaining the exact symbolic spelling in
`dimension_expressions` and `symbolic_capacity_ties`. Unknown symbolic bounds
remain unresolved; the reducer does not guess their extent.

## Generic stack-home composition

`reduce` consumes `mwcc_stack_object_lifetime_request/v1`. Each member of
`bound_reports` is a `{path,size,sha256}` descriptor. Each `stack_homes` row
names an observation and supplies descriptors for the packet and summary made
by `tools/capsule_stack_home_native.py`.

The reducer imports that producer and calls its existing `validate_packet`,
`canonical_hash`, and deterministic summary projection. It deliberately does
not copy the native hook table, packet schema, event validator, allocation
chronology, pointer-token logic, or compiler transport. The supplied summary
must be the producer's exact recomputation of the packet and must bind the same
source SHA-256 as the observation.

The report projects the earliest frontend Object stack-write event, the
lowest-home event, and named home mappings. For a selected matrix aggregate it
also binds the immediately preceding Object write. Only a predecessor target
slot of `0x34` plus `min(mapped_slots) - VarInfo.home == 0x8` supports the
classification `scoped_aggregate_coalescing_with_outgoing_call_area`.

Ownership stays `UNKNOWN`. A physical home or an adjacent outgoing-call area
does not prove source ownership by itself.

## Deterministic conclusions and no-go rules

The report `mwcc_stack_object_lifetime_reducer/v1` contains:

- proven source-declaration stride and remaining allocation class;
- the selected object's earliest hashed source-use interval;
- observation and cross-observation home-only/semantic classes;
- lexical-scope experiments, including `NO_GO_NO_OBJECT_EFFECT` when source
  scope changed but the strict report bytes did not;
- symbolic capacity ties, earliest frontend/lowest-home events, and the
  authenticated matrix lifetime event;
- deterministically ranked natural-C axes; and
- a `report_sha256`, with `diagnostic_only=true`, `board_admission=false`,
  `exactness_claim=false`, and `authority_advanced=false`.

A proven scope no-go suppresses another scope recommendation. The next axis
must preserve the tested scope/type and vary only real declaration/use
chronology. These axes are always forbidden:

- dead locals or padding;
- `register`/`volatile` shaping; and
- fake uses or unreachable branches.

## Bound CapThrow acceptance evidence

The concrete Jango v7 tuple is:

| Artifact | SHA-256 |
|---|---|
| strict objdiff | `d1926e2a63ddb9579540bfab8d1952a5c097c2817373a1e13a42cc732fffb0d5` |
| candidate source | `8d8188bb2f056978a0bd50f1f2b4cf2a91a55fd005cd560c1642ffedf7f3a71b` |
| VarInfo | `5302ae042d7d7053a9de689e537a6f04e547bdb20994709e608362bf4a823f62` |

It is 3068/3068 bytes and 767/767 instructions, with 536 exact instructions
and 231 argument-only differences. Target/source frames are `0x150`/`0x160`;
instruction shape is identical, so the difference is home-only. `hookMtx` is
declared at candidate line 1000 and first used at line 1079. Its earliest
interval is bytes 36314..39175 with SHA-256
`9daa22aaa577b58bb9185dff4a82d4795cd43f471c313801261292e704bb46d3`.

Fresh Jango evidence remains home-only:

- `jango-v11-bc-motion-reuse-y100`: source
  `1749d5f7941c74510290a0ca1007e6890e8e5c7d975d0366bec7b8d13ce60a9b`,
  object `1ac3e58242a03c40e3fe03c7ca2583bdb22fadc3668c1db122318ad433be86a5`,
  strict `9e8d0deb57713526701eb4dffdb1e839c6951fc2bf7197d3e3816072e32e425b`,
  and data `f3718d69e97de21e53701ff31bff06150be144913d47a55567a8a9901ed991f8`.
  It is 3068/3068, 606 exact plus 161 argument-only instructions, has 252/252
  relocation keys and 13/19 protected sites, and has equal `0x150` frames.
- `jango-v12-ab-camera-reuse-player-capacity-y100`: strict
  `3321060096173222c2a269cead3fca33c272fff4f790347587ca268749007ba0`.
  Its `motionId[GW_PLAYER_MAX]` declaration preserves the authentic
  four-player ID capacity tie. The upstream causal-reducer evidence is
  `2a52b971b68bdc69007a4518ff86065e5e360366c32ce9b6625abca63da26849`.

The concrete Patapata chronology is:

| Observation | Strict SHA-256 | Source frame | Key source fact |
|---|---|---:|---|
| v10 structural chronology | `8f380ebaac5f17c6f0ad695a14cf4c69679f3aec1a19eb08f94294429b2e7d14` | `0x150` | `motionId[2]`, outer stride 4 |
| v11 two-slot table | `4a38e6c32bea7042b34a8109409b13fdedc67ccb810cd76d1d8964958ddd341e` | `0x160` | `motionId[2][2]`, proven outer stride 8 |
| v12 loop-scoped `motFile` | same v11 strict hash | `0x160` | 16-byte `motFile[4]`, deeper scope |

The target frame is `0x170`. The motion-table change closes 16 bytes of the
32-byte gap without changing instruction shape, leaving a bound 16-byte
allocation class. Moving `motFile` deeper changes source but not strict-report
bytes, so lexical scope is a no-go. The v12 earliest `motFile` interval is
candidate lines 1290..1292, bytes 47184..47201, SHA-256
`3b2c67a8965dc1beefe7f169be7af8097fae017ea9958b117e072c820287635c`.

Canonical Graphify evidence was queried before implementation from the shared
read-only graph. It anchors `capthrow.c` at `game/src/board/capthrow.c:L1`, the
`EVCAPWORK` four-player arrays at lines 24, 30, and 31, the nearby natural
`motionId[4]` pattern at line 763, `HuPrcCurrentGet` at
`game/src/game/process.c:L134`, `HuPrcSleep`/`HuPrcVSleep` at lines 191/203,
and `Hu3DModelObjMtxGet` at `game/src/game/hsfdraw.c:L3641`.

## Verification

Run only the focused module while expensive background scans are active:

```text
python -m unittest tools.tests.test_stack_object_lifetime_reducer -v
python -m py_compile tools/stack_object_lifetime_reducer.py
```

The acceptance tests bind the concrete Jango and Patapata files above. A
small protocol fixture exercises the producer-composition seam, and a tamper
case proves that a modified bound fails closed.
