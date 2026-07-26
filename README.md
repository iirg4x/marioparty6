Mario Party 6  
[![Code Progress]][status] [![DOL Progress]][status] [![DLL Progress]][status]
=============

[Code Progress]: https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fiirg4x%2Fmarioparty6%2Fmain%2Fprogress%2Fall.json
[DOL Progress]: https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fiirg4x%2Fmarioparty6%2Fmain%2Fprogress%2Fdol.json
[DLL Progress]: https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fiirg4x%2Fmarioparty6%2Fmain%2Fprogress%2Fdlls.json
[status]: STATUS.md

A work-in-progress decompilation of Mario Party 6. The currently supported build is the USA Revision 0 release, with active work focused on the main game flow, menus, party mode, boards, results, and ending.

There is **NO** working PC port yet.

This repository does **not** contain game assets or original binaries. A legally obtained copy of the game is required.

Version Completion:

- `GP6E01`: Rev 0 (USA) 🚧

The progress badges show code completion for the complete project, the main DOL, and all DLL/REL modules. They are generated from the latest verified build. See [STATUS.md](STATUS.md) for the dated snapshot and verification notes.

Dependencies
============

Windows
-------

Native Windows tooling is recommended. WSL and MSYS2 are not required. When a checkout is accessed across the Windows/WSL boundary, [objdiff](#diffing) filesystem notifications may not work reliably.

- Install [Git](https://git-scm.com/download/win).
- Install [Python](https://www.python.org/downloads/) and add it to `%PATH%`.
- Download [Ninja](https://github.com/ninja-build/ninja/releases) and add it to `%PATH%`.
  - Quick install via pip: `python -m pip install ninja`

macOS
-----

- Install Git, Python, and Ninja:

  ```sh
  brew install git python ninja
  ```

- Install [Wine Crossover](https://github.com/Gcenx/homebrew-wine) for the Metrowerks compiler executables:

  ```sh
  brew install --cask --no-quarantine gcenx/wine/wine-crossover
  ```

After some macOS upgrades, Wine Crossover may need to be unquarantined again:

```sh
sudo xattr -rd com.apple.quarantine '/Applications/Wine Crossover.app'
```

Linux
-----

- Install Git, Python, and [Ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages) through your package manager.
- On x86 and x86-64 systems, [wibo](https://github.com/decompals/wibo) is downloaded automatically.
- Other architectures generally require Wine or an explicitly provided compiler wrapper.

Building
========

- Clone the repository:

  ```sh
  git clone https://github.com/iirg4x/marioparty6.git
  cd marioparty6
  ```

- Using a legally obtained USA Revision 0 copy of Mario Party 6, extract the game into `orig/GP6E01` while preserving the disc directory layout. The build expects files such as:

  ```text
  orig/GP6E01/sys/main.dol
  orig/GP6E01/files/dll/*.rel
  ```

  Dolphin's **Extract Entire Disc** command can be used for a GameCube disc image.

- Configure:

  ```sh
  python configure.py
  ```

- Build and refresh the committed progress badge data:

  ```sh
  python tools/build.py
  ```

`tools/build.py` runs Ninja normally, then converts `build/GP6E01/progress.json` into the small files under `progress/`. If progress has not changed, those files remain untouched.

The first configuration may download the pinned support tools and compilers used by the project. Additional options are available through `python configure.py --help`.

To verify a successful build against the configured retail hashes:

```sh
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

On Windows, use `build/tools/dtk.exe`.

Diffing
=======

Once configuration succeeds, an `objdiff.json` file should exist in the project root.

Download the latest release of [objdiff](https://github.com/encounter/objdiff). Under project settings, set the project directory to this repository; the configuration should load automatically.

Select an object from the left sidebar to begin diffing. Changes to source files, headers, `configure.py`, `splits.txt`, or `symbols.txt` can trigger automatic rebuilds.

See [CONTRIBUTING.md](CONTRIBUTING.md) for source-recovery and verification expectations.
