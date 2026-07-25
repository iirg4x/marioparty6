# GitHub Actions

## Public-safe workflow

The active workflow is:

```text
.github/workflows/recovery-metadata.yml
```

It runs without retail Mario Party 6 files and is safe for the public
repository. The workflow checks out full history, installs Python, and executes
the same agent-facing public gate used locally.

The public gate covers:

- Python syntax compilation;
- unit tests under `tools/tests`;
- recovery metadata and cross-reference validation;
- deterministic SQLite index generation;
- bounded context and human-report smoke generation;
- required agent entrypoints and template-cleanup policy;
- changed private/generated paths;
- diff whitespace;
- newly added source-shape controls that lack scoped evidence.

Generated context, databases, and reports remain ephemeral workflow output. They
are not uploaded as source artifacts and are not committed.

## Draft pull requests and notification noise

Claude and Codex may push many intermediate commits from separate worktrees.
Automatic Actions jobs are therefore skipped while a pull request is a draft.
During active development, each agent runs the local public gate:

```sh
python tools/agent.py check --base origin/main
```

Mark the pull request **Ready for review** when the branch is ready for remote
validation. GitHub Actions then runs automatically, and later commits to that
non-draft pull request trigger fresh checks. A remote check can also be started
earlier through manual `workflow_dispatch`.

A draft synchronization still appears in the Actions history as `skipped`; it
is not a failed check and should not produce failure-notification emails. This
policy avoids one failure email per exploratory agent commit while retaining a
required remote gate before merge.

## Required branch check

After this branch is merged, configure the repository’s `main` protection to
require the **Recovery metadata** workflow before merge. Keep pull requests and
full history enabled so changed-line checks can compare against the real base
SHA. Draft PRs must be marked ready before merge, which activates the required
check.

## Why the retail build is not public

A complete build needs copyrighted retail inputs under `orig/GP6E01/`. Those
files must never be committed, downloaded by public CI, exposed through caches,
or uploaded as artifacts.

The public workflow therefore cannot prove:

- DOL or REL byte identity;
- the DTK `build.sha1` gate;
- private objdiff reports generated from extracted target objects;
- consumer comparisons that require the retail split objects.

A successful public workflow must not be described as a successful retail
build.

## Private retail-build automation

Use one of these private environments:

- a self-hosted runner with locally provisioned retail inputs;
- a private build repository/container with access restricted to this project;
- a local integration worktree.

The private environment should provide the extracted `orig/GP6E01` layout at
runtime and remove it after the job. Pin any container image by immutable digest,
not a mutable `main` tag.

A private source-promotion job should run, in order:

```sh
python tools/agent.py check --base <base-sha>
python configure.py --map
ninja -j1
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

Use `build/tools/dtk.exe` on Windows. Then perform explicit byte comparisons for
`main.dol` and every affected REL, and check that generated symbol files contain
no unexplained diff.

Upload only permitted logs and compact summaries. Do not upload retail binaries,
rebuilt DOL/REL files, extracted objects, disc contents, or reports that embed
copyrighted data.

## Progress publishing

Progress publishing is intentionally not configured in the public workflow.
Add it only after the project has a real progress service slug and secret. Keep
progress upload separate from verification so a service outage cannot hide or
invalidate a failed build gate.

## Workflow changes

Any edit to workflow files, compiler/tool pins, `configure.py`, object status,
symbols, splits, or link configuration requires careful review. Public CI can
validate workflow structure and repository policy, but a private retail build is
required before treating those changes as integration-safe.
