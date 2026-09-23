#define _MATH_H
#include "humath.h"
#include "game/frand.h"
void fn_1_164C(HuVecF *dst, Mtx mtx, const HuVecF *src)
{
    PSMTXMultVec(mtx, src, dst);
}
u32 fn_1_1684(Mtx dst, Mtx src)
{
    return PSMTXInverse(src, dst);
}

f32 fn_1_16B4(f32 min, f32 max)
{
    return ((max - min) * frandf()) + min;
}
