#ifndef M678_LINK_ENTRY_H
#define M678_LINK_ENTRY_H

#include "dolphin/types.h"

struct M678LinkGroup;
struct M678ChainGroup;
struct M678LinkEntry {
    s32 index;
    s32 variant;
    s32 kind;
    s32 category;
    s32 unk10; /* the game's code never reads this field */
    s32 age;
    s32 groupMaximum;
    u8 unknown_1C[4]; /* No code names this field. */
    f32 gridRow;
    f32 gridColumn;
    f32 fallSpeed;
    f32 floorColumnAtRelease;
    u8 unknown30[4]; /* No code names this field. */
    s32 fadeTimer;
    s32 stillFrames;
    f32 previousColumn;
    s32 releaseSerial;
    struct M678LinkEntry *sequenceNextInRow;
    struct M678LinkEntry *sequencePreviousInRow;
    struct M678LinkEntry *nextFalling;
    struct M678LinkEntry *previousFalling;
    void *sequencePreviousInGroup;
    void *sequenceNextInGroup;
    struct M678ChainGroup *sequenceGroup;
    struct M678LinkEntry *nextInRow;
    struct M678LinkEntry *previousInRow;
    struct M678LinkEntry *previousInGroup[2];
    struct M678LinkEntry *nextInGroup[2];
    struct M678LinkGroup *group[2];
    s32 connected;
    s32 overlay;
    u8 unk88[4]; /* No code names this field. */
    f32 opacityScale;
};

typedef struct M678LinkEntry M678LinkEntry;

#endif
