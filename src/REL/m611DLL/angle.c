// Angle wrapping and bounded angular differences.
#include "REL/m611DLL/angle.h"
#include "REL/m611DLL/angle_inline.h"

// Returns the signed shortest angular difference in degrees.
// Nothing in this minigame calls this.
f32 fn_1_0(f64 firstAngle, f64 secondAngle)
{
    s32 firstUnits = AngleToUnits(firstAngle);
    s32 secondUnits = AngleToUnits(secondAngle);
    s32 difference = WrapAngleDifference(firstUnits - secondUnits);
    return (f32)((180.0 / 32768.0) * (f64)difference);
}

// Clamps the wrapped angular difference to the turn limit.
f32 fn_1_B8(f64 firstAngle, f64 secondAngle, f32 limit)
{
    f32 difference = DifferenceInDegrees(firstAngle, secondAngle);
    return ClampAngleStep(difference, limit);
}
