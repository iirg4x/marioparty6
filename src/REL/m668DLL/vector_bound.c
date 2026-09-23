#define _MATH_H
#include "humath.h"

/* Consumed layout: three vector components followed by one float bound. */
typedef struct M668VecBound_s {
    HuVecF xyz;
    f32 margin;
} M668VecBound;
typedef char M668VecBound_size_check[(sizeof(M668VecBound) == 16) ? 1 : -1];
extern f32 fn_1_404(const HuVecF *a, const HuVecF *b);

s32 fn_1_1844(const M668VecBound *a, const M668VecBound *b)
{
    f32 dot = fn_1_404(&a->xyz, &b->xyz);
    return dot <= (a->margin + b->margin);
}
