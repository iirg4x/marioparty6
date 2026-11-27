#ifndef REL_M661DLL_MOTION_H
#define REL_M661DLL_MOTION_H

#include "dolphin/mtx.h"

typedef struct MotionControl {
    s16 active;
    s16 mode;
    u8 unknown04[2]; /* No code names this field. */
    s16 directionMode;
    s16 motionIds[2];
    s16 animationState;
    s16 previousAnimationState;
    s8 inputDirection[2];
    s8 inputLimit[2];
    Point3d input[2];
    f32 inputMagnitude[2];
    f32 radius;
    s16 waitFrames;
    f32 movementSpeed;
    f32 angle;
    f32 endSpeed;
    s16 controlA;
    s16 pieceContact[2];
    s16 randomizeAngle;
} MotionControl;
void fn_1_A6C0(void);

#endif
