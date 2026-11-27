#ifndef REL_MINIGAME_COLLISION_POSITION_H
#define REL_MINIGAME_COLLISION_POSITION_H

#include "dolphin/mtx.h"
#include "math.h"

/* Reject positions with a non-finite coordinate before collision updates. */
static inline s32 collisionPositionInvalid(Vec *position)
{
    return isnan(position->x) ||
        isinf(position->x) ||
        isnan(position->y) ||
        isinf(position->y) ||
        isnan(position->z) ||
        isinf(position->z);
}

#endif
