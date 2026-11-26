#ifndef M615DLL_GEOMETRY_H
#define M615DLL_GEOMETRY_H

#include "humath.h"

typedef struct M615Bounds {
    HuVecF min;
    HuVecF max;
} M615Bounds;

typedef struct M615Segment {
    HuVecF start;
    HuVecF delta;
} M615Segment;

typedef struct M615Capsule {
    M615Segment segment;
    f32 radius;
} M615Capsule;

typedef struct M615Triangle {
    HuVecF start;
    HuVecF edge1;
    HuVecF edge2;
} M615Triangle;

typedef struct M615Box {
    HuVecF center;
    HuVecF axis[3];
    f32 halfExtents[3];
} M615Box;

void fn_1_A0C(M615Bounds *out, M615Capsule *capsule);
void fn_1_BC4(M615Bounds *bounds, M615Triangle *triangle);
void fn_1_E38(M615Bounds *out, M615Triangle *triangle);
void fn_1_109C(M615Bounds *bounds, const M615Box *transform);
s32 fn_1_1478(M615Bounds *a, M615Bounds *b);
f32 fn_1_14E8(HuVecF *queryPoint, M615Triangle *triangle,
             f32 *barycentricS, f32 *barycentricT);
f32 fn_1_181C(M615Segment *firstSegment, M615Segment *secondSegment, f32 *parameterAOut,
              f32 *parameterBOut);
f32 fn_1_2544(M615Segment *segment, M615Triangle *triangle, f32 *segmentTOut,
              f32 *triangleSOut, f32 *triangleTOut);
f32 fn_1_42CC(HuVecF *queryPoint, M615Triangle *triangle, f32 *barycentricSOut,
              f32 *barycentricTOut);
f32 fn_1_4C9C(M615Segment *segment, M615Triangle *triangle, f32 *segmentTOut,
              f32 *triangleSOut, f32 *triangleTOut);
float fn_1_643C(HuVecF *queryPoint, M615Segment *lineSegment);
f32 fn_1_8D08(const M615Segment *line, const M615Box *box, f32 *time,
                 f32 *closestX, f32 *closestY, f32 *closestZ);

#endif
