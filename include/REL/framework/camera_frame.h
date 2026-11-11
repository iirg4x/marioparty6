#ifndef REL_FRAMEWORK_CAMERA_FRAME_H
#define REL_FRAMEWORK_CAMERA_FRAME_H
#include "dolphin/mtx.h"
#include "dolphin/types.h"

// Stores the eye position, up direction, and look target used to orient a minigame camera.
struct CameraFrame {
    Vec position; // Camera eye position in world space.
    Vec up;       // Camera's up direction.
    Vec target;   // World-space point the camera looks toward.
};

// A camera pose stores the eye position and its orientation independently of the look target.
struct CameraPose {
    Vec position;        // World-space camera eye position.
    Quaternion rotation; // Camera orientation.
};

// camera.cpp
void cameraRotateAroundRightAndWorldYAxis(CameraFrame *, float, float);
void cameraTiltToAngle(CameraFrame *, float);
void cameraRollAroundViewAxis(CameraFrame *, float);
void cameraRebuildUpVector(CameraFrame *);
void cameraExtractBasis(CameraFrame *, Vec *, Vec *, Vec *);
void cameraInverseLookAt(const CameraFrame *, Mtx);
void cameraTranslateByLocalOffset(CameraFrame *, Vec *);
void cameraTranslate(CameraFrame *, Vec *);
void cameraSetDefaultFrame(CameraFrame *);
void cameraBlendEyePosition(CameraFrame *, const Vec *, const Vec *, float);
void cameraApplyToHud(CameraFrame *, s32);
void cameraReadFromHud(CameraFrame *, s32);
void cameraApplyToShadow(CameraFrame *);

// camera_ut1.cpp
void set_camera_frame_from_pose(CameraFrame *, const CameraPose *);
void get_camera_pose_from_frame(const CameraFrame *, CameraPose *);
void interpolate_camera_poses(CameraFrame *, const CameraPose *, const CameraPose *, float);
void interpolate_camera_frames(CameraFrame *, const CameraFrame *, const CameraFrame *, float);
#endif
