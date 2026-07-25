# Recovery index and context workflow

## Why this exists

Large repository prompts are expensive and low signal. Decompilation questions
are anchored by exact identities: owner, module, target address, symbol,
relocation, caller, global, access width, string, compiler, and known probe. The
primary index is therefore deterministic SQLite, not an embeddings store.

Embeddings may later help discover broadly similar behavior, but they must not
replace exact owner, address, symbol, call, and evidence lookup.

## Agent front door

Use the unified command for normal work:

```sh
python tools/agent.py doctor
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --budget 12000
python tools/agent.py check --base origin/main
```

The lower-level tools remain available for investigation and scripting.

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

## Deterministic index

Build and query the index directly when needed:

```sh
python tools/recovery_index.py check
python tools/recovery_index.py build
python tools/recovery_index.py query mdpartydll:0xBBD8
python tools/recovery_index.py query fn_1_BBD8
python tools/recovery_index.py query "audio header"
```

The index contains:

- owner and multidimensional recovery state;
- file-scope function spans and stable identities;
- include edges;
- accepted and rejected evidence;
- semantic and naming debt;
- source-shape exceptions;
- compiler patterns, examples, and counterexamples;
- exact-first text-search records.

Exact stable IDs and current symbols are resolved before substring search.

## Function context pack

```sh
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --budget 12000
```

Unless `--stdout` is supplied, the command writes an ignored Markdown packet
under `build/context/`. Use `--output` to choose a different ignored path.

The packet contains, in priority order:

1. global recovery contract;
2. owner state and summary;
3. stable identity, signature, location, and current function source;
4. bounded owner-neighbourhood signatures;
5. accepted and rejected evidence;
6. authenticated source-shape constraints;
7. semantic and naming debt;
8. local report availability;
9. acceptance criteria.

The token budget is tokenizer-independent and conservatively estimated from
characters. Oversized sections are clipped explicitly. Increase the budget only
after naming the information that is missing.

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
large translation unit. They are appropriate for semantic-cleanup planning and
owner debt review.

## Expand context deliberately

A context pack is the first packet, not an artificial ceiling. Expand one
specific unresolved item:

- direct caller or callee;
- structure declaration;
- shared data owner;
- message, archive, state, or resource domain;
- retained evidence report;
- object-diff report;
- compiler-pattern record.

Do not automatically attach:

- all of `STATUS.md`;
- every historical wave report;
- every transitively included header;
- a complete large translation unit;
- unrelated exact functions;
- an old agent transcript.

Write reusable findings back to the recovery manifests before handoff.

## Changed-line source review

The unified public gate reviews added C/C++ lines:

```sh
python tools/agent.py check --base origin/main
```

The lower-level command is:

```sh
python tools/source_quality.py --changed origin/main --strict
```

Changed-line review avoids blocking a task on unrelated historical debt.
Findings identify constructs that commonly indicate match-only workarounds. An
authenticated exception suppresses only the exact scoped rule. A temporary
exception remains visible debt and can be rejected explicitly:

```sh
python tools/source_quality.py \
  --changed origin/main \
  --strict \
  --reject-temporary
```

A full audit is available for research and backlog creation:

```sh
python tools/source_quality.py --all
```

The full audit is intentionally not the merge gate.

## Human-readable report

```sh
python tools/agent.py report
```

or:

```sh
python tools/recovery_report.py \
  --output build/context/recovery-report.md
```

The report shows the owner matrix, dimension counts, recovery debt, naming
ledger, source-shape exceptions, and compiler knowledge. It complements binary
progress; it does not replace DTK progress or checksum verification.

## Updating an owner

When an investigation changes what is known:

1. update only status dimensions supported by evidence;
2. add concise accepted and rejected evidence;
3. add or resolve debt;
4. update `names.json` without discarding stable identity;
5. add a narrowly scoped authenticated or temporary exception when required;
6. add compiler behavior with conditions and counterexamples;
7. run the public agent gate;
8. run private object, consumer, DOL/REL, and checksum gates when the change
   affects source or build output.

The public gate and private retail gate are deliberately separate. Neither
should be claimed when it was not run.
