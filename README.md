# Mario Party 6

A work-in-progress decompilation of **Mario Party 6** for Nintendo GameCube.

Supported build:

- `GP6E01` — USA, Revision 0

The project currently prioritizes the byte-identical non-minigame game flow: boot, menus, party mode, boards, results, and ending. See the concise [project status](STATUS.md) for the latest verified snapshot.

This repository does **not** contain game assets or original binaries. A legally obtained copy of the game is required. This is a source-recovery project, not a finished PC port.

## Requirements

- Git
- Python
- [Ninja](https://ninja-build.org/)

Platform-specific setup is covered in [docs/dependencies.md](docs/dependencies.md).

## Building

1. Clone the repository:

   ```sh
   git clone https://github.com/iirg4x/marioparty6.git
   cd marioparty6
   ```

2. Extract the USA Revision 0 game into `orig/GP6E01`, preserving the disc directory layout. The configured build expects files such as:

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

The first configuration may download the pinned support tools and compilers used by the project. Additional options are available through:

```sh
python configure.py --help
```

For a complete walkthrough, see [docs/getting_started.md](docs/getting_started.md).

## Comparing changes

Configuration generates `objdiff.json` in the repository root. Open the repository in [objdiff](https://github.com/encounter/objdiff) to rebuild and compare individual objects.

Source recovery should be checked with relocation-aware object comparison and against any affected consumers. A matching binary is required, but readable and evidence-supported source remains the goal.

## Verification

Check a successful build against the configured retail hashes:

```sh
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

On Windows, use `build/tools/dtk.exe`.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for source-quality and verification expectations.

The small documentation index is available at [docs/README.md](docs/README.md).

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
