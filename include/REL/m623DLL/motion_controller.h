#ifndef REL_M623DLL_MOTION_CONTROLLER_H
#define REL_M623DLL_MOTION_CONTROLLER_H
#include "dolphin/mtx.h"

// Holds the analog input and the targets used for computer steering.
struct MotionController {
    MotionController();
    ~MotionController();
    float horizontalInput;
    // The game's code never reads this field.
    float unk04;
    float verticalInput;
    float smoothedHorizontalInput;
    // The game's code never reads this field.
    float unk10;
    float smoothedVerticalInput;
    Vec targetPosition;
    Vec spinyPosition;
    unsigned char hasTarget;
    unsigned char hasSpinyTarget;
    int framesRemaining;
    float randomVerticalInput;
    float randomHorizontalInput;
};

#endif
