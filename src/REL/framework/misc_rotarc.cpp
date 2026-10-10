/* Quaternion arcs align directions for orientation updates and segment rendering. */
static const char rcsid[] = "$Id: misc_rotarc.cpp,v 1.23 2004/09/09 12:24:38 saf Exp $";

#include "REL/framework/misc_rotarc.h"

/* Called by orientation updates and segment-matrix helpers to rotate start onto end.
 * Input lengths are discarded; zero-length vectors have no special handling. */
void makeRotationArc(Quaternion *rotation, const Vec *start, const Vec *end)
{
    Vector3 startUnit;
    Vector3 endUnit;
    Vector3 crossAxis;
    double directionDot;
    double quaternionScale;

    PSVECNormalize(start, &startUnit);
    PSVECNormalize(end, &endUnit);
    directionDot = startUnit.dot(endUnit);
    quaternionScale = sqrt(2.0 + 2.0 * directionDot);
    rotation->w = quaternionScale / 2.0;
    /* Opposite directions have no fallback when the half-angle factor is zero. */
    quaternionScale = 1.0 / quaternionScale;
    PSVECCrossProduct(&startUnit, &endUnit, &crossAxis);
    rotation->x = crossAxis.x * quaternionScale;
    rotation->y = crossAxis.y * quaternionScale;
    rotation->z = crossAxis.z * quaternionScale;
}
