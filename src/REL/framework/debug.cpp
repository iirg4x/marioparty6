// Debug camera frame helpers and controller-stick input scaling for minigames.
static const char rcsid[] = "$Id: debug.cpp,v 1.13.2.2 2004/04/16 11:22:07 shohyama Exp $";

extern "C" {
#include "dolphin/mtx.h"
#include "game/pad.h"
}

// Three components of a camera position or direction, ordered X, Y, Z.
struct CameraVector {
    float coordinates[3]; // X, Y, and Z components.
};

// Describes the eye, up direction, and look target of the debug camera.
struct CameraFrame {
    CameraVector eye;    // Camera position in world space.
    CameraVector up;     // Direction above the camera.
    CameraVector target; // World-space point the camera looks toward.
};

// Holds a camera's eye position and orientation for a pose query.
struct CameraPose {
    Vec position;        // World-space camera eye position.
    Quaternion rotation; // Camera orientation.
};
void get_camera_pose_from_frame(const CameraFrame *, CameraPose *);

// A debug camera pose query converts the frame to a pose, then discards the result.
void copyDebugCameraPose(const CameraFrame *frame)
{
    CameraPose pose;
    get_camera_pose_from_frame(frame, &pose);
}

// Debug controls read the main stick's horizontal deflection, scaled by 56 raw units.
float getDebugStickX(int padIndex) { return (1.0f / 56.0f) * HuPadStkX[padIndex]; }
// Debug controls read the main stick's vertical deflection, scaled by 56 raw units.
float getDebugStickY(int padIndex) { return (1.0f / 56.0f) * HuPadStkY[padIndex]; }
// Debug controls read the sub-stick's horizontal deflection, scaled by 44 raw units.
float getDebugSubStickX(int padIndex) { return (1.0f / 44.0f) * HuPadSubStkX[padIndex]; }
// Debug controls read the sub-stick's vertical deflection, scaled by 44 raw units.
float getDebugSubStickY(int padIndex) { return (1.0f / 44.0f) * HuPadSubStkY[padIndex]; }

// Debug event processing can call this hook; it performs no work.
void debugEventHook() {}

// Combines the debug camera frame with its packed flags.
struct CameraSetup {
    CameraFrame frame; // Eye, up direction, and target used by the debug camera.
    unsigned char highFlag : 1, // Highest flag bit; initialized to zero.
                  middleFlag : 1, // Next flag bit; initialized to zero.
                  lowFlag : 1, // Last single-bit flag; initialized to one.
                  remainingFlagBits : 5; // The constructor leaves these bits unchanged.
    CameraSetup();
};
void cameraSetDefaultFrame(CameraFrame *);

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
void debugResetHook() {}

// Replacing a debug camera frame copies its eye, up direction, and target from another frame.
void copyDebugCameraFrame(CameraFrame *destination, const CameraFrame *source)
{
    CameraFrame *frame = destination;
    frame->eye = source->eye;
    frame->up = source->up;
    frame->target = source->target;
}
