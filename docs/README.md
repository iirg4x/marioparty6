# Documentation index

## Start here

- [`../README.md`](../README.md): project scope and top-level commands
- [`agent_quickstart.md`](agent_quickstart.md): shortest safe agent workflow
- [`../CONTRIBUTING.md`](../CONTRIBUTING.md): task isolation, verification, and handoff
- [`recovery_standard.md`](recovery_standard.md): evidence hierarchy and faithful-source standard
- [`context_workflow.md`](context_workflow.md): deterministic index and token-bounded context design
- [`getting_started.md`](getting_started.md): project-specific local setup and build workflow
- [`github_actions.md`](github_actions.md): public-safe CI and private retail-build boundaries

## Build and configuration reference

- [`dependencies.md`](dependencies.md): required and optional local tools
- [`symbols.md`](symbols.md): symbol-file format and ownership
- [`splits.md`](splits.md): translation-unit and section splits
- [`common_bss.md`](common_bss.md): common-BSS behavior
- [`comment_section.md`](comment_section.md): CodeWarrior `.comment` sections

These files retain useful decomp-toolkit background but should be opened only
when relevant to the active owner or build problem.

## Recovery evidence archive

Files named `native_matching_wave*.md` and other owner-specific reports are
historical laboratory records. They preserve exact probes, rejected approaches,
object results, and retail gates. They are not the default agent context.

Reusable conclusions from those reports belong in:

```text
config/recovery/owners/
config/recovery/names.json
config/recovery/exceptions.json
config/recovery/compiler_patterns.json
```

Use the generated recovery index and context pack to retrieve the small relevant
subset. Do not concatenate the full evidence archive into a prompt.

## Generated documentation

The following are generated locally and ignored:

```text
build/context/recovery.sqlite
build/context/recovery-report.md
build/context/*context*.md
```

Generate them through `python tools/agent.py check` or the lower-level recovery
tools. Never commit generated reports in place of durable metadata.
