#ifndef REL_M663DLL_EFFECTS_H
#define REL_M663DLL_EFFECTS_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "dolphin/gx.h"

typedef struct M663EffectParameters {
    u32 unobservedValue; /* Initialized to zero; never read. */
    f32 fields04[3];
    GXColor color;
    u8 fields14[4];
    f32 velocityScale[3];
    f32 fields24[2];
    f32 verticalPosition;
    f32 verticalScale;
    f32 opacityChange;
    f32 fields38;
} M663EffectParameters;

typedef struct M663EffectEntry {
    s16 state;
    s16 ownerIndex;
    u8 fields04[4];
    Vec velocity;
    Vec origin;
    f32 fields20[3];
    f32 height;
    u8 fields30[8];
    Vec position;
    GXColor color;
} M663EffectEntry;

typedef struct M663EffectContext {
    u8 fields00[2];
    s16 lastEntry;
    u8 fields04[0x18];
    M663EffectParameters *parameters;
    u8 fields20[0x10];
    s16 entryCount;
    u8 fields32[2];
    void *field34;
    u8 fields38[0x10];
    M663EffectEntry *entries;
} M663EffectContext;

s16 fn_1_5094(s16 modelNumber, s16 ownerIndex, Vec position, Vec velocity, f32 alpha, s16 state, M663EffectParameters *parameters);
void fn_1_5294(s32 model, M663EffectContext *context);
void fn_1_54C8(s32 model, M663EffectContext *context);
void fn_1_4FA8(void);

#endif
