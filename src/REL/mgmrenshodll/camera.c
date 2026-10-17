/* Sets up and selects the camera animations for this minigame mode. */
#include "dolphin/math.h"
#include "game/object.h"
#include "game/data.h"
#include "game/memory.h"
#include "datadir_enum.h"

HU3D_MOTIONID lbl_1_bss_B0[4];
HU3D_MODELID lbl_1_bss_A8[4];
s32 lbl_1_data_0[4] = {
    DATANUM(DATA_mgmrensho, 22),
    DATANUM(DATA_mgmrensho, 23),
    DATANUM(DATA_mgmrensho, 23),
    DATANUM(DATA_mgmrensho, 24)
};

/* Called during mode setup to load the four camera views used by Endurance Alley. */
void fn_1_E9C(void) {
    s16 cameraIndex;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 20.0f, 10.0f, 5000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    cameraIndex = 0;
    while (cameraIndex < 4) {
        lbl_1_bss_B0[cameraIndex] = Hu3DMotionCreate(
            HuDataSelHeapReadNum(lbl_1_data_0[cameraIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_A8[cameraIndex] = Hu3DModelCameraCreate(lbl_1_bss_B0[cameraIndex], 1U);
        Hu3DCameraMotionOff(lbl_1_bss_A8[cameraIndex]);
        cameraIndex += 1;
    }
    Hu3DCameraMotionOn(lbl_1_bss_A8[0], 1U);
    Hu3DModelAttrSet(lbl_1_bss_A8[0], (HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                                       HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE));
}

void fn_1_1028(void)
{
}

/* Reaction states 10 and 16 reverse this sight direction to turn the character toward the
 * camera. */
void fn_1_102C(Vec *difference) {
    PSVECSubtract(&Hu3DCamera[0].target, &Hu3DCamera[0].pos, difference);
}

/* Selects a camera view; cameras other than 2 play once to the end, while camera 2 is held at its
 * final frame. */
void fn_1_106C(s16 selectedCamera) {
    f32 motionEnd;
    s16 cameraIndex;

    if ((selectedCamera < 0) || (selectedCamera >= 4)) {
        return;
    }
    cameraIndex = 0;
    while (cameraIndex < 4) {
        Hu3DCameraMotionOff(lbl_1_bss_A8[cameraIndex]);
        cameraIndex += 1;
    }
    if (selectedCamera != 2) {
        Hu3DModelAttrReset(lbl_1_bss_A8[selectedCamera],
                           (HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                            HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE));
        Hu3DCameraMotionStart(lbl_1_bss_A8[selectedCamera], 1U);
        do {
            motionEnd = Hu3DMotionMaxTimeGet(lbl_1_bss_A8[selectedCamera]);
            if (Hu3DMotionTimeGet(lbl_1_bss_A8[selectedCamera]) >= motionEnd) {
                break;
            }
            HuPrcVSleep();
        } while (1);
        Hu3DCameraMotionOff(lbl_1_bss_A8[selectedCamera]);
        return;
    }
    Hu3DCameraMotionOn(lbl_1_bss_A8[selectedCamera], 1U);
    Hu3DModelAttrSet(lbl_1_bss_A8[selectedCamera],
                     (HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE | HU3D_MOTATTR_SHAPE_PAUSE |
                      HU3D_MOTATTR_OVL_PAUSE));
    /* Camera 2 is held at the end of its motion for the next mode step. */
    Hu3DMotionTimeSet(lbl_1_bss_A8[selectedCamera],
                      Hu3DMotionMaxTimeGet(lbl_1_bss_A8[selectedCamera]));
    HuPrcVSleep();
    Hu3DCameraMotionOff(lbl_1_bss_A8[selectedCamera]);
}
