# Recovery index, knowledge, and context workflow

## Design goal

Decompilation tasks are anchored by exact owners, target addresses, symbols,
relocations, callers, data, compiler configuration, and known probes. The normal
retrieval path is deterministic and exact-first—not a whole-repository prompt,
embeddings-only search, or scan of every wave report.

## Operational owner catalog

Generate the non-semantic scheduling inventory:

```sh
python tools/agent.py catalog build
python tools/agent.py catalog query REL:mdpartydll:mdparty
```

The catalog parses `configure.py` and source includes to record:

- configured DOL/REL owner IDs and source paths;
- configured Matching/NonMatching status;
- source size and existence;
- direct includes and header consumers;
- reviewed owner metadata when available.

It enables scheduling and dependency checks. It does not claim authentic
semantics, names, or source shape.

## Durable knowledge

Human-authored records live in:

```text
config/recovery/project.json
config/recovery/owners/*.json
config/recovery/names.json
config/recovery/exceptions.json
config/recovery/compiler_patterns.json
config/recovery/knowledge_freshness.json
```

Generated data remains ignored under `build/context/`.

## Actionable knowledge cards

Each card records classification, category, compiler/confidence, source trigger,
preconditions, emitted effects, recognizable mismatch signatures, one rule,
safe actions, structured applicability, examples, counterexamples, related
exceptions, and evidence.

Classifications:

- `confirmed_rule`: repeatable under stated conditions;
- `contextual_heuristic`: a bounded high-value diagnostic;
- `owner_constraint`: authenticated only for explicit owners/stable identities.

Ranking is deterministic:

1. exact counterexample;
2. exact stable identity;
3. confirmed target example;
4. explicit owner scope;
5. confirmed owner example;
6. module/tag scope;
7. compiler-wide diagnostic;
8. project-wide rule.

Owner constraints never leak to unrelated owners. Compiler-wide cards remain
diagnostics rather than copyable source templates.

## Knowledge freshness

Every card has a freshness record with validated commit/date, watched paths,
status, and supersession. If a watched source or evidence file changes, the card
is still shown but marked stale until revalidated.

```sh
python tools/knowledge_cards.py freshness
```

This prevents an old valid finding from silently surviving incompatible source,
header, evidence, or toolchain changes.

## Symptom-aware selection

Normal context retains exact target/owner constraints and counterexamples. It may
filter broad compiler diagnostics by task symptoms:

```sh
python tools/agent.py knowledge function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --symptom "saved register lifetime" \
  --symptom "helper boundary"
```

Symptoms may come from the investigator, owner evidence, or a compact objdiff
summary. They reduce generic card noise without hiding exact constraints.

## Local object-diff evidence

Provide one or more local JSON reports:

```sh
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --local-evidence \
  --report build/GP6E01/mdpartydll/report.json
```

The report parser is schema-tolerant and includes only compact fields such as
exact/total function counts, match percentages, section/relocation counts,
stack/register facts, and diff-list sizes. It does not dump unbounded JSON into
context.

## Section budgets

Context uses explicit section weights rather than one final blind truncation.
Space is reserved for:

```text
recovery contract
owner state and stable identity
selected knowledge and freshness
local evidence
constraints and acceptance criteria
target source
neighbourhood/evidence/debt
```

Large source, neighbourhood, and historical evidence are clipped before
knowledge, constraints, or acceptance criteria. The final packet still obeys the
requested approximate token budget.

```sh
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --symptom "header visibility" \
  --budget 12000
```

Use `--knowledge-limit 0` only for diagnostics. It should not be the normal
worker mode.

## Deterministic index

```sh
python tools/recovery_index.py check
python tools/recovery_index.py build
python tools/recovery_index.py query mdpartydll:0xBBD8
python tools/recovery_index.py query "broad header"
python tools/recovery_index.py query "inspect caller and consumer widths"
```

The SQLite index contains owner state, function spans, stable identities,
includes, evidence, debt, names, exceptions, and complete searchable card
triggers/effects/rules/actions/examples/counterexamples.

## Historical wave reports

Wave reports remain forensic laboratory records. Their bodies are never
injected automatically.

```text
historical probe
  → distill reusable conclusion once
  → card / owner evidence / exception / counterexample
  → deterministic selection into bounded task context
```

Audit the extraction backlog without opening every report:

```sh
python tools/agent.py knowledge audit
```

A wave without a card may contain only owner-specific history; the audit must not
invent a global rule from a filename or matching percentage.

## Updating knowledge

When a task produces a reusable result:

1. update only evidence-supported owner dimensions;
2. record accepted and rejected evidence;
3. update stable naming and debt;
4. add/refine a card, example, counterexample, or scoped exception;
5. update its freshness record;
6. run card/index/context tests;
7. record worker and integration proof separately.

Generated context, indexes, catalogs, reports, and local queue state are never a
substitute for durable metadata.
