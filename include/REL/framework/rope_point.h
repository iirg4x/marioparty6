#ifndef REL_FRAMEWORK_ROPE_POINT_H
#define REL_FRAMEWORK_ROPE_POINT_H

#include "dolphin/mtx.h"

// Stores the current and previous positions of a rope point and its damped displacement.
struct RopePoint {
    Vec position;
    Vec previousPosition;
    Vec displacement;

    void fn_1_1DE00();
    void update();
};

#endif
