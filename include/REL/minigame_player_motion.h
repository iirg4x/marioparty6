#ifndef REL_MINIGAME_PLAYER_MOTION_H
#define REL_MINIGAME_PLAYER_MOTION_H

#include "dolphin/types.h"

typedef struct MinigamePlayerMotionParams {
    u16 motion; /* Motion-array slot played for this player action. */
    s16 overlay; /* Overlay motion-array slot, or -1 when no overlay is used. */
    f32 blend; /* Motion-shift blend fraction, multiplied by 60 frames. */
    u32 attr; /* Motion attributes applied while the action plays. */
} MinigamePlayerMotionParams;

#endif
