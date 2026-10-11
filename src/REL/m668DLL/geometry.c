/* Burnstile rotor collision half-space tests and a general segment-plane test. */
#include "dolphin/math.h"
#include "humath.h"

typedef struct M668VecBound_s {
    HuVecF xyz; /* Bound center or normal vector, in world coordinates. */
    f32 margin; /* Plane offset or bound radius used by the projection tests. */
} M668VecBound;
typedef char M668VecBound_size_check[(sizeof(M668VecBound) == 16) ? 1 : -1];

typedef struct M668SegmentBound_s {
    HuVecF startAndDisplacement[2]; /* Segment start and displacement in world coordinates. */
    f32 margin; /* Allowed distance from the segment to a tested plane. */
} M668SegmentBound;
typedef char M668SegmentBound_size_check[(sizeof(M668SegmentBound) == 28) ? 1 : -1];

extern f32 fn_1_404(const HuVecF *a, const HuVecF *b);

extern inline f32 m668_fabs32(f32 value)
{
    return fabs((f64)value);
}

/* Tests whether a segment crosses a plane or comes within its collision margin. */
s32 fn_1_16FC(const M668VecBound *plane, const M668SegmentBound *segment)
{
    HuVecF normal = plane->xyz;
    f32 planeMargin = plane->margin;
    f32 begin = fn_1_404(&normal, &segment->startAndDisplacement[0]) - planeMargin;
    f32 end = begin + fn_1_404(&normal, &segment->startAndDisplacement[1]);
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

#include "humath.h"

/* Tests a bound position against a plane normal, allowing the sum of both margins. */
s32 fn_1_1844(const M668VecBound *a, const M668VecBound *b)
{
    f32 dot = fn_1_404(&a->xyz, &b->xyz);
    return dot <= (a->margin + b->margin);
}

#include "dolphin/types.h"
extern s16 *lbl_1_bss_2C[4];
extern s16 *lbl_1_bss_28[];
s16 *fn_1_18B4(s32 index) { return lbl_1_bss_2C[index]; }
s16 *fn_1_18CC(s32 index) { return lbl_1_bss_28[index]; }
