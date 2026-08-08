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

For ordinary recovered source, only a verified canonical source/header blob may be
transferred by the recovered-source promotion tool:

```text
src/**/*.c
src/**/*.cp
src/**/*.cpp
src/**/*.h
src/**/*.hpp
include/**/*.h
include/**/*.hpp
```

The transfer starts from a fresh branch and worktree based on current `main`. It
copies explicitly selected canonical source/header files from the verified worker commit and proves
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

- any path outside the canonical source/header paths above;
- noncanonical headers and build/configuration files;
- AI/agent attribution in source comments, branch names, or commit messages;
- co-author trailers;
- blobs that differ from the verified worker commit;
- unverified queue tasks unless an explicit diagnostic override is used.

### Authenticated original data

The dedicated `tools/promote_original_data.py` path is the only transfer path
for an allowlisted authenticated original-data record in
`config/recovery/original_data.json`. The current allowlist contains the exact
`src/musyx/runtime/dsp_import.c` source blob. The tool creates a fresh
`recovery/*` branch from `main` and transfers exact bytes and source provenance
only; it transfers no AI metadata, documentation, tooling, or generated state.

Original data is not clean-C recovery and earns zero clean-C credit. This narrow
path does not relax the raw numeric hexadecimal prohibition for ordinary
recovered source, and it leaves the separate original-data path unchanged. The
object, relocation, linked-retail, and checksum gates are complete for the
current record; the clean main-based promotion must still generate and review
the public status/progress sidecars before marking the owner `Matching`. The
record is `native_verified`, remains data-only (0/0 functions expected), and
does not change clean-C totals. See
[`docs/original_data_promotion.md`](docs/original_data_promotion.md) for the
source, object, linked-retail, and checksum evidence.

## Supporting changes

A recovered source may reveal that `main` needs a canonical header, symbol, split,
or build configuration update. Select and verify canonical headers with their
source in the recovered-source promotion; symbols, splits, and build
configuration remain separate supporting changes.

Promote them with `tools/promote_supporting_change.py`, which creates a second
branch (`project/*`) directly from `main`, transfers only declared verified
blobs, enforces the same attribution and contamination scans, and requires every
affected Matching consumer to be re-verified
(see `docs/supporting_change_promotion.md`). Review it as a separate
human-facing PR. Merge that support first when necessary, then recreate/rebase
and revalidate the canonical recovered-source promotion.

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
