#ifndef REL_MICQUIZISHIDLL_OBJECT_WORK_H
#define REL_MICQUIZISHIDLL_OBJECT_WORK_H

#include "dolphin/types.h"
#include "humath.h"

typedef struct MicquizObjectWork_s {
    s32 initialFrames;
    s32 remainingFrames;
    f32 animationSpeed;
    HuVecF savedPosition;
    HuVecF position;
    HuVecF previousPosition;
    HuVecF rotation;
    s32 motionType;
    s32 currentMotion;
    s32 state;
    s32 objectIndex;
    s16 modelId;
    s16 motionIds[4];
} MicquizObjectWork;

#endif
