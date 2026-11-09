// Camera frame helpers used by minigame camera controls.

extern "C" {
#include "dolphin/mtx.h"
#include "game/hu3d.h"
}


// Stores the eye, up direction, and look target used to orient a minigame camera.
struct CameraView {
    Vec pos;    // Camera position in world space.
    Vec up;     // Camera's up direction.
    Vec target; // World-space point the camera looks toward.
    void fn_1_259E4(int);
    void fn_1_25A24(int);
    void fn_1_25A64();
};

void CameraView::fn_1_25A64()
{
    Hu3DShadowPosSet(&pos, &up, &target);
}
