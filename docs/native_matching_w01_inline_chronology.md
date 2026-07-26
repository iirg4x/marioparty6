# w01Dll inline ownership and source chronology

This record distills the reusable evidence from the exact `w01Dll` recovery. It
is scoped to the US `GP6E01` REL, MWCC `GC/2.7`, the `GC/2.6` linker, and DTK
`0.9.2`. It documents emitted-code and section ownership; it does not
authenticate semantic names by itself.

## Verified result

The retained `world01` source owner has these exact target dimensions:

- concatenated text: `.text 0x14108` + `.text.common 0x2344` +
  `.text.after_common 0x5C`, SHA-256
  `3e0d85ba153012a26f985cbdd639032a6845abbf6710cb2f691b5be5ce2160ec`;
- `.rodata 0x440`, SHA-256
  `16b4b065eea43a4bdcbc0088151392e2a189614247dc1d4b4df8ee82c5ee7d22`;
- `.data 0x804`, `.bss 0x1624`, and 5,673 relocation annotations;
- linked `w01Dll.rel` SHA-1
  `196d7075abbe6eec3031c9484d25216de9dc0889`;
- linked `main.dol` SHA-1
  `b897e6ade6b3a0cd2f9907689f38a3b19c327e70`;
- one serialized full build and DTK checksum with all 137 configured files
  exact.

The clean public source result is anchored by main commit
`03531bd40870fe23643506d56d9af8cb6d9d119d`; its separately recreated support
commit is `2ee1b7607ce08fa360d75bc713c2c6e48c6b6e3c`.

## How the missing owner was authenticated

The target prefix could not be authenticated from a surviving MP6 header. Three
independent retail modules supplied stronger evidence:

- `w01Dll`, `s01Dll`, and `w04Dll` contain the same target rodata bytes at
  `0x10..0x77`;
- each contains an instruction-identical `0x2344` deferred function cluster,
  SHA-256
  `a650eb99ca9ccb350fdbd4b2cab8452feb48158c65926d59fb381aee0f39e010`;
- the shared function-size sequence is
  `1A8,210,31C,2B4,A4,BC,BC,88,D8,2C0,3B0,170,574,2BC,1DC,150,64,2A0`;
- the literal chronology of that complete cluster explains the common prefix.

Matching only the literal multiset was insufficient. Complete code identity,
function boundaries, literal chronology, and linked proof were required before
the missing inline owner was accepted.

## Compiler and linker mechanism

Under the pinned `GC/2.7` profile, a C++ `extern inline` function with
function-local `static const` values can emit weak inactive rodata while
emitting no active text. The first linked copy may be retained and later copies
deduplicated. A compiler control reproduced `.text 0`, `.rodata 0x10`, zero
relocations, and weak inactive `_half`/`_three` objects.

That control establishes capability, not source ownership. An invented header
with 19 static constants seeded the prefix at zero text cost, but the real
function bodies still materialized their own literals, producing a duplicated
pool. The retail triangulation above was needed to authenticate the actual
shared inline owner.

The authenticated cluster had to be visible early enough to seed its literal
pool while its machine code belonged later in retail text order. The retained
source compiles the cluster under dedicated `code_type` csects. The REL linker
script then concatenates file-qualified `world01.o(.text)`,
`world01.o(.text.common)`, and `world01.o(.text.after_common)` before the
wildcard text selector. An unqualified wildcard placed another object between
these csects and failed the final layout.

## DTK boundary evidence

Before the source chronology was recovered, target/source rodata differed by a
single alignment word while their literal multisets otherwise agreed. Boundary
experiments did not authorize a resplit:

- the independently exact runtime slice owns `0x440..0x458`;
- a `0x43C` world/runtime boundary leaves direct `lbl_1_rodata_43C` consumers
  unresolved;
- a `0x444` boundary bisects runtime `__constants` and DTK rejects it.

The mismatch was pool order/alignment inside the world owner, not missing data
and not a movable owner boundary.

## Counterexamples and limits

- Nearby changed headers were not automatically causal: the examined `mtx.h`
  was byte-identical and `gamework.h` had no relevant w01 consumer.
- MP5-relative include order left the complete object unchanged.
- Ordinary unused inline literals emitted nothing.
- An invented uncalled constant seeder demonstrated emission but duplicated the
  pool and did not authenticate ownership.
- A matching literal multiset without matching code chronology is not evidence
  for a shared inline owner.
- Moving a DTK split to relabel padding is not section-ownership proof.
- These findings do not permit fabricated literals, fake padding, register
  forcing, byte injection, or unscoped linker rewrites.
