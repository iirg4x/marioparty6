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
| Source SHA-256 | `c1ae58e9266eec264439de252eeb42fd486c63933b0aeef58b5120daea6c5989` |
| Payload | `dspSlave`, 7872 bytes, 32-byte alignment; SHA-256 `ccfb81d9afa40688deff1a70e5da4bba1038a23713b4813a22ad900689a79e22` |
| Length symbol | `dspSlaveLength = 7872` |
| Target sections | `.data` at `0x80242C60`, size 7872; `.sdata` length at `0x802BFB30`, size 8 |
| Relocations | None |
| Donor | `repos/marioparty4/extern/musyx/src/musyx/runtime/dsp_import.c`, commit `5b5e325e22f593a708393aef363cccf0732d7222`, blob `37ef068dffe78e4e13dfd7a91db1f1addd8d57e4` |
| Status | `static_authenticated_pending_native` |

Evidence is [`native_matching_wave41.md`](native_matching_wave41.md),
[`config/GP6E01/splits.txt`](../config/GP6E01/splits.txt), and
[`config/GP6E01/symbols.txt`](../config/GP6E01/symbols.txt). Wave 41 records
`dsp_import.c` as binary data excluded from clean-C totals; this record keeps
that boundary while preserving the authenticated donor and byte hashes. No
object proof is claimed yet.

## Gates before `Matching`

The static record must remain pending until all native gates are recorded:

1. relocation-aware object proof;
2. relocation proof (the manifest currently records zero relocations);
3. linked-retail byte proof at the target sections and symbols;
4. retail checksum proof; and
5. progress/status proof showing the owner can be marked `Matching` without
   changing clean-C totals or regressing another owner.

Until those gates pass, this path provides exact byte/source provenance only and
does not establish a `Matching` owner.
