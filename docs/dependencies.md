# Dependencies

The project requires Git, Python, and Ninja. The remaining tools and pinned
compiler packages are prepared by `configure.py` when explicit paths are not
provided.

## Windows

Native Windows tooling is recommended. WSL and MSYS2 are not required, and
objdiff filesystem notifications may not work reliably when the checkout is
accessed across the Windows/WSL boundary.

- Install [Git](https://git-scm.com/download/win).
- Install [Python](https://www.python.org/downloads/) and add it to `%PATH%`.
- Install [Ninja](https://github.com/ninja-build/ninja/releases) and add it to
  `%PATH%`.

Ninja can also be installed with:

```sh
python -m pip install ninja
```

## macOS

Install Git, Python, and Ninja. The Metrowerks compiler executables require a
compatible Wine environment.

```sh
brew install git python ninja
brew install --cask --no-quarantine gcenx/wine/wine-crossover
```

After some macOS upgrades, Wine Crossover may need to be unquarantined again:

```sh
sudo xattr -rd com.apple.quarantine '/Applications/Wine Crossover.app'
```

## Linux

Install Git, Python, and Ninja through the distribution package manager.

On x86 and x86-64 systems, the configured wibo wrapper is downloaded
automatically. Other architectures generally require Wine or an explicitly
provided wrapper.

## Optional local tool paths

`configure.py` accepts explicit paths for the compiler package and support tools:

```sh
python configure.py --help
```

Common options include:

```text
--compilers
--binutils
--dtk
--sjiswrap
--wrapper
```
