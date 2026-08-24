# Board recovery tools

This is the durable entry point for tools supplied by the MP6 Recovery
Manager to the four Board source orchestrators. Handoffs and task messages are
notifications, not documentation. An orchestrator must be able to rediscover a
tool, its command, its evidence boundary, and its failure behavior from this
file and the linked command reference alone.

The central entry point is:

```text
rtk C:\Python313\python.exe tools\agent.py --root <owner-worktree> match <command> ...
```

Run commands from the AI workflow repository containing the installed tool
commit. Generated evidence belongs under the caller's private `build/`
directory. These commands are diagnostic unless a linked document explicitly
says otherwise. None may retain source, update progress, promote evidence, or
advance authority.

## Documentation acceptance gate

A tool is not ready for integration or push until durable documentation names:

1. its installed commit and owning source/test files;
2. the exact central and direct invocation;
3. every input and output schema, version, and self-hash field;
4. authenticated inputs and explicit caller-attested inputs;
5. trust boundaries and operations the tool does **not** prove;
6. fail-closed conditions, limits, and deterministic ordering;
7. at least one representative Board acceptance case;
8. focused and full-suite verification receipts;
9. compatibility and migration behavior; and
10. `authority_advanced:false` for diagnostic output.

Tool behavior must not depend on chat memory, an orchestrator's recollection,
or an unpublished command line. A handoff should link this index and the
specific detailed section.

## Anti-duplication gate

Before claiming or implementing any queued tooling request, the Manager must
search, in order:

1. this installed-tool index and the cumulative AI workflow commit history;
2. completed central queue tasks and their verification records;
3. currently claimed and pending tasks, including dependencies;
4. existing command schemas, aliases, tests, and acceptance receipts; and
5. repository tools that provide an adjacent or stricter capability.

The result is recorded as one of: `new_capability`, `extension`,
`acceptance_case`, `documentation_only`, or `duplicate_no_work`. New evidence
for an installed capability becomes a regression/acceptance case on the
existing tool; it must not start a parallel implementation. For example, the
CapSpecial DiceExec +0x18 typed pool gap extends the installed `match pools`
decoder and the queued pool-prefix diagnosis task—it is not a second pool
decoder.

## Graph-first source discovery gate

All Board source and interface discovery starts from the shared canonical
Graphify graph at:

```text
D:\Games\Emulation\GameCube-Wii\_mp6_rebuild\port\mp6-native\graphify-out\graph.json
```

Run `rtk graphify query`, `rtk graphify path`, and `rtk graphify explain` from
that `mp6-native` repository before searching files. Owner worktrees must not
rebuild or copy the graph. A narrowly scoped named-file or named-symbol search
may verify a returned `source_location`; broad recursive searches of a whole
repository or `C:\Users\Anony\.codex` are forbidden. Evidence packets record
the graph query and source location, or explicitly report that the graph had no
relevant node.

## Installed Manager A-G program

| Program | Command | Installed commit | Purpose | Detailed reference |
|---|---|---:|---|---|
| E — matrix/index duplicate repair | `match matrix` | `ba535d72e039fa409ddf13dab108820342c62619` | Audits immutable candidates while accepting legitimate duplicate objects and preserving canonical-first indexes. | [Matrix and generated layout](match_workbench.md#7-render-the-deterministic-matrix) |
| F — per-function recovery telemetry | `match telemetry` | `e13f1b6cf234c4554146c3bc0730041a05303a35` | Derives candidate/object/source counts, exact convergence, heavy-process coverage, and separate elapsed/active throughput. | [Function telemetry](match_workbench.md#8-measure-one-functions-recovery-campaign) |
| F — prospective crack/hour measurement | `match campaign-start`, `match campaign-event`, `match campaign-compare` | `8de5535b8bf41a55d2877bbe9082cd9b44df2612` | Records immutable start/pause/resume/exact events and compares non-overlapping before/after campaigns without claiming causation or inventing incomplete rates. | [Campaign timing and comparison](match_workbench.md#8-measure-one-functions-recovery-campaign) |
| C — causal objdiff cascade reducer | `match cascade` (`causal-reduce`) | `58f953347dce7adb9c63480c122a5ff128dd9f57` | Groups repeated instruction residuals into bounded stack, ABI, aggregate-copy, CFG, and relocation causes. | [Cascade reducer](match_workbench.md#9-reduce-one-functions-objdiff-cascade) |
| B — typed pool-owner decoder | `match pools` (`pool-decode`, `pool-owners`) | `37816ddf53ff2dff6a70fd9303d231157955b647` | Decodes relocation owners and typed literal contracts, separating semantic mismatch from pool chronology/identity. | [Pool decoder](match_workbench.md#10-decode-typed-pool-owner-mismatches) |
| D — factorial interaction planner | `match interactions` (`factorial-plan`, `interaction-plan`) | `931749192be507189d1a84e84e9fe6787ce20d9f` | Expands bounded evidence-backed axis products and deduplicates only explicit topology or measured hashes. | [Interaction planner](match_workbench.md#11-plan-factorial-source-axis-interactions) |
| compiler-context repair | `match attest-compile`, `provenance-audit`, `provenance-migrate` | `ce95e5d4e9f5d0e8777c2fe793e5de21c555fc9f` | Prevents cross-toolchain records and migrates only externally attested legacy attempts into the correct session. | [Compiler attestation](match_workbench.md#3-seal-the-compiler-provenance), [audit/migration](match_workbench.md#5-audit-and-migrate-legacy-compiler-provenance) |
| stack-home native producer | `rtk python tools/capsule_stack_home_native.py {prepare,preflight,capture,validate,summarize}` | `449b17615b2a663837a9e0573841c023456b5009` | Captures pointer-free, authority-bound Object↔VarInfo names and stack-home chronology, then deterministically joins exact requested names to physical slots without advancing ownership. | [Authenticated MWCC stack-home capture](capsule_stack_home_native.md) |
| stack-object/lifetime reducer | `rtk python tools/stack_object_lifetime_reducer.py {bind,reduce}` | `5531f507e57404c797254a594fcf2b3c8202e2fe` | Binds objdiff/source/VarInfo evidence, composes authenticated stack homes, classifies lifetime/allocation deltas, and ranks natural-C axes while rejecting shaping. | [Stack-object/lifetime reducer](stack_object_lifetime_reducer.md) |
| A — source-aware MWCC causal tracer | `rtk python tools/capsule_same_session_capture.py {prepare,preflight,capture,seal-source-spans,causal-map}` | pending integration commit | Composes the installed stack-home producer, frontend chronology, direct ownership correlator, and donor source parser; joins sealed source spans to vreg, physical GPR/FPR, stack, call-return, and evaluation chronology while every missing/ambiguous edge remains UNKNOWN. | [Source-aware MWCC causal trace](mwcc_source_aware_causal_trace.md) |
| G — full-owner causal map | `rtk python tools/board_causal_map.py REQUEST.json --root .` | `c6f6c89b0a51d9fbda0774f054a2f4a7ff0db642` | Composes the installed matrix/telemetry, causal reducer, typed pool decoder, factorial planner, compiler bindings, and optional donor/tracer/Graphify context into a self-hashed inventory for every residual function. Always read-only and non-authoritative. | [Full-owner causal map](board_owner_causal_map.md) |

## Supporting workbench commands

The standalone read-only stack-object/lifetime reducer is documented in
[Stack-object/lifetime reducer](stack_object_lifetime_reducer.md). Its `bind`
command hashes concrete objdiff/source/VarInfo evidence; its `reduce` command
composes the generic stack-home packet and summary without duplicating the
native capture boundary. It emits diagnostic natural-C axes only and never
advances recovery authority.

The complete workflow and safety model are documented in
[Match workbench](match_workbench.md). The normal sequence is:

```text
match init
match lookup
<private compile/proof transaction>
match attest-compile
match prepare
match record
match diagnose
match matrix
match telemetry
```

Legacy records must use `provenance-audit` and `provenance-migrate`; do not
silently reuse them. `lookup`, `materialize`, and diagnostic reuse reject a
candidate without a compiler-context attestation matching the immutable
session.

Other read-only commands exposed by the same module include `residuals`,
`stack-residue`, `donor-shapes`, `cascade`, `pools`, and `interactions`. Their
arguments and schemas are defined by `tools/match_workbench.py --help`; a
command used operationally must also have a narrative section in
`docs/match_workbench.md`. Help text alone is not a substitute for the trust
and failure contract.

## Evidence and authority boundary

Workbench reports bind diagnostic inputs and implementation hashes, but they
do not by themselves authenticate retail target selection, physical
relocations, linked consumers, source admissibility, or owner closure. The Sol
owner orchestrator still applies strict, data-value, relocation, section/pool,
consumer, and protected-sibling gates. Read-only recommendations never make a
source-policy or retention decision.

Every output intended for a crack report or cross-orchestrator handoff must be
stored in the owner's private artifact directory and referenced by path,
schema, SHA-256, command, installed tool commit, and verification result.

## Verification lineage

The AI workflow branch is `origin/agent/recovery-context-workflow`. Installed
checkpoints are cumulative and should be fast-forwarded in the order shown in
the table. Each checkpoint was required to pass its focused tests, the complete
`tools.tests.test_match_workbench` suite, the full central tools suite,
`compileall`, `git diff --check`, queue claim verification, and the public push
hook before handoff. Exact counts and representative-case receipt hashes belong
in the checkpoint commit message and queue verification record; they must not
be reconstructed from memory.
