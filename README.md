# Mario Party 6

[![Code][code-badge]][status] [![Data][data-badge]][status]
[![DOL][dol-badge]][status] [![DLLs][dll-badge]][status]

[code-badge]: https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fiirg4x%2Fmarioparty6%2Fmain%2Fprogress%2Fcode.json
[data-badge]: https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fiirg4x%2Fmarioparty6%2Fmain%2Fprogress%2Fdata.json
[dol-badge]: https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fiirg4x%2Fmarioparty6%2Fmain%2Fprogress%2Fdol.json
[dll-badge]: https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fiirg4x%2Fmarioparty6%2Fmain%2Fprogress%2Fdlls.json
[status]: STATUS.md

The Mario Party 6 build for GameCube (USA Revision 0, `GP6E01`) is complete and
reproduces `main.dol` and all 136 REL modules configured by `configure.py` byte for byte.
The retail disc also contains `m580Dll.rel` and `m433Dll.rel`, which the game
neither builds nor references (both have been commented out in
`config/GP6E01/config.yml` since the start and have no splits, symbols, or source).

This repository contains source code. A legally obtained copy of the game is required
for the original binaries and assets.

[Build status](STATUS.md) · [decomp.dev](https://decomp.dev/iirg4x/marioparty6)

The Data badge counts data bytes in matching units, while decomp.dev counts
data symbols paired by name.

## Build

Install Git, Python 3, and Ninja. Use native tools on Windows. On macOS, install
[Wine Crossover](https://github.com/Gcenx/homebrew-wine) to run the Metrowerks
tools. On x86 and x86-64 Linux, the build downloads
[wibo](https://github.com/decompals/wibo).

1. Clone the repository.

   ```sh
   git clone https://github.com/iirg4x/marioparty6.git
   cd marioparty6
   ```

2. Extract the USA Revision 0 disc to `orig/GP6E01` with its directory layout
   intact. For example, Dolphin's **Extract Entire Disc** command provides
   `orig/GP6E01/sys/main.dol` and `orig/GP6E01/files/dll/*.rel`.

3. Configure and build. The first build downloads the pinned tools as needed.
   Run `python configure.py --help` for local tool paths and other options.

   ```sh
   python configure.py
   ninja
   ```

## Verify

The default build checks all configured outputs against the retail SHA-1 hashes.
Run the same check directly with the command below.

```sh
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

On Windows, use `build/tools/dtk.exe`.

## Compare objects

Open the repository in [objdiff](https://github.com/encounter/objdiff) after
configuring. It loads `objdiff.json` from the project directory. Select an object
to compare its compiled output with the retail object. Enable automatic builds
to see the comparison update during edits.

See the [repository guide](docs/README.md) for source and build configuration
paths and [CONTRIBUTING.md](CONTRIBUTING.md) for verification before submission.
