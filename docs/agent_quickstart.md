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
function cluster. Avoid concurrent edits to central headers, `configure.py`,
symbol files, and recovery schemas.

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

The first packet contains the recovery contract, owner state, current source,
bounded owner neighbourhood, accepted and rejected evidence, constraints,
naming state, debt, report availability, and acceptance criteria. Expand one
specific missing dependency rather than increasing context indiscriminately.

## 5. Separate research from edits

Before touching source, record:

- owner and stable identity;
- target signature, instructions, relocations, and sections;
- direct callers/callees and referenced data;
- consumer widths and same-game semantic domains;
- relevant sibling evidence;
- known accepted and rejected compiler probes;
- unresolved semantic questions.

Then write the cleanest natural candidate. Only after that should compiler
reconciliation vary signedness, scope, lifetime, expression shape, loop form,
visibility, chronology, or helper boundaries.

## 6. Keep durable knowledge out of chat history

Update the relevant files under `config/recovery/`:

- owner state and debt;
- accepted/rejected evidence;
- stable naming decisions;
- scoped source-shape exceptions;
- compiler patterns with conditions and counterexamples.

Generated SQLite, reports, and context packs remain under ignored `build/`.

## 7. Run the public-safe gate

```sh
python tools/agent.py check --base origin/main
```

This runs Python compilation, unit tests, metadata validation, deterministic
index generation, a context/report smoke test, repository cleanup policy, diff
whitespace checks, generated/private-path checks, and changed-line source
quality review.

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
natural candidate, compiler reconciliation, exact/relocation impact, consumers,
metadata changes, verification, and remaining debt.

The handoff should be sufficient without the original agent conversation.
