# AI workspace documentation index

> This documentation belongs to the permanent AI recovery branch. It is not
> intended for `main` and must not be promoted with recovered source.

## Start here

- [`../AI_WORKSPACE.md`](../AI_WORKSPACE.md): permanent branch boundary
- [`../README.md`](../README.md): AI workspace scope and commands
- [`main_promotion.md`](main_promotion.md): exact canonical recovered-source transfer to clean `main`
- [`original_data_promotion.md`](original_data_promotion.md): narrow authenticated original-data transfer
- [`supporting_change_promotion.md`](supporting_change_promotion.md): audited header/symbol/split/build transfer to clean `main`
- [`agent_quickstart.md`](agent_quickstart.md): shortest worker path
- [`concurrent_agents.md`](concurrent_agents.md): queue, worktrees, resources, integration
- [`../CONTRIBUTING.md`](../CONTRIBUTING.md): AI workspace proof and handoff
- [`recovery_standard.md`](recovery_standard.md): faithful-source evidence standard
- [`context_workflow.md`](context_workflow.md): catalog, cards, symptoms, budgets, evidence
- [`blind_recovery_benchmark.md`](blind_recovery_benchmark.md): sealed holdouts and organicity
- [`../benchmarks/blind_recovery/README.md`](../benchmarks/blind_recovery/README.md): benchmark replay
- [`getting_started.md`](getting_started.md): local toolchain and retail build setup
- [`github_actions.md`](github_actions.md): draft-local versus ready-PR CI

## Branch boundary

```text
main
  human-facing source and ordinary project history

agent/recovery-context-workflow
  AI orchestration, prompts, metadata, benchmarks, queue and experiments
```

Never merge the second branch into the first. After a worker commit is fully
verified, create a fresh `recovery/*` worktree from `main` and copy only selected
canonical source/header blobs (`src/**/*.c`, `src/**/*.cp`, `src/**/*.cpp`,
`src/**/*.h`, `src/**/*.hpp`, `include/**/*.h`, or `include/**/*.hpp`):

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

Canonical headers may be selected and verified with their recovered source in
the `recovery/*` branch. Promote symbols, splits, and build configuration
separately with `tools/promote_supporting_change.py` onto a `project/*` branch
cut from `main`; see
[`supporting_change_promotion.md`](supporting_change_promotion.md).

Authenticated original data has a separate allowlisted path through
`tools/promote_original_data.py` and
[`original_data_promotion.md`](original_data_promotion.md). It transfers exact
bytes and source provenance only, earns zero clean-C credit, requires native
proof before `Matching`, and never transfers AI metadata. The ordinary
recovered-source path and its raw numeric hexadecimal prohibition remain
unchanged.

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

The catalog, queue and resource locks are AI workspace state. They never move to
`main`. Workers keep their real diff inside the claim, record clean commit-bound
proof, and stop at `ready`.

## Reusable recovery knowledge

```sh
python tools/agent.py knowledge function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --symptom "helper boundary"
python tools/knowledge_cards.py freshness
python tools/agent.py knowledge audit
```

Knowledge cards, freshness state and wave distillation stay on this branch. Only
the resulting verified C may cross the promotion boundary.

## Blind recovery evidence

```sh
python tools/blind_recovery.py audit
python tools/blind_recovery.py audit --strict --replay
```

Blind cases separate assembly equality, retained-source similarity, organicity,
and reproducibility. Benchmark artifacts remain AI workspace evidence and are
never part of a clean main promotion.

## Historical evidence archive

`native_matching_wave*.md` and owner reports remain forensic laboratory records.
Their bodies are not automatic prompt context. Reusable conclusions belong in
`config/recovery/` and stay on this branch.

## Generated local data

Ignored output includes:

```text
build/context/owner-catalog.json
build/context/recovery.sqlite
build/context/recovery-report.md
build/context/*context*.md
build/blind-recovery/
build/promotion/
```

Never commit generated reports or promotion manifests to either branch.
