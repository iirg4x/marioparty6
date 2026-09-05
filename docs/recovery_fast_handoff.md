# Fast, current-source recovery handoff

These are convenience commands, not new approval gates. They do not compile
automatically, retain source, or prove physical/link exactness.

## Fast cracking loop

Use this short loop against the current live report. Report elapsed time from
selection to independently verified crack, including evidence preparation and
required verification. Break out analysis, compilation, proof and optional tool
development separately; do not subtract overhead to imply faster delivery.

1. Select the current first **structural** mismatch (instruction, dataflow,
   stack home, relocation, or branch) and record its upstream producer/consumer
   path. Do not start from a cosmetic register difference when an earlier value
   or control-flow mismatch explains it.
2. Write one ranked semantic hypothesis and its predicted rows/bytes. Keep the
   prediction falsifiable and tied to the current report hash; do not fan out
   into five unranked source variants.
3. Parallelize only distinct unresolved evidence questions, for example:
   - does target dataflow produce the value and width at the predicted rows?
   - do source ABI and consumers require that type, lifetime, or register role?
   - which known failed equivalences already rule out a tempting explanation?
   Do not require three answers before compiling an already supported cell.
   Independent residual workers may stage and measure their own candidates in
   isolated output directories, using the existing compiler lock. One writer
   composes retained changes. This is not another approval round trip.
4. After two non-improving attempts against the same cause, change evidence
   method or residual. Also reassess after five minutes with neither a measured
   candidate nor a new discriminating fact. These are strategy-change triggers,
   not lifetime attempt caps, automatic rollbacks of gains, or function bans.
   Before a trace, name the exact field/event needed and confirm the producer
   actually records it. A successful capture missing that field is not useful
   evidence. Do not spend a run on a purposeless negative control. Retain every
   independently verified incremental gain.
5. Leave a compact checkpoint before moving on: function, report/object hashes,
   first mismatch and upstream path, one hypothesis, predicted versus observed
   rows, accepted/rejected evidence, and the next distinct question. Keep the
   best verified source/object and its proof beside the checkpoint.

Keep optional tool development off the cracking path. Use focused tests for a
small isolated utility; do not append an unrelated full-suite run to every
function crack. Exact-function gains and retained source improvements are the
outcome; captures, audit votes and report volume are not progress metrics.

## Reuse supported native captures

`tools/prepare_owner_capture.py prepare` accepts owner root, source, function,
expected source hash, a pinned compiler argv JSON file and a fresh output root.
It uses the existing capture implementation to derive the request and trust
bindings without compiling. `run PREPARED_JSON` executes once under the existing
owner compiler lock. No per-function Python adapter or new manager permit is
needed. It remains diagnostic-only.

The current producer does **not** record `VarInfo.usage+0x04`. Do not use this
capture to decide GC2.6 O0 local-FPR usage ranking. Check the stated capability
before spending preparation or capture time.

## Resume from one current index

After selecting the actual live source, its candidate object, and canonical
objdiff report, publish one overwrite-in-place index:

```text
python tools/recovery_frontier.py --root OWNER_ROOT snapshot --owner main:board/snpc --toolchain-key PINNED_KEY --source src/board/snpc.c --target-object build/GP6E01/obj/board/snpc.o --candidate-object build/GP6E01/src/board/snpc.o --strict build/current-strict.json --data build/current-data.json --out build/recovery/current.json
python tools/recovery_frontier.py --root OWNER_ROOT verify build/recovery/current.json
```

The index contains each function's score, size and first differing instruction
pair, plus exact input paths/hashes. It reads each report once (including when
strict/data name the same file), performs no recursive history search, and caps
output at 256 KiB. Repeated target/candidate relocation identities are grouped
across functions as diagnostic leads, not proven causes. `verify` detects stale
source, objects, reports and receipts.
Refresh after a retained source change; do not mistake the most recent filename
or filesystem timestamp for a source binding.

Use `--compile-receipt` with `recovery_candidate_compile/v1` when available. The
index checks its source/object hashes. Without it, compile binding explicitly
remains `not_supplied`. A caller-selected report is diagnostic evidence, not a
substitute for compiler, strict/data, relocation or final linked proof.

Keep one useful current report and the best working reconstruction. Failed
attempts need a compact source/object hash, first-divergence result and disposition,
not another full worktree or copied retail tree. Never automatically prune files
belonging to another owner or the current live frontier.

## Preserve compiler arguments without shell quoting

`compile_recovery_candidate.py` accepts either the existing `--compiler-script`
or `--command-json`. The latter is a JSON array of argument strings, for example:

```json
["C:/toolchain/sjiswrap.exe", "C:/toolchain/mwcceppc.exe", "-pragma", "cats off", "-pragma", "warn_notinlined off", "-c", "src/board/snpc.c", "-o", "build/GP6E01/src/board/snpc.o"]
```

Supply the complete pinned compilation flags, not merely this abbreviated
example. The first executable is hashed automatically; when using a launcher
such as `sjiswrap`, also declare the MWCC executable with `--tool` so both are
bound to the context. The runner passes the array directly to the process API: `cats off`
remains one argument. Use a prepared owner-local scratch directory with matching
headers; generated dependency/object files stay there. The live source is never
the staged compile source. `--preflight` checks context without a compiler launch.
Both modes remain bounded and hash their compiler context.

## Ask for useful context

```text
python tools/agent.py context function FUNCTION --owner OWNER --local-evidence --read-only --budget 1500
```

The printed budget is the requested budget. A compact target/owner/source capsule
is reserved before broad compiler diagnostics. Local mismatch observations are
labeled as unverified bindings until checked against a current evidence index;
old report prose cannot establish a current candidate result. Exact owner
constraints and counterexamples are retained ahead of general advice.

## Measured SNPC takeover lesson

The 2026-09-05 timed pass spent substantial inference time on stale handoff
artifacts, shell quoting and a TU-wide constant pool. Private compiles were fast.
Near-exact functions with identical conversion-constant rows should first be
checked for a shared pool/layout cause; changing each cast independently cannot
repair an earlier translation-unit producer. Typed literal reconstruction can
recover section extent while still moving physical owners or losing exact
siblings. A 440-byte pool matching a 440-byte target is not an exact owner.

## Trace a value's consumers before changing its source spelling

For one observed stack home, inspect all its accesses in the already-bound report:

```text
python tools/recovery_frontier.py --root OWNER_ROOT accesses --strict build/current-strict.json --function SNpcMoveExec --side target --base-register r1 --offset 0x88 --context 2
```

This is a bounded, read-only instruction view, not a semantic-owner proof or a
source generator. It distinguishes exact offsets and base registers, includes
row/address/context, and explicitly reports truncation. Use the current index to
verify the report/source binding first.

In SNPC's second timed pass, the six accesses at that home exposed a missing
eligibility counter and its actual decision consumer. Related list-consumer and
inline-loop lifetime reconstruction improved `SNpcMoveExec` from 86.849370% to
95.289660%, preserving all 84 sibling functions and all data sections. The owner
was still not exact. This is why an apparent register cascade should be checked
for missing values and wrong consumers before trying declaration permutations.

Keep a compact rejected-cell list beside the current index and read it before
compiling a familiar source class. A new source filename is not a new hypothesis.
Whole-ELF hashes also include filename metadata; when source-path recompilation
changes STT_FILE, compare allocated sections and relocation meanings before
classifying it as a code-generation regression.
# Repeated stack-home pairs

Use `python tools/recovery_frontier.py --root OWNER_ROOT stack-map --strict REPORT --function NAME`
to group aligned r1 accesses by observed target/candidate displacement pair.
It matches opcodes and non-displacement operands; addi pointer consumers are
reported separately. Each group includes counts and bounded row examples.
One-to-many relationships remain explicitly ambiguous, not inferred source owners.
The report is hash-bound, diagnostic-only and capped at 256 KiB with truncation
flagged. It introduces no compile/admission gate.

For the SNPC third-pass retained proof, use
`work/manager-third-20260905/retained-proof.json` and `SNpcMoveExec`.
The earlier ten repeated sound/attribute stack-pair inversions were exposed by
the same query; the retained result has 150 D-form and 69 pointer rows paired
with equal displacements. Superseded raw reports may be regenerated from the
preserved objects rather than kept as permanent history.

## Compare branch destinations before calling a residual an allocator problem

```text
python tools/recovery_frontier.py --root OWNER_ROOT branch-map --strict REPORT --function NAME
```

This read-only check resolves destinations to aligned instruction rows, so a
uniform address shift is not mistaken for a changed branch target. It shows
changed and unresolved destinations, excludes calls/returns, binds the report
hash, and caps output at 256 KiB. It does not prove source ownership, full CFG
equivalence, or source admissibility, and adds no compile/approval gate.

SNPC Pass 3's retained report had 100 paired branches but three real destination
differences (rows 22, 428, 522). The Pass 4 report has 100 equal destinations.
Those differences exposed an early-return boundary and two incorrect source
scopes; fixing them reduced 109 mismatching rows to 81. The early nonvoid bare
return is documented target-specific portability debt, not portable C proof.
Inspect actual control-flow relationships before changing volatile register
owners. Keep observed branch facts distinct from proposed source repairs.

## Diagnose reused compiler temporary IDs across distant mismatches

```text
python tools/mwcc_temp_pool_reuse.py --envelope CURRENT_CAPTURE --report CURRENT_STRICT_REPORT --function NAME
```

Use an already reproduced GC/2.6 capture. This read-only diagnostic finds
observed temporary-ID resets, conflicting target register roles, and bounded
suffix-offset hypotheses. It hashes its inputs, caps output at 256 KiB, and
does not create source, compile, or confer source ownership. When the report
omits raw instruction words, report-word identity is explicitly unverified;
the caller must independently bind the report to the captured object.

SNpcMoveExec Pass 5 exposed a native statement-boundary reset after the
temporary counter exceeded 256. A unique +1 suffix beginning at ID 165
resolved 32 conflicting register requirements across two distant regions.
Putting the existing stack-backed eligibility counter inside its zero-assignment
chain created the missing real expression result. Independent recompilation
closed all 79 remaining rows: 4,488 bytes, zero strict/data differences, and
84 unchanged siblings. The successful-source trace confirmed the new temporary
and its coalescing. This is a reusable diagnosis, not a universal chain spelling.

Same-session capture commands now print compact summaries by default while
preserving complete evidence files. Use `--full-output` only when a complete
CLI payload is required; the Python APIs and saved event streams are unchanged.
