# Agent instructions

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

Work from an isolated task worktree. Claim one owner before editing, or let the
bootstrap command create and claim it:

```sh
python tools/agent.py worktree create <owner> --agent claude
```

Codex uses `--agent codex`. Claude and Codex must never share a worktree, branch,
or build directory. Declare central headers, `configure.py`, symbols, splits,
and recovery schemas with `--shared` before editing them.

Generate bounded task context:

```sh
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --symptom "saved register lifetime" \
  --local-evidence \
  --budget 12000
```

The packet automatically selects exact-target findings, owner constraints,
compiler diagnostics, freshness warnings, and counterexamples before the source.
Historical wave bodies are not loaded automatically.

## Non-negotiable rules

- Never edit directly on `main`.
- Claim the source owner and every shared write path before editing.
- Run `python tools/agent.py queue check-diff --base origin/main` before commits
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

Commit first and leave the worktree clean. Then run:

```sh
python tools/agent.py check --base origin/main

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

## Integration

Workers stop at `ready`. The integration worktree acquires exclusive resources,
integrates the worker commit, runs the serialized DOL/REL build, consumer checks,
DTK checksum, and retail comparisons, then finalizes:

```sh
python tools/agent.py queue acquire-resource integration --agent integrator
python tools/agent.py queue acquire-resource retail-build --agent integrator

python tools/agent.py integration finalize <owner> \
  --agent integrator \
  --retail-gate pass \
  --checksum pass \
  --toolchain GC/1.3.2
```

Finalization verifies every claimed path in the integration tree still matches
the worker’s verified commit before setting the task to `done`.

## Blind benchmark evidence

Use `tools/blind_recovery.py` for controlled holdouts. A reproducible case must
preserve the exact evidence packet, frozen candidate, target and candidate
assembly, deterministic result, hashes, source path, source commit, and blindness
assertions. Cases missing those artifacts remain `legacy-reported` and do not
count toward strict benchmark totals.

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
evidence. Update its freshness record when revalidated.

A handoff must state owner and stable identity, selected cards, accepted/rejected
evidence, natural candidate, compiler reconciliation, exact/relocation impact,
consumers, worker proof, integration requirements, metadata changes, remaining
debt, and queue status.

See `docs/agent_quickstart.md`, `docs/concurrent_agents.md`, `CONTRIBUTING.md`,
`docs/blind_recovery_benchmark.md`, and `docs/recovery_standard.md`.
