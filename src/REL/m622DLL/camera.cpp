#include "REL/m622DLL/camera_frame.h"

extern "C" {
#include "game/hu3d.h"
}

void CameraFrame::fn_1_CFC0()
{
    Hu3DShadowPosSet(&eye, &up, &target);
}
