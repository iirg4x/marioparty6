#ifndef REL_M611DLL_ANGLE_INLINE_H
#define REL_M611DLL_ANGLE_INLINE_H

#include "dolphin/types.h"

// Converts degrees to the signed integer angle scale.
static inline s32 AngleToUnits(f64 angle)
{
    return (s32)((32768.0 / 180.0) * angle);
}

// Wraps an integer angle difference to a half-turn on either side of zero.
static inline s32 WrapAngleDifference(s32 difference)
{
    return (u16)(difference - 32768) - 32768;
}

// Clamps a signed angle difference to the turn limit.
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

// Subtracts integer angles and wraps the result to a signed half-turn.
static inline s32 WrapAngleUnitsDifference(s32 firstUnits, s32 secondUnits)
{
    return (u16)((firstUnits - secondUnits) - 32768) - 32768;
}

// Converts integer angle units to degrees.
static inline f32 AngleUnitsToDegrees(s32 angleUnits)
{
    return (f32)((180.0 / 32768.0) * (f64)angleUnits);
}

// Returns the wrapped difference between two angles in degrees.
static inline f32 DifferenceInDegrees(f64 firstAngle, f64 secondAngle)
{
    s32 firstUnits = AngleToUnits(firstAngle);
    s32 secondUnits = AngleToUnits(secondAngle);
    s32 difference = WrapAngleUnitsDifference(firstUnits, secondUnits);
    return AngleUnitsToDegrees(difference);
}

// Clamps a signed angular difference to the turn limit.
static inline f32 ClampAngleStep(f32 difference, f32 limit)
{
    if (difference < -limit) {
        return -limit;
    } else if (difference > limit) {
        return limit;
    }
    return difference;
}

#endif
