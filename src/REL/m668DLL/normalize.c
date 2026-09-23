#define _MATH_H
#include "humath.h"

extern const float lbl_1_rodata_18;
extern const double lbl_1_rodata_20;
extern const double lbl_1_rodata_28;
extern const float lbl_1_rodata_30;
extern const float lbl_1_rodata_14;

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

f32 fn_1_4DC(HuVecF *dst, const HuVecF *src)
{
    f32 magnitude2;
    f32 sqrt_result;
    f32 magnitude;

    magnitude2 = (src->z * src->z) + ((src->x * src->x) + (src->y * src->y));
    sqrt_result = M668Sqrt(magnitude2);
    magnitude = sqrt_result;
    if (magnitude > lbl_1_rodata_30) {
        f32 inverse = lbl_1_rodata_14 / magnitude;
        dst->x = src->x * inverse;
        dst->y = src->y * inverse;
        dst->z = src->z * inverse;
    } else {
        magnitude = lbl_1_rodata_18;
        dst->x = lbl_1_rodata_18;
        dst->y = lbl_1_rodata_18;
        dst->z = lbl_1_rodata_18;
    }
    return magnitude;
}
