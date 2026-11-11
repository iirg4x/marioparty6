#ifndef REL_FRAMEWORK_DEBUG_H
#define REL_FRAMEWORK_DEBUG_H
#include "dolphin/types.h"
#include "REL/framework/camera_frame.h"

// Combines the debug camera frame with its packed flags.
struct CameraSetup {
    CameraFrame frame; // Eye, up direction, and target used by the debug camera.
    unsigned char highFlag : 1, // Highest flag bit; initialized to zero.
                  middleFlag : 1, // Next flag bit; initialized to zero.
                  lowFlag : 1, // Last single-bit flag; initialized to one.
                  remainingFlagBits : 5; // The constructor leaves these bits unchanged.
    CameraSetup();
};

void copyDebugCameraPose(const CameraFrame *);
float getDebugStickX(int);
float getDebugStickY(int);
float getDebugSubStickX(int);
float getDebugSubStickY(int);
void debugEventHook(void *, int);
void debugResetHook(CameraFrame *, s32, s32);
void copyDebugCameraFrame(CameraFrame *, const CameraFrame *);
#endif
