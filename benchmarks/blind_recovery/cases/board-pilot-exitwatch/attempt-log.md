# Blind attempt log

## Attempt 1 - accepted

- Hypothesis: direct structured implementation with an ordinary pause-check local, a cached `ExitObjList *`, and a signed loop index; pre-tested `while` waits and a `for` traversal.
- Source-shape choice: declare `pauseNo`, `i`, then `list`; test exit-request globals and wait-call results explicitly against zero; index `list->entries` directly in the test and reset call.
- Compiler command: `rtk proxy powershell -NoProfile -ExecutionPolicy Bypass -File .\compile.ps1`.
- Assembly observation: exact 300-byte/75-instruction shape. Saved-register roles, branch offsets, operands, ordinary epilogue, and all 23 relocation-bearing entries match.
- Outcome: accepted; stopped after the first compile.
- Frozen candidate SHA-256: `d71f5d13d03d1dabbd212b8918408a4946b3a600d65880110465964901d728b7`.
- Elapsed time: not separately instrumented.
