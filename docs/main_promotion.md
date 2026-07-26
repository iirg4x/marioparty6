# Promoting recovered C to clean `main`

## Principle

The AI workspace and `main` are permanent parallel branches.

```text
AI workspace: investigation, orchestration, prompts, metadata, benchmarks
main:         human-readable project and accepted recovered source
```

Do not merge the AI workspace. Promotion is a **content transfer**, not a branch
merge or commit-history transfer.

## Preconditions

The worker task must be `ready` or `done` in the local queue and its verification
must be bound to the exact source commit being promoted. Proof should include the
object report, exact-function result, relocations, affected consumers, and
toolchain.

The automatic path accepts only added or modified:

```text
src/**/*.c
```

It does not accept headers, assembly, symbols, splits, configuration, scripts,
documentation, metadata, workflow files, or generated output.

## 1. Inspect the transfer

From the AI workspace:

```sh
python tools/promote_recovered_c.py plan \
  --base main \
  --source <verified-worker-commit> \
  --owner <queue-owner> \
  --path src/path/recovered.c
```

The plan resolves immutable commits, checks queue verification, scans C comments
for AI attribution, and records source/base blobs plus SHA-256 values.

Omitting `--path` selects every changed `src/**/*.c` file between `main` and the
verified source commit. Explicit paths are preferred.

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
3. stages only those C paths;
4. rejects AI/agent attribution in branch names, source comments, and commit
   messages;
5. creates one ordinary source commit;
6. confirms the promotion diff contains only selected C files;
7. confirms every promoted blob equals the verified source blob;
8. writes a local ignored manifest under `build/promotion/` in the AI workspace.

The promotion worktree does not contain AI workspace tools because it never
descends from that branch.

## 3. Supporting changes use a different human branch

When the C file depends on a header, symbol, split, object-status, or build
change, the automatic promotion still stops at C.

Do **not** add that supporting change to the C promotion branch. Create a second
branch directly from `main`, recreate the change from target evidence, and review
it as a separate human-authored PR.

```text
recovery/<subsystem>       verified C blobs only
project/<supporting-fix>   separately recreated human project change
```

This preserves a hard audit rule: the recovered-C PR always contains only
`src/**/*.c`. The supporting PR may be reviewed and merged first when the C
branch depends on it.

Never copy a header or configuration file wholesale from the AI workspace. This
prevents prompt-oriented comments, speculative names, temporary experiments,
queue identifiers, generated scaffolding, and AI attribution from entering
`main`.

## 4. Run source and retail proof

Inside the clean C promotion worktree:

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

When a separate supporting PR is required, rebase or recreate the C promotion
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
