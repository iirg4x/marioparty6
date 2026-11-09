#include "dolphin/types.h"
#include "dolphin/os.h"
#include "REL/m603DLL/framework/thread.h"

static inline s32 WrapAngleDifference_inlined(s32 difference);

static inline s32 AngleToUnits(f64 angle)
{
    return (s32)(182.04444444444445 * angle);
}

static inline s32 WrapAngleDifference_inlined(s32 difference)
{
    return (u16)(difference - 0x8000) - 0x8000;
}

static inline f32 AngleUnitsToDegrees(s32 angleUnits)
{
    return (f32)(0.0054931640625 * (f64)angleUnits);
}

f32 fn_1_0(f64 firstAngle, f64 secondAngle)
{
    s32 firstUnits = AngleToUnits(firstAngle);
    s32 secondUnits = AngleToUnits(secondAngle);
    s32 difference = WrapAngleDifference_inlined(firstUnits - secondUnits);
    return AngleUnitsToDegrees(difference);
}
static inline s32 WrapAngleDifference(s32 firstUnits, s32 secondUnits)
{
    return WrapAngleDifference_inlined(firstUnits - secondUnits);
}

static inline f32 DifferenceInDegrees(f64 firstAngle, f64 secondAngle)
{
    s32 firstUnits = AngleToUnits(firstAngle);
    s32 secondUnits = AngleToUnits(secondAngle);
    s32 difference = WrapAngleDifference(firstUnits, secondUnits);
    return AngleUnitsToDegrees(difference);
}

static inline f64 ClampAngleDifference(f64 angleDifference, f64 limit)
{
    if (angleDifference < -limit) {
        return -limit;
    }
    if (angleDifference > limit) {
        return limit;
    }
    return angleDifference;
}

f64 fn_1_B8(f64 firstAngle, f64 secondAngle, f64 limit)
{
    f32 difference = DifferenceInDegrees(firstAngle, secondAngle);
    return ClampAngleDifference(difference, limit);
}
