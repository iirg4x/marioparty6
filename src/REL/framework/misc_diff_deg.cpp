// Compares degree angles across the wrap point and optionally limits the turn.
static const char rcsid[] = "$Id: misc_diff_deg.cpp,v 1.23 2004/09/09 12:24:38 saf Exp $";

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

// Supplies degree-angle comparisons, including limitedDegreeDifference, with the shortest turn.
// Integer turn units quantize both inputs; a half-turn tie returns -180 degrees.
float degreeDifference(float targetDegrees, float currentDegrees)
{
    int targetAngle = degreeToAngle(targetDegrees);
    int currentAngle = degreeToAngle(currentDegrees);
    int turnUnits = wrappedAngleDifference(targetAngle, currentAngle);
    return angleToDegree(turnUnits);
}

// Supplies callers limiting a degree-angle turn with the shortest difference capped in either
// direction.
// The limit is used as supplied: negative values are not converted to a positive turn limit.
float limitedDegreeDifference(float targetDegrees, float currentDegrees, float maxTurnDegrees)
{
    float turnDegrees = degreeDifference(targetDegrees, currentDegrees);
    float limitedTurnDegrees;
    if (turnDegrees < -maxTurnDegrees)
        limitedTurnDegrees = -maxTurnDegrees;
    else if (turnDegrees > maxTurnDegrees)
        limitedTurnDegrees = maxTurnDegrees;
    else
        limitedTurnDegrees = turnDegrees;
    return limitedTurnDegrees;
}
