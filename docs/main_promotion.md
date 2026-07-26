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
must be bound to the exact source commit being promoted. For recovered source,
proof should include the object report, exact-function result, relocations,
affected consumers and toolchain.

The automatic path accepts only added or modified:

```text
src/**/*.c
```

It does not accept headers, assembly, symbols, splits, configuration, scripts,
documentation, metadata, workflow files or generated output.

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

Omitting `--path` selects all changed `src/**/*.c` files between `main` and the
verified source commit. Explicit paths are recommended for narrow human review.

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
2. copies the exact selected blobs from the verified commit;
3. stages only those C paths;
4. rejects AI/agent branch names, source comments, commit messages and co-author
   trailers;
5. creates one normal source commit;
6. confirms the promotion diff contains only selected C files;
7. confirms every promoted blob equals the verified source blob;
8. writes a local ignored manifest under `build/promotion/` in the AI workspace.

The promotion branch does not contain the AI workspace tools because it never
descends from that branch.

## 3. Handle supporting changes separately

When the C file depends on a header, symbol, split, object-status or build change,
the automatic promotion stops at C.

A human or integrator must recreate the supporting change in the clean promotion
worktree using evidence from the recovery task. Review it as an ordinary project
change. Do not copy a header or configuration file wholesale from the AI branch.

This protects `main` from:

- prompt-oriented comments;
- speculative semantic names;
- temporary compiler experiments;
- recovery metadata and queue identifiers;
- generated declarations or scaffolding;
- agent attribution and operational instructions.

After a supporting change is recreated, the final human PR may contain it, but
the `promote_recovered_c.py audit` will intentionally fail because the automatic
C-only boundary has been exceeded. The integrator must then document the manual
supporting change and review it separately. For the strongest separation, use a
second human-authored commit.

## 4. Run source and retail proof

Inside the clean promotion worktree:

```sh
python <ai-workspace>/tools/promote_recovered_c.py audit \
  --root . \
  --base main \
  --head HEAD \
  --source <verified-worker-commit> \
  --path src/path/recovered.c
```

Then run the normal project gates:

- relocation-aware object comparison;
- exact-function regression check;
- affected Matching consumers;
- serialized DOL/REL build;
- DTK checksum;
- explicit retail DOL/REL byte comparison;
- source readability and semantic-debt review.

## 5. Open a human-facing PR

Push only the clean `recovery/*` branch and open it against `main`.

The PR should discuss:

- the recovered subsystem and behavior;
- target and consumer evidence;
- object/relocation results;
- retail verification;
- unresolved semantic debt.

It should not mention Claude, Codex, prompts, agents, token usage, orchestration,
knowledge cards or AI provenance. Those details stay in the AI workspace's local
queue, manifests and recovery metadata.

## Diagnostic override

`--allow-unverified` exists only for tool tests and planning experiments. It must
not be used for a real promotion to `main`.
