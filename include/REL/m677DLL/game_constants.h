// Defines the ten-second rope-selection time limit and the gravity applied each frame.

#ifndef REL_SEER_TERROR_GAME_CONSTANTS_H
#define REL_SEER_TERROR_GAME_CONSTANTS_H

#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"

// Ten seconds at the game's frame rate. Only the board view reads its copy.
static const long tenSeconds = secondsToFrames<long>(10);

// Downward acceleration in world units per frame squared.
static const float gravity = (1.0 / 60.0) * ((1.0 / 60.0) * cmToWorld(980.665f));

#endif
