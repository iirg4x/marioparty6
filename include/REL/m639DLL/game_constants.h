// Each translation unit keeps its own initialized game durations and gravity.
#ifndef REL_M639DLL_GAME_CONSTANTS_H
#define REL_M639DLL_GAME_CONSTANTS_H

#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"

static int timeLimit = minutesToFrames(5L);
static int closingTime = secondsToFrames(30L);
// World units per frame squared, converted from 980.665 cm/s^2.
static float gravity = (1.0 / 60.0) * ((1.0 / 60.0) * cmToWorld(980.665f));

#endif
