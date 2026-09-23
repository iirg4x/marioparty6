#define _MATH_H
#include "humath.h"

typedef struct M668VecBound_s {
    HuVecF xyz;
    f32 margin;
} M668VecBound;
typedef char M668VecBound_size_check[(sizeof(M668VecBound) == 16) ? 1 : -1];

typedef struct M668SegmentBound_s {
    HuVecF endpoints[2];
    f32 margin;
} M668SegmentBound;
typedef char M668SegmentBound_size_check[(sizeof(M668SegmentBound) == 28) ? 1 : -1];

extern f32 fn_1_404(const HuVecF *a, const HuVecF *b);
extern const f32 lbl_1_rodata_60;

extern inline f64 m668_fabs64(f64 x)
{
    return __fabs(x);
}

extern inline f32 m668_fabs32(f32 x)
{
    return m668_fabs64((f64)x);
}

s32 fn_1_16FC(const M668VecBound *plane, const M668SegmentBound *segment)
{
    HuVecF normal = plane->xyz;
    f32 planeMargin = plane->margin;
    f32 begin = fn_1_404(&normal, &segment->endpoints[0]) - planeMargin;
    f32 end = begin + fn_1_404(&normal, &segment->endpoints[1]);
    s32 intersects;
    if ((begin * end) <= 0.0f) {
        return 1;
    }
    intersects = 1;
    if (!(m668_fabs32(begin) <= segment->margin) &&
        !(m668_fabs32(end) <= segment->margin)) {
        intersects = 0;
    }
    return intersects;
}
