extern "C" {
#include "game/hu3d.h"
}

/* Position, up direction, and target of the shadow camera. */
class CameraView {
public:
    Vec pos, up, target;
    void fn_1_25A64();
};

void CameraView::fn_1_25A64()
{
    Hu3DShadowPosSet(&pos, &up, &target);
}
