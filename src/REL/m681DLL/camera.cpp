// The shadow-camera setter this minigame adds after the shared camera.cpp block.
extern "C" {
#include "dolphin/mtx.h"
#include "game/hu3d.h"
}

// Stores the eye, up direction, and look target used to orient a minigame camera.
struct CameraView {
    void fn_1_25A64();
    Vec eye;    // Camera position in world space.
    Vec up;     // Camera's up direction.
    Vec target; // World-space point the camera looks toward.
};

void CameraView::fn_1_25A64()
{
    Hu3DShadowPosSet(&eye, &up, &target);
}
