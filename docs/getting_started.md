# Getting started

This repository is already configured for the US GameCube build `GP6E01`. Do
not rename the version, regenerate initial symbols from a template, or create a
new project configuration.

## 1. Install tools

See [`dependencies.md`](dependencies.md). For public-safe agent work, Git and
Python 3.10+ are enough. Local decompilation builds also require Ninja and the
legally obtained retail inputs.

## 2. Clone and inspect the workspace

```sh
git clone https://github.com/iirg4x/marioparty6.git
cd marioparty6
python tools/agent.py doctor
```

Create a task branch or worktree before editing:

```text
agent/<owner>-<goal>
```

Read the root and nearest nested `AGENTS.md` files. See
[`agent_quickstart.md`](agent_quickstart.md) for the task lifecycle.

## 3. Extract `GP6E01`

Using a legally obtained US Mario Party 6 disc image, extract the disc with
Dolphin into:

```text
orig/GP6E01/
```

Preserve the extracted directory layout. At minimum, the configured build uses:

```text
orig/GP6E01/sys/main.dol
orig/GP6E01/files/dll/*.rel
```

The exact configured module paths and retail hashes are authoritative in
`config/GP6E01/config.yml`. Retail files are ignored and must never be
committed, uploaded as CI artifacts, or included in an agent context pack.

Run the doctor again. `private retail inputs` should now pass.

## 4. Configure

From the repository root:

```sh
python configure.py
```

The project pins DTK, binutils, compiler, `sjiswrap`, and wrapper versions in
`configure.py`. Explicit local tool paths are optional; inspect all overrides
with:

```sh
python configure.py --help
```

Do not use `--debug` or `--non-matching` for retail matching proof. Those modes
are useful only for explicitly nonmatching or diagnostic work.

## 5. Build

Use a serialized build for final evidence:

```sh
ninja -j1
```

The first run performs analysis and downloads or prepares configured tools when
needed. Build products, `build.ninja`, objdiff configuration, and generated
reports are ignored.

## 6. Public-safe agent checks

These checks do not need retail inputs:

```sh
python tools/agent.py check --base origin/main
```

They cover Python compilation, unit tests, recovery metadata, the deterministic
index, bounded context/report generation, repository cleanup policy, changed
private/generated paths, whitespace, and changed-line source-quality review.

## 7. Context and recovery work

Generate context for the exact owner or function rather than loading the whole
repository:

```sh
python tools/agent.py context function fn_1_BBD8 \
  --owner REL:mdpartydll:mdparty \
  --budget 12000
```

For decomp.me preprocessing, `tools/decompctx.py` remains available. It is not a
replacement for the recovery evidence packet.

## 8. Objdiff

After configuration, open the repository in objdiff. The generated
`objdiff.json` should be discovered automatically. Use relocation-aware
comparison for source claims and record:

- exact functions and text/data bytes before and after;
- relocation differences;
- independently exact regressions;
- affected consumers.

Do not commit objdiff JSON reports under `build/`. Put reusable conclusions in
`config/recovery/` or a concise evidence report.

## 9. Retail verification

Before promoting recovered source or build configuration:

1. run the serialized build;
2. run DTK against `config/GP6E01/build.sha1`;
3. compare `build/GP6E01/main.dol` with `orig/GP6E01/sys/main.dol`;
4. compare every affected REL with its retail file;
5. ensure generated symbol files contain no unexplained changes.

The DTK executable is generated under `build/tools/`; its extension depends on
the host platform. A typical gate is:

```sh
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

On Windows, use `build/tools/dtk.exe`. Use `cmp`, `fc /b`, or another exact
binary comparison appropriate to the platform.

A passing checksum proves the container output. It does not automatically prove
that semantic names, types, data domains, or unusual source constructs are
authentic; update those recovery dimensions separately.

## 10. Handoff

Open a pull request with the repository template and state exactly which public,
object, consumer, DOL/REL, and checksum gates were run. Never imply that a
private retail gate passed when the inputs were unavailable.
