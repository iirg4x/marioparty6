# Documentation index

## Start here

- [`../README.md`](../README.md): project scope and top-level commands
- [`agent_quickstart.md`](agent_quickstart.md): shortest safe agent workflow
- [`../CONTRIBUTING.md`](../CONTRIBUTING.md): task isolation, verification, and handoff
- [`recovery_standard.md`](recovery_standard.md): evidence hierarchy and faithful-source standard
- [`context_workflow.md`](context_workflow.md): deterministic index, actionable knowledge cards, selection ranking, token budgets, and wave-distillation audit
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

## Reusable recovery knowledge

The normal task context automatically selects compact source-to-output cards
from `config/recovery/compiler_patterns.json`. Cards distinguish:

- confirmed rules under stated conditions;
- contextual heuristics that suggest the next bounded probe;
- owner constraints that must not leak to another translation unit;
- counterexamples that prevent cargo-cult application.

Inspect selected cards or the extraction backlog without opening historical
reports:

```sh
python tools/agent.py knowledge function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty
python tools/agent.py knowledge audit
```

## Recovery evidence archive

Files named `native_matching_wave*.md` and other owner-specific reports are
historical laboratory records. They preserve exact probes, rejected approaches,
object results, and retail gates. They are not the default agent context and
their bodies are not automatically indexed into prompts.

Reusable conclusions from those reports belong in:

```text
config/recovery/owners/
config/recovery/names.json
config/recovery/exceptions.json
config/recovery/compiler_patterns.json
```

The intended flow is:

```text
historical probe → structured rule/constraint/counterexample → bounded context
```

Use the recovery index and context generator to retrieve the small relevant
subset. Do not concatenate the full evidence archive into a prompt. The
knowledge audit identifies wave files without a reusable card, but it does not
assume every historical batch contains a global rule.

## Generated documentation

The following are generated locally and ignored:

```text
build/context/recovery.sqlite
build/context/recovery-report.md
build/context/*context*.md
```

Generate them through `python tools/agent.py check` or the lower-level recovery
tools. Never commit generated reports in place of durable metadata.
