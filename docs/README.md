# Documentation index

## Start here

- [`../README.md`](../README.md): project scope and main commands
- [`agent_quickstart.md`](agent_quickstart.md): shortest worker path
- [`concurrent_agents.md`](concurrent_agents.md): queue v2, worktrees, resources, and integration
- [`../CONTRIBUTING.md`](../CONTRIBUTING.md): proof and handoff requirements
- [`recovery_standard.md`](recovery_standard.md): faithful-source evidence standard
- [`context_workflow.md`](context_workflow.md): catalog, cards, symptoms, budgets, local evidence, freshness
- [`blind_recovery_benchmark.md`](blind_recovery_benchmark.md): controlled source-holdout results and remaining retail benchmark
- [`getting_started.md`](getting_started.md): local toolchain and retail build setup
- [`github_actions.md`](github_actions.md): draft-local versus ready-PR CI

## Build reference

- [`dependencies.md`](dependencies.md)
- [`symbols.md`](symbols.md)
- [`splits.md`](splits.md)
- [`common_bss.md`](common_bss.md)
- [`comment_section.md`](comment_section.md)

Open these only when relevant to the active owner or build problem.

## Operational coordination

```sh
python tools/agent.py catalog build
python tools/agent.py queue status
python tools/worktree_audit.py
python tools/agent.py hooks install
```

The owner catalog is generated scheduling data. The queue and resource locks
live under Git’s common directory. Neither is committed recovery evidence.

Workers must keep the real Git diff inside their claim, record clean
commit-bound proof, and stop at `ready`. The integration worktree performs
serialized retail/checksum gates and finalizes `done`.

## Reusable recovery knowledge

Normal context selects compact source-to-output cards with structured scope,
examples, counterexamples, safe actions, and freshness:

```sh
python tools/agent.py knowledge function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --symptom "helper boundary"
python tools/knowledge_cards.py freshness
python tools/agent.py knowledge audit
```

Cards distinguish confirmed rules, contextual heuristics, and owner constraints.
A stale card remains visible as a warning. Compiler-wide rules are diagnostics,
not source templates.

## Historical evidence archive

`native_matching_wave*.md` and owner reports remain forensic laboratory records.
Their bodies are not automatic prompt context. Reusable conclusions belong in:

```text
config/recovery/owners/
config/recovery/names.json
config/recovery/exceptions.json
config/recovery/compiler_patterns.json
config/recovery/knowledge_freshness.json
```

The intended flow is:

```text
historical/local probe
→ structured evidence/rule/constraint/counterexample
→ freshness validation
→ bounded task context
```

## Generated local data

Ignored output includes:

```text
build/context/owner-catalog.json
build/context/recovery.sqlite
build/context/recovery-report.md
build/context/*context*.md
```

Generate it through `python tools/agent.py check` or the lower-level tools. Never
commit generated reports in place of durable metadata.
