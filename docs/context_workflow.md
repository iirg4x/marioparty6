# Recovery index, knowledge cards, and context workflow

## Why this exists

Large repository prompts are expensive and low signal. Decompilation questions
are anchored by exact identities: owner, module, target address, symbol,
relocation, caller, global, access width, compiler, and known probe. The primary
retrieval path is therefore deterministic and exact-first, not an embeddings
store or a scan of every historical report.

Embeddings may later help discover broadly similar behavior. They must not
replace exact owner, address, symbol, evidence, and compiler-rule lookup.

## Agent front door

```sh
python tools/agent.py doctor

python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --budget 12000

python tools/agent.py knowledge function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty

python tools/agent.py check --base origin/main
```

The lower-level tools remain available for scripting.

## Files of record

Human-authored recovery knowledge lives in:

```text
config/recovery/project.json
config/recovery/owners/*.json
config/recovery/names.json
config/recovery/exceptions.json
config/recovery/compiler_patterns.json
```

Generated files live under ignored `build/context/`:

```text
build/context/recovery.sqlite
build/context/recovery-report.md
build/context/*context*.md
```

The generated database and Markdown are disposable. Never hand-edit or commit
them.

## Knowledge cards

`compiler_patterns.json` now stores actionable source-to-output cards, not only
free-form notes. Each card records:

```text
classification
category
compiler and confidence
source condition or change
affected emitted behavior
known output signatures
recovery rule
safe actions
explicit applicability
examples and counterexamples
related source-shape exceptions
evidence reports
```

The three classifications have different authority:

- `confirmed_rule`: repeatable evidence under stated conditions. A
  compiler-wide rule remains diagnostic; it does not prescribe one source form.
- `contextual_heuristic`: a high-value mismatch investigation path that still
  requires local evidence.
- `owner_constraint`: an authenticated source shape for one owner or stable
  identity. It must not be copied elsewhere.

A result such as “moving these definitions earlier changes `.text`, `.bss`, and
hundreds of functions” is therefore stored as both a reusable compiler warning
and, when appropriate, a narrower owner constraint. Counterexamples prevent the
warning from becoming cargo-cult style.

Validate and inspect cards directly:

```sh
python tools/knowledge_cards.py check
python tools/knowledge_cards.py function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty
python tools/knowledge_cards.py owner REL:mdpartydll:stage
python tools/knowledge_cards.py audit
```

The same operations are available through `tools/agent.py knowledge`.

## Automatic relevance selection

Every normal owner or function context automatically selects at most five cards
by default. Ranking is deterministic:

1. recorded counterexample for the exact target or owner;
2. exact stable identity;
3. confirmed stable-identity example;
4. explicit owner scope;
5. confirmed owner example;
6. module or owner-tag scope;
7. compiler-wide diagnostic;
8. project-wide rule.

Confidence and rule type break ties. Cards with a known compiler mismatch are
not selected. Owner constraints are selected only for their explicit owner or
stable identity; they never leak into unrelated files.

A counterexample ranks first because it prevents the most expensive mistake:
blindly applying a previously successful source shape where a local probe has
already shown that it does not work.

The selected cards appear before the target source, so they survive context
clipping and guide the first edit. Each compact card contains its trigger,
possible emitted effects, known signatures, rule, safe actions, counterexamples,
and evidence paths.

Override the default only deliberately:

```sh
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --knowledge-limit 3

# Diagnostic only: disables automatic card injection.
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --knowledge-limit 0
```

## Deterministic index

Build and query the index directly:

```sh
python tools/recovery_index.py check
python tools/recovery_index.py build
python tools/recovery_index.py query mdpartydll:0xBBD8
python tools/recovery_index.py query "broad header"
python tools/recovery_index.py query "inspect caller and consumer widths"
```

The index contains:

- owner and multidimensional recovery state;
- file-scope function spans and stable identities;
- include edges;
- accepted and rejected owner evidence;
- semantic and naming debt;
- source-shape exceptions;
- full knowledge-card text, including rules and safe actions;
- exact-first search records.

This means a conclusion extracted once can be found by its target, compiler
symptom, rule, or recommended action without reopening the wave document.

## Function context pack

```sh
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --budget 12000
```

Unless `--stdout` is supplied, the command writes an ignored Markdown packet
under `build/context/`.

The packet contains, in priority order:

1. global recovery contract;
2. owner state and summary;
3. automatically selected knowledge cards and counterexamples;
4. stable identity, signature, location, and current function source;
5. bounded owner-neighbourhood signatures;
6. accepted and rejected owner evidence;
7. authenticated source-shape exceptions;
8. naming state and recovery debt;
9. local report availability;
10. acceptance criteria.

The card section reserves part of the token budget before the base context is
generated. Oversized source/evidence content is clipped after high-priority
rules have been inserted.

The equivalent lower-level command is:

```sh
python tools/context_pack.py \
  --budget 12000 \
  --output build/context/mdparty_BBD8.md \
  function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty
```

## Owner context pack

```sh
python tools/agent.py context owner main:game/mgdata --budget 7000
```

Owner packets list bounded function signatures rather than dumping a complete
large translation unit. Compiler-wide cards are selected only when the owner
manifest identifies the compiler; exact owner constraints still work without a
compiler-wide assumption.

## Historical wave reports

Wave reports remain forensic laboratory records. They are not indexed as prompt
text and are never automatically injected into a task context.

The intended knowledge flow is:

```text
wave report or probe
        ↓ distill once
structured knowledge card / owner evidence / exception
        ↓ select automatically
bounded Claude or Codex task context
        ↓ new result
new card, example, counterexample, or refined condition
```

Audit the extraction backlog without loading the files:

```sh
python tools/agent.py knowledge audit
python tools/knowledge_cards.py audit --json
```

The audit reports:

- number of structured cards;
- card classifications;
- number of historical wave documents;
- waves referenced by cards, owner evidence, and exceptions;
- a bounded list of waves with no reusable card yet.

A wave with no card is not automatically useless: it may contain only
owner-specific history. The audit identifies candidates for review; it must not
fabricate a rule from a filename or matching percentage.

## Expand context deliberately

Expand one specific unresolved item:

- direct caller or callee;
- structure declaration;
- shared data owner;
- message, archive, state, or resource domain;
- one referenced evidence report;
- one object-diff report;
- one knowledge card requiring deeper evidence.

Do not automatically attach:

- all of `STATUS.md`;
- every historical wave report;
- every transitively included header;
- a complete large translation unit;
- unrelated exact functions;
- an old agent transcript.

Write reusable findings back to structured metadata before handoff.

## Changed-line source review

```sh
python tools/agent.py check --base origin/main
```

The lower-level command is:

```sh
python tools/source_quality.py --changed origin/main --strict
```

Changed-line review avoids blocking a task on unrelated historical debt.
Authenticated exceptions suppress only an exact scoped rule. Temporary
exceptions remain visible debt.

## Human-readable report

```sh
python tools/agent.py report
```

The report shows the owner matrix, recovery debt, naming ledger, exceptions,
actionable knowledge cards, and wave-distillation coverage. It complements DTK
binary progress and never replaces object, relocation, DOL/REL, or checksum
proof.

## Updating knowledge

When a source experiment produces reusable output knowledge:

1. decide whether it is a confirmed rule, contextual heuristic, or owner
   constraint;
2. state the exact source condition and required preconditions;
3. record possible output changes and recognizable signatures;
4. write one clear recovery rule and concrete safe actions;
5. scope it by stable identity, owner, module, tags, compiler, or project;
6. add examples and counterexamples;
7. link the retained evidence and related exception;
8. run `python tools/agent.py check --base origin/main`.

Never promote one owner’s exact trick into a compiler-wide rule without another
example or a carefully stated diagnostic scope. Never discard a counterexample
because it makes the rule less convenient.
