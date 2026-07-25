# Tooling instructions

These rules apply under `tools/`. Also follow the root `AGENTS.md`.

- Keep public-safe agent and recovery tooling Python-standard-library-only unless
  a dependency is already required by the project.
- Every script must work both as `python tools/name.py` and through imports from
  `tools.tests`.
- Use `pathlib`, explicit UTF-8, typed interfaces, useful exit codes, and error
  messages that identify the failing path or record.
- Never read or mutate retail files for a public-safe check.
- Generated output belongs under `build/`; tools must create parent directories
  and avoid partially written files.
- Prefer deterministic exact lookup before fuzzy search. Preserve stable target
  identities in generated output.
- Tests must use temporary directories and synthetic fixtures; do not require a
  configured compiler, Ninja build, network access, or `orig/`.
- A source-quality rule must avoid comments and string literals, scan changed
  lines by default, and require a narrowly scoped exception.

Run:

```sh
python -m unittest discover -s tools/tests -v
python -m compileall -q tools
python tools/agent.py check --base origin/main
```
