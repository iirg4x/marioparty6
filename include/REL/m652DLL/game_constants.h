// Each translation unit keeps these initialized values. Only game_owner reads the
// duration and limit; only player reads gravity. The other twelve values are unread.
#ifndef REL_M652DLL_GAME_CONSTANTS_H
#define REL_M652DLL_GAME_CONSTANTS_H
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
static long gameDuration = minutesToFrames(5L);
static long timeLimit = secondsToFrames(30L);
// Convert centimetres per second squared to world units per frame squared.
static float gravity = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
#endif
