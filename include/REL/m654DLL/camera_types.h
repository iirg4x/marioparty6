#ifndef M654_CAMERA_TYPES_H
#define M654_CAMERA_TYPES_H
#include "dolphin/mtx.h"
struct CameraFrame { Vec eye, up, target; };
struct CameraPose { Vec position; Quaternion rotation; };
void set_camera_frame_from_pose(CameraFrame *, const CameraPose *);
void get_camera_pose_from_frame(const CameraFrame *, CameraPose *);
void interpolate_camera_poses(CameraFrame *, const CameraPose *, const CameraPose *, float);
void cameraApplyToHud(CameraFrame *, s32);
void cameraApplyToShadow(CameraFrame *);
#endif
