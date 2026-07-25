# GitHub Actions

## Public-safe workflow

The active workflow is `.github/workflows/recovery-metadata.yml`. It does not
contain or download retail Mario Party 6 files.

It validates:

- required agent/coordination/context entrypoints;
- template cleanup and untracked retail/generated policy;
- recovery metadata, knowledge cards, and freshness records;
- Python compilation and all synthetic public-safe tests;
- operational owner catalog generation and queries;
- queue schema migration and empty-queue health;
- active worktree assignment audit;
- deterministic recovery index and searchable rules/actions;
- symptom-aware knowledge selection;
- local objdiff summary injection and section-budget preservation;
- owner context, report, and wave-distillation audit;
- changed-path, whitespace, and source-quality policy.

Queue, catalog, index, context, and report output remains ephemeral and is not
uploaded as source evidence.

## Draft pull requests and notification noise

Claude and Codex may push many intermediate commits. Automatic jobs are skipped
while a PR is a draft. Workers run locally:

```sh
python tools/agent.py hooks install
python tools/agent.py queue check-diff --base origin/main
python tools/agent.py check --base origin/main
```

Mark the PR **Ready for review** when remote validation is desired. Later pushes
to a non-draft PR run the workflow again. `workflow_dispatch` remains available
for a deliberate early remote check.

A draft synchronization appears as `skipped`, not failed. This prevents an email
for every exploratory commit while retaining a real pre-merge gate.

## Branch protection

After merge, protect `main` and require the **Recovery metadata** job. Keep full
checkout history so changed-line checks can compare against the exact base SHA.

## Why retail verification is private

A complete build requires copyrighted inputs under `orig/GP6E01/`. Public CI
cannot prove DOL/REL identity, private target-object reports, DTK retail hashes,
or consumer comparisons dependent on extracted objects.

A successful public workflow must never be described as a successful retail
build.

## Serialized private integration

Use a local integration worktree, private self-hosted runner, or restricted
private environment with legally provisioned inputs.

Acquire exclusive resources before the full build:

```sh
python tools/agent.py queue acquire-resource integration \
  --agent integrator --owner <owner>
python tools/agent.py queue acquire-resource retail-build \
  --agent integrator --owner <owner>
```

Then run:

```sh
python tools/agent.py check --base <base-sha>
python configure.py --map
ninja -j1
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

Use `dtk.exe` on Windows. Compare `main.dol` and every affected REL explicitly,
check Matching consumers, and ensure generated symbols contain no unexplained
diff.

Finalize only after all private gates pass:

```sh
python tools/agent.py integration finalize <owner> \
  --agent integrator \
  --retail-gate pass \
  --checksum pass \
  --consumer <consumer>=exact \
  --toolchain GC/1.3.2
```

Finalization confirms that each claimed path in the integrated tree still
matches the worker’s verified commit. Upload only permitted compact logs and
summaries—never retail/rebuilt binaries, extracted assets, or reports embedding
copyrighted data.

## Workflow and build-configuration changes

Changes to workflows, tool pins, `configure.py`, compiler flags, object status,
symbols, splits, or link order require public validation and a private retail
integration run before being considered integration-safe.

Progress publishing remains separate from verification and is intentionally not
configured until a real service slug and secret exist.
