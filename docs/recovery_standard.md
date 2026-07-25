# Source recovery standard

## Purpose

This project targets the most likely original Mario Party 6 source, not merely
C that happens to emit the retail bytes. Binary identity remains the final,
objective verification gate. It does not by itself authenticate names, types,
data domains, structure meaning, declaration visibility, or natural source
shape.

A useful distinction is:

- **matching proof:** the configured compiler and linker reproduce the target;
- **source recovery:** the source form and semantics are supported by target,
  consumer, ownership, sibling, or controlled compiler evidence;
- **readability:** a human can follow the code without unsupported certainty.

All three matter. None should be substituted for another.

## Evidence hierarchy

Use the strongest available evidence and state uncertainty honestly.

### 1. Same-game artifacts

Highest-confidence evidence includes debug information, maps, symbols, source
remnants, authenticated tables, and other artifacts originating from the same
game and build lineage.

### 2. Target binary evidence

Instructions, relocations, section ownership, access widths, call contracts,
stack layout, strings, archive indices, function chronology, and object layout
can prove concrete constraints. They are especially strong for signedness,
width, ownership, helper calls, and data extent.

### 3. Same-game consumers

Callers and consumers often authenticate meaning better than one isolated
producer. Check how arguments, returns, fields, globals, tables, messages,
resources, and archive indices are used across the game.

### 4. Sibling source

Closely related Hudson and Mario Party source can support naming, layout, and
source-family hypotheses. Sibling source never overrides contradictory MP6
target or consumer evidence. Copying a sibling wholesale is not recovery.

### 5. Compiler probes

Controlled probes can authenticate:

- declaration visibility and chronology;
- block scope and temporary lifetime;
- signed or narrow expression nodes;
- helper and inline boundaries;
- loop and condition form;
- expression-tree grouping;
- target compiler behavior under specific flags.

A probe cannot, by itself, prove that a field means `cameraTarget`, that a
function is `CharacterSelectStart`, or that a numeric ID belongs to a semantic
domain.

### 6. Readability hypotheses

A plausible interpretation with no stronger evidence must remain provisional.
Prefer `unk_10`, a stable address identity, or an explicit TODO over a polished
but fabricated name.

## Independent recovery dimensions

Each governed owner records five dimensions in `config/recovery/owners/`.

### Binary

- `fallback`: linked from original or generated fallback material;
- `partial`: some source functions or sections are represented exactly;
- `exact`: the governed source owner is exact under the required object and
  container gates.

### Source shape

- `scaffold`: byte-oriented or placeholder source;
- `plausible`: natural-looking but incompletely evidenced source;
- `evidence_backed`: non-obvious form is supported by target, consumer,
  sibling, or compiler evidence;
- `authenticated`: the owner shape is strongly supported across the important
  contracts and contains no known match-only scaffolding.

### Semantics

- `opaque`: behavior and domains are largely unknown;
- `partial`: meaningful contracts are recovered but important unknowns remain;
- `recovered`: the owner’s important behavior and domains are evidence-backed.

### Naming

- `address_only`: mostly address-derived identifiers;
- `provisional`: semantic proposals exist but are not accepted;
- `partially_semantic`: a mixed owner with accepted names and unresolved
  identities;
- `evidence_backed`: important public and private names are supported.

### Data

- `raw`: literals and byte-oriented scaffolds dominate;
- `typed_partial`: widths and some structures are known, but domains remain
  opaque;
- `typed`: natural typed ownership is recovered;
- `domain_recovered`: identifiers, dimensions, sentinels, resources, flags,
  messages, and archive domains are meaningfully represented.

Do not collapse these fields into a single percentage. Exact binary status can
coexist with address-only naming and partial semantics. Conversely, a natural
semantic candidate can be valuable before it reaches an exact match.

## Work phases

### Phase 1: evidence research

Do not edit source. Record:

- owner and stable identity;
- target signature, instructions, relocations, stack frame, and sections;
- direct callers and callees;
- referenced globals, strings, resources, and consumer access widths;
- relevant same-game data domains and headers;
- sibling source and why it is comparable;
- existing accepted and rejected compiler probes;
- unresolved semantic questions.

Generate a bounded context pack and expand only named dependencies that remain
material.

### Phase 2: natural source candidate

Write the most readable evidence-supported source without forcing the final
code generation. Preserve old-C or Hudson patterns when evidence supports them,
but do not begin from a cast ladder, fake branch, pragma, or artificial object.

A natural nonmatching candidate is an important intermediate result because it
separates semantic and structural recovery from compiler reconciliation.

### Phase 3: compiler reconciliation

Compare the natural candidate with the target. Change one evidenced dimension
at a time:

- signedness or narrowing;
- expression grouping;
- declaration visibility;
- block scope and lifetime;
- loop or branch form;
- helper boundary;
- automatic inline behavior;
- definition chronology.

Record the probe, affected functions, instruction or section delta, result, and
conditions. A local result must not become a global compiler rule without
examples and counterexamples.

### Phase 4: adversarial review

A fresh review should ask:

- Did matching pressure create an invented name or type?
- Does a raw numeric domain already have a same-game owner?
- Was padding or a literal object fabricated instead of left to natural layout?
- Is an opaque blob hiding fields that consumers already type?
- Did a broad header improve modern style but contradict retail visibility?
- Is a strange construct authenticated, temporary debt, or unexplained?
- Did an exact target regress another exact function or consumer?
- Is the semantic claim stronger than its evidence confidence?

Only after this review should normal object, relocation, DOL/REL, and checksum
gates determine binary promotion.

## Stable identity and semantic names

Stable identity survives renaming. Use module plus target address where known:

```text
mdpartydll:0xBBD8
main:0x80012345
```

For naturally named exports without a recovered address, use an owner-qualified
identity:

```text
main:game/mgdata:MgDecaScoreCalc
```

`config/recovery/names.json` records the current symbol, proposed name, status,
confidence, and evidence summary. Accepted renames should preserve the stable
identity in the index and, when useful, in a compact source comment or generated
report.

Naming rules:

- target or same-game names may be `confirmed`;
- strong consumer plus sibling agreement may be `high`;
- behavior-only interpretation is usually `medium` or lower;
- compiler behavior alone cannot raise semantic naming confidence;
- a rejected proposal remains recorded so it is not repeatedly rediscovered.

## Authentic oddities versus matching hacks

Readable source does not mean mechanically modern source. The original compiler
and codebase may legitimately depend on:

- late definitions after earlier declarations;
- local prototypes rather than a broad public header;
- narrow signed returns;
- seemingly redundant helpers;
- old-style lifetime and block scope;
- unusual but consistent Hudson control flow.

An oddity belongs in final recovered source when target and compiler evidence
authenticate it. Record it in `config/recovery/exceptions.json` with scope,
classification, rationale, and evidence.

The following require review when newly introduced:

- compiler pragmas;
- forced inline or no-inline controls;
- `volatile` or `register` used to influence code generation;
- inline assembly;
- defining another header’s include guard;
- synthetic padding objects;
- opaque raw/tail/blob arrays;
- compile-time dead branches;
- fabricated literals or globals;
- duplicated paths or comma expressions created only for register allocation.

Classify each as:

- `authenticated`: target-backed source shape that may remain;
- `temporary`: explicit recovery debt with a removal condition;
- `forbidden`: a known shortcut that should not be introduced.

An authenticated exception is owner-specific. It must not be copied to another
translation unit merely because the compiler version is the same.

## Model semantic cleanup: `mgdata`

`src/game/mgdata.c` is the model for a zero-byte-gain source-quality recovery.
The prior owner matched while retaining raw IDs, explicit padding, synthetic
objects, opaque arrays, and an include-guard override. The semantic pass used
same-game consumers and sibling shape to recover named overlay, flag, message,
asset, time, and subgame domains; natural structure dimensions; correct buffer
widths; natural padding; shared ownership; and the original expression family.

Apply the same sequence to other exact-but-opaque owners:

1. enumerate raw fields, literals, arrays, and synthetic objects;
2. find same-game consumers and access widths;
3. identify message, archive, overlay, flag, state, and resource domains;
4. compare only relevant sibling source;
5. recover natural structures and dimensions;
6. remove fake padding and compiler coercion;
7. re-prove the owner and all affected consumers;
8. update semantic and data status even when matching progress is unchanged.

## Required task record

A durable task report or manifest update should capture:

```text
owner / stable identity
research question
accepted evidence
rejected evidence or probes
natural candidate summary
compiler reconciliation, if any
exact functions before and after
affected consumers
relocation and section result
container/checksum result
semantic, naming, shape, and data status change
remaining debt
```

Keep long narrative wave reports as laboratory records. Put reusable facts,
constraints, names, and debt in structured metadata so agents do not need the
whole history in context.

## Merge standard

A source owner may be binary-promoted only when the required object,
relocation, linked-container, and checksum gates pass with no unexplained
regression.

A source-quality status may be promoted only when the new state is supported by
the evidence hierarchy and remaining uncertainty is recorded. A 100% match
with fake padding, raw semantic domains, invented names, or unexplained compiler
controls remains recovery debt.
