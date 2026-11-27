#ifndef REL_MINIGAME_GROUND_EFFECT_H
#define REL_MINIGAME_GROUND_EFFECT_H

#include "dolphin/mtx.h"

typedef struct MinigameGroundEffect {
    s32 active; /* Nonzero enables drawing this registration's ground ring. */
    f32 radius; /* World-space radius around position. */
    Vec *position; /* Address of the registered effect's world position. */
} MinigameGroundEffect;

#endif
