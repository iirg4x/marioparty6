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

#endif
