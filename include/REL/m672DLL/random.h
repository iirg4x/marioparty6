#ifndef M672_RANDOM_H
#define M672_RANDOM_H

#include "dolphin/types.h"

void RandomSeed(unsigned long);
u32 RandomUnsigned();
float RandomFloat(float minimum, float maximum);

#endif
