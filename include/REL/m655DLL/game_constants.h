// Each translation unit keeps its own time limit and gravity.
#ifndef REL_M655DLL_GAME_CONSTANTS_H
#define REL_M655DLL_GAME_CONSTANTS_H

#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"

static long timeLimit = secondsToFrames(10L);
static const float gravity = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));

// Move a coordinate one twentieth of the remaining distance toward its target.
static inline float weightedHeight(float start, float end) {
    return (float)(0.95 * start + 0.05 * end);
}

#endif
