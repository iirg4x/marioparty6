/* Vector scaling and capsule bounding boxes for Clean Team. */
#include "REL/m645DLL/shared_types.h"
#include "humath.h"
#include "game/object.h"





void fn_1_480(HuVecF *out, const HuVecF *a, const HuVecF *b);

void fn_1_6D0(HuVecF *dst, const HuVecF *src, f32 scale)
{
    PSVECScale(src, dst, scale);
}

void fn_1_708(M645Bounds *out, M645Capsule *capsule)
{
    HuVecF points[2];
    points[0] = capsule->segment.start;
    fn_1_480(&points[1], &capsule->segment.start, &capsule->segment.delta);
    out->min.x = (points[0].x < points[1].x ? points[0].x : points[1].x) - capsule->radius;
    out->max.x = capsule->radius + (points[0].x >= points[1].x ? points[0].x : points[1].x);
    out->min.y = (points[0].y < points[1].y ? points[0].y : points[1].y) - capsule->radius;
    out->max.y = capsule->radius + (points[0].y >= points[1].y ? points[0].y : points[1].y);
    out->min.z = (points[0].z < points[1].z ? points[0].z : points[1].z) - capsule->radius;
    out->max.z = capsule->radius + (points[0].z >= points[1].z ? points[0].z : points[1].z);
}
