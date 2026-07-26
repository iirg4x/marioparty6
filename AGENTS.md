# Agent instructions

## Permanent branch boundary

This is the AI-forward recovery workspace. **Never merge, squash, rebase, or
cherry-pick this branch into `main`.**

`main` is the clean, human-facing project. The only automatic transfer allowed
from this workspace is an exact verified `src/**/*.c` blob copied into a fresh
`recovery/*` branch created directly from `main`.

Read [`AI_WORKSPACE.md`](AI_WORKSPACE.md) and
[`docs/main_promotion.md`](docs/main_promotion.md).

## Mission

Recover the most likely original Mario Party 6 source. Retail binary identity is
required for final promotion, but a byte match produced through unsupported
compiler manipulation is not faithful source recovery.

Use the nearest `AGENTS.md` for the directory you edit. Do not load every
instruction or historical evidence document by default.

## Start here

```sh
python tools/agent.py doctor
python tools/agent.py hooks install
python tools/agent.py queue status
```

Work from an isolated task worktree. Claim one owner before editing, or create
and claim it automatically:

```sh
python tools/agent.py worktree create <owner> \
  --agent claude \
  --base <AI_BASE_COMMIT>
```

Codex uses `--agent codex`. Claude and Codex must never share a worktree, branch,
or build directory. Declare central headers, `configure.py`, symbols, splits,
and recovery schemas with `--shared` before editing them.

`AI_BASE_COMMIT` is the pinned commit of `agent/recovery-context-workflow` for
the active batch. Worker branches and diff checks use that exact commit. Only
`tools/promote_recovered_c.py` starts a new branch from `main`.

Generate bounded task context:

```sh
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --symptom "saved register lifetime" \
  --local-evidence \
  --budget 12000
```

The packet selects exact-target findings, owner constraints, compiler
diagnostics, freshness warnings, and counterexamples before the source.
Historical wave bodies are not loaded automatically.

## Non-negotiable rules

- Never edit directly on `main`.
- Never open or maintain a merge PR from this workspace to `main`.
- Never transfer agent tooling, prompts, queue files, metadata, benchmarks,
  workflow files, reports, docs, or commit history to `main`.
- Claim the source owner and every shared write path before editing.
- Run `python tools/agent.py queue check-diff --base <AI_BASE_COMMIT>` before commits
  and handoff. Committed, staged, unstaged, and untracked paths must remain
  inside the claim and outside every other active claim.
- Do not edit or commit `orig/`, `build/`, `build.ninja`, `objdiff.json`, generated
  context, or rebuilt retail containers.
- Read applicable knowledge cards and counterexamples before repeating a known
  compiler probe. Compiler-wide cards are diagnostics, not source templates.
- Do not invent semantic names, types, padding, globals, branches, or numeric
  domains. Honest unknowns are preferable to unsupported readability.
- Do not regress an independently exact function or consumer to close another
  target.
- A pragma, forced inline/no-inline control, code-generation `volatile` or
  `register`, inline assembly, fake storage, or dead branch must be authenticated,
  temporary debt, or rejected.
- Compiler experiments can prove source shape; they cannot prove semantic names.
- A blind benchmark must freeze and hash the candidate before reveal. Assembly
  equality, retained-source similarity, organicity, and reproducibility are
  separate results.
- Token-identical C may inherit questionable retained-source structure. Never
  describe source fidelity alone as proof that a candidate is organic or original.

## Evidence order

1. Same-game maps, symbols, source remnants, and authenticated artifacts.
2. Target instructions, relocations, widths, sections, and call contracts.
3. Same-game callers, consumers, messages, archives, and ownership.
4. Authenticated sibling source.
5. Controlled compiler probes for source shape.
6. Readability hypotheses, kept provisional.

## Work phases

1. **Research:** target, relocations, callers, consumers, selected cards, local
   reports, and rejected probes. Do not edit yet.
2. **Natural candidate:** write the cleanest evidence-supported C without forcing
   code generation.
3. **Compiler reconciliation:** vary one evidenced dimension at a time.
4. **Adversarial review:** check for invented semantics, match-only constructs,
   hidden consumer regressions, and claims stronger than the evidence.

Update queue status through `researching`, `coding`, `verifying`, `blocked`, and
`ready`.

## Worker verification

Commit first and leave the worktree clean:

```sh
python tools/agent.py check --base <AI_BASE_COMMIT>

python tools/agent.py queue verify <owner> \
  --agent <claude-or-codex> \
  --public-gate pass \
  --object-report build/GP6E01/<report>.json \
  --functions-exact <exact/total> \
  --relocations exact \
  --consumer <owner>=exact \
  --toolchain GC/1.3.2

python tools/agent.py queue update <owner> \
  --agent <claude-or-codex> --status ready
```

Verification is tied to the clean current commit. Any later change requires a
new proof.

## Integration and clean promotion

Workers stop at `ready`. The AI integration worktree may run serialized DOL/REL,
consumer, checksum, and retail comparisons, but its branch still does not merge
to `main`.

After the source commit is fully verified, create a separate human-facing branch
from `main`:

```sh
python tools/promote_recovered_c.py create \
  --base main \
  --source <verified-worker-commit> \
  --owner <queue-owner> \
  --path src/path/recovered.c \
  --branch recovery/<human-topic> \
  --worktree ../marioparty6-promotion-<topic> \
  --title "Recover <subsystem>"
```

The command accepts only `src/**/*.c`, copies exact verified blobs, rejects AI
attribution, and proves the clean branch contains none of this workspace's
infrastructure.

Header, symbol, split, or build changes must be recreated and reviewed separately
from the clean `main` worktree. Never copy them automatically from this branch.

## Blind benchmark evidence

Use `tools/blind_recovery.py` for controlled holdouts. A reproducible case must
preserve the evidence packet, frozen candidate, target and candidate assembly,
result, hashes, source path, source commit, and blindness assertions.

```sh
python tools/blind_recovery.py audit
python tools/blind_recovery.py audit --strict --replay
```

Automated organicity findings are review prompts. Confirm or reject them using
target, consumer, sibling, and compiler evidence.

## Durable knowledge and handoff

Store reusable findings in `config/recovery/`, not only in chat or a wave report.
A repeated source-to-output relationship belongs in `compiler_patterns.json`
with trigger, effects, rule, safe actions, scope, examples, counterexamples, and
evidence. These records stay on the AI workspace branch.

A handoff must state owner and stable identity, selected cards, accepted/rejected
evidence, natural candidate, compiler reconciliation, exact/relocation impact,
consumers, worker proof, clean-promotion requirements, metadata changes,
remaining debt, and queue status.
