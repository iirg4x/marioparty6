#define _MATH_H
#include "humath.h"

extern const float lbl_1_rodata_18;
extern const double lbl_1_rodata_20;
extern const double lbl_1_rodata_28;

/* SDK sqrtf refinement, retaining its volatile single-precision result. */
static inline float M668Sqrt(float x)
{
    volatile float y;
    if (x > lbl_1_rodata_18) {
        double guess = __frsqrte((double)x);
        guess = lbl_1_rodata_20 * guess * (lbl_1_rodata_28 - guess * guess * x);
        guess = lbl_1_rodata_20 * guess * (lbl_1_rodata_28 - guess * guess * x);
        guess = lbl_1_rodata_20 * guess * (lbl_1_rodata_28 - guess * guess * x);
        y = (float)(x * guess);
        return y;
    }
    return x;
}

float fn_1_2B4(const HuVecF *vec)
{
    return M668Sqrt(vec->z * vec->z + (vec->x * vec->x + vec->y * vec->y));
}
