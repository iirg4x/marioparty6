#ifndef GAME_MATRIX_ANGLE_H
#define GAME_MATRIX_ANGLE_H

#include <math.h>
#include "humath.h"

/* Returns the direction from +Y toward (x, y), in radians. */
static inline float HuAngleXY(float x, float y)
{
    if (y == 0.0f) {
        if (x >= 0.0f) return M_PI / 2;
        else return -(M_PI / 2);
    }
    return atan2f(x, y);
}

/* Clamps matrix roundoff to the inverse-sine endpoints. */
static inline f32 ClampedAsin(f32 value)
{
    if (value >= 1.0f) return (1.5707964f);
    if (value <= (-1.0f)) return (-1.5707964f);
    return asinf(value);
}

/* Returns +/-pi/2 when X is zero, including +pi/2 for (0, 0). */
static inline f32 GuardedAtan2(f32 y, f32 x)
{
    if (0.0f == x) {
        if (y >= 0.0f) return (1.5707964f);
        return (-1.5707964f);
    }
    return atan2f(y, x);
}

#endif
