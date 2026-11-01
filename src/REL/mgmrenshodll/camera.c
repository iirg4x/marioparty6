/* Sets up and selects the camera animations for this minigame mode. */
#include "dolphin/math.h"
#include "game/object.h"
#include "game/data.h"
#include "game/memory.h"
#include "datadir_enum.h"

HU3D_MOTIONID lbl_1_bss_B0[4];
HU3D_MODELID lbl_1_bss_A8[4];
s32 lbl_1_data_0[4] = {
    DATANUM(0xa00000, 22),
    DATANUM(0xa00000, 23),
    DATANUM(0xa00000, 23),
    DATANUM(0xa00000, 24)
};

/* Called during mode setup to load the four camera views used by Endurance Alley. */
void fn_1_E9C(void) {
    s16 cameraIndex;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 20.0f, 10.0f, 5000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    cameraIndex = 0;
    while (cameraIndex < 4) {
        lbl_1_bss_B0[cameraIndex] = fn_8002C4C0(
            fn_80008DD0(lbl_1_data_0[cameraIndex], 0x10000000, 2));
        lbl_1_bss_A8[cameraIndex] = fn_800283F4(lbl_1_bss_B0[cameraIndex], 1U);
        fn_8002858C(lbl_1_bss_A8[cameraIndex]);
        cameraIndex += 1;
    }
    fn_80028478(lbl_1_bss_A8[0], 1U);
    fn_800269E8(lbl_1_bss_A8[0], (((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                                       ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10))));
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
        fn_8002858C(lbl_1_bss_A8[cameraIndex]);
        cameraIndex += 1;
    }
    if (selectedCamera != 2) {
        fn_80026A44(lbl_1_bss_A8[selectedCamera],
                           (((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                            ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10))));
        fn_800284C8(lbl_1_bss_A8[selectedCamera], 1U);
        do {
            motionEnd = fn_8002D530(lbl_1_bss_A8[selectedCamera]);
            if (fn_8002D4E0(lbl_1_bss_A8[selectedCamera]) >= motionEnd) {
                break;
            }
            HuPrcVSleep();
        } while (1);
        fn_8002858C(lbl_1_bss_A8[selectedCamera]);
        return;
    }
    fn_80028478(lbl_1_bss_A8[selectedCamera], 1U);
    fn_800269E8(lbl_1_bss_A8[selectedCamera],
                     (((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) | ((1 << 30) | (1 << 7)) |
                      ((1 << 30) | (1 << 10))));
    /* Camera 2 is held at the end of its motion for the next mode step. */
    fn_8002D414(lbl_1_bss_A8[selectedCamera],
                      fn_8002D530(lbl_1_bss_A8[selectedCamera]));
    HuPrcVSleep();
    fn_8002858C(lbl_1_bss_A8[selectedCamera]);
}
