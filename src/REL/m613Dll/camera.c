/* Creates the game camera and switches it to the result view. */
#include "REL/minigame_camera.h"
#include "REL/m613Dll/types.h"
#include <math.h>
#include "game/hu3d.h"

MinigameCameraParams lbl_1_data_178 = {
    1, 15.0f, 10.0f, 10000.0f, 1.2f,
    0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f,
    {0.0f,0.0f,1000.0f}, {0.0f,1.0f,0.0f}, {0.0f,0.0f,0.0f}
};
MinigameCameraPose lbl_1_data_1C8 = {{0.0f,3300.0f,3712.5f}, {0.0f,1.0f,0.0f}, {0.0f,0.0f,0.0f}};
MinigameCameraPose lbl_1_data_1EC = {{0.0f,430.0f,900.0f}, {0.0f,1.0f,0.0f}, {0.0f,180.0f,-110.0f}};

/* Called when the opening phase starts to create and aim the board camera. */
void fn_1_F048(void)
{
    s32 scissorY;
    s32 scissorWidth;
    s32 scissorHeight;

    Hu3DCameraCreate(lbl_1_data_178.cameraBit);
    Hu3DCameraViewportSet(lbl_1_data_178.cameraBit, lbl_1_data_178.viewportX,
                          lbl_1_data_178.viewportY, lbl_1_data_178.viewportWidth,
                          lbl_1_data_178.viewportHeight, lbl_1_data_178.minDepth,
                          lbl_1_data_178.maxDepth);
    Hu3DCameraPerspectiveSet(lbl_1_data_178.cameraBit, lbl_1_data_178.fieldOfView,
                             lbl_1_data_178.nearPlane, lbl_1_data_178.farPlane,
                             lbl_1_data_178.aspectRatio);
    scissorHeight = (u32) lbl_1_data_178.viewportHeight;
    scissorWidth = (u32) lbl_1_data_178.viewportWidth;
    scissorY = (u32) lbl_1_data_178.viewportY;
    Hu3DCameraScissorSet(lbl_1_data_178.cameraBit,
                         (u32) lbl_1_data_178.viewportX, scissorY, scissorWidth,
                         scissorHeight);
    lbl_1_data_178.position = lbl_1_data_1C8.position;
    lbl_1_data_178.up = lbl_1_data_1C8.up;
    lbl_1_data_178.target = lbl_1_data_1C8.target;
    Hu3DCameraPosSetV(lbl_1_data_178.cameraBit, &lbl_1_data_178.position,
                      &lbl_1_data_178.up, &lbl_1_data_178.target);
}

/* Returns whether the intro frame is at least 230; the phase-relative frame values are
 * discarded. */
u8 fn_1_F23C(s16 frame) {
    s16 frameWithinRange;

    if (frame < 60) {
        frameWithinRange = frame;
    } else if (frame < 180) {
        frameWithinRange = frame - 60;
    } else if (frame < 230) {
        frameWithinRange = frame - 180;
    } else {
        return 1;
    }
    return 0;
}

/* Called by gameplay and result callbacks; this phase keeps the current camera pose. */
void fn_1_F2A0(void) {
}

/* Moves the camera to its closer result pose and sets the field of view to 40 degrees. */
void fn_1_F2A4(void)
{
    lbl_1_data_178.position = lbl_1_data_1EC.position;
    lbl_1_data_178.up = lbl_1_data_1EC.up;
    lbl_1_data_178.target = lbl_1_data_1EC.target;
    lbl_1_data_178.fieldOfView = 40.0f;
    Hu3DCameraPerspectiveSet(lbl_1_data_178.cameraBit, lbl_1_data_178.fieldOfView,
                             lbl_1_data_178.nearPlane, lbl_1_data_178.farPlane,
                             lbl_1_data_178.aspectRatio);
    Hu3DCameraPosSetV(lbl_1_data_178.cameraBit, &lbl_1_data_178.position,
                      &lbl_1_data_178.up, &lbl_1_data_178.target);
}
