# Contributing

Contributions should improve the accuracy, readability, or documentation of the
Mario Party 6 source recovery.

## Before starting

1. Build the project successfully from a clean checkout.
2. Use objdiff to record the current state of the object you plan to change.
3. Keep the change focused on one translation unit or a small, related function
   group whenever possible.

## Source recovery

The goal is the most likely original source—not merely C that happens to emit the
same bytes.

- Prefer natural, readable C.
- Keep uncertain semantics explicit rather than inventing names or types.
- Use callers, consumers, relocations, access widths, strings, data ownership,
  and closely related source as evidence.
- Avoid pragmas, artificial `volatile` or `register` usage, dead branches,
  fabricated padding, synthetic globals, and other compiler-control techniques
  unless the target clearly requires them.
- Do not regress an independently matching function to improve another one.
- Treat a matching object as binary proof, not automatic proof that every name,
  type, or source construct is authentic.

## Verification

For a private C implementation change, record at minimum:

- object comparison before and after;
- exact functions and text bytes gained or lost;
- relocation differences;
- previously exact functions affected;
- direct consumers that require checking.

Changes to shared headers, symbols, splits, compiler flags, object status, or
link configuration require broader consumer checks and a full serialized build.
Before submitting a matching change, run the configured retail hash check:

```sh
build/tools/dtk shasum -q -c config/GP6E01/build.sha1
```

On Windows, use `build/tools/dtk.exe`.

## Pull requests

A pull request should explain:

- the source owner and functions changed;
- the evidence supporting the recovered source shape;
- important alternatives that were tested and rejected;
- object, relocation, consumer, and full-build results actually run;
- remaining uncertainty or semantic debt.

Keep commits scoped and avoid unrelated formatting changes.

## Repository hygiene

Do not commit:

- files from `orig/`;
- rebuilt DOL or REL binaries;
- files under `build/`;
- generated `build.ninja`, `objdiff.json`, or context files;
- copyrighted game assets.
