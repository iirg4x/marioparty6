# Agent instructions

## Mission

Recover the most likely original Mario Party 6 source. Retail binary identity is
required for final promotion, but a byte match produced through unsupported
compiler manipulation is not faithful source recovery.

Use the nearest `AGENTS.md` for the directory you edit. This root file contains
only repository-wide rules; do not load every instruction or evidence document
into context by default.

## Start here

Run the workspace check:

```sh
python tools/agent.py doctor
```

Before editing, inspect and claim one owner in the shared local queue:

```sh
python tools/agent.py queue status
python tools/agent.py queue claim <owner> --agent claude
```

Codex uses `--agent codex`. Claude and Codex must use worktrees from the same
Git repository, different branches, and different build directories. Declare
central headers, `configure.py`, symbols, splits, and recovery schemas with
`--shared` before editing them. See `docs/concurrent_agents.md`.

Generate bounded context for the exact task:

```sh
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --budget 12000

python tools/agent.py context owner main:game/mgdata --budget 7000
```

The context generator automatically selects up to five applicable recovery
knowledge cards before the source: exact-target findings, owner constraints,
module/tag rules, compiler-wide diagnostics, and recorded counterexamples. Read
those cards before starting a compiler probe.

Inspect the selected cards directly when needed:

```sh
python tools/agent.py knowledge function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty
python tools/agent.py knowledge audit
```

Use exact owner IDs, stable identities, symbols, callers, consumers, and
recorded evidence first. Expand only a named dependency that remains material.
Do not attach all of `STATUS.md`, every historical wave report, or an entire
large translation unit to an agent prompt.

## Non-negotiable rules

- Work on a task branch in an isolated worktree, never directly on `main`.
- Claim the owner before editing; one agent owns one translation unit or one
  tightly connected function cluster.
- Never share a worktree, branch, or build directory between Claude and Codex.
- Add shared files to the claim before touching them. A queue conflict must be
  resolved by the orchestrator or integration worktree, not bypassed.
- Do not edit `orig/`, `build/`, `build.ninja`, `objdiff.json`, or generated
  context/report files.
- Review applicable knowledge cards and their counterexamples before repeating a
  source-shape experiment already investigated elsewhere.
- A compiler-wide card is a diagnostic rule, not permission to copy an
  owner-specific source shape.
- Do not invent semantic names, types, padding, globals, branches, or numeric
  domains. An honest `unk_*` or address symbol is better than unsupported
  certainty.
- Do not regress an independently exact function to close another target.
- Do not copy an authenticated compiler oddity from one owner into another.
- A pragma, forced inline/no-inline control, code-generation `volatile` or
  `register`, inline assembly, fake storage, or dead branch must be authenticated
  in recovery metadata, explicitly temporary debt, or rejected.
- Compiler experiments can prove source shape; they cannot prove semantic names.
- Preserve permanent target identity when accepting a semantic rename.

## Evidence order

1. Same-game maps, symbols, source remnants, and authenticated artifacts.
2. Target instructions, relocations, access widths, sections, and call contracts.
3. Same-game callers, consumers, messages, archives, and data ownership.
4. Authenticated sibling source from the Hudson/Mario Party lineage.
5. Controlled compiler probes for shape only.
6. Readability hypotheses, which remain provisional.

## Required work phases

1. **Research:** inspect the target, owner, direct dependencies, consumers,
   automatically selected knowledge cards, existing evidence, and rejected
   probes. Do not edit source yet.
2. **Natural candidate:** write the cleanest evidence-supported C without forcing
   the final instructions. A natural nonmatching candidate is useful evidence.
3. **Compiler reconciliation:** vary one evidenced dimension at a time, such as
   signedness, scope, lifetime, expression grouping, loop form, declaration
   visibility, or helper boundary.
4. **Adversarial review:** look for matching-only constructs, fabricated names or
   data, hidden consumer regressions, and claims stronger than their evidence.

Update the queue as the task moves through `researching`, `coding`, `verifying`,
`blocked`, or `ready`.

## Verification

Run the public-safe branch gate before handoff:

```sh
python tools/agent.py check --base origin/main
```

This validates tests, metadata, structured knowledge cards, the deterministic
recovery index, generated context/report paths, repository cleanup policy, and
newly added source-shape controls. It does **not** prove a retail build.

Any C/C++ source, shared header, compiler flag, object status, symbol, split, or
link configuration change also requires the relevant relocation-aware object
comparison, affected-consumer checks, serialized DOL/REL build, DTK checksum,
and explicit retail byte comparison before promotion.

Record the last verified commit, then release the claim after handoff:

```sh
python tools/agent.py queue release <owner> \
  --agent <claude-or-codex> \
  --status done \
  --verified-commit HEAD
```

Record reusable findings in `config/recovery/` rather than leaving them only in
an agent transcript. A repeated relationship between a source condition and
emitted output belongs in `compiler_patterns.json` with a rule, safe actions,
scope, examples, counterexamples, and evidence. Update binary, source-shape,
semantic, naming, and data status independently.

## Task handoff

A handoff or pull request must state:

- owner and stable target identity;
- research question and accepted evidence;
- applicable knowledge cards and whether a new card or counterexample was added;
- rejected probes or alternatives;
- natural candidate and compiler reconciliation, if any;
- exact-function and relocation impact;
- affected consumers;
- public-safe and private verification actually run;
- recovery metadata changed and remaining debt;
- queue status and last verified commit.

See `docs/agent_quickstart.md`, `docs/concurrent_agents.md`, `CONTRIBUTING.md`,
and `docs/recovery_standard.md` for the detailed workflow.
