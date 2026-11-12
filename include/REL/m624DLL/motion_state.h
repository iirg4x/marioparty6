#ifndef REL_M624DLL_MOTION_STATE_H
#define REL_M624DLL_MOTION_STATE_H

#include "REL/framework/frame_interpolator.h"

typedef struct MotionState {
    u8 unknown00[0x20]; /* No code names this field. */
    FrameInterpolator<float> transition;
    u8 unk_34[0x14]; /* No code names this field. */
    u8 active;
} MotionState;

#endif
