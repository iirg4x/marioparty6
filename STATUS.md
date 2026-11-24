# Mario Party 6 build status

The Mario Party 6 build for GameCube (USA Revision 0, `GP6E01`) is complete and
reproduces `main.dol` and all 136 REL modules configured by `configure.py` byte for byte.
The retail disc also contains `m580Dll.rel` and `m433Dll.rel`, which the game
neither builds nor references (both have been commented out in
`config/GP6E01/config.yml` since the start and have no splits, symbols, or source).

All 33,477 functions match retail. The configured RELs include 128 nonempty
modules and eight empty modules.

## Progress

| Area | Code | Data |
| --- | ---: | ---: |
| Entire project | 100.00% | 100.00% |
| Main DOL | 100.00% | 100.00% |
| REL modules | 100.00% | 100.00% |

These percentages measure bytes in matching source units from the
[committed build snapshot](progress/GP6E01.json).

## Verification

[Ninja and DTK](README.md#verify) check all 137 configured outputs against the
retail hashes. The rebuilt `main.dol` has SHA-1
`b897e6ade6b3a0cd2f9907689f38a3b19c327e70`.

[Progress site](https://iirg4x.github.io/marioparty6/)
