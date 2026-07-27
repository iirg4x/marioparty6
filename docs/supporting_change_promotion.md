# Promoting supporting changes to clean `main`

## Principle

Recovered C is promoted by `tools/promote_recovered_c.py` and always stops at
`src/**/*.c`. The supporting changes a recovery depends on — shared headers,
symbols, splits, TU declarations, and `configure.py` object-status flips — use
this workflow instead. The same permanent boundary applies: the AI workspace is
never merged; promotion is a content transfer onto a `project/*` branch created
directly from `main`.

```text
recovery/<subsystem>       verified C blobs only
project/<supporting-fix>   audited supporting change, also cut from main
```

## Preconditions

The queue task must be `ready` or `done`, its verification bound to the exact
source commit, its change class `shared-interface` or `build-configuration`,
its proof recording consumer results, and every promoted path declared in the
claim's shared files.

The tool accepts only explicitly declared added or modified:

```text
include/**
config/**        (never config/recovery/**)
configure.py
```

It refuses `src/**/*.c` (use `tools/promote_recovered_c.py`), AI-workspace
metadata, tools, docs, workflows, and everything else. There is no automatic
path selection: declare each path.

## 1. Plan the transfer

From the AI workspace:

```sh
python tools/promote_supporting_change.py plan \
  --base main \
  --source <verified-worker-commit> \
  --owner <queue-owner> \
  --path include/game/<header>.h
```

The plan resolves immutable commits, checks the queue binding above, computes
the minimal per-file diff against `main`, records source/base blobs plus
SHA-256 values, and scans full file contents for AI attribution and
AI-workspace contamination: prompt-style comments, `TODO(ai...)`, queue owner
identifiers, coordination paths, and Co-authored-by trailers. Findings refuse
the plan; nothing is ever stripped silently.

For every changed header the plan lists the affected consumers through the
transitive header-include closure, split into configured `Matching` TUs and
others. That fan-out list is the re-verification obligation.

## 2. Create a clean `project/*` worktree

```sh
python tools/promote_supporting_change.py create \
  --base main \
  --source <verified-worker-commit> \
  --owner <queue-owner> \
  --path include/game/<header>.h \
  --branch project/<supporting-fix> \
  --worktree ../marioparty6-project-<fix> \
  --title "Correct <interface> <property>"
```

The command:

1. creates the branch directly from `main`;
2. copies exact declared blobs from the verified commit;
3. stages only the declared paths and confirms nothing else is staged;
4. rejects AI attribution and workspace contamination in branch name, file
   contents, and commit message;
5. creates one ordinary project commit;
6. audits that the diff against `main` contains only the declared paths and
   that every promoted blob equals the verified source blob;
7. writes a local ignored manifest under `build/promotion/` in the AI
   workspace.

The branch never descends from the AI workspace, so it contains none of its
tools, prompts, queue state, or metadata.

## 3. Verify in the clean worktree

Inside the `project/*` worktree run the serialized gates:

```sh
python configure.py
ninja -j1
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

Compare `build/GP6E01/main.dol` with `orig/GP6E01/sys/main.dol` and every
affected REL with its retail file. Re-verify each Matching TU from the plan's
fan-out list with a relocation-aware object comparison; a header change is not
proven until every Matching consumer is exact again.

## 4. Audit with evidence

The audit refuses to pass until the evidence is stated explicitly:

```sh
python <ai-workspace>/tools/promote_supporting_change.py audit \
  --root . \
  --base main \
  --head HEAD \
  --source <verified-worker-commit> \
  --path include/game/<header>.h \
  --build-gate pass \
  --checksum pass \
  --consumer <matching-owner>=exact
```

Every Matching consumer of every changed header must appear as
`--consumer <owner>=exact`. The audit recomputes the fan-out in the clean
worktree, so a consumer cannot be omitted by editing the plan. It also
re-checks path scope, blob equality, commit messages, and contamination.

## 5. Open a human-facing project PR

Push only the clean `project/*` branch and open it against `main`.

The PR discusses the interface or configuration change, the target evidence
that authorizes it (for example the shipped DOL's signature), the affected
consumers, and the retail verification results. It does not mention agents,
prompts, queue state, or AI provenance.

When a `recovery/*` C branch depends on this change, merge the supporting PR
first, then rebase or recreate the C promotion branch on the updated `main`,
rerun its blob audit, and repeat the retail gates.

## Diagnostic override

`--allow-unverified` exists only for tool tests and planning experiments. It
must not be used for a real promotion to `main`.
