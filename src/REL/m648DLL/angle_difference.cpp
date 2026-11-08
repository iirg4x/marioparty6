#include "dolphin/types.h"
#include "REL/m648DLL/framework/linknode.h"
#include "REL/m648DLL/motion_lists.h"
#include "REL/m648DLL/angle_difference.h"

static inline s32 AngleToUnits(f64 angle)
{
    return (s32)(182.04444444444445 * angle);
}

static inline s32 WrapAngleDifference(s32 firstUnits, s32 secondUnits)
{
    return (u16)((firstUnits - secondUnits) - 0x8000) - 0x8000;
}

static inline f32 AngleUnitsToDegrees_inlined(s32 angleUnits)
{
    return (f32)(0.0054931640625 * (f64)angleUnits);
}

f32 fn_1_36C4(f64 firstAngle, f64 secondAngle)
{
    s32 firstUnits = AngleToUnits(firstAngle);
    s32 secondUnits = AngleToUnits(secondAngle);
    s32 difference = WrapAngleDifference(firstUnits, secondUnits);
    return AngleUnitsToDegrees_inlined(difference);
}
static inline f32 ClampAngleDifference_inlined(f32 angleDifference, f32 limit)
{
    if (angleDifference < -limit) {
        return -limit;
    }
    if (angleDifference > limit) {
        return limit;
    }
    return angleDifference;
}

f32 fn_1_377C(f64 firstAngle, f64 secondAngle, f32 limit)
{
    f32 difference = fn_1_36C4(firstAngle, secondAngle);
    return ClampAngleDifference_inlined(difference, limit);
}
