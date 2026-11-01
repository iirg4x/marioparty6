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
        fn_8002D4E0((*(s16 *) ((s8 *) (&lbl_1_bss_0) + (PIXEL_CAMERA_MODEL_OFFSET))));
}

/* The fade-in sequence calls this at reveal start and after the picture introduction to switch
 * between the opening and play-view cameras. */
void fn_1_6398(s32 motionIndex)
{
    s16 cameraModel;

    cameraModel = fn_800283F4(
        (*(s16 *) ((s8 *) &lbl_1_bss_0 + (motionIndex * 2) + (PIXEL_CAMERA_MOTION_LIST_OFFSET))),
        (1 << 0));
    fn_800284C8(cameraModel, (1 << 0));
    (*(f32 *)((s8 *)&lbl_1_bss_0 + (PIXEL_CAMERA_FRAME_OFFSET))) = (0.0);
    (*(f32 *) ((s8 *) &lbl_1_bss_0 + (PIXEL_CAMERA_LAST_FRAME_OFFSET))) =
        fn_8002D530(cameraModel);
    /* Create the new motion-driven camera before releasing the previous one. */
    if ((*(s16 *)((s8 *)&lbl_1_bss_0 + (PIXEL_CAMERA_MODEL_OFFSET))) != -1) {
        fn_800262D0((*(s16 *)((s8 *)&lbl_1_bss_0 + (PIXEL_CAMERA_MODEL_OFFSET))));
    }
    (*(s16 *)((s8 *)&lbl_1_bss_0 + (PIXEL_CAMERA_MODEL_OFFSET))) = cameraModel;
}

/* Checks whether the active camera motion reached its playback endpoint, but discards the
 * result. */
void fn_1_644C(void) {
    fn_8002D75C((*(s16 *)((s8 *)(&lbl_1_bss_0) + (PIXEL_CAMERA_MODEL_OFFSET))));
}
