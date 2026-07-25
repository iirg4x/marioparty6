# Contributing

This repository accepts both human- and agent-assisted recovery work. The goal
is faithful source recovery with retail binary identity as the final objective
check—not byte closure through unexplained source tricks.

## Choose, queue, and isolate a task

Use a GitHub recovery-task issue or create one from the issue form. The task must
name:

- one translation-unit owner or a tightly connected function cluster;
- stable target identity and current symbol when known;
- the research question;
- current binary and source-quality status;
- expected consumers and verification scope.

Use worktrees from the same Git repository and isolated branches:

```text
agent/<agent>-<owner>-<goal>
```

Claude and Codex operating on the same PC must use separate worktrees, branches,
and build directories. Before editing, claim the owner:

```sh
python tools/agent.py queue claim <owner> --agent claude
```

Codex uses `--agent codex`. The orchestrator may create pending bulk work with
`queue add`. Shared headers, `configure.py`, central symbol files, splits, and
recovery schemas must be added to the claim before they are edited. Conflicting
claims are rejected rather than silently merged.

See `docs/concurrent_agents.md` for queue storage, same-PC setup, and integration
rules.

## Prepare the workspace

```sh
python tools/agent.py doctor
python tools/agent.py queue status
python tools/agent.py context function <symbol> --owner <owner-id>
```

The context pack automatically selects applicable recovery knowledge cards and
counterexamples before the source. Inspect just those rules with:

```sh
python tools/agent.py knowledge function <symbol> --owner <owner-id>
```

Read the root and nearest nested `AGENTS.md`. Do not start with the whole
repository or a complete historical status file in context.

## Recovery workflow

1. Claim the owner and declare expected shared files.
2. Research without editing: target code, relocations, calls, data references,
   consumers, same-game domains, selected knowledge cards, counterexamples, and
   existing probes.
3. Write a natural evidence-supported candidate.
4. Reconcile compiler shape one variable at a time.
5. Review adversarially for invented semantics and matching-only constructs.
6. Update durable owner metadata, recovery debt, and reusable source-to-output
   knowledge.
7. Record the last verified commit and release the claim after handoff.

Keep the queue status current:

```sh
python tools/agent.py queue update <owner> \
  --agent claude \
  --status coding
```

A compiler-wide card is a diagnostic rule, not permission to copy an example’s
source. An owner constraint applies only to its explicit owner or stable
identity. A counterexample must remain visible.

See `docs/recovery_standard.md` and `docs/context_workflow.md` for evidence,
selection, and promotion rules.

## Files that must not be committed

Never commit:

- retail inputs under `orig/`;
- generated files under `build/`;
- `build.ninja`, `objdiff.json`, or `ctx.c`;
- the local queue under Git's common directory;
- local editor, agent, or virtual-environment state;
- rebuilt DOL/REL binaries or extracted game assets.

Generated objdiff, audit, and context reports may be referenced by path in
evidence, but the reusable conclusion belongs in `config/recovery/`.

## Verification matrix

| Change | Public agent gate | Object/consumer proof | Retail DOL/REL gate |
| --- | --- | --- | --- |
| Documentation only | Required | Not normally | Not normally |
| Python tools or recovery metadata | Required | Not normally | Not normally |
| Private C implementation | Required | Required | Required before promotion |
| Shared header, type, data owner, or symbol | Required | Required for every affected Matching consumer | Required |
| Compiler flags, object status, splits, link order, or `configure.py` | Required | Required | Required |

Run the public gate:

```sh
python tools/agent.py queue update <owner> \
  --agent <claude-or-codex> \
  --status verifying \
  --verified-commit HEAD

python tools/agent.py check --base origin/main
```

It validates queue health, knowledge-card schema and references, tests,
indexing, context generation, and source-quality policy. It deliberately does
not claim a retail build. For source promotion, also run the configured
serialized build, relocation-aware comparisons, DTK checksum, and explicit
DOL/REL byte comparisons.

## Source changes

Do not regress independently exact functions. Do not introduce a pragma,
forced-inline control, code-generation `volatile`/`register`, inline assembly,
fake padding, opaque blob, or dead branch without a scoped recovery exception
and evidence. A temporary exception must include a removal condition.

Semantic names and fields require evidence. Keep uncertain identifiers unknown
rather than improving readability through fiction.

When a probe reveals a reusable relation between source and emitted output, add
or update a knowledge card with:

- the exact triggering source condition;
- required preconditions;
- possible emitted changes and recognizable signatures;
- one clear coding/investigation rule;
- concrete safe actions;
- explicit stable-ID, owner, module, tag, compiler, or project scope;
- examples, counterexamples, related exceptions, and evidence.

Do not leave that conclusion only in a wave report or agent conversation.

## Commits

Keep commits scoped and descriptive. Separate semantic cleanup, compiler-shape
reconciliation, shared-interface changes, knowledge extraction, and
workflow/tooling changes when they can be reviewed independently.

Useful commit details include:

```text
Owner: REL:mdpartydll:mdparty
Stable-Identity: mdpartydll:0xBBD8
Agent: claude / codex
Queue-Status: ready / done
Last-Verified-Commit: <sha>
Knowledge-Cards: reviewed IDs; added/refined IDs or none
Functions-Exact: before -> after
Relocations: exact / changed / not run
Consumers: names or none
Public-Gate: pass
Retail-Gate: pass / not run
Evidence: durable report or manifest path
```

Do not claim a gate that was not run.

## Pull requests and handoff

Complete the pull request template. State accepted and rejected evidence,
applicable knowledge cards, any new rule or counterexample, natural candidate,
compiler reconciliation, exact-function impact, consumers, verification,
metadata changes, remaining debt, queue status, and last verified commit.

After handoff, release the claim:

```sh
python tools/agent.py queue release <owner> \
  --agent <claude-or-codex> \
  --status done \
  --verified-commit HEAD
```

A useful handoff must allow another contributor to continue without reading the
entire agent transcript.
