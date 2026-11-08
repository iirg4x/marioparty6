#ifndef REL_M648_CAMERA_VIEW_H
#define REL_M648_CAMERA_VIEW_H
#include "REL/m648DLL/camera_frame.h"
typedef CameraFrame CameraView;
void cameraApplyToHud(CameraFrame *, s32);
void cameraReadFromHud(CameraFrame *, s32);
#endif
