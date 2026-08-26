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

## Bound CapTrap Bomhei interaction evidence

Graphify resolves `mbev_CapBomhei` to `src/board/captrap.c:L1615`. A single
Graft lookup returned no indexed node, so the bounded review continued only in
that named function and its authenticated artifacts. The retained exact result
is candidate 67: source
`a6d09391133935c45bc102e5580c2bdc89b1caac9fa92bf07824ac934676a4cb`,
object `6e44d75b3bc1f077946e84662e07b715c76306733e0693a3bcbcaa2d7a1a0bc3`,
strict report
`b8664886f585d755e9bbd3cf34ba2a7244c767df0eea42ab9e3ffbf1d85af75a`,
and record
`e4cbdca49b4d3d819dfa0b7913a1252123fac347fa48d668a447074e633f3171`.
The function is 3176/3176 bytes with zero strict/data rows and 292/292 physical
relocations; all twelve protected siblings remain exact.

The causal precursor is candidate 61. Its same-session diagnostic binds source
`aa3de2349082d76712897f04ae44025e6499173cf6eba115cf015dd5168ad562`
to envelope
`4673ea466f50501460e66fe94eeccd8712fbe4cb92b4f1f627ac097d167b717a`,
stack stream
`9878535da407ccfa0de5214c657b6415de7a56a1971ff1be802482f626109b18`,
PCode stream
`8b25237a6523c2ae9f2f80d00fccfef422122c6db4fc620a30cbe18265057a53`,
sealed source spans
`558256df0a21eb4aa93780840ee87e0907e917a923ebd065fb9222626d714f4f`,
and causal map
`4e1b4c2b8319d82feb217d9af00140280c08535b2ee2d440c229178932cec97d`.
That historical capture remains `UNKNOWN`: it lacks the current authenticated
frontend/vreg join and cannot itself advance ownership or source authority.

It does prove enough physical structure to rank one bounded interaction after a
fresh current-producer join:

- candidate `itemHook` occupies `r25`, while retail uses `r19` only across the
  nested hook call. Moving the same named local into branch scope was
  object-neutral, so another lexical-scope permutation is suppressed; rank the
  direct nested `CharModelItemHookGet(...)` consumer instead;
- retail's two trajectory loops write the `0x50/0x54/0x58` aggregate home bound
  to candidate `ringPos`, while candidate 61 writes `effectPos` at
  `0x8c/0x90/0x94`; rank reuse of the already-live `ringPos` aggregate for both
  loops rather than inventing another object; and
- the two causes are disjoint. Submit both singles and their combined cell to
  the interaction planner before declaration, scope, or register experiments.

The measured factorial confirms the ranking: direct nested consumption alone
reached 99.989920%, `ringPos` reuse alone reached 99.911840%, and their combined
cell was exact. This is acceptance evidence for the existing generic
stack-home/source-owner join, not a reason to create another capture producer.
Fresh uses must still enter through `capsule_stack_home_native.py`, validate its
packet and deterministic summary here, and remain `UNKNOWN` when the current
Object-to-source join is incomplete.

## Bound CapTrap Tumujikun direct-helper lifetime evidence

Graphify resolves `mbev_CapTumujikunTrap` to `src/board/captrap.c:L1194` and
binds the relevant helper edges, including `mbBranchAttrGet` and
`mbMasuMAttrGet`. A single exact-name Graft lookup returned no indexed node, so
the review stopped searching and narrowed to that function, its strict report,
and its retained source. Before the final source change, size, CFG, call order,
data, and all 193 physical relocations were already exact. The residual was one
complete lifetime exchange: long-lived `randomStart` occupied candidate `r23`
instead of target `r24`, while a repeatedly assigned but immediately consumed
`branchAttr` mask occupied candidate `r24` instead of target `r23`.

The graph-first lifetime rule is:

- when a helper result local is assigned only at the use boundary, consumed
  exactly once by the following mask expression, and not observed elsewhere,
  first verify the helper's call count, call position, side effects, consumer,
  and relocation are already target-exact;
- if the strict residual is then a complete exchange between that transient
  helper result and one authenticated long-lived owner, rank direct helper
  consumption at each existing boundary before declaration order, lexical
  scope, aliases, or a runtime trace; and
- preserve the original number and ordering of helper calls. Do not hoist,
  merge, duplicate, or delete a helper evaluation, and do not apply this rule
  when the local has a second consumer or crosses a call/branch boundary.

Candidate 79 applies exactly that bounded transformation: it removes only the
single-use `branchAttr` local and directly consumes `mbBranchAttrGet()` in the
same three mask expressions. The source SHA-256 is
`d806b8cb5483fed8c5000f92beb77af7b52e205d5c08800c8643ef9dd1a25660`,
the object SHA-256 is
`d623d3ddd784f9b6d5f70ba235049126a948fa966ae9e2ba7f9c3c8bb41df03c`,
and the byte-identical strict/data report SHA-256 is
`334b7ad8c438468b2cad62e7459063e63cba342db22b5edf01f9af85440bfdbf`.
`mbev_CapTumujikunTrap` is 3356/3356 bytes with zero rows and 193/193
relocations; all fourteen protected exact siblings remain exact. The causal
receipt `5f50f0f4a034601aa407231b42a131b97b4cd385b37c79c5d8eda4adc3252921`
contains zero residual groups. Candidate 80 independently preserves the exact
function in the composed owner source (`b14844c85badd3405ff66f80896e5bc6ad8702ccdfbbde329f9853b463bb2718`),
so this is a reusable source-boundary rule, not a function-specific register
hint and not a reason to create another trace producer.

## Verification

Run only the focused module while expensive background scans are active:

```text
python -m unittest tools.tests.test_stack_object_lifetime_reducer -v
python -m py_compile tools/stack_object_lifetime_reducer.py
```

The acceptance tests bind the concrete Jango and Patapata files above. A
small protocol fixture exercises the producer-composition seam, and a tamper
case proves that a modified bound fails closed.
