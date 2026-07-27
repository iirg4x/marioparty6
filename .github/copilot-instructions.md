Follow root `AGENTS.md`, `AI_WORKSPACE.md`, and the nearest nested `AGENTS.md`.

This branch is the permanent AI recovery workspace. Never merge it or open a
merge PR to `main`. `main` is human-facing.

Use a dedicated task worktree, branch, and build directory. Claim the owner and
shared paths before editing. Run `queue check-diff` before commits. Generate
bounded context with symptoms and local evidence.

A worker commits a clean candidate, passes the public/object gates, records
structured verification, and stops at `ready`. Private integration verifies the
worker commit but still does not merge this branch.

Only selected exact `src/**/*.c` blobs may move to a fresh main-based
`recovery/*` branch through `tools/promote_recovered_c.py`. Verified supporting
changes (headers, symbols, splits, `configure.py`) move only through
`tools/promote_supporting_change.py` onto their own `project/*` branch. Do not
transfer Copilot/AI attribution, prompts, tooling, metadata, docs, or
benchmarks.

Binary identity is required but does not authenticate names, types, domains, or
source shape. Never invent semantics or add unexplained matching hacks.
