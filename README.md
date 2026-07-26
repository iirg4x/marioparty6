# Mario Party 6

A work-in-progress decompilation of **Mario Party 6** for Nintendo GameCube.

The supported build is:

- `GP6E01` — USA, Revision 0

The current recovery priority is the byte-identical non-minigame game flow: boot,
menus, party mode, boards, results, and ending. See [STATUS.md](STATUS.md) for the
latest verified snapshot.

This repository does **not** contain game assets or original binaries. A legally
obtained copy of the game is required. This is a source-recovery project, not a
finished PC port.

## Requirements

- Git
- Python
- [Ninja](https://ninja-build.org/)

Platform-specific setup is documented in
[docs/dependencies.md](docs/dependencies.md).

## Building

1. Clone the repository:

   ```sh
   git clone https://github.com/iirg4x/marioparty6.git
   cd marioparty6
   ```

2. Extract the USA Revision 0 game into `orig/GP6E01`, preserving the disc
   directory layout. The configured build expects files such as:

   ```text
   orig/GP6E01/sys/main.dol
   orig/GP6E01/files/dll/*.rel
   ```

3. Generate the build files:

   ```sh
   python configure.py
   ```

4. Build:

   ```sh
   ninja
   ```

The first configuration may download the pinned support tools and compilers used
by the project. Additional configuration options are available through:

```sh
python configure.py --help
```

For a fuller walkthrough, see
[docs/getting_started.md](docs/getting_started.md).

## Diffing

After configuration, an `objdiff.json` file is generated in the repository root.
Download [objdiff](https://github.com/encounter/objdiff), set this repository as
the project directory, and select an object from the left sidebar.

Changes to source files, headers, `configure.py`, symbols, and splits can then be
rebuilt and compared automatically.

## Verification

A successful local build can be checked against the configured retail hashes:

```sh
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

On Windows, use `build/tools/dtk.exe`.

Matching source changes should also be checked with relocation-aware object
comparison and against any affected consumers. Binary equality is required, but
readable and evidence-supported source remains the goal.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for the project workflow and source-quality
expectations.

Useful technical references:

- [`symbols.txt`](docs/symbols.md)
- [`splits.txt`](docs/splits.md)
- [Common BSS](docs/common_bss.md)
- [CodeWarrior `.comment` sections](docs/comment_section.md)

## Project layout

- `src/` — recovered source files
- `include/` — shared headers and data definitions
- `config/GP6E01/` — DOL configuration, symbols, splits, and retail hashes
- `config/dll/rels/` — REL symbols and split ownership
- `tools/` — build and analysis helpers
- `orig/GP6E01/` — locally extracted game files; ignored by Git
- `build/` — generated build output; ignored by Git

## Related projects and tools

- [Mario Party 4 decompilation](https://github.com/mariopartyrd/marioparty4)
- [decomp-toolkit](https://github.com/encounter/decomp-toolkit)
- [objdiff](https://github.com/encounter/objdiff)
- [decomp.me](https://decomp.me/)
