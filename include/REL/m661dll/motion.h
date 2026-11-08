#ifndef REL_M661DLL_MOTION_H
#define REL_M661DLL_MOTION_H

#include "dolphin/mtx.h"

typedef struct MotionObject {
    s16 modelId;
    s16 state;
    f32 x;
    f32 unknown08;
    f32 z;
    /* ModelSetupRecord in object_setup.c and player_paths.c views this same
     * array with shadowPosition at 0x10 and rotation at 0x1C. HuVecF is Vec. */
    Vec shadowPosition;
    f32 rotationX;
    f32 angle;
    u8 unknown24[4]; /* No code names this field. */
} MotionObject;

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
