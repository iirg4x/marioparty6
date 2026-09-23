#define _MATH_H
#include "humath.h"
void fn_1_6BC(HuVecF *dst, const HuVecF *src, f32 scale)
{
    PSVECScale(src, dst, scale);
}

void fn_1_6F4(HuVecF *dst, const HuVecF *src, Mtx mtx)
{
    dst->x = (src->z * mtx[2][0]) + ((src->x * mtx[0][0]) + (src->y * mtx[1][0]));
    dst->y = (src->z * mtx[2][1]) + ((src->x * mtx[0][1]) + (src->y * mtx[1][1]));
    dst->z = (src->z * mtx[2][2]) + ((src->x * mtx[0][2]) + (src->y * mtx[1][2]));
}
