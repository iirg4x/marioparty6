#define _MATH_H
#include "humath.h"
#include <string.h>

extern const f32 lbl_1_rodata_10;
extern const f32 lbl_1_rodata_14;
extern const f32 lbl_1_rodata_18;
extern const f64 lbl_1_rodata_20;
extern const f64 lbl_1_rodata_28;
extern const f32 lbl_1_rodata_1C;
extern const Vec lbl_1_rodata_34;
extern const Vec lbl_1_rodata_40;
extern const f32 lbl_1_rodata_4C;
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

static inline f32 M668Magnitude(const Vec *vec)
{
    return M668Sqrt(vec->z * vec->z + (vec->x * vec->x + vec->y * vec->y));
}

static inline f32 M668Dot(const Vec *a, const Vec *b)
{
    return PSVECDotProduct(a, b);
}

static inline void M668Scale(Vec *dst, const Vec *src, f32 scale)
{
    PSVECScale(src, dst, scale);
}

s32 fn_1_788(Mtx matrix, const Vec *start, const Vec *end, const Vec *direction)
{
    Vec edge;
    Vec transverse;
    Vec cross;
    f32 length;
    s32 i;

    PSVECSubtract(end, start, &edge);
    length = M668Magnitude(&edge);
    if (length < lbl_1_rodata_4C) {
        return 0;
    }
    if (lbl_1_rodata_18 != length) {
        M668Scale(&edge, &edge, lbl_1_rodata_14 / length);
    }
    M668Scale(&transverse, &edge, M668Dot(direction, &edge));
    PSVECSubtract(direction, &transverse, &transverse);
    length = M668Magnitude(&transverse);
    if (lbl_1_rodata_4C > length) {
        Vec up = lbl_1_rodata_34;
        M668Scale(&transverse, &edge, edge.y);
        PSVECSubtract(&up, &transverse, &transverse);
        length = M668Magnitude(&transverse);
        if (lbl_1_rodata_4C > length) {
            Vec forward = lbl_1_rodata_40;
            M668Scale(&transverse, &edge, edge.z);
            PSVECSubtract(&forward, &transverse, &transverse);
            length = M668Magnitude(&transverse);
            if (lbl_1_rodata_4C > length) {
                return 0;
            }
        }
    }
    if (lbl_1_rodata_18 != length) {
        M668Scale(&transverse, &transverse, lbl_1_rodata_14 / length);
    }
    PSVECCrossProduct(&transverse, &edge, &cross);
    memset(matrix, 0, 36U);
    for (i = 0; i < 3; i++) {
        matrix[i][i] = lbl_1_rodata_14;
    }
    matrix[0][0] = -cross.x;
    matrix[0][1] = transverse.x;
    matrix[0][2] = edge.x;
    matrix[1][0] = cross.y;
    matrix[1][1] = transverse.y;
    matrix[1][2] = edge.y;
    matrix[2][0] = cross.z;
    matrix[2][1] = transverse.z;
    matrix[2][2] = edge.z;
    return 1;
}
