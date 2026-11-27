#ifndef REL_W02DLL_EVENT_MODEL_H
#define REL_W02DLL_EVENT_MODEL_H

#include "humath.h"

/* Model and motion parameters for a board event's flying model. */
typedef struct W02EventModelEntry {
    s32 modelId;
    s32 unk4; /* The game's code never reads this field. */
    s32 flightStage;
    s32 age;
    s32 travelFrames;
    float jumpHeight;
    float rotationSpeed;
    HuVecF start;
    HuVecF end;
    HuVecF bend;
    HuVecF rotation;
} W02EventModelEntry;

#endif
