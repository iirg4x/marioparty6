# Contributing

This repository accepts both human- and agent-assisted recovery work. The goal
is faithful source recovery with retail binary identity as the final objective
check—not byte closure through unexplained source tricks.

## Choose and isolate a task

Use a GitHub recovery-task issue or create one from the issue form. The task must
name:

- one translation-unit owner or a tightly connected function cluster;
- stable target identity and current symbol when known;
- the research question;
- current binary and source-quality status;
- expected consumers and verification scope.

Use an isolated branch or worktree:

```text
agent/<owner>-<goal>
```

One agent should own one source owner at a time. Shared headers, `configure.py`,
central symbol files, and recovery schemas require explicit integration scope.

## Prepare the workspace

```sh
python tools/agent.py doctor
python tools/agent.py context function <symbol> --owner <owner-id>
```

Read the root and nearest nested `AGENTS.md`. Do not start with the whole
repository or a complete historical status file in context.

## Recovery workflow

1. Research without editing: target code, relocations, calls, data references,
   consumers, same-game domains, sibling evidence, and existing probes.
2. Write a natural evidence-supported candidate.
3. Reconcile compiler shape one variable at a time.
4. Review adversarially for invented semantics and matching-only constructs.
5. Update durable recovery metadata and remaining debt.

See `docs/recovery_standard.md` for evidence and promotion rules.

## Files that must not be committed

Never commit:

- retail inputs under `orig/`;
- generated files under `build/`;
- `build.ninja`, `objdiff.json`, or `ctx.c`;
- local editor, agent, or virtual-environment state;
- rebuilt DOL/REL binaries or extracted game assets.

Generated objdiff and context reports may be referenced by path in evidence, but
the reusable conclusion belongs in `config/recovery/` or a concise evidence
report.

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
python tools/agent.py check --base origin/main
```

It deliberately does not claim a retail build. For source promotion, also run
the configured serialized build, relocation-aware comparisons, DTK checksum,
and explicit DOL/REL byte comparisons.

## Source changes

Do not regress independently exact functions. Do not introduce a pragma,
forced-inline control, code-generation `volatile`/`register`, inline assembly,
fake padding, opaque blob, or dead branch without a scoped recovery exception
and evidence. A temporary exception must include a removal condition.

Semantic names and fields require evidence. Keep uncertain identifiers unknown
rather than improving readability through fiction.

## Commits

Keep commits scoped and descriptive. Separate semantic cleanup, compiler-shape
reconciliation, shared-interface changes, and workflow/tooling changes when they
can be reviewed independently.

Useful commit details include:

```text
Owner: REL/mdpartydll/mdparty.c
Stable-Identity: mdpartydll:0xBBD8
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
natural candidate, compiler reconciliation, exact-function impact, consumers,
verification, metadata changes, and remaining debt.

A useful handoff must allow another contributor to continue without reading the
entire agent transcript.
