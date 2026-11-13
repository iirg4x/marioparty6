// Each including translation unit retains these unused initialized values.
#ifndef REL_M681DLL_GAME_CONSTANTS_H
#define REL_M681DLL_GAME_CONSTANTS_H
#include "dolphin/types.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
static s32 timeLimit = minutesToFrames(60L);
static f32 gravity = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
#endif
