# Agent entrypoint

Read the root [`AGENTS.md`](AGENTS.md) and the nearest nested `AGENTS.md` for
every file you edit. Use `python tools/agent.py doctor`, inspect the shared queue,
and claim one owner before editing. Use a Gemini-only worktree, branch, and build
directory, and declare shared files in the claim.

Generate bounded context through `python tools/agent.py context ...`, and run
`python tools/agent.py check --base origin/main` before handoff.

There are no Gemini-specific recovery rules; `AGENTS.md` is the single source of
truth.
