#ifndef REL_M649DLL_EFFECT_WORK_H
#define REL_M649DLL_EFFECT_WORK_H

#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "game/animdata.h"

typedef struct {
    s32 active, camera; /* Draw flag and camera index selecting the shadow model. */
    float radius; /* Shadow fan radius in world units. */
    Vec *position, *offset; /* Live shadow center and the offset added when drawing it. */
} M649ShadowRecord;

void fn_1_A018(M649ShadowRecord *effect);

typedef struct {
    s32 visible, generation, phase, spawnDelay; /* Draw flag, trail depth, and update state. */
    s32 unk10; /* The game's code never reads this field. */
    float size; /* Radius of the rendered score effect in world units. */
    float *speed; /* Live travel speed of the prop that created this effect. */
    Vec position, trailPosition; /* Moving effect position and rising trail center. */
    GXColor color; /* Trail color and fading alpha. */
} M649EffectRecord;

typedef struct {
    ANIMDATA *animations[2]; /* Loaded effect animations. */
} M649EffectOwner;

#endif
