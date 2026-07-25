## Scope and coordination

- Owner:
- Stable identity / current symbol:
- Task issue and batch:
- Assigned worker: Claude / Codex / human
- Worker branch and worktree:
- Queue status: researching / coding / verifying / ready / done
- Change class:
- Claimed source and shared paths:
- Dependencies and capabilities:

## Research question

What semantic, ownership, data-domain, or compiler-shape question is answered?

## Selected recovery knowledge

- Confirmed rules:
- Contextual heuristics:
- Owner constraints:
- Counterexamples:
- Freshness warnings or cards revalidated:
- Existing card/report that prevented a repeated probe:

Knowledge changes:

- New/refined cards:
- New examples/counterexamples:
- Superseded cards:
- Historical findings distilled:

State `None` where appropriate. Compiler-wide cards are diagnostics, not source
templates.

## Evidence

### Accepted

- Same-game target/artifact evidence:
- Callers, consumers, data domains, and ownership:
- Sibling source:
- Compiler probes:
- Local objdiff/report summary:

### Rejected or inconclusive

- Probe or alternative:
- Why rejected:

## Natural source candidate

Describe the readable evidence-supported candidate before compiler
reconciliation.

## Compiler reconciliation

List only required source-shape adjustments and how they align with or refine the
selected cards.

## Binary and consumer impact

- Exact functions before → after:
- Exact text/data bytes before → after:
- Relocations:
- Previously exact regressions:
- Consumers and results:

## Source-quality impact

- Binary / source-shape / semantic / naming / data status:
- New or resolved debt:
- Exceptions added or removed:
- Candidate organicity findings reviewed:
- Retained-source debt inherited or removed:

## Blind benchmark evidence

Complete when this PR reports a blind/source-holdout result; otherwise state
`not applicable`.

- Benchmark case ID and status: reproducible / legacy-reported
- Candidate frozen before source reveal:
- Evidence packet preserved:
- Target and candidate assembly preserved:
- Source commit and reference SHA-256:
- Candidate SHA-256 and freeze time:
- Assembly equivalence:
- Retained-source similarity:
- Candidate organicity:
- Candidate-only versus inherited retained-source debt:
- Replay command and result:

A source-token match alone is not an organicity or originality claim.

## Worker verification

- Worker verified commit:
- Actual diff stayed inside claim:
- Public gate:
- Object report:
- Functions exact:
- Relocations:
- Consumer results:
- Toolchain:

Mark checks actually run:

- [ ] `python tools/agent.py queue check-diff --base origin/main`
- [ ] `python tools/knowledge_cards.py check`
- [ ] `python tools/blind_recovery.py audit`
- [ ] `python tools/agent.py check --base origin/main`
- [ ] `python tools/agent.py queue verify ...`
- [ ] Task is `ready`

## Integration verification

Complete only after integration; otherwise state `pending`.

- Integration commit:
- Integration/retail-build resources acquired:
- Serialized DOL/REL build:
- DTK checksum:
- Explicit retail comparisons:
- Integration consumer results:
- `python tools/agent.py integration finalize ...`:

Private retail gates are not required because:

## Remaining uncertainty and handoff

State unresolved semantics, stale cards, undistilled evidence, deferred shared
changes, unavailable private proof, remaining debt, and the next concrete task.
The handoff must not require the original agent transcript.
