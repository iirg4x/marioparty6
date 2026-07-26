# Gemini entrypoint

This is the permanent AI recovery workspace. Never merge it or open a merge PR
to `main`. `main` receives only verified recovered C through a fresh main-based
`recovery/*` branch.

Read root `AGENTS.md`, `AI_WORKSPACE.md`, and the nearest nested `AGENTS.md`.

Use a dedicated worktree, branch, and build directory. Claim one owner, declare
shared paths, generate symptom-aware context, and run queue diff checks before
commits.

Commit a clean candidate, run the public/object gates, record worker proof, and
stop at `ready`. After private integration, use `tools/promote_recovered_c.py` to
copy only selected exact `src/**/*.c` blobs to a clean branch from `main`.

Do not transfer Gemini attribution, prompts, tooling, metadata, docs, benchmarks,
headers, or build configuration automatically.

There are no separate Gemini recovery rules; `AGENTS.md` is authoritative.
