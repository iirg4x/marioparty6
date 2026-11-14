// Vector operations shared by the framework and minigame collision code.
#ifndef REL_FRAMEWORK_VECTOR_MATH_H
#define REL_FRAMEWORK_VECTOR_MATH_H

#include "dolphin/mtx.h"

static inline f32 vectorDot(const Vec *first, const Vec *second)
{
    return PSVECDotProduct(first, second);
}

static inline f32 vectorMagnitude(const Vec *vector)
{
    return PSVECMag(vector);
}

static inline void vectorSet(Vec *vector, double x, double y, double z)
{
    vector->x = x;
    vector->y = y;
    vector->z = z;
}

#endif
