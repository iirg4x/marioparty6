#ifndef M669_COLLISION_H
#define M669_COLLISION_H

#include "humath.h"
#include "REL/m669DLL/vector.h"

typedef struct M669Sphere { HuVecF center; f32 radius; } M669Sphere;
/* Vertices are start, start plus edge1, and start plus edge2. */
typedef struct M669Triangle { HuVecF start, edge1, edge2; } M669Triangle;
typedef struct M669Bounds { HuVecF min, max; } M669Bounds; /* Minimum and maximum on each axis. */

void fn_1_2504(M669Bounds *out, M669Sphere *sphere);
s32 fn_1_27FC(M669Bounds *a, M669Bounds *b);
s32 fn_1_286C(M669Sphere *firstSphere, HuVecF *firstVelocity, M669Sphere *secondSphere,
              HuVecF *secondVelocity, f32 maxTime, f32 *hitTime, HuVecF *contactPointOut);
f32 fn_1_2C24(M669Sphere *firstSphere, M669Sphere *secondSphere);
f32 fn_1_2E54(Point3d *queryPoint, M669Triangle *triangle, f32 *barycentricSOut,
              f32 *barycentricTOut);

/* Computes the clamped sphere term shared by collision-distance queries. */
static inline float M669SphereDistanceTerm(const M669Sphere *firstSphere,
    const M669Sphere *secondSphere)
{
    HuVecF centerOffset;
    float gapSquared;
    M669VecSubtract(&centerOffset, &firstSphere->center, &secondSphere->center);
    gapSquared = M669VecLengthSquared(&centerOffset);
    gapSquared -=
        firstSphere->radius * firstSphere->radius + secondSphere->radius * secondSphere->radius;
    if (gapSquared < 0.0f) gapSquared = 0.0f;
    return gapSquared;
}

#endif
