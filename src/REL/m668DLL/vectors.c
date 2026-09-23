#define _MATH_H
#include "humath.h"
f32 fn_1_3D4(const HuVecF *v)
{
    return (v->z * v->z) + ((v->x * v->x) + (v->y * v->y));
}

f32 fn_1_404(const HuVecF *a, const HuVecF *b)
{
    return PSVECDotProduct(a, b);
}
void fn_1_434(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECCrossProduct(a, b, dst);
}
void fn_1_46C(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECAdd(a, b, dst);
}
void fn_1_4A4(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECSubtract(a, b, dst);
}
