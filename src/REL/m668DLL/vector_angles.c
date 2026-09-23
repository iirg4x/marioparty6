/* Recovered column scales and guarded Euler-angle operations. The nested
 * float math returns are source-level boundaries, not register constraints. */
#define _MATH_H
#include "humath.h"

extern const f32 lbl_1_rodata_10;
extern const f32 lbl_1_rodata_14;
extern const f32 lbl_1_rodata_18;
extern const f64 lbl_1_rodata_20;
extern const f64 lbl_1_rodata_28;
extern const f32 lbl_1_rodata_1C;
extern const f32 lbl_1_rodata_50;
extern const f32 lbl_1_rodata_54;
extern const f32 lbl_1_rodata_58;
extern f64 asin(f64 x);
extern f64 cos(f64 x);
extern f64 atan2(f64 y, f64 x);

static inline f32 M668Sqrt(f32 x)
{
    volatile f32 result;
    if (x > lbl_1_rodata_18) {
        f64 guess = __frsqrte((f64)x);
        guess = lbl_1_rodata_20 * guess * (lbl_1_rodata_28 - guess * guess * x);
        guess = lbl_1_rodata_20 * guess * (lbl_1_rodata_28 - guess * guess * x);
        guess = lbl_1_rodata_20 * guess * (lbl_1_rodata_28 - guess * guess * x);
        result = (f32)(x * guess);
        return result;
    }
    return x;
}

static inline f32 M668Asinf(f32 x)
{
    return (f32)asin((f64)x);
}

static inline f32 M668Atan2f(f32 y, f32 x)
{
    return (f32)atan2((f64)y, (f64)x);
}

static inline f32 M668ClampedAsin(f32 value)
{
    if (value >= lbl_1_rodata_14) return lbl_1_rodata_54;
    if (value <= lbl_1_rodata_10) return lbl_1_rodata_58;
    return M668Asinf(value);
}

static inline f32 M668GuardedAtan2(f32 y, f32 x)
{
    if (lbl_1_rodata_18 == x) {
        if (y >= lbl_1_rodata_18) return lbl_1_rodata_54;
        return lbl_1_rodata_58;
    }
    return M668Atan2f(y, x);
}

s32 fn_1_E80(Vec *angles, Mtx matrix)
{
    f32 xLength;
    f32 yLength;
    f32 zLength;
    f32 cosine;

    xLength = M668Sqrt(matrix[2][0] * matrix[2][0] +
                       (matrix[0][0] * matrix[0][0] + matrix[1][0] * matrix[1][0]));
    if (xLength < lbl_1_rodata_50) goto fail;
    yLength = M668Sqrt(matrix[2][1] * matrix[2][1] +
                       (matrix[0][1] * matrix[0][1] + matrix[1][1] * matrix[1][1]));
    if (yLength < lbl_1_rodata_50) goto fail;
    zLength = M668Sqrt(matrix[2][2] * matrix[2][2] +
                       (matrix[0][2] * matrix[0][2] + matrix[1][2] * matrix[1][2]));
    if (zLength < lbl_1_rodata_50) goto fail;
    angles->y = -M668ClampedAsin(-matrix[2][0] / xLength);
    cosine = (f32)cos((f64)angles->y);
    if (cosine > lbl_1_rodata_50) {
        angles->x = -M668GuardedAtan2(matrix[2][1] / yLength, matrix[2][2] / zLength);
        angles->z = -M668GuardedAtan2(matrix[1][0], matrix[0][0]);
    } else {
        angles->x = -M668GuardedAtan2(matrix[0][1], matrix[1][1]);
        angles->z = lbl_1_rodata_1C;
    }
    return 1;
fail:
    angles->x = lbl_1_rodata_18;
    angles->y = lbl_1_rodata_18;
    angles->z = lbl_1_rodata_18;
    return 0;
}
