#include "REL/framework/debug.h"
// Debug camera frame helpers and controller-stick input scaling for minigames.
static const char rcsid[] = "$Id: debug.cpp,v 1.13.2.2 2004/04/16 11:22:07 shohyama Exp $";

extern "C" {
#include "dolphin/mtx.h"
#include "game/pad.h"
}

#include "REL/framework/camera_frame.h"

// A debug camera pose query converts the frame to a pose, then discards the result.
void copyDebugCameraPose(const CameraFrame *frame)
{
    CameraPose pose;
    get_camera_pose_from_frame(frame, &pose);
}

// Debug controls read the main stick's horizontal deflection, scaled by 56 raw units.
float getDebugStickX(int padIndex) { return (1.0f / 56.0f) * lbl_802BFFFC[padIndex]; }
// Debug controls read the main stick's vertical deflection, scaled by 56 raw units.
float getDebugStickY(int padIndex) { return (1.0f / 56.0f) * lbl_802BFFF8[padIndex]; }
// Debug controls read the sub-stick's horizontal deflection, scaled by 44 raw units.
float getDebugSubStickX(int padIndex) { return (1.0f / 44.0f) * lbl_802BFFF4[padIndex]; }
// Debug controls read the sub-stick's vertical deflection, scaled by 44 raw units.
float getDebugSubStickY(int padIndex) { return (1.0f / 44.0f) * lbl_802BFFF0[padIndex]; }

// Debug event processing can call this hook; it performs no work.
void debugEventHook(void *, int) {}

// Constructing a debug camera setup sets its flags and the default frame at Z = 5000.
CameraSetup::CameraSetup()
{
    highFlag = 0;
    middleFlag = 0;
    lowFlag = 1;
    // The remaining flag bits are untouched; the default frame looks at the origin with Y up.
    cameraSetDefaultFrame(&frame);
}

// Debug reset processing can call this hook; it performs no work.
void debugResetHook(CameraFrame *, s32, s32) {}

// Replacing a debug camera frame copies its eye, up direction, and target from another frame.
void copyDebugCameraFrame(CameraFrame *destination, const CameraFrame *source)
{
    CameraFrame *frame = destination;
    *frame = *source;
}
