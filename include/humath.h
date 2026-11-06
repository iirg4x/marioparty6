#ifndef _HU_MATH_H
#define _HU_MATH_H

#include "dolphin/mtx.h"

/* pi as a double: the genuine MSL math.h does not define it (it is not standard C) */
#ifndef M_PI
#define M_PI 3.141592653589793
#endif

typedef Vec HuVecF;

typedef struct HuVec2f_s {
    float x;
    float y;
} HuVec2f;

/* Returns the absolute value of a float. */
static inline float FloatAbs(register float x)
{
    asm {
        fabs x, x
    }
    return x;
}

#define HuSetVecF(vec, x, y, z) \
    (vec)->x = (x); \
    (vec)->y = (y); \
    (vec)->z = (z)

#define HuSubVecF(dst, src1, src2) VECSubtract(src1, src2, dst)

#endif
