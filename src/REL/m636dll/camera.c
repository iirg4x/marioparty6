/* Opening and play-view camera motion control for Pixel Perfect. */
#include <math.h>
#include "REL/m636dll.h"

/* The camera object's update callback records the active camera motion frame each object-manager
 * tick. */
void fn_1_6360(OMOBJ *object) {
    lbl_1_bss_0.cameraFrame = fn_8002D4E0(lbl_1_bss_0.cameraModel);
}

/* The fade-in sequence calls this at reveal start and after the picture introduction to switch
 * between the opening and play-view cameras. */
void fn_1_6398(s32 motionIndex)
{
    s16 cameraModel;

    cameraModel = fn_800283F4(lbl_1_bss_0.cameraMotions[motionIndex], (1 << 0));
    fn_800284C8(cameraModel, (1 << 0));
    lbl_1_bss_0.cameraFrame = 0.0;
    lbl_1_bss_0.cameraLastFrame = fn_8002D530(cameraModel);
    /* Create the new motion-driven camera before releasing the previous one. */
    if (lbl_1_bss_0.cameraModel != -1) {
        fn_800262D0(lbl_1_bss_0.cameraModel);
    }
    lbl_1_bss_0.cameraModel = cameraModel;
}

/* Checks whether the active camera motion reached its playback endpoint, but discards the
 * result. */
void fn_1_644C(void) {
    fn_8002D75C(lbl_1_bss_0.cameraModel);
}
