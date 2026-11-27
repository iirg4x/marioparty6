#ifndef REL_MINIGAME_MENU_TRAJECTORY_H
#define REL_MINIGAME_MENU_TRAJECTORY_H

#include "dolphin/mtx/GeoTypes.h"

typedef struct MinigameMenuTrajectory {
    s16 state; /* Current state of this movement. */
    HuVecF startPosition; /* Position at the start of the curve. */
    HuVecF controlPosition; /* Control point shaping the curve. */
    HuVecF endPosition; /* Destination position. */
    f32 elapsed; /* Time progressed along the curve. */
    f32 duration; /* Total time for the curve. */
} MinigameMenuTrajectory;

#endif
