# Contributing

Use a focused pull request to improve names, types, or comments. Explain the
evidence for the change, such as a same-game symbol, a caller, or the width and
layout of a field. Keep uncertain names and types explicit.

1. Follow the [build instructions](README.md#build) from a clean checkout.
2. Update the relevant declarations, definitions, callers, and symbol rows
   together. The [repository guide](docs/README.md) describes the build files.
3. Compare the affected objects in [objdiff](README.md#compare-objects) before
   and after the change. Check relocations and consumers of shared changes.
4. Run `ninja` and the [retail checksum command](README.md#verify). The DOL
   and every REL must remain byte-identical. Include the results in the pull
   request.

To refresh progress files after a change that affects the published report, run
`python tools/build.py` and review the generated diff. Commit generated progress
files when they change. Keep original game files, rebuilt binaries, and the
generated build directory out of commits.
