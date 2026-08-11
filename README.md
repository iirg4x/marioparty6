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

The progress badges show code completion for the complete project, the main DOL, and all DLL/REL modules. They are generated from the latest verified build. See [STATUS.md](STATUS.md) for the current percentage breakdown.

How is the codebase structured?
===============================

The code under `src` is organized around the libraries and modules that make up the retail game. Some code is linked into the always-resident `main.dol`; game modes and other self-contained flows can instead be loaded as REL modules at runtime. Original directory and module names are preserved even when their abbreviations are not yet understood.

Main game code
--------------

| Path | Purpose | Notes |
| --- | --- | --- |
| `src/game` | Core game runtime and shared gameplay systems. | Startup, processes, objects and overlays, data loading, rendering, audio, UI, saving, minigame services, and other common code. |
| `src/board` | Shared party-board code. | Board lifecycle, players, spaces and paths, dice, capsules, stars, events, camera, effects, audio, and board UI. A few foundational files are linked with the `Game` library; the rest form the `board` library. |
| `src/REL` | Runtime-loaded game modules. | Contains the shared REL runtime plus one directory per recovered module. |

REL modules
-----------

The disc stores these modules as `files/dll/*.rel`, and the game and project often call them DLLs. They are GameCube REL modules, **not** Windows DLL files. Each module has its own symbol and split configuration under `config/dll/rels/<module>`.

The recovered module sources currently include:

| Group | Module directories |
| --- | --- |
| Front end and flow | `bootDll`, `openingDll`, `fileseldll`, `selmenuDll`, `mdseldll`, `optionDll`, `sequencedll` |
| Party, world, and numbered modules | `mdpartydll`, `w01Dll`, `s01Dll`, `s02Dll`, `s03Dll` |
| Results and special modes | `endingdll`, `mdpresultdll`, `mdsingdll`, `mdbankdll`, `miraclebookdll`, `staffdll` |
| Managers and checks | `actmanDLL`, `meschkdll`, `motchkDll` |

Names such as `md`, `s01`, and `motchk` are retained from the original project. They are left unexpanded until their meanings can be supported by evidence.

Libraries and runtime code
--------------------------

| Path | Purpose |
| --- | --- |
| `src/dolphin` | Nintendo's Dolphin SDK implementations for GameCube hardware and operating-system services. |
| `src/MSL_C.PPCEABI.bare.H` | MSL C library sources, including memory, strings, file I/O, formatted I/O, and math. |
| `src/Runtime.PPCEABI.H` | C/C++ ABI and compiler runtime support, including allocation, exceptions, destructors, varargs, and pointer-to-member helpers. |
| `src/TRK_MINNOW_DOLPHIN` | MetroTRK target/debug runtime and its Dolphin platform glue. |
| `src/musyx` | MusyX audio runtime: sequencing, synthesis, DSP and hardware support, 3D audio, and effects. |
| `src/gssdk_lib` | Speech and audio SDK code, including the `gsapi` and `asrpho` trees used by microphone features. |
| `src/msm` | Sound-management code layered over MusyX for music, sound effects, streams, files, and memory. |
| `src/libhu` | Two small Hu vector helpers, `HuSetVecF` and `HuSubVecF`. |
| `src/zlib` | zlib inflate/decompression support. |
| `src/OdemuExi2` | Debugger transport over the GameCube EXI interface. |
| `src/amcstubs` | No-op EXI2 stubs used when the AMC transport is unavailable. |
| `src/odenotstub` | The small non-stub marker implementation used by the alternate debugger path. |

`src/dolphin`
---------------

The Dolphin SDK preserves its original short subsystem names:

| Short | Area | Notes |
| --- | --- | --- |
| `ai` | Audio Interface | Audio output hardware. |
| `ar` | Auxiliary RAM | ARAM management and request queues. |
| `base` | Base platform | Low-level PowerPC support. |
| `card` | Memory Card | Memory Card access and filesystem operations. |
| `db` | Debug | SDK debug support. |
| `demo` | Demo helpers | SDK demonstration and diagnostic helpers. |
| `dsp` | DSP | DSP control and task management. |
| `dvd` | Disc/DVD | Optical-disc access and filesystem support. |
| `exi` | External Interface | EXI device and UART communication. |
| `gx` | Graphics | Graphics pipeline and command generation. |
| `mic` | Microphone | GameCube microphone SDK support. |
| `mtx` | Math | Matrices, vectors, and quaternions. |
| `os` | Operating system | Threads, interrupts, memory, timing, linking, and other kernel services. |
| `pad` | Controller | GameCube controller input. |
| `si` | Serial Interface | Serial-interface devices and sampling. |
| `thp` | THP | THP movie and audio decoding. |
| `vi` | Video Interface | Display timing and framebuffer presentation. |

Related directories
-------------------

| Path | Purpose |
| --- | --- |
| `include` | Project, SDK, runtime, and generated identifier headers. It follows the source layout where useful, but is not a strict mirror of `src`. |
| `config/GP6E01` | Main DOL symbols, splits, checksums, and configuration for the supported USA Revision 0 build. |
| `config/dll/rels/<module>` | Section-relative symbols and splits for each REL module. |
| `tools` | Build, configuration, progress, and project-maintenance scripts. |
| `progress` | Generated badge data for the complete project, main DOL, and REL modules. |
| `orig/GP6E01` | Local retail inputs supplied by the user; these files are not committed. |
| `build/GP6E01` | Generated objects, executables, REL files, reports, and support tools. |

Terminology
-----------

| Term | Meaning in this repository |
| --- | --- |
| `GP6E01` | The supported USA Revision 0 build identifier. |
| DOL | The main executable, `main.dol`. |
| REL | A relocatable runtime module stored as a `.rel` file. |
| DLL | Game and project shorthand for a REL module; it does not mean a Windows DLL here. |

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
