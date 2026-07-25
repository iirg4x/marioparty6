# Claude entrypoint

Read root `AGENTS.md` and the nearest nested `AGENTS.md` before editing.

Use a Claude-only worktree, branch, and build directory. Claim one owner or take
the next eligible queued task:

```sh
python tools/agent.py doctor
python tools/agent.py queue claim-next --agent claude --capability mwcc
```

Before commits, run `python tools/agent.py queue check-diff --base origin/main`.
Generate context with `--symptom` and `--local-evidence` when available. Commit a
clean candidate, run the public gate, record `queue verify`, and stop at `ready`.
The integration worktree—not Claude’s worker tree—attaches retail/checksum proof
and finalizes `done`.

There are no separate Claude recovery rules; `AGENTS.md` is authoritative.
