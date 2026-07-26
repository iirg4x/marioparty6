# Claude entrypoint

This is the permanent AI recovery workspace. **Never merge or open a merge PR
from this branch to `main`.** `main` is human-facing and receives only verified
C blobs through a fresh main-based `recovery/*` branch.

Read root `AGENTS.md`, `AI_WORKSPACE.md`, and the nearest nested `AGENTS.md`.

Use a Claude-only worktree, branch, and build directory:

```sh
python tools/agent.py doctor
python tools/agent.py queue claim-next --agent claude --capability mwcc
```

Before commits, run:

```sh
python tools/agent.py queue check-diff --base origin/main
```

Generate focused context, commit a clean candidate, run the public and object
proof, record `queue verify`, and stop at `ready`.

After private integration, use `tools/promote_recovered_c.py` to create a clean
`recovery/*` branch from `main`. Only selected `src/**/*.c` blobs may transfer.
Do not transfer Claude attribution, prompts, metadata, tooling, docs, benchmarks,
headers, or build configuration automatically.

There are no separate Claude recovery rules; `AGENTS.md` is authoritative.
