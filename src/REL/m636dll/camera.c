/* Opening and play-view camera motion control for Pixel Perfect. */
#include "dolphin/math.h"
#include "REL/m636dll.h"

#define PIXEL_CAMERA_MOTION_LIST_OFFSET 16
#define PIXEL_CAMERA_MODEL_OFFSET 20
#define PIXEL_CAMERA_FRAME_OFFSET 24
#define PIXEL_CAMERA_LAST_FRAME_OFFSET 28

/* The camera object's update callback records the active camera motion frame each object-manager
 * tick. */
void fn_1_6360(OMOBJ *object) {
    (*(f32 *) ((s8 *) (&lbl_1_bss_0) + (PIXEL_CAMERA_FRAME_OFFSET))) =
        Hu3DMotionTimeGet((*(s16 *) ((s8 *) (&lbl_1_bss_0) + (PIXEL_CAMERA_MODEL_OFFSET))));
}

/* The fade-in sequence calls this at reveal start and after the picture introduction to switch
 * between the opening and play-view cameras. */
void fn_1_6398(s32 motionIndex)
{
    s16 cameraModel;

    cameraModel = Hu3DModelCameraCreate(
        (*(s16 *) ((s8 *) &lbl_1_bss_0 + (motionIndex * 2) + (PIXEL_CAMERA_MOTION_LIST_OFFSET))),
        HU3D_CAM0);
    Hu3DCameraMotionStart(cameraModel, HU3D_CAM0);
    (*(f32 *)((s8 *)&lbl_1_bss_0 + (PIXEL_CAMERA_FRAME_OFFSET))) = (0.0);
    (*(f32 *) ((s8 *) &lbl_1_bss_0 + (PIXEL_CAMERA_LAST_FRAME_OFFSET))) =
        Hu3DMotionMaxTimeGet(cameraModel);
    /* Create the new motion-driven camera before releasing the previous one. */
    if ((*(s16 *)((s8 *)&lbl_1_bss_0 + (PIXEL_CAMERA_MODEL_OFFSET))) != -1) {
        Hu3DModelKill((*(s16 *)((s8 *)&lbl_1_bss_0 + (PIXEL_CAMERA_MODEL_OFFSET))));
    }
    (*(s16 *)((s8 *)&lbl_1_bss_0 + (PIXEL_CAMERA_MODEL_OFFSET))) = cameraModel;
}

/* Checks whether the active camera motion reached its playback endpoint, but discards the
 * result. */
void fn_1_644C(void) {
    Hu3DMotionEndCheck((*(s16 *)((s8 *)(&lbl_1_bss_0) + (PIXEL_CAMERA_MODEL_OFFSET))));
}
