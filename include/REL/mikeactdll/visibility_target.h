#ifndef MIKEACTDLL_VISIBILITY_TARGET_H
#define MIKEACTDLL_VISIBILITY_TARGET_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"
typedef struct {
    s16 model_id;
    u8 unk02[2]; /* No code names this field. */
    HuVecF position;
    HuVecF movement;
    f32 scale;
    f32 transparency;
    u32 frame;
    s16 active;
    u8 unk2A[2]; /* No code names this field. */
} MikeActModelEffect;

typedef struct {
    s16 state;
    s16 positionIndex;
    s16 playerIndex;
    s16 mode;
    s32 triggered;
    s32 paused;
    MikeActModelEffect items[0x14];
    s16 activeItemCount;
    s16 updateCount;
    s32 soundHandle;
} MikeActVisibilityTarget;
#endif
