/* Tests whether the two spheres overlap. */
#include "REL/m628DLL/range_record.h"

#include "dolphin/mtx.h"

// Tests whether the two spherical collision records overlap.
s32 fn_1_4548(ActorRangeRecord *first, ActorRangeRecord *second)
{
    Vec firstPosition = static_cast<const Vec &>(first->position);
    Vec secondPosition = static_cast<const Vec &>(second->position);
    Vec difference;
    f32 firstRadiusForComparison;
    f32 secondRadiusForComparison;
    f32 firstRadius;
    f32 secondRadius;
    f32 distance;

    firstRadius = first->radius;
    firstRadiusForComparison = firstRadius;
    secondRadius = second->radius;
    secondRadiusForComparison = secondRadius;
    PSVECSubtract(&firstPosition, &secondPosition, &difference);
    distance = PSVECMag(&difference);
    if (distance > firstRadiusForComparison + secondRadiusForComparison)
    {
        return 0;
    }
    return 1;
}
