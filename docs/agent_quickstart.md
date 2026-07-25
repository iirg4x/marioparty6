# Agent quickstart

This is the shortest safe path from a fresh checkout to a reviewable task.

## 1. Read only the applicable instructions

Read:

1. root `AGENTS.md`;
2. the nearest nested `AGENTS.md` for the files you will edit;
3. this quickstart.

Open `docs/recovery_standard.md` when making source-authenticity decisions. Do
not begin by loading all of `STATUS.md` or the complete wave history.

## 2. Inspect the workspace

```sh
python tools/agent.py doctor
```

Resolve failures before editing. Warnings about missing `orig/` or Ninja are
acceptable for documentation and public-safe metadata work, but they mean a
retail build cannot be claimed.

## 3. Claim one owner

Use a recovery-task GitHub issue and an isolated branch/worktree:

```text
agent/<owner>-<goal>
```

One task should normally own one translation unit or one tightly connected
function cluster. Claude and Codex working on the same PC must use separate
worktrees, branches, and generated build directories. Avoid concurrent edits to
central headers, `configure.py`, symbol files, and recovery schemas.

## 4. Generate bounded context

For a function:

```sh
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --budget 12000
```

For an owner:

```sh
python tools/agent.py context owner main:game/mgdata --budget 7000
```

The default output is an ignored Markdown file under `build/context/`. Use
`--stdout` only when the caller needs the packet directly.

Before the current source, the packet automatically includes up to five relevant
knowledge cards ranked by exact target, owner, compiler, and counterexample.
These cards state the source condition, expected output effects, known
signatures, coding rule, safe actions, and evidence. Read them before trying a
compiler-shape experiment.

Inspect just the selected knowledge when needed:

```sh
python tools/agent.py knowledge function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty
```

Expand one specific missing dependency rather than increasing context
indiscriminately.

## 5. Separate research from edits

Before touching source, record:

- owner and stable identity;
- automatically selected rules, owner constraints, and counterexamples;
- target signature, instructions, relocations, and sections;
- direct callers/callees and referenced data;
- consumer widths and same-game semantic domains;
- relevant sibling evidence;
- known accepted and rejected compiler probes;
- unresolved semantic questions.

Then write the cleanest natural candidate. Only after that should compiler
reconciliation vary signedness, scope, lifetime, expression shape, loop form,
visibility, chronology, or helper boundaries.

A compiler-wide card is a diagnostic. Do not copy its example source into the
current owner without local evidence. A recorded counterexample is a warning
that the tempting rule has already failed in that scope.

## 6. Keep durable knowledge out of chat history

Update the relevant files under `config/recovery/`:

- owner state and debt;
- accepted/rejected evidence;
- stable naming decisions;
- scoped source-shape exceptions;
- source-to-output knowledge cards with conditions and safe actions;
- confirmed examples and counterexamples.

When an experiment teaches a reusable relation such as “adding this declaration
visibility changes these call sites,” add or refine a card in
`compiler_patterns.json`. Do not create another wave document as the only place
where the reusable conclusion exists.

Audit the historical extraction backlog with:

```sh
python tools/agent.py knowledge audit
```

Generated SQLite, reports, audit output, and context packs remain under ignored
`build/`.

## 7. Run the public-safe gate

```sh
python tools/agent.py check --base origin/main
```

This runs Python compilation, unit tests, owner and knowledge-card validation,
deterministic index generation, context/report smoke tests, repository cleanup
policy, diff whitespace checks, generated/private-path checks, and changed-line
source-quality review.

It does not run the retail build.

## 8. Run private verification when required

Any recovered C/C++, shared header, symbol, split, compiler flag, object status,
or link change requires the appropriate local private gates:

- relocation-aware object comparison;
- exact-function regression check;
- affected Matching consumers;
- serialized DOL/REL build;
- DTK checksum;
- explicit retail DOL/REL byte comparison.

State exactly which gates ran in the pull request.

## 9. Handoff

Complete the pull request template. Include accepted and rejected evidence,
applicable knowledge cards, new cards or counterexamples, natural candidate,
compiler reconciliation, exact/relocation impact, consumers, metadata changes,
verification, and remaining debt.

The handoff should be sufficient without the original agent conversation.
