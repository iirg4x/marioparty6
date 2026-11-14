// Compares degree angles across the wrap point and optionally limits the turn.
#include "REL/framework/misc_diff_deg.h"

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
