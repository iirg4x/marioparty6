## Scope

- Owner:
- Stable target identity:
- Current symbol(s):
- Task issue:
- Change class: documentation / tooling / metadata / private source / shared interface / build configuration

## Research question

What source, semantic, ownership, or compiler-shape question does this change answer?

## Evidence

### Accepted

- Same-game artifacts or target evidence:
- Callers, consumers, data domains, or ownership:
- Sibling source:
- Compiler probes:

### Rejected or inconclusive

- Probe or alternative:
- Why it was rejected:

## Natural source candidate

Describe the readable evidence-supported candidate before compiler reconciliation.

## Compiler reconciliation

List only the source-shape adjustments actually required. State `None` when this
change does not perform compiler reconciliation.

## Binary and consumer impact

- Exact functions before → after:
- Exact text/data bytes before → after:
- Relocations:
- Previously exact regressions:
- Affected consumers and their results:

## Source-quality impact

- Binary status:
- Source-shape status:
- Semantic status:
- Naming status:
- Data status:
- New or resolved debt:
- Exceptions added or removed:

## Verification

Mark only checks that were actually run.

- [ ] `python tools/agent.py check --base origin/main`
- [ ] Relocation-aware object comparison
- [ ] Affected Matching consumers compared
- [ ] Serialized DOL/REL build
- [ ] DTK checksum gate
- [ ] Explicit retail DOL/REL byte comparison
- [ ] Generated/private files are not committed

Private retail gates not required for this change because:

## Remaining uncertainty

State unresolved semantics, names, source-shape questions, unavailable reports,
or private gates that were not run.

## Handoff

Summarize the next concrete task without requiring the original agent transcript.
