# Gemini entrypoint

This is the permanent AI recovery workspace. Never merge it or open a merge PR
to `main`. `main` receives only verified content through fresh main-based
branches: `recovery/*` for recovered source/header blobs, `project/*` for audited supporting
changes.

Read root `AGENTS.md`, `AI_WORKSPACE.md`, and the nearest nested `AGENTS.md`.

Use a dedicated worktree, branch, and build directory. Claim one owner, declare
shared paths, generate symptom-aware context, and run queue diff checks before
commits.

Commit a clean candidate, run the public/object gates, record worker proof, and
stop at `ready`. After private integration, use `tools/promote_recovered_c.py` to
copy only selected exact canonical source/header blobs (`src/**/*.c`,
`src/**/*.cp`, `src/**/*.cpp`, `src/**/*.h`, `src/**/*.hpp`, `include/**/*.h`,
or `include/**/*.hpp`) to a
clean branch from `main`. Verified supporting changes (symbols, splits,
`configure.py`) use `tools/promote_supporting_change.py` and their own
`project/*` branch.

Do not transfer Gemini attribution, prompts, tooling, metadata, docs, or
benchmarks.

There are no separate Gemini recovery rules; `AGENTS.md` is authoritative.
