# Getting started

This repository is configured for the USA Revision 0 build of Mario Party 6:

```text
GP6E01
```

It is not a generic project template. Do not rename the version directory or
regenerate the project configuration from scratch.

## 1. Install the required tools

Follow [dependencies.md](dependencies.md) to install Git, Python, Ninja, and the
platform-specific compiler wrapper requirements.

Confirm the basic tools are available:

```sh
python --version
ninja --version
git --version
```

## 2. Clone the repository

```sh
git clone https://github.com/iirg4x/marioparty6.git
cd marioparty6
```

## 3. Prepare the original game files

Using a legally obtained USA Revision 0 copy of Mario Party 6, extract the game
into:

```text
orig/GP6E01/
```

Preserve the disc directory layout. The configured build expects the main DOL
and REL files at paths such as:

```text
orig/GP6E01/sys/main.dol
orig/GP6E01/files/dll/mdpartydll.rel
orig/GP6E01/files/dll/*.rel
```

Dolphin's **Extract Entire Disc** command can be used for a GameCube disc image.
The extracted files are ignored by Git and must not be committed.

## 4. Configure the build

From the repository root:

```sh
python configure.py
```

This generates `build.ninja`, `objdiff.json`, build includes, and the required
support-tool configuration. The project pins its DTK, compiler, binutils,
`sjiswrap`, and wibo versions in `configure.py`.

To inspect optional tool-path and build arguments:

```sh
python configure.py --help
```

## 5. Build

```sh
ninja
```

For final verification work, prefer a serialized build:

```sh
ninja -j1
```

Generated output is written under:

```text
build/GP6E01/
```

## 6. Verify the retail hashes

After a successful build:

```sh
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

On Windows:

```bat
build\tools\dtk.exe shasum -q -c config\GP6E01\build.sha1
```

The expected retail `main.dol` SHA-1 is recorded in
`config/GP6E01/config.yml` and `config/GP6E01/build.sha1`.

## 7. Configure objdiff

After configuration, open the repository in
[objdiff](https://github.com/encounter/objdiff). The generated `objdiff.json`
should be detected automatically.

Select an object from the left sidebar to compare the current source output with
the target. Source, header, symbol, split, and configuration changes can trigger
automatic rebuilds.

## 8. Make a focused contribution

Before changing source:

1. Record the current object and relocation state.
2. Work on one translation unit or a small related function group.
3. Keep unrelated formatting out of the change.
4. Check all previously matching functions and affected consumers.
5. Run the full retail hash gate before submitting a matching promotion.

See [../CONTRIBUTING.md](../CONTRIBUTING.md) for the source-recovery and pull
request expectations.

## Additional technical references

- [symbols.md](symbols.md)
- [splits.md](splits.md)
- [common_bss.md](common_bss.md)
- [comment_section.md](comment_section.md)
