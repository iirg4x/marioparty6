# Permanent AI recovery workspace

This branch is an **AI-forward recovery laboratory**. It is not a replacement for
`main`, and it must never be merged into `main`.

## Branch roles

### `main`

`main` is the human-facing project:

- normal source tree and build configuration;
- readable recovered C;
- human-oriented commit history and pull requests;
- no prompts, queue metadata, knowledge-card machinery, benchmark harness, AI
  attribution, or orchestration documentation.

### `agent/recovery-context-workflow`

This branch is the recovery operating system for Claude, Codex, and local
orchestration:

- worktree and claim management;
- context generation and dependency indexing;
- knowledge cards and freshness records;
- blind benchmark and organicity tooling;
- agent instructions, hooks, queue state, and experimental CI.

Its history and files intentionally stay on this branch.

## No merge rule

Never:

- merge this branch into `main`;
- squash this branch into `main`;
- rebase this branch onto `main` and merge it;
- cherry-pick tooling, agent docs, metadata, or workflow commits to `main`;
- open a pull request whose diff contains this branch's infrastructure.

The closed PR titled **“AI recovery workspace — do not merge into main”** is a
historical marker, not a merge candidate.

## Allowed transfer

Only a verified recovered C blob may be transferred automatically:

```text
src/**/*.c
```

The transfer starts from a fresh branch and worktree based on current `main`. It
copies explicitly selected C files from the verified worker commit and proves
each promoted Git blob is identical.

```sh
python tools/promote_recovered_c.py plan \
  --base main \
  --source <verified-worker-commit> \
  --owner <queue-owner>

python tools/promote_recovered_c.py create \
  --base main \
  --source <verified-worker-commit> \
  --owner <queue-owner> \
  --path src/path/recovered.c \
  --branch recovery/<human-topic> \
  --worktree ../marioparty6-promotion-<topic> \
  --title "Recover <human-readable subsystem>"
```

The tool rejects:

- any path outside `src/**/*.c`;
- headers and build/configuration files;
- AI/agent attribution in source comments, branch names, or commit messages;
- co-author trailers;
- blobs that differ from the verified worker commit;
- unverified queue tasks unless an explicit diagnostic override is used.

## Supporting changes

A recovered C file may reveal that `main` needs a header, symbol, split, or build
configuration update. Those changes are not imported from the AI branch and do
not belong in the C promotion PR.

Create a second branch directly from `main`, recreate the supporting change from
target evidence, and review it as a separate human-facing PR. Merge that support
first when necessary, then recreate/rebase and revalidate the C-only promotion.

This prevents prompts, scaffolding, generated metadata, speculative names, and
agent-specific structure from leaking into public history while keeping the C
promotion mechanically auditable.

## Promotion verification

In the clean C promotion worktree:

1. audit the branch boundary;
2. run relocation-aware object comparison;
3. verify affected consumers;
4. run the serialized DOL/REL build;
5. run the DTK checksum and explicit retail byte comparisons;
6. inspect source readability and remaining semantic debt;
7. push the clean `recovery/*` branch;
8. open a normal human-facing pull request to `main`.

```sh
python <ai-workspace>/tools/promote_recovered_c.py audit \
  --root <promotion-worktree> \
  --base main \
  --head HEAD \
  --source <verified-worker-commit>
```

Promotion manifests and queue evidence remain under ignored `build/` in the AI
workspace. They are never committed to the clean promotion branch.
