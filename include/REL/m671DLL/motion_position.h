#ifndef REL_M671DLL_MOTION_POSITION_H
#define REL_M671DLL_MOTION_POSITION_H

#include "dolphin/mtx.h"

struct MotionPosition {
    Vec value;
    MotionPosition() {}
    MotionPosition(const Vec &source) : value(source) {}
};

#endif
