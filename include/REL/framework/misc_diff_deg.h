// Degree-angle differences across the wrap point (misc_diff_deg.cpp, and the copies of its code
// in m603 and m648).
#ifndef REL_FRAMEWORK_MISC_DIFF_DEG_H
#define REL_FRAMEWORK_MISC_DIFF_DEG_H

enum {
    ANGLE_HALF_TURN = 32768,
    ANGLE_WRAP_MASK = 0xFFFF
};

// Wraps target minus current into [-32768, 32767] units of a 65536-unit turn.
inline int wrappedAngleDifference(int targetAngle, int currentAngle)
{
    return ((targetAngle - currentAngle - ANGLE_HALF_TURN) & ANGLE_WRAP_MASK) - ANGLE_HALF_TURN;
}

// Converts degrees to turn units, discarding the fractional unit toward zero.
inline int degreeToAngle(float degrees)
{
    return (32768.0 / 180.0) * degrees;
}

// Converts the signed turn-unit difference back to degrees.
inline float angleToDegree(int angleUnits)
{
    return (180.0 / 32768.0) * angleUnits;
}

float degreeDifference(float targetDegrees, float currentDegrees);
float limitedDegreeDifference(float targetDegrees, float currentDegrees,
                              float maxTurnDegrees);

#endif
