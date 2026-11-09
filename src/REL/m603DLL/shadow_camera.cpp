#include "dolphin/mtx.h"

extern "C" {
#include "game/hu3d.h"
}

struct CameraView {
    Vec pos;
    Vec up;
    Vec target;

    void fn_1_25A64();
};

void CameraView::fn_1_25A64()
{
    Hu3DShadowPosSet(&pos, &up, &target);
}
