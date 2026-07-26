# GitHub Actions on the AI workspace

## Permanent boundary

This branch is never merged into `main`. Its workflows validate AI recovery
infrastructure only; they are not intended to become `main` workflows.

`.github/workflows/ai-workspace-boundary.yml` rejects any pull request whose head
is `agent/recovery-context-workflow` and whose base is `main`.

The closed PR **“AI recovery workspace — do not merge into main”** records the
policy. Do not reopen it.

## Public-safe AI workspace validation

`.github/workflows/recovery-metadata.yml` does not contain or download retail
Mario Party 6 files. It validates:

- required AI workspace and promotion entrypoints;
- template cleanup and untracked retail/generated policy;
- recovery metadata, cards, freshness, and blind cases;
- Python compilation and synthetic tests;
- C-only promotion branch creation and rejection rules;
- confirmation that the full AI workspace cannot pass as a clean main promotion;
- owner catalog, queue migration, worktree audit, index, context, local evidence,
  organicity, reports, and source-quality policy.

Queue, catalog, promotion manifests, index, context, and report output remains
local or ephemeral.

## Draft pull requests and notifications

Internal AI-workspace PRs may push many exploratory commits. Automatic jobs are
skipped while such a PR is draft. Workers run locally:

```sh
python tools/agent.py hooks install
python tools/agent.py queue check-diff --base origin/main
python tools/agent.py check --base origin/main
```

Mark an **internal PR targeting the AI workspace branch** ready when remote
validation is desired. Never mark or open a workspace-to-main PR for validation;
the boundary workflow intentionally fails it.

## Main promotion has different CI

A clean source promotion is created from `main` with:

```sh
python tools/promote_recovered_c.py create ...
```

That branch contains only the selected recovered C blobs and whatever supporting
changes a human recreates from the clean checkout. It does not contain the AI
workflows, agent docs, metadata, or test harness.

`main` may keep its own ordinary build/source checks. Do not copy these AI
workspace workflows to it automatically.

## Why retail verification is private

A complete build requires copyrighted inputs under `orig/GP6E01/`. Public CI
cannot prove DOL/REL identity, private target-object reports, DTK retail hashes,
or consumer comparisons dependent on extracted objects.

A successful public AI-workspace run is not a successful retail build.

## Serialized private verification

Use a local integration or clean promotion worktree with legally provisioned
inputs. Acquire exclusive AI-workspace resources before the full build:

```sh
python tools/agent.py queue acquire-resource integration \
  --agent integrator --owner <owner>
python tools/agent.py queue acquire-resource retail-build \
  --agent integrator --owner <owner>
```

Run:

```sh
python tools/agent.py check --base <base-sha>
python configure.py --map
ninja -j1
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

Compare `main.dol` and every affected REL, check Matching consumers, and inspect
generated symbols.

After the AI worker commit is verified, create the clean C-only promotion branch
from `main` and rerun the relevant object, consumer, DOL/REL, checksum, byte, and
readability gates there. Only that clean branch is eligible for a human-facing
pull request to `main`.

Upload only permitted compact logs—never retail/rebuilt binaries, extracted
assets, or reports embedding copyrighted data.
