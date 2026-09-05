# Fast, current-source recovery handoff

These are convenience commands, not new approval gates. They do not compile
automatically, retain source, or prove physical/link exactness.

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
