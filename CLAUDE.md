# Agent entrypoint

Read the root [`AGENTS.md`](AGENTS.md) and the nearest nested `AGENTS.md` for
every file you edit. Use `python tools/agent.py doctor`, inspect the shared queue,
and claim one owner with:

```sh
python tools/agent.py queue claim <owner> --agent claude
```

Use a Claude-only worktree, branch, and build directory. Declare shared files in
the claim before editing them. Generate bounded context through
`python tools/agent.py context ...`, and run
`python tools/agent.py check --base origin/main` before handoff.

There are no Claude-specific recovery rules; `AGENTS.md` is the single source of
truth.
