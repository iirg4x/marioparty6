# Promoting recovered C to clean `main`

## Principle

The AI workspace and `main` are permanent parallel branches.

```text
AI workspace: investigation, orchestration, prompts, metadata, benchmarks
main:         human-readable project and accepted recovered source
```

Do not merge the AI workspace. Promotion is a **content transfer**, not a branch
merge or commit-history transfer.

The ordinary recovered-source path below accepts only canonical C/C++ source and
header suffixes and keeps its raw numeric hexadecimal prohibition. Authenticated original data uses the separate,
allowlisted path documented in [`original_data_promotion.md`](original_data_promotion.md);
it is not clean-C recovery.

## Preconditions

The worker task must be `ready` or `done` in the local queue and its verification
must be bound to the exact source commit being promoted. Proof should include the
object report, exact-function result, relocations, affected consumers, and
toolchain.

The automatic path accepts only added or modified:

```text
src/**/*.c
src/**/*.cp
src/**/*.cpp
src/**/*.h
src/**/*.hpp
include/**/*.h
include/**/*.hpp
```

It does not accept assembly, symbols, splits, configuration, scripts,
documentation, metadata, workflow files, or generated output. Canonical headers
are promoted here when explicitly selected and verified with their source.

## 1. Inspect the transfer

From the AI workspace:

```sh
python tools/promote_recovered_c.py plan \
  --base main \
  --source <verified-worker-commit> \
  --owner <queue-owner> \
  --path src/path/recovered.c
```

The plan resolves immutable commits, checks queue verification, scans source and
header comments for AI attribution, and records source/base blobs plus SHA-256
values.

Omitting `--path` selects every changed canonical source/header file
(`src/**/*.c`, `src/**/*.cp`, `src/**/*.cpp`, `src/**/*.h`, `src/**/*.hpp`,
`include/**/*.h`, or `include/**/*.hpp`) between `main` and the verified source commit. Explicit
paths are preferred.

## 2. Create a clean promotion worktree

```sh
python tools/promote_recovered_c.py create \
  --base main \
  --source <verified-worker-commit> \
  --owner <queue-owner> \
  --path src/path/recovered.c \
  --branch recovery/<subsystem> \
  --worktree ../marioparty6-promotion-<subsystem> \
  --title "Recover <subsystem>"
```

The command:

1. creates the branch directly from `main`;
2. copies exact selected blobs from the verified commit;
3. stages only those canonical source/header paths;
4. rejects AI/agent attribution in branch names, source comments, and commit
   messages;
5. creates one ordinary source/header commit;
6. confirms the promotion diff contains only selected canonical source/header files;
7. confirms every promoted blob equals the verified source blob;
8. writes a local ignored manifest under `build/promotion/` in the AI workspace.

The promotion worktree does not contain AI workspace tools because it never
descends from that branch.

## 3. Supporting changes use a different human branch

When the recovered source depends on a symbol, split, object-status, or build
change, the automatic promotion still stops at canonical source/header files.
Select and verify dependent canonical headers in the same promotion when needed.

Do **not** add symbols, splits, object-status, or build configuration to the
recovered-source promotion branch. Promote those supporting changes with
`tools/promote_supporting_change.py`, which cuts a second branch directly
from `main`, transfers only declared verified blobs, and enforces the same
attribution, contamination, and consumer-re-verification gates. See
[`supporting_change_promotion.md`](supporting_change_promotion.md).

```text
recovery/<subsystem>       verified canonical source/header blobs only
project/<supporting-fix>   separately recreated human project change
```

This preserves a hard audit rule: the recovered-source PR always contains only
the canonical source/header paths selected and verified for that owner. The
supporting PR may be reviewed and merged first when symbols, splits, or build
configuration are required.

Never copy an unselected header or configuration file to `main` outside the
applicable tool. The promotion scans exist to keep prompt-oriented comments,
speculative names, temporary
experiments, queue identifiers, generated scaffolding, and AI attribution out
of `main`; findings refuse the promotion rather than being stripped silently.

## 4. Run source and retail proof

Inside the clean recovered-source promotion worktree:

```sh
python <ai-workspace>/tools/promote_recovered_c.py audit \
  --root . \
  --base main \
  --head HEAD \
  --source <verified-worker-commit> \
  --path src/path/recovered.c
```

The audit must pass. Then run:

- relocation-aware object comparison;
- exact-function regression check;
- affected Matching consumers;
- serialized DOL/REL build;
- DTK checksum;
- explicit retail DOL/REL byte comparison;
- source readability and semantic-debt review.

When a separate supporting PR is required, rebase or recreate the recovered-source promotion
branch on the updated `main`, rerun the blob audit, and repeat the retail gates.

## 5. Open a human-facing C PR

Push only the clean `recovery/*` branch and open it against `main`.

The PR discusses:

- the recovered subsystem and behavior;
- target and consumer evidence;
- object/relocation results;
- retail verification;
- unresolved semantic debt.

It does not mention Claude, Codex, prompts, agents, token usage, orchestration,
knowledge cards, or AI provenance. Those details stay in the AI workspace's
local queue, manifests, and recovery metadata.

## Diagnostic override

`--allow-unverified` exists only for tool tests and planning experiments. It must
not be used for a real promotion to `main`.

## Authenticated original-data path

Use `tools/promote_original_data.py` only for an exact record listed in
`config/recovery/original_data.json`. The tool creates a fresh `recovery/*`
branch directly from `main` and transfers the allowlisted source blob with its
exact byte/source provenance. It must not transfer AI metadata, documentation,
tooling, generated output, or queue state.

The current record is `musyx-dsp-import-mp4-201` for
`src/musyx/runtime/dsp_import.c`. It is `native_verified`: object, relocation,
linked-retail, and checksum gates passed. The clean main-based promotion must
still generate and review the public status/progress sidecars before marking
the owner `Matching`. It remains a data-only owner (0/0 functions expected),
earns zero clean-C credit, and does not change clean-C totals. See
[`original_data_promotion.md`](original_data_promotion.md) for the record and
exact native evidence.
