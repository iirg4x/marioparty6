# Authenticated original-data promotion

This page covers one narrow path: an authenticated original data payload such
as the MusyX DSP image. It is a byte/source-provenance transfer, not recovered-C
source recovery.

## Boundary

- `tools/promote_original_data.py` is the only promotion path for records in
  `config/recovery/original_data.json`.
- The tool may transfer only the exact allowlisted source blob to a fresh
  `recovery/*` branch created directly from `main`.
- The current allowlist record is
  `src/musyx/runtime/dsp_import.c`; no other C file is authorized by this path.
- No AI metadata, prompts, queue state, tooling, documentation, generated
  output, or commit history may enter `main`.
- Original data earns zero clean-C credit. The ordinary
  `tools/promote_recovered_c.py` path remains unchanged, including its raw
  numeric hexadecimal prohibition for recovered C. Hashes and addresses in
  this manifest and documentation are provenance, not recovered-C literals.

## Current record

The single record in [`../config/recovery/original_data.json`](../config/recovery/original_data.json)
is `musyx-dsp-import-mp4-201`:

| Field | Authenticated value |
| --- | --- |
| Owner | `main:musyx/runtime/dsp_import#mp4-201-dsp-image-v1` |
| Source | `src/musyx/runtime/dsp_import.c` |
| Source SHA-256 | `49a5f2442a39746b5736c6c08807a3544fb70d188c7ab36453ad06fcc855dc7e` (canonical worker source) |
| Authenticated v1 source SHA-256 | `c1ae58e9266eec264439de252eeb42fd486c63933b0aeef58b5120daea6c5989` after removing only `#include "dolphin/math.h"` and its following blank line |
| Payload | `dspSlave`, 7872 bytes, 32-byte alignment; SHA-256 `ccfb81d9afa40688deff1a70e5da4bba1038a23713b4813a22ad900689a79e22` |
| Length symbol | `dspSlaveLength = 7872` |
| Target sections | `.data` at `0x80242C60`, 7872/7872 bytes exact; `.sdata` length at `0x802BFB30`, size 8, value `0x1ec0` exact with six zero alignment-tail bytes |
| Relocations | None |
| Donor | `repos/marioparty4/extern/musyx/src/musyx/runtime/dsp_import.c`, commit `5b5e325e22f593a708393aef363cccf0732d7222`, blob `37ef068dffe78e4e13dfd7a91db1f1addd8d57e4` |
| Status | `native_verified` |

Evidence is [`native_matching_wave41.md`](native_matching_wave41.md),
[`config/GP6E01/splits.txt`](../config/GP6E01/splits.txt), and
[`config/GP6E01/symbols.txt`](../config/GP6E01/symbols.txt). The canonical
worker commit is `35c5d046d4c4147711cdb9ac2af8836c2302eb10`; its source hash is
the manifest value above, and removing only the math-header line plus the
following blank line recovers the authenticated v1 hash. The source has no
`.sdata2`; both objects have zero relocations. V3 reports
`build/recovery-pass/main_musyx_runtime_dsp_import_math_header_v3/strict.json`
and `data-value.json` both have SHA-256
`a980254a40e2f878385ab42cd3d8f9fc687d43da8c5301332d8b1b317e49224e`.
The full serialized retail build passed 137 files (`OK`), and the built
`main.dol` SHA-1 is `b897e6ade6b3a0cd2f9907689f38a3b19c327e70`. Wave 41's
binary-data boundary remains unchanged: this data-only owner expects 0/0
functions and earns zero clean-C credit.

## Native verification result

The native proof gates are recorded for this record:

1. relocation-aware object proof, with both objects at zero relocations;
2. linked-retail byte proof at the target sections and symbols;
3. retail checksum proof from the serialized 137-file build; and
4. data-only accounting proof showing the owner adds no recovered-C functions
   or clean-C credit.

The record is therefore `native_verified`. The clean main-based promotion must
still generate and review `STATUS.md` and the progress sidecars before the
owner is marked `Matching`. This path provides exact byte/source provenance
only; it does not add recovered-C functions or clean-C credit.
