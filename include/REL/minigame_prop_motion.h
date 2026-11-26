#ifndef REL_MINIGAME_PROP_MOTION_H
#define REL_MINIGAME_PROP_MOTION_H

#include "dolphin/types.h"

typedef struct MinigamePropMotionParams {
    s32 resource; /* Motion resource index; -1 skips this slot and -2 ends the list. */
    f32 blend; /* Blend duration multiplier used when switching from another motion. */
    u32 attributes; /* Motion attributes applied when this motion starts. */
} MinigamePropMotionParams;

#endif
