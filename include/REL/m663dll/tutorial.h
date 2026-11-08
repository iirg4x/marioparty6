#ifndef REL_M663DLL_TUTORIAL_H
#define REL_M663DLL_TUTORIAL_H

#include "dolphin/types.h"

typedef struct M663TutorialWork {
    s16 phase;
    s16 motionTimer;
    s16 firstMotion;
    s16 motionSlot;
    s16 actorSlot;
    char *objectName;
    s16 updateTimer;
    f32 startAngle;
    f32 targetAngle;
    s16 delayTimer;
    s16 angleIndex;
} M663TutorialWork;

extern M663TutorialWork lbl_1_bss_378[5];

#endif
