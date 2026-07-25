# Source recovery instructions

These rules apply to files under `src/`. Also follow the repository root
`AGENTS.md`.

## Before editing

- Identify the configured object owner, compiler version, current binary status,
  and stable target identity.
- Generate a bounded function or owner context pack.
- Inspect target instructions and relocations, direct callers/callees, referenced
  globals, consumer access widths, strings/resources, existing evidence, and
  rejected probes.
- Write down the semantic question separately from the code-generation question.

## Source standard

Begin with natural, readable, evidence-supported C. Do not begin with a pragma,
cast ladder, fake branch, synthetic object, opaque tail array, or copied sibling
implementation merely because it moves objdiff.

Keep unknowns explicit. A field named `unk_10` is preferable to an invented
meaning. Accept semantic names only when same-game evidence or strong,
non-conflicting consumer and sibling evidence supports them.

Preserve historically plausible old-C and Hudson patterns when they are
supported by target/compiler evidence. Do not modernize declaration order,
header visibility, helper boundaries, or lifetimes when a local probe shows the
retail translation unit used a different shape.

## Compiler reconciliation

Change one dimension at a time and retain the result:

- signedness and narrowing;
- expression grouping and assignment conditions;
- block scope and temporary lifetime;
- loop and branch form;
- declaration visibility;
- definition chronology;
- helper and automatic-inline boundaries.

A successful isolated match is not enough. Prefer patterns reproduced across
related call sites with no exact regressions. Record accepted and rejected
behavior in `config/recovery/compiler_patterns.json` or owner evidence.

## Shared interfaces

A change to a header, structure, enum, global owner, message/resource domain, or
function signature requires checks of every affected Matching consumer. Do not
hide imports in public headers or duplicate an owner declaration locally unless
target-visible scope is authenticated and policy-backed.

## Required verification

For source changes, provide:

- relocation-aware object comparison;
- before/after exact function and byte counts;
- no regression to independently exact functions;
- affected-consumer comparisons;
- linked DOL/REL comparison and DTK checksum before promotion;
- source-quality metadata updates independent of matching progress.

Do not commit generated reports from `build/`; record their paths and concise
findings in durable evidence instead.
