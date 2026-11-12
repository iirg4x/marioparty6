#ifndef REL_M653DLL_RANDOM_H
#define REL_M653DLL_RANDOM_H
#include "dolphin/types.h"

void RandomSeed(u32 seed);
u32 RandomUnsigned();
float RandomFloat(float maximum);
float RandomFloat(float minimum, float maximum);
#endif
