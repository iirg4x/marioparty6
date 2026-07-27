# Exact s01Dll application recovery

The GC/2.6 `s01Dll` application owner is source-linked and byte-identical to
retail. Its 44 application functions occupy `0x366C` text bytes and use a
source-owned `0x78` rodata prefix; the separate runtime owner begins at text
`0x366C` and rodata `0xB8`.

## Exact proof

- Ordinary application text is `0x12CC`; the shared curve cluster is
  `.text.common` `0x2344`; the terminal tick pair is
  `.text.after_common` `0x5C`.
- File-qualified linker selectors concatenate those csects into the retail
  `0x366C` order.
- The source-owned rodata prefix is `0x78` bytes and the complete linked owner
  has 741 real relocations.
- `s01Dll.rel` is byte-identical at SHA-1
  `7f0cfdb2d2b0b2c50b92675e5bef55d72cf94dd7`.
- The serialized build and explicit DTK checksum report all 137 outputs exact;
  `main.dol` and `w01Dll.rel` remain byte-identical.

## Reusable integration findings

MWCC assigned `fn_1_3610` and `fn_1_363C` to the default `.text` csect when
their prototypes appeared before the later section pragma. Removing those
unneeded early declarations let the definitions bind to
`.text.after_common`, after which the final link order matched retail. For code
csects, audit the first declaration rather than only the definition site.

The recovered common cluster naturally emitted the exact pool prefix under
local compiler symbols. Application functions initially referenced configured
external labels in that same prefix, which could not link once the prefix was
source-owned. Restoring the authenticated literal expressions made MWCC reuse
the recovered pool offsets. This is acceptable because the complete section,
linked relocations, consumers, and final REL are exact; it is not permission to
invent literals for an incomplete owner.
