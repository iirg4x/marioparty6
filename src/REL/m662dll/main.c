/* Trick or Tree scene setup, input, motions, lighting, particles and positional audio. */
#include <math.h>
#include "humath.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "REL/m662dll/game.h"
#include <string.h>
#include "game/printfunc.h"
#include "msm_stream.h"

#define MSM_SE_M662_INTRO 2118
#define MSM_SE_M662_PATH_MOVE 2119
#define MSM_SE_M662_MODEL_MOTION 2120
#define MSM_SE_M662_CUE 2121

extern s16 lbl_802C0278;
extern void fn_1_1DC8(s16);
extern void fn_1_1DFC(void), fn_1_223C(void), fn_1_8B34(void), fn_1_4D04(void);
extern s32 fn_1_8B8C(void), fn_1_8BCC(void), fn_1_57C8(void), fn_1_A430(void);
extern s32 fn_1_A4B0(void), fn_1_A758(void), fn_1_A780(void);
void fn_1_1918(s16);
void fn_1_A0(void);
void fn_1_130(s16, s16);
void fn_1_260(s16, s16);
void fn_1_28C(s16, s16);
void fn_1_2B8(s16, s16);
void fn_1_2E4(s16, s16);
void fn_1_310(s16, s16);
void fn_1_33C(s16, s16);
void fn_1_368(s16, s16);
void fn_1_394(s16, s16);

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

/* Runs static constructors, then starts Trick or Tree when the module is loaded. */
int _prolog(void)
{
    const VoidFunc *constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    fn_1_A0();
    return 0;
}

/* Runs static destructors when Trick or Tree is unloaded. */
void _epilog(void)
{
    const VoidFunc *destructor = _dtors;
    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}

/* Creates the scene manager and starts the minigame phase controller at module startup. */
void fn_1_A0(void) {
    lbl_1_bss_C50[0] = fn_8003A8BC(256, 1000);
    fn_80069F1C(lbl_1_bss_C50[0]);
    fn_8003AB38(lbl_1_bss_C50[0], 32730, 0U, 0U, -1, fn_8003D4FC);
    fn_80070788(&lbl_1_data_0);
    fn_80071CE8(2, 1000);
    fn_80071CE8(7, 1000);
}

/* Initial sequence callback: clears scene state, builds the cameras and models, and prepares
 * audio. */
void fn_1_130(s16 mode, s16 frameNo) {
    s16 nightMode;

    memset(&lbl_1_bss_B08, 0, 328);
    memset(&lbl_1_bss_ACC, 0, 60);
    memset(&lbl_1_bss_A7C, 0, 80);
    memset(&lbl_1_bss_A74, 0, 8);
    memset(&lbl_1_bss_4B4, 0, 1472);
    memset(&lbl_1_bss_3F4, 0, 192);
    memset(&lbl_1_bss_2F4, 0, 256);
    memset(&lbl_1_bss_210, 0, 228);
    memset(&lbl_1_bss_D0, 0, 320);
    memset(&lbl_1_bss_0, 0, 208);
    nightMode = lbl_802C0278;
    lbl_1_bss_210.nightMode = nightMode;
    fn_1_1918(0);
    fn_1_1DFC();
    fn_1_223C();
    Hu3DBGColorSet(0, 0, 0);
    fn_1_8B34();
    fn_1_4D04();
    fn_80071A9C();
}

/* Fade-in callback: advances when the introduction updater reports completion. */
void fn_1_260(s16 mode, s16 frameNo)
{
    if (fn_1_8B8C() != 0) fn_80071A9C();
}

/* Start callback: runs the introduction update; its current updater returns zero. */
void fn_1_28C(s16 mode, s16 frameNo)
{
    if (fn_1_8BCC() != 0) fn_80071A9C();
}

/* Main callback: updates the round and advances after a winner has been selected. */
void fn_1_2B8(s16 mode, s16 frameNo)
{
    if (fn_1_57C8() != 0) fn_80071A9C();
}

/* Finish callback: runs the ending-camera update; its current updater returns zero. */
void fn_1_2E4(s16 mode, s16 frameNo)
{
    if (fn_1_A430() != 0) fn_80071A9C();
}

/* Pre-winner callback: advances when the ending-camera sequence reports completion. */
void fn_1_310(s16 mode, s16 frameNo)
{
    if (fn_1_A474() != 0) fn_80071A9C();
}

/* Winner callback: updates result motions and the ending camera. */
void fn_1_33C(s16 mode, s16 frameNo)
{
    if (fn_1_A4B0() != 0) fn_80071A9C();
}

/* Fade-out callback: keeps the ending camera and scene transforms updated. */
void fn_1_368(s16 mode, s16 frameNo)
{
    if (fn_1_A758() != 0) fn_80071A9C();
}

/* Close callback: runs the audio shutdown update; its updater currently returns zero. */
void fn_1_394(s16 mode, s16 frameNo)
{
    if (fn_1_A780() != 0) fn_80071A9C();
}

/* Empty scene callback. */
void fn_1_3C0(void)
{
}

/* During model setup, selects one player per group and creates their character models at the two
 * starting positions. */
void fn_1_3C4(void)
{
    s16 playerForGroup[2] = { -1, -1 };
    HuVecF spawnPosition[2] = {
        { -700.0f, 0.0f, 550.0f },
        { 700.0f, 0.0f, 550.0f },
    };
    HuVecF spawnRotation[2] = {
        { 0.0f, 135.0f, 0.0f },
        { 0.0f, -135.0f, 0.0f },
    };
    s16 playerIndex = 0;
    s16 group = 0;

    while (playerIndex < 4) {
        group = lbl_80265728[playerIndex].grpNo;
        if (((group == 0) || (group == 1)) &&
            (playerForGroup[group] == -1)) {
            playerForGroup[group] = playerIndex;
        }
        playerIndex++;
    }

    playerIndex = 0;
    while (playerIndex < 2) {
        lbl_1_bss_2F4[playerIndex].playerNo = playerForGroup[playerIndex];
        lbl_1_bss_2F4[playerIndex].characterIndex = playerIndex;
        lbl_1_bss_2F4[playerIndex].characterNo =
            lbl_80265728[playerForGroup[playerIndex]].charNo;
        lbl_1_bss_2F4[playerIndex].padNo =
            lbl_80265728[playerForGroup[playerIndex]].padNo;
        lbl_1_bss_2F4[playerIndex].groupNo =
            lbl_80265728[playerForGroup[playerIndex]].grpNo;
        lbl_1_bss_2F4[playerIndex].playerType =
            lbl_80265728[playerForGroup[playerIndex]].type;
        lbl_1_bss_2F4[playerIndex].comDifficulty =
            lbl_80265728[playerForGroup[playerIndex]].comDif;
        lbl_1_bss_2F4[playerIndex].position = spawnPosition[playerIndex];
        lbl_1_bss_2F4[playerIndex].rotation = spawnRotation[playerIndex];
        lbl_1_bss_2F4[playerIndex].result = -1;

        lbl_1_bss_4B4[playerIndex].modelId =
            fn_80057068(lbl_1_bss_2F4[playerIndex].characterNo, 1);
        fn_800269E8(lbl_1_bss_4B4[playerIndex].modelId, (1 << 0));
        fn_80026A44(lbl_1_bss_4B4[playerIndex].modelId, (1 << 0));
        fn_80026E44(lbl_1_bss_4B4[playerIndex].modelId);
        fn_8005DFC0(lbl_1_bss_2F4[playerIndex].characterNo, 1);
        fn_8005F77C(lbl_1_bss_2F4[playerIndex].characterNo, 0);
        playerIndex++;
    }
    fn_8005DDEC(1);
}

/* During model setup, loads six motions per character and requests the initial looping motion. */
void fn_1_808(void)
{
    u32 motionIds[6] = { CHARMOT_HSF_c000m1_300, CHARMOT_HSF_c000m1_302,
                              CHARMOT_HSF_c000m1_303, CHARMOT_HSF_c000m1_304,
                              CHARMOT_HSF_c000m1_306, CHARMOT_HSF_c000m1_351 };
    s16 playerIndex;
    s16 motionIndex;
    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        for (motionIndex = 0; motionIndex < 6; motionIndex++) {
            lbl_1_bss_3F4[playerIndex].motionIds[motionIndex] =
                fn_8005B0B8(
                    lbl_1_bss_2F4[playerIndex].characterNo,
                    motionIds[motionIndex]);
        }
        fn_8002C95C(lbl_1_bss_4B4[playerIndex].modelId,
                      lbl_1_bss_3F4[playerIndex].motionIds[0]);
        lbl_1_bss_3F4[playerIndex].requestedMotion = 0;
        lbl_1_bss_3F4[playerIndex].requestedSpeed =
            1.0f;
        lbl_1_bss_3F4[playerIndex].requestedAttributes = ((1 << 30) | (1 << 0));
    }
}

/* Before player updates, snapshots all pads and clears player input when either control flag is
 * set. */
void fn_1_984(void)
{
    s32 index = 0;
    s32 adjustedStickValue;

    while (index < 2) {
        if ((lbl_1_bss_2F4[index].playerType != 0) ||
            (lbl_1_bss_2F4[index].controlState != 0)) {
            memset(lbl_1_bss_2F4 + index, 0, 20);
        } else {
            s32 stickValue;

            lbl_1_bss_2F4[index].input.buttons = lbl_802C0010[lbl_1_bss_2F4[index].padNo];
            lbl_1_bss_2F4[index].input.buttonsDown = lbl_802C0008[lbl_1_bss_2F4[index].padNo];
            lbl_1_bss_2F4[index].input.buttonsRepeat = lbl_802C0000[lbl_1_bss_2F4[index].padNo];
            if (lbl_802BFFFC[lbl_1_bss_2F4[index].padNo] > 0) {
                adjustedStickValue = lbl_802BFFFC[
                    lbl_1_bss_2F4[index].padNo];
            } else {
                if ((lbl_802BFFFC[lbl_1_bss_2F4[index].padNo] * -1) < 10) {
                    stickValue = 0;
                } else {
                    stickValue = lbl_802BFFFC[
                        lbl_1_bss_2F4[index].padNo];
                }
                adjustedStickValue = stickValue;
            }
            /* The dead zone affects only nonpositive horizontal stick input. */
            lbl_1_bss_2F4[index].input.stickX = adjustedStickValue;
            lbl_1_bss_2F4[index].input.stickY =
                (s8)lbl_802BFFF8[lbl_1_bss_2F4[index].padNo];
            lbl_1_bss_2F4[index].input.subStickX =
                (s8)lbl_802BFFF4[lbl_1_bss_2F4[index].padNo];
            lbl_1_bss_2F4[index].input.subStickY =
                (s8)lbl_802BFFF0[lbl_1_bss_2F4[index].padNo];
            lbl_1_bss_2F4[index].input.triggerL = lbl_802BFFEC[lbl_1_bss_2F4[index].padNo];
            lbl_1_bss_2F4[index].input.triggerR = lbl_802BFFE8[lbl_1_bss_2F4[index].padNo];
            lbl_1_bss_2F4[index].input.dpad = lbl_802BFFE4[lbl_1_bss_2F4[index].padNo];
            lbl_1_bss_2F4[index].input.dpadRepeat = lbl_802BFFE0[lbl_1_bss_2F4[index].padNo];
        }
        index++;
    }

    index = 0;
    while (index < 4) {
        lbl_1_bss_210.controllers[index].buttons = lbl_802C0010[index];
        lbl_1_bss_210.controllers[index].buttonsDown = lbl_802C0008[index];
        lbl_1_bss_210.controllers[index].buttonsRepeat = lbl_802C0000[index];
        lbl_1_bss_210.controllers[index].stickX = (s8)lbl_802BFFFC[index];
        lbl_1_bss_210.controllers[index].stickY = (s8)lbl_802BFFF8[index];
        lbl_1_bss_210.controllers[index].subStickX = (s8)lbl_802BFFF4[index];
        lbl_1_bss_210.controllers[index].subStickY = (s8)lbl_802BFFF0[index];
        lbl_1_bss_210.controllers[index].triggerL = lbl_802BFFEC[index];
        lbl_1_bss_210.controllers[index].triggerR = lbl_802BFFE8[index];
        lbl_1_bss_210.controllers[index].dpad = lbl_802BFFE4[index];
        lbl_1_bss_210.controllers[index].dpadRepeat = lbl_802BFFE0[index];
        index++;
    }
}

/* Synchronizes stored scene and character transforms, leaving the four moving puzzle carriers to
 * their own updater. */
void fn_1_F44(void)
{
    s32 modelIndex;
    modelIndex = 2;
    while (modelIndex < 46) {
        if (modelIndex < 36 || modelIndex > 39) {
            fn_80026760(lbl_1_bss_4B4[modelIndex].modelId,
                lbl_1_bss_4B4[modelIndex].position.x, lbl_1_bss_4B4[modelIndex].position.y,
                lbl_1_bss_4B4[modelIndex].position.z);
            fn_80026808(lbl_1_bss_4B4[modelIndex].modelId,
                lbl_1_bss_4B4[modelIndex].rotation.x, lbl_1_bss_4B4[modelIndex].rotation.y,
                lbl_1_bss_4B4[modelIndex].rotation.z);
        }
        modelIndex++;
    }
    modelIndex = 0;
    while (modelIndex <= 1) {
        fn_80026760(lbl_1_bss_4B4[modelIndex].modelId,
            lbl_1_bss_2F4[modelIndex].position.x, lbl_1_bss_2F4[modelIndex].position.y,
            lbl_1_bss_2F4[modelIndex].position.z);
        fn_80026808(lbl_1_bss_4B4[modelIndex].modelId,
            lbl_1_bss_2F4[modelIndex].rotation.x, lbl_1_bss_2F4[modelIndex].rotation.y,
            lbl_1_bss_2F4[modelIndex].rotation.z);
        modelIndex++;
    }
}

/* During scene updates, applies each character's pending motion, speed, attribute and blend
 * requests. */
void fn_1_10F0(void)
{
    s16 playerIndex = 0;

    while (playerIndex < 2) {
        if (lbl_1_bss_3F4[playerIndex].requestedMotion !=
            lbl_1_bss_3F4[playerIndex].currentMotion) {
            fn_8002C95C(
                lbl_1_bss_4B4[playerIndex].modelId,
                lbl_1_bss_3F4[playerIndex].motionIds[lbl_1_bss_3F4[playerIndex].requestedMotion]);
            lbl_1_bss_3F4[playerIndex].currentMotion = lbl_1_bss_3F4[playerIndex].requestedMotion;
            lbl_1_bss_3F4[playerIndex].maximumTime = fn_8002D530(
                lbl_1_bss_4B4[playerIndex].modelId);
        }
        if (lbl_1_bss_3F4[playerIndex].requestedSpeed != lbl_1_bss_3F4[playerIndex].currentSpeed) {
            fn_8002D86C(lbl_1_bss_4B4[playerIndex].modelId,
                               lbl_1_bss_3F4[playerIndex].requestedSpeed);
            lbl_1_bss_3F4[playerIndex].currentSpeed = lbl_1_bss_3F4[playerIndex].requestedSpeed;
        }
        if (lbl_1_bss_3F4[playerIndex].requestedAttributes !=
            lbl_1_bss_3F4[playerIndex].currentAttributes) {
            fn_800269E8(lbl_1_bss_4B4[playerIndex].modelId,
                             lbl_1_bss_3F4[playerIndex].requestedAttributes);
            lbl_1_bss_3F4[playerIndex].currentAttributes =
                lbl_1_bss_3F4[playerIndex].requestedAttributes;
        }
        if (lbl_1_bss_3F4[playerIndex].resetAttributes != 0) {
            fn_80026A44(lbl_1_bss_4B4[playerIndex].modelId,
                lbl_1_bss_3F4[playerIndex].resetAttributes);
            lbl_1_bss_3F4[playerIndex].resetAttributes =
                ~lbl_1_bss_3F4[playerIndex].resetAttributes;
            lbl_1_bss_3F4[playerIndex].currentAttributes =
                lbl_1_bss_3F4[playerIndex].requestedAttributes =
                    lbl_1_bss_3F4[playerIndex].requestedAttributes &
                    lbl_1_bss_3F4[playerIndex].resetAttributes;
            lbl_1_bss_3F4[playerIndex].resetAttributes = 0;
        }
        if (lbl_1_bss_3F4[playerIndex].requestedTime != lbl_1_bss_3F4[playerIndex].previousTime) {
            fn_8002D414(lbl_1_bss_4B4[playerIndex].modelId,
                              lbl_1_bss_3F4[playerIndex].requestedTime);
        }
        switch (lbl_1_bss_3F4[playerIndex].shiftState) {
        case 0:
            break;
        case 1:
            fn_8002CCC0(
                lbl_1_bss_4B4[playerIndex].modelId,
                lbl_1_bss_3F4[playerIndex].motionIds[lbl_1_bss_3F4[playerIndex].shiftMotion],
                lbl_1_bss_3F4[playerIndex].shiftStartFrame,
                lbl_1_bss_3F4[playerIndex].blendDuration,
                lbl_1_bss_3F4[playerIndex].shiftAttributes);
            lbl_1_bss_3F4[playerIndex].shiftMaximumTime = fn_8002D5D4(
                lbl_1_bss_4B4[playerIndex].modelId);
            lbl_1_bss_3F4[playerIndex].shiftState = 2;
            break;
        case 2:
            lbl_1_bss_3F4[playerIndex].shiftTime = fn_8002D508(
                lbl_1_bss_4B4[playerIndex].modelId);
            /* A nonlooping shift finishes at blend duration minus one on the incoming playback
             * clock, rather than at the incoming clip's maximum time. */
            if (((lbl_1_bss_3F4[playerIndex].shiftAttributes & ((1 << 30) | (1 << 0))) == 0) &&
                (lbl_1_bss_3F4[playerIndex].shiftTime >=
                 lbl_1_bss_3F4[playerIndex].blendDuration - 1.0f)) {
                lbl_1_bss_3F4[playerIndex].shiftState = 3;
            }
            break;
        case 3:
            lbl_1_bss_3F4[playerIndex].shiftState = 0;
            break;
        }
        if (lbl_1_bss_3F4[playerIndex].shiftState == 0) {
            lbl_1_bss_3F4[playerIndex].requestedTime = fn_8002D4E0(
                lbl_1_bss_4B4[playerIndex].modelId);
            lbl_1_bss_3F4[playerIndex].previousTime = lbl_1_bss_3F4[playerIndex].requestedTime;
        } else {
            lbl_1_bss_3F4[playerIndex].previousTime =
                lbl_1_bss_3F4[playerIndex].requestedTime =
                    -1.0f;
        }
        lbl_1_bss_3F4[playerIndex].motionEnded = fn_8002D75C(
            lbl_1_bss_4B4[playerIndex].modelId);
        if ((lbl_1_bss_3F4[playerIndex].motionEnded != 0) &&
            (lbl_1_bss_3F4[playerIndex].completionEvent != 0)) {
            lbl_1_bss_3F4[playerIndex].motionEnded = 0;
            lbl_1_bss_3F4[playerIndex].completionEvent = 0;
            lbl_1_bss_3F4[playerIndex].requestedMotion = lbl_1_bss_3F4[playerIndex].nextMotion;
            lbl_1_bss_3F4[playerIndex].requestedTime = lbl_1_bss_3F4[playerIndex].nextTime;
            lbl_1_bss_3F4[playerIndex].requestedAttributes =
                lbl_1_bss_3F4[playerIndex].nextAttributes;
            lbl_1_bss_3F4[playerIndex].resetAttributes =
                lbl_1_bss_3F4[playerIndex].nextResetAttributes;
        }
        playerIndex++;
    }
}

/* Loads a camera preset during setup and writes its position, rotation and zoom to the shared
 * camera controls. */
void fn_1_1918(s16 cameraIndex) {
    f32 presets[4][13] = {
        { 0.0f, 1573.0f, -446.0f, -15.2f, 0.0f, 0.0f, 3317.7f, 1.33f, 0.0f, 0.0f, 640.0f, 480.0f, 17.55f },
        { 0.0f, 293.0f, -326.0f, -15.2f, 0.0f, 0.0f, 3409.7f, 1.33f, 0.0f, 0.0f, 640.0f, 480.0f, 17.5f },
        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f, 0.0f, 0.0f, 640.0f, 480.0f, 45.0f },
        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f, 0.0f, 0.0f, 640.0f, 480.0f, 45.0f }
    };
    s16 cameraIds[4] = { 1, 2, 4, 8 };
    lbl_1_bss_B08.cameras[cameraIndex].cameraIndex = cameraIndex;
    lbl_1_bss_B08.cameras[cameraIndex].position.x = presets[cameraIndex][0];
    lbl_1_bss_B08.cameras[cameraIndex].position.y = presets[cameraIndex][1];
    lbl_1_bss_B08.cameras[cameraIndex].position.z = presets[cameraIndex][2];
    lbl_1_bss_B08.cameras[cameraIndex].rotation.x = presets[cameraIndex][3];
    lbl_1_bss_B08.cameras[cameraIndex].rotation.y = presets[cameraIndex][4];
    lbl_1_bss_B08.cameras[cameraIndex].rotation.z = presets[cameraIndex][5];
    lbl_1_bss_B08.cameras[cameraIndex].viewport[0] = presets[cameraIndex][6];
    lbl_1_bss_B08.cameras[cameraIndex].viewport[1] = presets[cameraIndex][7];
    lbl_1_bss_B08.cameras[cameraIndex].viewport[2] = presets[cameraIndex][8];
    lbl_1_bss_B08.cameras[cameraIndex].viewport[3] = presets[cameraIndex][9];
    lbl_1_bss_B08.cameras[cameraIndex].viewport[4] = presets[cameraIndex][10];
    lbl_1_bss_B08.cameras[cameraIndex].viewport[5] = presets[cameraIndex][11];
    lbl_1_bss_B08.cameras[cameraIndex].fieldOfView = presets[cameraIndex][12];

    Hu3DCameraCreate(cameraIds[cameraIndex]);
    Hu3DCameraViewportSet(cameraIds[cameraIndex],
                          lbl_1_bss_B08.cameras[cameraIndex].viewport[2],
                          lbl_1_bss_B08.cameras[cameraIndex].viewport[3],
                          lbl_1_bss_B08.cameras[cameraIndex].viewport[4],
                          lbl_1_bss_B08.cameras[cameraIndex].viewport[5],
                          0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(cameraIds[cameraIndex],
                             lbl_1_bss_B08.cameras[cameraIndex].fieldOfView,
                             20.0f, 40000.0f,
                             lbl_1_bss_B08.cameras[cameraIndex].viewport[1]);
    lbl_80266A4C.x = presets[cameraIndex][0];
    lbl_80266A4C.y = presets[cameraIndex][1];
    lbl_80266A4C.z = presets[cameraIndex][2];
    lbl_80266A40.x = presets[cameraIndex][3];
    lbl_80266A40.y = presets[cameraIndex][4];
    lbl_80266A40.z = presets[cameraIndex][5];
    lbl_802C028C = presets[cameraIndex][6];
    fn_1_1DC8(cameraIndex);
}

/* Phase callbacks used by the minigame sequence process. */
MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, fn_1_130, fn_1_260, fn_1_28C, fn_1_2B8, fn_1_2E4, fn_1_310, fn_1_33C, fn_1_368, fn_1_394
};

void fn_1_1FD0(void), fn_1_234C(void), fn_1_10F0(void);
void fn_1_3258(s16, s16);
void fn_1_4470(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);
s16 fn_1_4270(s16, s16, Vec, Vec, f32, s16, M662ParticleParameters *);

/* Called after camera setup and during scene updates; copies the camera masks locally without
 * changing scene state. */
void fn_1_1DC8(s16 cameraIndex) {
    struct Values {
        u16 cameraOneMask;
        u16 cameraTwoMask;
        u16 cameraThreeMask;
        u16 cameraFourMask;
    } cameraMasks = { 1, 2, 4, 8 };
}

/* Initializes the scene's white directional light and applies its starting angular settings. */
void fn_1_1DFC(void) {
    fn_800285BC();

    lbl_1_bss_ACC.position.x = 0.0f;
    lbl_1_bss_ACC.position.y = 0.0f;
    lbl_1_bss_ACC.position.z = 0.0f;
    lbl_1_bss_ACC.aim.x = 0.0f;
    lbl_1_bss_ACC.aim.y = 0.0f;
    lbl_1_bss_ACC.aim.z = 0.0f;
    lbl_1_bss_ACC.orientation.x = -70.0f;
    lbl_1_bss_ACC.orientation.y = 0.0f;
    lbl_1_bss_ACC.orientation.z = 0.0f;
    lbl_1_bss_ACC.range = 1000.0f;

    lbl_1_bss_ACC.color.r = 255;
    lbl_1_bss_ACC.color.g = 255;
    lbl_1_bss_ACC.color.b = 255;
    lbl_1_bss_ACC.color.a = 255;

    PSVECSubtract(&lbl_1_bss_ACC.aim, &lbl_1_bss_ACC.position,
                  &lbl_1_bss_ACC.direction);
    lbl_1_bss_ACC.id = fn_800286D0(
        &lbl_1_bss_ACC.position, &lbl_1_bss_ACC.direction,
        &lbl_1_bss_ACC.color);
    fn_1_1FD0();
    Hu3DGLightStaticSet(lbl_1_bss_ACC.id, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_ACC.id);
}

/* At setup and during scene updates, recomputes the light's position and normalized direction and
 * applies its color. */
inline void fn_1_1FD0(void) {
    lbl_1_bss_ACC.position.x =
        lbl_1_bss_ACC.aim.x +
        lbl_1_bss_ACC.range *
            (cos(3.141592653589793 * lbl_1_bss_ACC.orientation.x /
                 180.0) *
             sin(3.141592653589793 * lbl_1_bss_ACC.orientation.y /
                 180.0));
    lbl_1_bss_ACC.position.y =
        lbl_1_bss_ACC.aim.y +
        lbl_1_bss_ACC.range *
            -sin(3.141592653589793 * lbl_1_bss_ACC.orientation.x /
                 180.0);
    lbl_1_bss_ACC.position.z =
        lbl_1_bss_ACC.aim.z +
        lbl_1_bss_ACC.range *
            (cos(3.141592653589793 * lbl_1_bss_ACC.orientation.x /
                 180.0) *
             cos(3.141592653589793 * lbl_1_bss_ACC.orientation.y /
                 180.0));
    PSVECSubtract(&lbl_1_bss_ACC.aim, &lbl_1_bss_ACC.position,
                  &lbl_1_bss_ACC.direction);
    PSVECNormalize(&lbl_1_bss_ACC.direction, &lbl_1_bss_ACC.direction);
    Hu3DGLightPosSetV(lbl_1_bss_ACC.id, &lbl_1_bss_ACC.position,
                      &lbl_1_bss_ACC.direction);
    Hu3DGLightColorSet(lbl_1_bss_ACC.id, lbl_1_bss_ACC.color.r,
                       lbl_1_bss_ACC.color.g, lbl_1_bss_ACC.color.b,
                       lbl_1_bss_ACC.color.a);
}

/* Creates the scene shadow map, sets its orbit and opacity, and applies the starting shadow-camera
 * transform. */
void fn_1_223C(void) {
    Hu3DShadowCreate(5.0f, 16000.0f, 21000.0f);
    Hu3DShadowSizeSet(480);
    lbl_1_bss_A7C.angle[0] = -90.0f;
    lbl_1_bss_A7C.angle[1] = 0.0f;
    lbl_1_bss_A7C.angle[2] = 0.0f;
    lbl_1_bss_A7C.target.x = 0.0f;
    lbl_1_bss_A7C.target.y = -100.0f;
    lbl_1_bss_A7C.target.z = 0.0f;
    lbl_1_bss_A7C.textureLevel = 0.2f;
    lbl_1_bss_A7C.radius = 21000.0f;
    fn_1_234C();
}

/* At setup and during scene updates, positions the shadow camera and its up vector from the stored
 * orbit angles. */
inline void fn_1_234C(void)
{
    lbl_1_bss_A7C.position.x = (f32)((f64)lbl_1_bss_A7C.target.x +
        ((f64)lbl_1_bss_A7C.radius *
         (cos((3.141592653589793 * (f64)lbl_1_bss_A7C.angle[0]) / 180.0) *
          sin((3.141592653589793 * (f64)lbl_1_bss_A7C.angle[1]) / 180.0))));
    lbl_1_bss_A7C.position.y = (f32)((f64)lbl_1_bss_A7C.target.y +
        ((f64)lbl_1_bss_A7C.radius *
         -sin((3.141592653589793 * (f64)lbl_1_bss_A7C.angle[0]) / 180.0)));
    lbl_1_bss_A7C.position.z = (f32)((f64)lbl_1_bss_A7C.target.z +
        ((f64)lbl_1_bss_A7C.radius *
         (cos((3.141592653589793 * (f64)lbl_1_bss_A7C.angle[0]) / 180.0) *
          cos((3.141592653589793 * (f64)lbl_1_bss_A7C.angle[1]) / 180.0))));

    lbl_1_bss_A7C.up.x = (f32)(
        sin((3.141592653589793 * (f64)lbl_1_bss_A7C.angle[0]) / 180.0) *
        sin((3.141592653589793 * (f64)lbl_1_bss_A7C.angle[1]) / 180.0));
    lbl_1_bss_A7C.up.y = (f32)cos(
        (3.141592653589793 * (f64)lbl_1_bss_A7C.angle[0]) / 180.0);
    lbl_1_bss_A7C.up.z = (f32)(
        sin((3.141592653589793 * (f64)lbl_1_bss_A7C.angle[0]) / 180.0) *
        cos((3.141592653589793 * (f64)lbl_1_bss_A7C.angle[1]) / 180.0));

    Hu3DShadowTPLvlSet(lbl_1_bss_A7C.textureLevel);
    Hu3DShadowPosSet(&lbl_1_bss_A7C.position, &lbl_1_bss_A7C.up,
                     &lbl_1_bss_A7C.target);
}

/* After game updates, applies model and character transforms, processes motion requests, and
 * refreshes lighting and shadows. */
void fn_1_2670(void) {
    s32 modelIndex;

    fontcolor = FONT_COLOR_WHITE;
    modelIndex = 2;
    while (modelIndex < 46) {
        if ((modelIndex < 36) || (modelIndex > 39)) {
            fn_80026760(lbl_1_bss_4B4[modelIndex].modelId,
                            lbl_1_bss_4B4[modelIndex].position.x,
                            lbl_1_bss_4B4[modelIndex].position.y,
                            lbl_1_bss_4B4[modelIndex].position.z);
            fn_80026808(lbl_1_bss_4B4[modelIndex].modelId,
                            lbl_1_bss_4B4[modelIndex].rotation.x,
                            lbl_1_bss_4B4[modelIndex].rotation.y,
                            lbl_1_bss_4B4[modelIndex].rotation.z);
        }
        modelIndex++;
    }

    modelIndex = 0;
    while (modelIndex <= 1) {
        fn_80026760(
            lbl_1_bss_4B4[modelIndex].modelId,
            lbl_1_bss_2F4[modelIndex].position.x,
            lbl_1_bss_2F4[modelIndex].position.y,
            lbl_1_bss_2F4[modelIndex].position.z);
        fn_80026808(
            lbl_1_bss_4B4[modelIndex].modelId,
            lbl_1_bss_2F4[modelIndex].rotation.x,
            lbl_1_bss_2F4[modelIndex].rotation.y,
            lbl_1_bss_2F4[modelIndex].rotation.z);
        modelIndex++;
    }

    fn_1_10F0();
    fn_1_1DC8(0);

    fn_1_1FD0();

    fn_1_234C();
}

/* Advances a scalar by one fixed interpolation step, estimates steps remaining, and snaps to the
 * endpoint when finished. */
s32 fn_1_2DA0(f32 *currentValue, f32 durationFrames, f32 start, f32 end) {
    f32 endpointTestValue;
    f32 currentTestValue;
    s32 framesRemaining = 0;
    f32 increment = (end - start) / durationFrames;

    *currentValue += increment;
    if (increment) {
        framesRemaining = (s32)((end - *currentValue) / increment);
    }
    framesRemaining++;
    if (start - end > 0.0f) {
        endpointTestValue = start - end;
    } else {
        endpointTestValue = start - -1.0f * end;
    }
    increment = endpointTestValue;
    if (start - *currentValue > 0.0f) {
        currentTestValue = start - *currentValue;
    } else {
        currentTestValue = start - -1.0f * *currentValue;
    }
    /* The stopping values use a positive difference, otherwise a sum; the step variable and start
     * argument are reused for this test. */
    start = currentTestValue;
    if (increment <= start) {
        *currentValue = end;
        framesRemaining = 0;
    }
    return framesRemaining;
}

/* Creates a round timer at the requested screen position, converting its duration and endpoint from
 * seconds to frames. */
MGTIMER *fn_1_2EC0(int timerType, int durationSeconds, int endSeconds, float xPosition,
                   float yPosition)
{
    MGTIMER *timer=MgTimerCreate(timerType);
    MgTimerParamSet(timer,durationSeconds*60,endSeconds*60,0);
    MgTimerPosSet(timer,xPosition,yPosition);
    return timer;
}

/* Starts the round timer when the player reaches the selection phase. */
void fn_1_2F34(MGTIMER *timer) {
    MgTimerModeOnSet(timer, 1);
}

/* Returns whether the round timer has reached its configured end time. */
s32 fn_1_2F60(MGTIMER *timer) {
    return MgTimerDoneCheck(timer);
}

/* Hides and destroys the round timer after the timed selection has finished. */
void fn_1_2F88(MGTIMER *timer) {
    MgTimerDispOff(timer);
    MgTimerKill(timer);
}

/* Debug input entry point: selects a model within the supplied range, edits it, and can print the
 * range's transforms. */
void fn_1_2FC0(s16 padPort, s16 lower, s16 upper)
{
    s16 index;

    if (lbl_802BFFEC[padPort] > 50) {
        if (lbl_802C0008[padPort] & PAD_BUTTON_X) {
            lbl_1_bss_210.selectionValue++;
        }
        if (lbl_802C0008[padPort] & PAD_BUTTON_Y) {
            lbl_1_bss_210.selectionValue--;
        }
    }

    if (lbl_1_bss_210.selectionValue < lower) {
        lbl_1_bss_210.selectionValue = upper;
    }
    if (lbl_1_bss_210.selectionValue > upper) {
        lbl_1_bss_210.selectionValue = lower;
    }

    fn_1_3258(padPort, lbl_1_bss_210.selectionValue);

    if ((lbl_802C0008[padPort] & PAD_BUTTON_X) && (lbl_802C0008[padPort] & PAD_BUTTON_Y)) {
        for (index = lower; index <= upper; index++) {
            if (index >= 0 && index <= 1) {
                printf("OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f\n", index,
                       lbl_1_bss_2F4[index].position.x,
                       lbl_1_bss_2F4[index].position.y,
                       lbl_1_bss_2F4[index].position.z,
                       lbl_1_bss_2F4[index].rotation.y);
            } else {
                printf("{{%.1ff,%.1ff,%.1ff},{0.0f,%.1ff,0.0f}},\n", index,
                       lbl_1_bss_4B4[index].position.x,
                       lbl_1_bss_4B4[index].position.y,
                       lbl_1_bss_4B4[index].position.z,
                       lbl_1_bss_4B4[index].rotation.y);
            }
        }
    }
}

/* During debug input processing, moves or rotates the selected character or scene model and prints
 * its transform. */
void fn_1_3258(s16 padPort, s16 selection)
{

    f32 step = 5.0f;
    s16 printX;
    s16 printY;

    printX = 25;
    printY = 430;

    if ((selection >= 0) && (selection <= 1)) {

        if (lbl_802C0000[padPort] & PAD_BUTTON_X) {
            if (lbl_802BFFFC[padPort] > 10) {
                lbl_1_bss_2F4[selection].position.x += step;
            }
            if (lbl_802BFFFC[padPort] < -10) {
                lbl_1_bss_2F4[selection].position.x -= step;
            }
            if (lbl_802BFFF8[padPort] > 10) {
                lbl_1_bss_2F4[selection].position.z -= step;
            }
            if (lbl_802BFFF8[padPort] < -10) {
                lbl_1_bss_2F4[selection].position.z += step;
            }
            /* These independent tests overlap, cancelling the vertical step near the center. */
            if (lbl_802BFFF0[padPort] < 10) {
                lbl_1_bss_2F4[selection].position.y -= step;
            }
            if (lbl_802BFFF0[padPort] > -10) {
                lbl_1_bss_2F4[selection].position.y += step;
            }
        } else if (lbl_802C0008[padPort] & PAD_BUTTON_X) {
            if (lbl_802BFFFC[padPort] > 10) {
                lbl_1_bss_2F4[selection].position.x += step;
            }
            if (lbl_802BFFFC[padPort] < -10) {
                lbl_1_bss_2F4[selection].position.x -= step;
            }
            if (lbl_802BFFF8[padPort] > 10) {
                lbl_1_bss_2F4[selection].position.z -= step;
            }
            if (lbl_802BFFF8[padPort] < -10) {
                lbl_1_bss_2F4[selection].position.z += step;
            }
            if (lbl_802BFFF0[padPort] < 10) {
                lbl_1_bss_2F4[selection].position.y -= step;
            }
            if (lbl_802BFFF0[padPort] > -10) {
                lbl_1_bss_2F4[selection].position.y += step;
            }
        }
        if (lbl_802C0008[padPort] & PAD_BUTTON_A) {
            lbl_1_bss_2F4[selection].rotation.y += 90.0f;
        }
        if (lbl_802C0008[padPort] & PAD_BUTTON_B) {
            lbl_1_bss_2F4[selection].rotation.y -= 90.0f;
        }
        print8(printX, printY, 1.5f, "OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f",
               selection, lbl_1_bss_2F4[selection].position.x,
               lbl_1_bss_2F4[selection].position.y, lbl_1_bss_2F4[selection].position.z,
               lbl_1_bss_2F4[selection].rotation.y);
        return;
    }

    {

        if (lbl_802C0000[padPort] & PAD_BUTTON_X) {
            if (lbl_802BFFFC[padPort] > 10) {
                lbl_1_bss_4B4[selection].position.x += step;
            }
            if (lbl_802BFFFC[padPort] < -10) {
                lbl_1_bss_4B4[selection].position.x -= step;
            }
            if (lbl_802BFFF8[padPort] > 10) {
                lbl_1_bss_4B4[selection].position.z -= step;
            }
            if (lbl_802BFFF8[padPort] < -10) {
                lbl_1_bss_4B4[selection].position.z += step;
            }
        } else if (lbl_802C0008[padPort] & PAD_BUTTON_X) {
            if (lbl_802BFFFC[padPort] > 10) {
                lbl_1_bss_4B4[selection].position.x += step;
            }
            if (lbl_802BFFFC[padPort] < -10) {
                lbl_1_bss_4B4[selection].position.x -= step;
            }
            if (lbl_802BFFF8[padPort] > 10) {
                lbl_1_bss_4B4[selection].position.z -= step;
            }
            if (lbl_802BFFF8[padPort] < -10) {
                lbl_1_bss_4B4[selection].position.z += step;
            }
        }
        if (lbl_802C0008[padPort] & PAD_BUTTON_A) {
            lbl_1_bss_4B4[selection].rotation.y += 90.0f;
        }
        if (lbl_802C0008[padPort] & PAD_BUTTON_B) {
            lbl_1_bss_4B4[selection].rotation.y -= 90.0f;
        }
        print8(printX, printY, 1.5f, "    OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f",
               selection, lbl_1_bss_4B4[selection].position.x,
               lbl_1_bss_4B4[selection].position.y, lbl_1_bss_4B4[selection].position.z,
               lbl_1_bss_4B4[selection].rotation.y);
    }
}

/* Attaches a local light to a model; returns 1 on insertion or -1 when already present or no slot
 * is free. */
s16 fn_1_3AC4(s16 modelIndex, s16 lightId)
{
    s16 result = 0;
    HU3D_MODEL *model = &lbl_802C01C8[modelIndex];
    s16 index = 0;

    while (index < 8) {
        if ((s16)model->LLightId[index] == lightId) {
            result = -1;
            break;
        }
        index++;
    }
    if (result != -1) {
        index = 0;
        while (index < 8) {
            if (model->LLightId[index] == -1) {
                model->LLightId[index] = lightId;
                model->attr |= (1 << 12);
                result = 1;
                break;
            }
            index++;
        }
    }
    if (result == 0) {
        printf("ModelIntoLLight False model:%d light:%d\n", modelIndex, lightId);
        result = -1;
    }
    return result;
}

/* Removes a local light from a scene model, returning 1 when found or -1 when absent. */
s32 fn_1_3BDC(HU3D_MODELID modelId, HU3D_LIGHTID lightId) {
    HU3D_MODEL *model;
    s32 result;
    s16 index;

    result = -1;
    model = &lbl_802C01C8[modelId];
    index = 0;
    while (index < 8) {
        if (model->LLightId[index] == lightId) {
            model->LLightId[index] = -1;
            result = 1;
            break;
        }
        index++;
    }
    return result;
}

/* Provides ambient-model placement with a floating-point integer below the truncated random
 * limit. */
f32 fn_1_3C74(f32 limit) {
    s32 randomValue = limit;
    randomValue = frandmod(randomValue);
    limit = randomValue;
    return limit;
}

/* Creates the scene particle pool, allocates its per-particle settings, and installs the
 * draw-update hook. */
void fn_1_3CEC(void)
{
    u32 dataNumber[1] = { DATANUM(0x790000, 37) };
    s16 maximumCount[1] = { 100 };
    s16 layer[1] = { 0 };
    s16 camera[1] = { 1 };
    s16 blendMode[1] = { 1 };
    HU3D_PARTICLE_HOOK hook[1] = { fn_1_4470 };
    s16 effectIndex;

    for (effectIndex = 0; effectIndex < 1; effectIndex++) {
        ANIMDATA *animation;
        HU3D_PARTICLE *context;
        HU3D_PARTICLE_DATA *entry;
        s16 particleIndex;

        lbl_1_bss_A74.maximumCount[effectIndex] = maximumCount[effectIndex];
        animation = HuSprAnimRead(fn_80008DD0(
            dataNumber[effectIndex], 0x10000000, 2));
        lbl_1_bss_A74.allocation[effectIndex] = HuMemDirectMallocNum(
            HEAP_HEAP, maximumCount[effectIndex] * 60, 0x10000000);
        lbl_1_bss_A74.modelId[effectIndex] = fn_8003216C(
            animation, maximumCount[effectIndex]);
        fn_80026B8C(lbl_1_bss_A74.modelId[effectIndex], layer[effectIndex]);
        fn_800326D0(lbl_1_bss_A74.modelId[effectIndex], hook[effectIndex]);
        fn_80026B64(lbl_1_bss_A74.modelId[effectIndex], camera[effectIndex]);
        fn_8003269C(lbl_1_bss_A74.modelId[effectIndex],
                                 (u8)blendMode[effectIndex]);

        context = lbl_802C01C8[lbl_1_bss_A74.modelId[effectIndex]].hookData;
        context->emitCnt = 0;
        context->work = lbl_1_bss_A74.allocation[effectIndex];
        context->count = 0;
        particleIndex = 0;
        entry = context->data;
        for (; particleIndex < context->maxCnt; particleIndex++, entry++) {
            entry->scale = 0.0f;
            entry->time = 0;
        }
    }
}

/* Emits a white particle at the requested position with randomized lateral and upward velocity and
 * a shrinking scale. */
void fn_1_3F94(Vec *position)
{
    Vec velocity;
    Vec inputPosition;
    M662ParticleParameters parameters;
    s32 randomLimit;
    s32 randomHeightLimit;
    f32 randomValue;
    f32 heightRandom;
    f32 initialScale;
    f32 centeredRandomValue;
    f32 horizontalVelocity;
    f32 baseHeight;
    f32 verticalVelocity;

    parameters.control = 0;
    parameters.initialColor.r = 255;
    parameters.initialColor.g = 255;
    parameters.initialColor.b = 255;
    parameters.initialColor.a = 255;
    parameters.finalColor[0] = 0;
    parameters.finalColor[1] = 0;
    parameters.finalColor[2] = 0;
    parameters.finalColor[3] = 0;
    parameters.rotationStep = 0.0f;
    parameters.alphaStep = 0.01f;
    parameters.colorBlendStep = 0.01f;
    parameters.gravity = 0.0f;

    inputPosition.x = position->x;
    inputPosition.y = position->y;
    inputPosition.z = position->z;
    initialScale = 30.0f;
    parameters.scaleStep = -0.1f;
    parameters.scaleStepMultiplier = 1.2f;
    parameters.velocityScale[0] = 1.05f;
    parameters.velocityScale[1] = 0.95f;
    parameters.velocityScale[2] = 1.0f;
    randomValue = 3.0f;
    randomLimit = (s32)randomValue;
    randomLimit = frandmod(randomLimit);
    randomValue = (f32)randomLimit;
    centeredRandomValue = randomValue;
    horizontalVelocity = centeredRandomValue;
    velocity.x = 0.4f *
                 (horizontalVelocity - 1.0f);
    heightRandom = 4.0f;
    randomHeightLimit = (s32)heightRandom;
    randomHeightLimit = frandmod(randomHeightLimit);
    heightRandom = (f32)randomHeightLimit;
    baseHeight = heightRandom;
    verticalVelocity = baseHeight;
    velocity.y = 3.0f + verticalVelocity;
    velocity.z = 0.0f;

    fn_1_4270(0, -1, inputPosition, velocity, initialScale, 20, &parameters);
}

/* Particle emission helper: initializes the first inactive entry and returns its index, or -1 when
 * the pool is full. */
s16 fn_1_4270(s16 effectIndex, s16 ownerIndex, Vec position, Vec velocity,
              f32 initialScale, s16 lifetimeFrames, M662ParticleParameters *parameters) {
    HU3D_MODEL *model;
    s16 result;
    HU3D_PARTICLE *context;
    M662ParticleParameters *currentParameters;
    s16 entryIndex;
    HU3D_PARTICLE_DATA *entry;

    result = -1;
    model = &lbl_802C01C8[lbl_1_bss_A74.modelId[effectIndex]];
    context = model->hookData;
    currentParameters = context->work;
    entryIndex = 0;
    entry = context->data;

    while (entryIndex < context->maxCnt) {
        if (entry->time <= 0) {
            currentParameters[entryIndex] = *parameters;
            currentParameters[entryIndex].resetValues[2] =
                currentParameters[entryIndex].resetValues[1] =
                    currentParameters[entryIndex].resetValues[0] = 0.0f;

            entry->pos = position;
            entry->vel = velocity;
            /* The renderer's acceleration vector receives the spawn position as well. */
            entry->accel = position;
            entry->scaleBase = entry->colorIdx =
                entry->speedDecay = 0.0f;
            entry->color = currentParameters[entryIndex].initialColor;
            entry->scale = initialScale;
            entry->time = lifetimeFrames;
            entry->parManId = ownerIndex;
            context->emitCnt = entryIndex;
            result = entryIndex;
            break;
        }
        entryIndex++;
        entry++;
    }
    return result;
}

/* Particle draw hook: advances lifetimes, motion, rotation, color and scale before the entries are
 * rendered. */
void fn_1_4470(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    M662ParticleParameters *parameters =
        particle->work;
    HU3D_PARTICLE_DATA *entry = particle->data;
    s16 entryNo = 0;

    while (entryNo < particle->maxCnt) {
        if (entry->time-- > 0) {
            s16 channelValue;

            entry->vel.x *= parameters[entryNo].velocityScale[0];
            entry->vel.y *= parameters[entryNo].velocityScale[1];
            entry->vel.z *= parameters[entryNo].velocityScale[2];
            PSVECAdd(&entry->vel, &entry->pos,
                     &entry->pos);
            entry->vel.y += parameters[entryNo].gravity;
            entry->zRot += parameters[entryNo].rotationStep;

            channelValue = (s16)((f32)entry->color.r +
                                 (parameters[entryNo].colorBlendStep *
                                  (f32)(parameters[entryNo].finalColor[0] -
                                        parameters[entryNo].initialColor.r)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.r = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.g +
                                 (parameters[entryNo].colorBlendStep *
                                  (f32)(parameters[entryNo].finalColor[1] -
                                        parameters[entryNo].initialColor.g)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.g = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.b +
                                 (parameters[entryNo].colorBlendStep *
                                  (f32)(parameters[entryNo].finalColor[2] -
                                        parameters[entryNo].initialColor.b)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.b = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.a +
                                 parameters[entryNo].alphaStep);
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.a = (u8)channelValue;
            if (channelValue <= 0) {
                entry->time = 0;
                entry->scale = 0.0f;
            }

            parameters[entryNo].scaleStep *= parameters[entryNo].scaleStepMultiplier;
            entry->scale += parameters[entryNo].scaleStep;
            if (entry->scale <= 0.0f) {
                entry->scale = 0.0f;
                entry->time = 0;
            }
            DCStoreRangeNoSync(particle->data,
                               particle->maxCnt * 72);
        } else {
            entry->scale = 0.0f;
            entry->time = -1;
        }
        entryNo++;
        entry++;
    }
}

/* During player updates, sets character voice pan and volume from position or the requested fixed
 * spatial mode. */
void fn_1_48E8(M662Player *player, s16 spatialMode)
{
    s16 pan;
    s16 volume;
    f32 panScale;
    f32 volumeScale;
    f32 positionOffset;

    if ((spatialMode == 0) || (spatialMode == 1) || (spatialMode == 6)) {
        if ((lbl_1_bss_210.xMaximum >= player->position.x) &&
            (lbl_1_bss_210.xMinimum <= player->position.x)) {
            panScale =
                (f32)(lbl_1_bss_210.panMaximum -
                      lbl_1_bss_210.panMinimum) /
                (lbl_1_bss_210.xMaximum - lbl_1_bss_210.xMinimum);
            positionOffset = player->position.x - lbl_1_bss_210.xMinimum;
            positionOffset *= panScale;
            pan = (s16)positionOffset;
            pan += lbl_1_bss_210.panMinimum;
        } else if (lbl_1_bss_210.xMaximum < player->position.x) {
            pan = lbl_1_bss_210.panMaximum;
        } else if (lbl_1_bss_210.xMinimum > player->position.x) {
            pan = lbl_1_bss_210.panMinimum;
        }
    }

    if ((spatialMode == 0) || (spatialMode == 2)) {
        if ((lbl_1_bss_210.zMaximum >= player->position.z) &&
            (lbl_1_bss_210.zMinimum <= player->position.z)) {
            volumeScale =
                (f32)(lbl_1_bss_210.volumeMaximum -
                      lbl_1_bss_210.volumeMinimum) /
                (lbl_1_bss_210.zMaximum - lbl_1_bss_210.zMinimum);
            positionOffset = player->position.z - lbl_1_bss_210.zMinimum;
            positionOffset *= volumeScale;
            volume = (s16)positionOffset;
            volume += lbl_1_bss_210.volumeMinimum;
        } else if (lbl_1_bss_210.zMaximum < player->position.z) {
            volume = lbl_1_bss_210.volumeMaximum;
        } else if (lbl_1_bss_210.zMinimum > player->position.z) {
            volume = lbl_1_bss_210.volumeMinimum;
        }
    }

    if ((spatialMode == 2) || (spatialMode == 3) || (spatialMode == 4) || (spatialMode == 5)) {
        pan = lbl_1_bss_210.panMinimum +
              (lbl_1_bss_210.panMaximum - lbl_1_bss_210.panMinimum) / 2;
    }
    if ((spatialMode == 1) || (spatialMode == 3)) {
        volume = lbl_1_bss_210.volumeMinimum +
                 (lbl_1_bss_210.volumeMaximum -
                  lbl_1_bss_210.volumeMinimum) / 2;
    }
    if (spatialMode == 4) {
        volume = lbl_1_bss_210.volumeMinimum;
    }
    if ((spatialMode == 5) || (spatialMode == 6)) {
        volume = lbl_1_bss_210.volumeMaximum;
    }

    fn_80060F14(player->characterNo, pan);
    /* These two modes change pan while preserving the current voice volume. */
    if ((spatialMode != 5) && (spatialMode != 6)) {
        fn_80060F40(player->characterNo, volume);
    }
}

/* During scene initialization, resets music and effect handles and configures the positional-audio
 * ranges. */
void fn_1_4D04(void) {
    s16 streamIndex;

    lbl_1_bss_210.bgmHandle = -1;
    lbl_1_bss_210.jingleHandle = -1;
    lbl_1_bss_210.bgmId = MSM_STREAM_MGMUS_27;
    lbl_1_bss_210.jingleId = -1;
    lbl_1_bss_210.panMinimum = 48;
    lbl_1_bss_210.panMaximum = 80;
    lbl_1_bss_210.xMinimum = (f32) -500.0f;
    lbl_1_bss_210.xMaximum = (f32) 500.0f;
    lbl_1_bss_210.volumeMinimum = 32;
    lbl_1_bss_210.volumeMaximum = 96;
    lbl_1_bss_210.zMinimum = (f32) -800.0f;
    lbl_1_bss_210.zMaximum = (f32) 300.0f;
    streamIndex = 0;
    while (streamIndex < 4) {
        lbl_1_bss_210.soundState[streamIndex] = -1;
        lbl_1_bss_210.soundHandle[streamIndex] = -1;
        streamIndex += 1;
    }
}

/* Music update helper: starts a requested jingle when no jingle handle is active. */
void fn_1_4E48(void) {
    if (((s32) lbl_1_bss_210.jingleId != -1) && ((s32) lbl_1_bss_210.jingleHandle == -1)) {
        lbl_1_bss_210.jingleHandle = HuAudJinglePlay((s16) lbl_1_bss_210.jingleId);
    }
}

/* During the introduction, starts the requested background music once when its handle is
 * inactive. */
void fn_1_4EAC(void) {
    if ((s32) lbl_1_bss_210.bgmHandle == -1) {
        lbl_1_bss_210.bgmHandle = HuAudBGMPlay((s16) lbl_1_bss_210.bgmId);
    }
}

/* Passes the requested fade speed to the selected music stream and immediately clears its stored
 * handle. */
void fn_1_4EFC(s16 streamSlot, s16 fadeSpeed) {
    s16 streamId = streamSlot != 0
                       ? lbl_1_bss_210.bgmHandle
                       : lbl_1_bss_210.jingleHandle;
    if (streamId != -1) {
        HuAudSStreamFadeOut(streamId, fadeSpeed);
        if (streamSlot != 0) {
            lbl_1_bss_210.bgmHandle = -1;
            return;
        }
        lbl_1_bss_210.jingleHandle = -1;
    }
}

/* Stops the selected music stream immediately; the stored handle is left unchanged. */
void fn_1_4FA8(s16 streamSlot)
{
    s16 stream;

    if (streamSlot != 0) {
        stream = (s16)lbl_1_bss_210.bgmHandle;
    } else {
        stream = (s16)lbl_1_bss_210.jingleHandle;
    }
    HuAudSStreamStop(stream);
}

/* Plays a scene effect slot with position-based, centered or fixed pan and volume according to its
 * spatial mode. */
void fn_1_5008(s16 soundSlot, s16 spatialMode, HuVecF *position)
{
    s16 pan;
    s16 volume;
    f32 panScale;
    f32 volumeScale;
    f32 positionOffset;

    if (spatialMode <= 7) {
        if ((spatialMode == 0) || (spatialMode == 1) || (spatialMode == 6)) {
            if ((lbl_1_bss_210.xMaximum >= position->x) &&
                (lbl_1_bss_210.xMinimum <= position->x)) {
                panScale =
                    (f32)(lbl_1_bss_210.panMaximum -
                          lbl_1_bss_210.panMinimum) /
                    (lbl_1_bss_210.xMaximum - lbl_1_bss_210.xMinimum);
                positionOffset = position->x - lbl_1_bss_210.xMinimum;
                positionOffset *= panScale;
                pan = (s16)positionOffset;
                pan += lbl_1_bss_210.panMinimum;
            } else if (lbl_1_bss_210.xMaximum < position->x) {
            pan = lbl_1_bss_210.panMaximum;
            } else if (lbl_1_bss_210.xMinimum > position->x) {
                pan = lbl_1_bss_210.panMinimum;
        }
        }
        if ((spatialMode == 0) || (spatialMode == 2)) {
            if ((lbl_1_bss_210.zMaximum >= position->z) &&
                (lbl_1_bss_210.zMinimum <= position->z)) {
                volumeScale =
                    (f32)(lbl_1_bss_210.volumeMaximum -
                          lbl_1_bss_210.volumeMinimum) /
                    (lbl_1_bss_210.zMaximum - lbl_1_bss_210.zMinimum);
                positionOffset = position->z - lbl_1_bss_210.zMinimum;
                positionOffset *= volumeScale;
                volume = (s16)positionOffset;
                volume += lbl_1_bss_210.volumeMinimum;
            } else if (lbl_1_bss_210.zMaximum < position->z) {
                volume = lbl_1_bss_210.volumeMaximum;
            } else if (lbl_1_bss_210.zMinimum > position->z) {
                volume = lbl_1_bss_210.volumeMinimum;
            }
        }
        if ((spatialMode == 2) || (spatialMode == 3) || (spatialMode == 4) || (spatialMode == 5)) {
            pan = lbl_1_bss_210.panMinimum +
                  (lbl_1_bss_210.panMaximum -
                   lbl_1_bss_210.panMinimum) / 2;
        }
        if ((spatialMode == 1) || (spatialMode == 3)) {
            volume = lbl_1_bss_210.volumeMinimum +
                     (lbl_1_bss_210.volumeMaximum -
                      lbl_1_bss_210.volumeMinimum) / 2;
        }
        if (spatialMode == 4) {
            volume = lbl_1_bss_210.volumeMinimum;
        }
        if ((spatialMode == 5) || (spatialMode == 6) || (spatialMode == 7)) {
            volume = lbl_1_bss_210.volumeMaximum;
        }
        if (spatialMode == 7) {
            pan = lbl_1_bss_210.panCenter;
        }

        /* Starting a cue replaces this slot's handle without stopping any earlier sound. */
        lbl_1_bss_210.soundState[soundSlot] = 1;
        lbl_1_bss_210.soundHandle[soundSlot] = HuAudFXPlayVolPan(
            lbl_1_data_F6[soundSlot], volume, pan);
    }
}

/* Intro, path movement, model motion and ending-cue effects. */
s16 lbl_1_data_F6[5] = {
    MSM_SE_M662_INTRO, MSM_SE_M662_PATH_MOVE, MSM_SE_M662_MODEL_MOTION, MSM_SE_M662_CUE, 0
};

u16 lbl_1_bss_C60;
s16 lbl_1_bss_C58[4];
HUPROCESS *lbl_1_bss_C50[2];
M662CameraState lbl_1_bss_B08;
M662Light lbl_1_bss_ACC;
M662Shadow lbl_1_bss_A7C;
M662ParticleModule lbl_1_bss_A74;
M662Model lbl_1_bss_4B4[46];
M662Motion lbl_1_bss_3F4[2];
M662Player lbl_1_bss_2F4[2];
M662Sequence lbl_1_bss_210;
M662Puzzle lbl_1_bss_D0[4];
M662Bounds lbl_1_bss_0[13];
