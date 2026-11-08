extern "C" {
#include "game/hu3d.h"
}

#include "REL/m648DLL/camera_view.h"

void CameraFrame::fn_1_25A64()
{
    Hu3DShadowPosSet(&position, &up, &target);
}
