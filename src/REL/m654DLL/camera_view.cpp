/* Points the shadow camera with a view's three vectors. The set and get beside it come from camera.cpp. */
extern "C" {
#include "dolphin/mtx.h"
#include "game/hu3d.h"
}

#include "REL/m654DLL/camera_types.h"

void cameraApplyToShadow(CameraFrame *frame)
{
    Hu3DShadowPosSet(&frame->eye, &frame->up, &frame->target);
}
