# Mario Party 6 recovery workspace

This branch contains the matching-decompilation workbench for the USA Revision 0
GameCube release (`GP6E01`). For the public project, build instructions and
published progress, see [main](https://github.com/iirg4x/marioparty6/tree/main).

**Do not merge this branch into main.** Verified source and headers are transferred
through a fresh main-based recovery branch; experimental tools, reports, agent
instructions and work history stay here. See [the branch boundary](AI_WORKSPACE.md)
and [source promotion](docs/main_promotion.md).

## What counts as progress

A retained function improvement is useful, but it is not a completed owner or a
change to main. Owner completion requires matching instructions and data,
relocation checks, preserved exact siblings and affected callers, and an actual
source-selected retail-identical link. Published progress comes from main's
verified build, not this branch's experiment scores.

The default build can use original objects for unrecovered files. A successful
checksum by itself therefore does not mean every file has been reconstructed.
Game assets and original binaries are not distributed here.

## Working loop

1. Read [AGENTS.md](AGENTS.md), claim the relevant source/shared paths and use an
   isolated worktree. Keep the current champion and known compiler configuration.
2. Inspect the first meaningful target/source mismatch. Check structure, types,
   evaluation precision and real value lifetimes before register permutations.
3. Compile one coherent evidence-supported change. Preserve admissible gains;
   discard regressions without banning the function. Support workers answer
   bounded questions while the primary continues reconstruction.
4. Verify completed owners and promote them in useful batches. Do not make an
   unrelated tool or worker a prerequisite for the next local experiment.

Shell commands in this workspace use `rtk proxy` to avoid buffered output.
Run startup and broad checks at workspace/integration boundaries, not per probe.

## Useful entry points

```sh
rtk proxy python tools/agent.py queue status
rtk proxy python tools/recovery_causal_groups.py --strict REPORT.json --owner-summary
rtk proxy python tools/recovery_causal_groups.py --strict REPORT.json --function FUNCTION --producers 8
```

The causal-group tool reports observed mismatch families and signedness/precision
clues; it does not infer original variable names or authorize source retention.
For a small local-model question, add `--support-source SOURCE.c --source-lines START:END`.
The excerpt preserves input hashes and omitted-row counts, and refuses oversized
requests. Compile the full original translation unit, never the excerpt.

## Where things belong

- `src/`, `include/`: reconstructed game, SDK and runtime code.
- `config/`: symbols, splits, build inputs and recovery knowledge.
- `tools/`, `tools/tests/`: shared recovery/build tools and their tests.
- `docs/`: focused guides and compact findings.
- `build/`: ignored objects, active experiments and proof; keep it bounded.
- `orig/GP6E01/`: your local retail inputs, never committed.

Further details: [reconstruction guide](docs/recovery_search.md),
[recovery standard](docs/recovery_standard.md),
[supporting-change promotion](docs/supporting_change_promotion.md).
