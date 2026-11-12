// The game's time limit and gravity. Each file that includes this keeps its own copy, set by its
// static initialiser.
#ifndef REL_M624DLL_GAME_CONSTANTS_H
#define REL_M624DLL_GAME_CONSTANTS_H

#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"

static long timeLimit = secondsToFrames(30L);
// 980.665 cm/s^2 in units per frame squared.
static float gravity = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));

#endif
