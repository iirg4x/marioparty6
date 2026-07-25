# Recovery index and context workflow

## Why this exists

Large repository prompts are expensive and usually low signal. Decompilation
questions are anchored by exact identities: owner, module, function address,
symbol, relocation, caller, global, access width, string, compiler, and known
probe. The primary index is therefore deterministic SQLite, not an embeddings
store.

Embeddings may later help discover broadly similar behavior, but they must not
replace exact symbol, address, owner, call, and evidence lookup.

## Files of record

Human-authored recovery knowledge lives in:

```text
config/recovery/project.json
config/recovery/owners/*.json
config/recovery/names.json
config/recovery/exceptions.json
config/recovery/compiler_patterns.json
```

Generated files live under `build/context/` and are ignored by Git:

```text
build/context/recovery.sqlite
build/context/*.md
```

The generated database is disposable. Never hand-edit it.

## Validate and build the index

```sh
python tools/recovery_index.py check
python tools/recovery_index.py build
```

`check` validates cross-references and parses every governed source owner.
`build` atomically recreates `build/context/recovery.sqlite` with:

- owner and multidimensional recovery state;
- file-scope function spans and stable identities;
- include edges;
- accepted and rejected evidence;
- semantic and naming debt;
- source-shape exceptions;
- compiler patterns, examples, and counterexamples;
- exact-first text search records.

Query it directly from the command line:

```sh
python tools/recovery_index.py query mdpartydll:0xBBD8
python tools/recovery_index.py query fn_1_BBD8
python tools/recovery_index.py query "audio header"
```

Exact stable IDs and current symbols are resolved before substring search.

## Generate a function context pack

```sh
python tools/context_pack.py \
  --budget 12000 \
  --output build/context/mdparty_BBD8.md \
  function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty
```

The packet contains, in priority order:

1. the global recovery contract;
2. owner state and summary;
3. stable identity, signature, location, and current function source;
4. bounded one-hop function signatures;
5. accepted and rejected evidence;
6. authenticated source-shape constraints;
7. semantic and naming debt;
8. declared local object-diff report availability;
9. acceptance criteria.

The token budget is tokenizer-independent and conservatively estimated from
characters. Oversized source and evidence sections are clipped with an explicit
marker. Increase the budget deliberately only after identifying what is missing.

## Generate an owner context pack

```sh
python tools/context_pack.py \
  --budget 7000 \
  --output build/context/mgdata_owner.md \
  owner main:game/mgdata
```

Owner packets list bounded function signatures instead of dumping the whole
translation unit. This is appropriate for planning a semantic cleanup or
reviewing debt across an owner.

## Expand context deliberately

A context pack is the first packet, not an artificial ceiling. Expand only a
specific unresolved item:

- one caller or callee;
- one structure declaration;
- one shared data owner;
- one message or archive domain;
- one retained wave report;
- one object-diff report;
- one compiler-pattern record.

Do not automatically attach:

- all of `STATUS.md`;
- every historical wave report;
- every header transitively included by the owner;
- a complete large translation unit;
- unrelated exact functions;
- an old agent transcript.

Reusable findings must be written back to the manifests or compiler-pattern
records before the task is considered complete.

## Source-quality changed-lines review

During a branch or pull request:

```sh
python tools/source_quality.py --changed origin/main --strict
```

CI scans only added C/C++ lines. It does not fail the branch for unrelated
historical debt. Findings identify constructs that commonly indicate a
match-only workaround. An authenticated exception suppresses the finding; a
temporary exception is reported as debt and may optionally be rejected:

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

The full audit is intentionally not the default merge gate.

## Human-readable report

```sh
python tools/recovery_report.py \
  --output build/context/recovery-report.md
```

The report shows the owner matrix, independent dimension counts, recovery debt,
naming ledger, source-shape exceptions, and compiler knowledge. It complements
binary progress; it does not replace DTK progress or checksum verification.

## Updating an owner

When an investigation changes what is known:

1. update the owner status only for dimensions supported by evidence;
2. add accepted and rejected evidence summaries;
3. add or resolve debt records;
4. update `names.json` without discarding stable identity;
5. add a scoped exception only for an authenticated or explicitly temporary
   unusual construct;
6. add compiler behavior with conditions and counterexamples;
7. run tests, metadata validation, index build, context generation, and the
   changed-lines source-quality review.

Recommended public-safe validation:

```sh
python -m unittest discover -s tools/tests -v
python tools/recovery_index.py check
python tools/recovery_index.py build
python tools/recovery_report.py --output build/context/recovery-report.md
```

The private original-file build and retail checksum gates remain separate and
must run before binary promotion.
