# Mario Party 6

[![Code Progress]][status] [![DOL Progress]][status] [![REL Progress]][status]

[Code Progress]: https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fiirg4x%2Fmarioparty6%2Fmain%2Fprogress%2Fall.json
[DOL Progress]: https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fiirg4x%2Fmarioparty6%2Fmain%2Fprogress%2Fdol.json
[REL Progress]: https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fiirg4x%2Fmarioparty6%2Fmain%2Fprogress%2Fdlls.json
[status]: STATUS.md

A matching decompilation of **Mario Party 6**, targeting the GameCube USA
Revision 0 release (`GP6E01`). This project reconstructs the original GameCube
executable and modules; it is not a native PC port.

You must provide a legally obtained copy of the game. This repository does not
distribute game assets or original binaries.

## Progress

The badges and [STATUS.md](STATUS.md) show generated progress from main.
The DOL is the main executable; RELs are runtime-loaded modules, also called
DLLs by the game. They are not Windows DLLs.

Source completion and a matching build are different: unrecovered files can
still be linked from original objects. A retail-identical build does **not**
mean the entire game has been decompiled.

## Build

Install Git, Python and [Ninja](https://ninja-build.org/). Native Windows is
supported; WSL/MSYS2 are not required. Non-Windows hosts need a supported
compiler wrapper: x86/x86-64 Linux builds can use the automatically downloaded
`wibo`; macOS and other architectures generally use Wine. Run
`python configure.py --help` for wrapper and other build options.

```sh
git clone https://github.com/iirg4x/marioparty6.git
cd marioparty6
```

Extract the entire USA Revision 0 disc into `orig/GP6E01`, preserving its layout.
Dolphin's **Extract Entire Disc** command can do this. The expected inputs include:

```text
orig/GP6E01/sys/main.dol
orig/GP6E01/files/dll/*.rel
```

Then configure and build:

```sh
python configure.py
python tools/build.py
```

Configuration downloads the project's pinned tools and compilers when needed.
`tools/build.py` runs Ninja and refreshes the progress badge files when progress
changes.

Verify the built files against the retail checksums:

```sh
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

On Windows, use `build/tools/dtk.exe`.

## Working on a match

Configure once, then open [objdiff](https://github.com/encounter/objdiff) with
this repository as the project directory. It loads the generated `objdiff.json`;
select an object to compare its reconstructed source against the target.

See [CONTRIBUTING.md](CONTRIBUTING.md) for source-recovery and verification
requirements.

## Source layout

- `src/game/`: shared game runtime and services.
- `src/board/`: party-board systems, players, spaces, dice and events.
- `src/REL/`: loadable game modes and other modules.
- Other `src/` directories: Dolphin SDK, C/compiler runtimes, audio, speech and
  supporting libraries.
- `include/`, `config/`: headers, symbols, splits and build configuration.
- `tools/`, `progress/`: build utilities and generated progress data.
