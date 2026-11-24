# Repository guide

Follow the [README](../README.md#build) to install dependencies, configure, and
build `GP6E01`. Run `python configure.py --help` to point the build at local
copies of the compiler package, binutils, DTK, or the compiler wrapper.

| Path | Contents |
| --- | --- |
| `src/game`, `src/board` | Core game and shared board code. |
| `src/REL` | Runtime-loaded modules. The disc stores these as `files/dll/*.rel`. |
| `src/dolphin`, `src/MSL_C.PPCEABI.bare.H`, `src/Runtime.PPCEABI.H` | GameCube SDK, C library, and compiler runtime. |
| `include` | Game, SDK, and runtime headers. |
| `configure.py` | Source selection, compiler flags, and tool versions. |
| `config/GP6E01` | Main DOL configuration, file splits, symbols, and retail checksums. |
| `config/dll/rels/<module>` | Per-module splits and symbols. |
| `build/GP6E01` | Generated objects, executable, RELs, and reports. |

## Splits

Each `splits.txt` maps section ranges to a translation unit. The source path is
relative to `src/` and corresponds to a unit in `configure.py`. Ranges start at
`start` and end before `end`. The DOL uses absolute addresses and RELs use
section-relative offsets. For example, [the DOL splits](../config/GP6E01/splits.txt)
assign the range below to `src/game/process.c`.

```text
game/process.c:
    .text       start:0x8000D804 end:0x8000E6A8
```

Section attributes include `align:` for alignment, `rename:` for an alternate
output section name, `common` for common BSS, and `skip` for data omitted from
the split object. A file-level `comment:` overrides the CodeWarrior
`mw_comment_version` in [the build configuration](../config/GP6E01/config.yml).
A value of `0` disables generation of a `.comment` section for that object.

## Symbols

Each `symbols.txt` names symbols and records their sections, addresses, and
attributes. This row comes from [the DOL symbols](../config/GP6E01/symbols.txt).

```text
HuPrcInit = .text:0x8000D804; // type:function size:0x14
```

Use the CodeWarrior mangled name for C++ symbols. Attributes describe facts such
as type, size, scope, and alignment. Preserve those facts when renaming a
symbol, and update its declarations, definitions, and callers together. DTK
can rewrite these files during the build, so review any generated changes.
