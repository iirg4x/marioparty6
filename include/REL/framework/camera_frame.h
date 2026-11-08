#ifndef REL_FRAMEWORK_CAMERA_FRAME_H
#define REL_FRAMEWORK_CAMERA_FRAME_H
#include "dolphin/mtx.h"
#include "dolphin/types.h"

struct CameraFrame {
    Vec vectors[3];
};
void cameraApplyToHud(CameraFrame *, s32);
#endif
