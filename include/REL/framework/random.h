// The minigame framework's shared random number stream (random.cpp).
#ifndef REL_FRAMEWORK_RANDOM_H
#define REL_FRAMEWORK_RANDOM_H

#include "dolphin/types.h"

void RandomSeed(u32 seed);
u32 RandomUnsigned();
s32 RandomInteger(s32 limit);
float RandomFloat();
float RandomFloat(float maximum);
float RandomFloat(float minimum, float maximum);

#endif
