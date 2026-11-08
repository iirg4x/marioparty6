#include "REL/framework/debug.h"
#ifndef REL_M622_CAMERA_FRAME_H
#define REL_M622_CAMERA_FRAME_H
#include "dolphin/mtx.h"

struct CameraFrame {
    Vec eye, up, target;
    void fn_1_CFC0();
};
struct CameraPose { Vec eye; Quaternion rotation; };
struct CameraSetup {
    CameraFrame frame; // Eye, up direction, and target used by the debug camera.
    unsigned char highFlag : 1, // Highest flag bit; initialized to zero.
                  middleFlag : 1, // Next flag bit; initialized to zero.
                  lowFlag : 1, // Last single-bit flag; initialized to one.
                  remainingFlagBits : 5; // The constructor leaves these bits unchanged.
    CameraSetup();
};
struct CameraPositionState;

void cameraRotateAroundRightAndWorldYAxis(CameraFrame *, float, float);
void cameraTiltToAngle(CameraFrame *, float);
void cameraRollAroundViewAxis(CameraFrame *, float);
void cameraRebuildUpVector(CameraFrame *);
void cameraExtractBasis(CameraFrame *, Vec *, Vec *, Vec *);
void cameraInverseLookAt(const CameraFrame *, Mtx);
void cameraTranslateByLocalOffset(CameraFrame *, Vec *);
void cameraTranslate(CameraFrame *, Vec *);
void cameraSetDefaultFrame(CameraFrame *);
void cameraUpdateTrackedPosition(CameraPositionState *, Vec *);
void cameraBlendEyePosition(CameraFrame *, const Vec *, const Vec *, float);
void cameraApplyToHud(CameraFrame *, s32);
void cameraReadFromHud(CameraFrame *, s32);
void set_camera_frame_from_pose(CameraFrame *, const CameraPose *);
void get_camera_pose_from_frame(const CameraFrame *, CameraPose *);
void interpolate_camera_poses(CameraFrame *, const CameraPose *, const CameraPose *, float);
void interpolate_camera_frames(CameraFrame *, const CameraFrame *, const CameraFrame *, float);
void copyDebugCameraPose(const CameraFrame *);
float getDebugStickX(int);
float getDebugStickY(int);
float getDebugSubStickX(int);
float getDebugSubStickY(int);
void debugEventHook();
void copyDebugCameraFrame(CameraFrame *, const CameraFrame *);
#endif
