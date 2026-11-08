/* Game setup: players, actors, stage models, background models, timer, and the opening camera
 * sweep. */
#include "REL/m663dll/transition.h"
#include "dolphin/types.h"
#include "REL/m663dll/player.h"
#include "REL/m663dll/camera.h"
#include "REL/m663dll/input.h"
#include "REL/m663dll/models.h"
#include "REL/m663dll/tutorial.h"

#include "game/gamework.h"

#include "game/mg/actman.h"

#include "game/hu3d.h"

#include "game/memory.h"

#include "game/charman.h"

#include "game/pad.h"

#include "dolphin/gx.h"

#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/wipe.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "dolphin/mtx.h"
#include "REL/m663dll/actors.h"
#include <math.h>
#include "REL/m663dll/score.h"
#include "REL/m663dll/effects.h"
#include "REL/m663dll.h"

extern s32 lbl_1_data_160[];

extern void fn_1_A4E8(void);

extern void fn_1_A85C(void);

extern void fn_1_ABF0(void);

extern void fn_1_AEAC(void);

s32 fn_1_9EC8(void)
{
    MGACTOR_PARAM actorParam;
    u32 rumbleTable[4] = { PAD_CHAN0_BIT, PAD_CHAN1_BIT, PAD_CHAN2_BIT, PAD_CHAN3_BIT };
    s32 playerIndex;
    s32 rumbleEnabled;
    BOOL vibrateEnabled;

    fn_1_408();
    fn_1_4720();
    vibrateEnabled = lbl_80265E30.vibrateF;
    if (vibrateEnabled == 1) {
        rumbleEnabled = 1;
    } else {
        rumbleEnabled = 0;
    }
    lbl_1_bss_424.rumbleEnabled = rumbleEnabled;
    if (((rumbleTable[(s8)lbl_1_bss_4F0[0].padNo] & fn_80006FC0()) == 0 &&
         lbl_1_bss_4F0[0].playerType == 0) ||
        ((rumbleTable[(s8)lbl_1_bss_4F0[1].padNo] & fn_80006FC0()) == 0 &&
         lbl_1_bss_4F0[1].playerType == 0)) {
        lbl_1_bss_424.rumbleEnabled = 0;
    }
    if (lbl_1_bss_424.rumbleEnabled == 0) {
        fn_1_4FA8();
    }
    fn_1_A4E8();
    fn_1_A85C();
    fn_1_ABF0();
    fn_1_AEAC();

    lbl_1_bss_750[2].modelId =
        fn_80025A04(fn_80008DD0(lbl_1_data_160[0], 0x10000000, 2));
    fn_800269E8(lbl_1_bss_750[2].modelId, (1 << 0));
    fn_80026B64(lbl_1_bss_750[2].modelId, 1);
    fn_80026B8C(lbl_1_bss_750[2].modelId, 0);
    lbl_1_bss_750[2].position.x = 0.0f;
    lbl_1_bss_750[2].position.y = 0.0f;
    lbl_1_bss_750[2].position.z = 0.0f;
    lbl_1_bss_750[2].rotation.x = 0.0f;
    lbl_1_bss_750[2].rotation.y = 0.0f;
    lbl_1_bss_750[2].rotation.z = 0.0f;
    lbl_1_bss_418.modelId = lbl_1_bss_750[2].modelId;
    fn_800804E8(&lbl_1_bss_418.modelId, 1, 0xA);

    actorParam.height = 150.0f;
    actorParam.radius = 50.0f;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.narrowHook = (COL_NARROW_HOOK)fn_1_94E0;
    actorParam.correctHookParam = 0;
    actorParam.correctHook = (MGACTOR_CORRECT_HOOK)fn_1_959C;
    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        actorParam.param = playerIndex;
        lbl_1_bss_418.players[playerIndex] = fn_80080728(lbl_1_bss_4F0[playerIndex].playerNo,
                                &actorParam, 4, 1, -0x3F, NULL);
        lbl_1_bss_750[playerIndex].modelId = lbl_1_bss_418.players[playerIndex]->actor->mdlId;
        lbl_1_bss_418.players[playerIndex]->actor->pos.x = lbl_1_bss_4F0[playerIndex].position.x;
        lbl_1_bss_418.players[playerIndex]->actor->pos.y = lbl_1_bss_4F0[playerIndex].position.y;
        lbl_1_bss_418.players[playerIndex]->actor->pos.z = lbl_1_bss_4F0[playerIndex].position.z;
        lbl_1_bss_418.players[playerIndex]->actor->rotY = lbl_1_bss_4F0[playerIndex].rotation.y;
        fn_80026E44(lbl_1_bss_750[playerIndex].modelId);
        fn_80026FCC(lbl_1_bss_418.players[playerIndex]->actor->mdlId);
        fn_80026B8C(lbl_1_bss_418.players[playerIndex]->actor->mdlId, 1);
        fn_8002D414(lbl_1_bss_750[playerIndex].modelId, 20.0f);
        fn_8005F77C(lbl_1_bss_4F0[playerIndex].characterNo, 0);
        lbl_1_bss_4F0[playerIndex].transformMode = 1;
        lbl_1_bss_4F0[playerIndex].inputMode = 1;
        lbl_1_bss_4F0[playerIndex].objectName =
                fn_8005C0F0(lbl_1_bss_4F0[playerIndex].characterNo, 4, 0);
        fn_8005DFC0(lbl_1_bss_4F0[playerIndex].characterNo, 0);
        }

    fn_1_3588();
    fn_1_EF0();
    fn_1_C4F8();
    fn_1_2538();
    fn_1_D83C();
    lbl_1_bss_424.timer =
        fn_1_39F4(0, 0x1E, 0, 44.0f, 420.0f);
    fn_1_8528(0);
    return 0;
}

extern s32 fn_1_B590(void);

/* Fade-in phase: runs the opening camera sweep and one frame of input, gameplay, actor and
 * model updates; returns 1 when the sweep is done. */
s16 fn_1_A464(void) {
    s16 result = 0;
    result = fn_1_B590();
    fn_1_754();
    fn_1_69E8();
    fn_1_3588();
    fn_1_2A00();
    return result;
}

/* Start phase: continues the opening camera sweep and runs the same frame updates; always
 * returns 0. */
int fn_1_A4A8(void) {
    int result = 0;
    fn_1_B590();
    fn_1_754();
    fn_1_69E8();
    fn_1_3588();
    fn_1_2A00();
    return result;
}

/* Creates models 3-12 from the module data list at the origin with motion stopped at frame 0
 * and sets their camera and layers; model 12 stays hidden and is marked for shadow-map
 * rendering, and models 7 and 11 play their motions. */
void fn_1_A4E8(void) {
    s16 dataIndex;
    s16 modelIndex;

    modelIndex = 3;
    dataIndex = 1;
    while (modelIndex <= 0xC) {
        lbl_1_bss_750[modelIndex].modelId = fn_80025A04(fn_80008DD0(lbl_1_data_160[dataIndex], 0x10000000, 2));
        fn_800269E8(lbl_1_bss_750[modelIndex].modelId, (1 << 0));
        fn_80026A44(lbl_1_bss_750[modelIndex].modelId, (1 << 0));
        fn_80026B64(lbl_1_bss_750[modelIndex].modelId, 1U);
        fn_80026B8C(lbl_1_bss_750[modelIndex].modelId, 0);
        if (modelIndex == 6) {
            fn_80026B8C(lbl_1_bss_750[modelIndex].modelId, 7);
        }
        if ((modelIndex == 0xA) || (modelIndex == 0xB) || (modelIndex == 0xC)) {
            fn_80026B8C(lbl_1_bss_750[modelIndex].modelId, 1);
        }
        if (modelIndex == 0xC) {
            fn_800269E8(lbl_1_bss_750[modelIndex].modelId, (1 << 0));
            fn_80026FFC(lbl_1_bss_750[modelIndex].modelId);
        }
        fn_80026A44(lbl_1_bss_750[modelIndex].modelId, ((1 << 30) | (1 << 0)));
        fn_8002D86C(lbl_1_bss_750[modelIndex].modelId, 0.0f);
        fn_8002D414(lbl_1_bss_750[modelIndex].modelId, 0.0f);
        if ((modelIndex == 7) || (modelIndex == 0xB)) {
            fn_800269E8(lbl_1_bss_750[modelIndex].modelId, ((1 << 30) | (1 << 0)));
            fn_8002D86C(lbl_1_bss_750[modelIndex].modelId, 1.0f);
        }
        lbl_1_bss_750[modelIndex].position.x = (f32) 0.0f;
        lbl_1_bss_750[modelIndex].position.y = (f32) 0.0f;
        lbl_1_bss_750[modelIndex].position.z = (f32) 0.0f;
        lbl_1_bss_750[modelIndex].rotation.x = (f32) 0.0f;
        lbl_1_bss_750[modelIndex].rotation.y = (f32) 0.0f;
        lbl_1_bss_750[modelIndex].rotation.z = (f32) 0.0f;
        modelIndex += 1;
        dataIndex += 1;
    }
}

extern char lbl_1_data_1E4[5][32];

void fn_1_A85C(void)
{
    Vec setupVectors[5] = {
        {-651.0f, 0.0f, -211.0f},
        {-403.0f, 0.0f, -554.0f},
        {0.0f, 0.0f, -686.0f},
        {403.0f, 0.0f, -554.0f},
        {651.0f, 0.0f, -211.0f}
    };
    s16 modelIndex = 0xD;
    s16 dataIndex = 0xB;

    while (modelIndex <= 0x16) {
        lbl_1_bss_750[modelIndex].modelId = fn_80025A04(
            fn_80008DD0(lbl_1_data_160[dataIndex],
                                 0x10000000, 2));
        fn_800269E8(
            lbl_1_bss_750[modelIndex].modelId, (1 << 0));
        fn_80026A44(
            lbl_1_bss_750[modelIndex].modelId, (1 << 0));
        fn_80026B64(
            lbl_1_bss_750[modelIndex].modelId, 1);
        fn_80026B8C(
            lbl_1_bss_750[modelIndex].modelId, 0);
        fn_80026A44(
            lbl_1_bss_750[modelIndex].modelId,
            ((1 << 30) | (1 << 0)));
        fn_8002D86C(
            lbl_1_bss_750[modelIndex].modelId,
            0.0f);
        fn_8002D414(
            lbl_1_bss_750[modelIndex].modelId,
            0.0f);
        lbl_1_bss_750[modelIndex].position.x = 0.0f;
        lbl_1_bss_750[modelIndex].position.y = 0.0f;
        lbl_1_bss_750[modelIndex].position.z = 0.0f;
        lbl_1_bss_750[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_750[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_750[modelIndex].rotation.z = 0.0f;
        modelIndex++;
        dataIndex++;
    }

    {
        for (modelIndex = 0; modelIndex < 5; modelIndex++) {
            M663TutorialWork *tutorial = &lbl_1_bss_378[modelIndex];
            tutorial->phase = 2;
            tutorial->actorSlot = modelIndex + 0xD;
            tutorial->motionSlot = modelIndex + 0x12;
            tutorial->objectName = lbl_1_data_1E4[modelIndex];
            tutorial->updateTimer = frandmod(0xFA) + 0x32;
            lbl_1_bss_750[tutorial->motionSlot].position =
                    setupVectors[modelIndex];
        }
    }
}

/* Creates the goal model (23) and models 24-25 at height 300 with motion stopped at frame 0;
 * only the goal model is shown, and each gets its own layer. */
void fn_1_ABF0(void) {
    s16 dataIndex;
    s16 modelIndex;

    modelIndex = 0x17;
    dataIndex = 0x15;
    while (modelIndex <= 0x19) {
        lbl_1_bss_750[modelIndex].modelId = fn_80025A04(fn_80008DD0(lbl_1_data_160[dataIndex], 0x10000000, 2));
        fn_800269E8(lbl_1_bss_750[modelIndex].modelId, (1 << 0));
        if (modelIndex == 0x17) {
            fn_80026A44(lbl_1_bss_750[modelIndex].modelId, (1 << 0));
        }
        fn_80026B64(lbl_1_bss_750[modelIndex].modelId, 1U);
        fn_80026B8C(lbl_1_bss_750[modelIndex].modelId, 3);
        if (modelIndex == 0x19) {
            fn_80026B8C(lbl_1_bss_750[modelIndex].modelId, 1);
        }
        if (modelIndex == 0x18) {
            fn_80026B8C(lbl_1_bss_750[modelIndex].modelId, 2);
        }
        fn_80026A44(lbl_1_bss_750[modelIndex].modelId, ((1 << 30) | (1 << 0)));
        fn_8002D86C(lbl_1_bss_750[modelIndex].modelId, 0.0f);
        fn_8002D414(lbl_1_bss_750[modelIndex].modelId, 0.0f);
        lbl_1_bss_750[modelIndex].position.x = (f32) 0.0f;
        lbl_1_bss_750[modelIndex].position.y = (f32) 300.0f;
        lbl_1_bss_750[modelIndex].position.z = (f32) 0.0f;
        lbl_1_bss_750[modelIndex].rotation.x = (f32) 0.0f;
        lbl_1_bss_750[modelIndex].rotation.y = (f32) 0.0f;
        lbl_1_bss_750[modelIndex].rotation.z = (f32) 0.0f;
        modelIndex += 1;
        dataIndex += 1;
    }
}

extern void fn_1_56E8(HU3D_MODEL *model, Mtx *viewMatrix);

void fn_1_AEAC(void)
{
    Vec effectPositions[6] = {
        {170.0f, 636.0f, 480.0f},
        {62.0f, 833.0f, 354.0f},
        {1184.0f, 643.0f, 546.0f},
        {19.0f, 685.0f, 538.0f},
        {-1817.0f, 739.0f, 523.0f},
        {136.0f, 888.0f, 436.0f}
    };
    f32 effectAngles[6] = {0.7f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    s16 index;
    s16 dataIndex;
    s16 effectIndex;

    index = 0x1A;
    dataIndex = 0x18;
    while (index <= 0x4E) {
        if (index >= 0x1C) {
            dataIndex = 0x19;
        }
        if (index >= 0x4E) {
            dataIndex = 0x1A;
        }
        lbl_1_bss_750[index].modelId = fn_80025A04(
            fn_80008DD0(lbl_1_data_160[dataIndex], 0x10000000, 2));
        fn_800269E8(lbl_1_bss_750[index].modelId, (1 << 0));
        fn_80026B64(lbl_1_bss_750[index].modelId, 1);
        fn_80026B8C(lbl_1_bss_750[index].modelId, 4);
        if (index == 0x1A || index == 0x1B || index == 0x4E) {
            fn_80026A44(lbl_1_bss_750[index].modelId, ((1 << 30) | (1 << 0)));
            fn_8002D86C(lbl_1_bss_750[index].modelId, 0.0f);
            fn_8002D414(lbl_1_bss_750[index].modelId, 0.0f);
        } else {
            fn_80026A44(lbl_1_bss_750[index].modelId, ((1 << 30) | (1 << 6)));
            fn_8002CF70(lbl_1_bss_750[index].modelId, 0.0f);
            fn_8002CF98(lbl_1_bss_750[index].modelId, 0.0f);
        }
        lbl_1_bss_750[index].position.x = 0.0f;
        lbl_1_bss_750[index].position.y = 0.0f;
        lbl_1_bss_750[index].position.z = 0.0f;
        lbl_1_bss_750[index].rotation.x = 0.0f;
        lbl_1_bss_750[index].rotation.y = 0.0f;
        lbl_1_bss_750[index].rotation.z = 0.0f;
        index++;
    }

    index = 0x50;
    dataIndex = 0x1B;
    effectIndex = 0;
    while (index <= 0x55) {

        lbl_1_bss_750[index].modelId = fn_80025A04(
            fn_80008DD0(lbl_1_data_160[dataIndex], 0x10000000, 2));
        fn_800269E8(lbl_1_bss_750[index].modelId, (1 << 0));
        fn_80026B64(lbl_1_bss_750[index].modelId, 1);
        fn_80026B8C(lbl_1_bss_750[index].modelId, 6);
        fn_80026A44(lbl_1_bss_750[index].modelId, ((1 << 30) | (1 << 0)));
        fn_8002D86C(lbl_1_bss_750[index].modelId, 0.0f);
        fn_8002D414(lbl_1_bss_750[index].modelId, 0.0f);
        lbl_1_bss_750[index].position = effectPositions[effectIndex];
        lbl_1_bss_750[index].scale = effectAngles[effectIndex];
        lbl_1_bss_750[index].rotation.x = -19.0f;
        lbl_1_bss_750[index].rotation.y = 0.0f;
        lbl_1_bss_750[index].rotation.z = 0.0f;
        lbl_1_bss_0[effectIndex].playerIndex = effectIndex;
        lbl_1_bss_0[effectIndex].position = effectPositions[effectIndex];
        index++;
        dataIndex++;
        effectIndex++;
    }

    index = 0x4F;
    lbl_1_bss_750[index].modelId = fn_800260F4(fn_1_56E8);
    fn_80026760(lbl_1_bss_750[index].modelId,
                    0.0f, 0.0f, 0.0f);
    fn_800268B0(lbl_1_bss_750[index].modelId,
                      1.0f, 1.0f, 1.0f);
    fn_80026808(lbl_1_bss_750[index].modelId,
                    0.0f, 0.0f, 0.0f);
    fn_80026B8C(lbl_1_bss_750[index].modelId, 6);
    fn_80026B64(lbl_1_bss_750[index].modelId, 1);

}

s32 fn_1_B590(void)
{
    s32 result = 0;
    s32 motionFrames = 0;
    s16 modelIndex;
    s16 effectIndex;

    switch (lbl_1_bss_424.phase) {
    case 0: {
        fn_1_5EBC();
        lbl_1_bss_424.orbitAngle = 250.0f;
        lbl_80266A4C.x = 200.0 *
                   cos((M_PI * (f64)lbl_1_bss_424.orbitAngle) /
                       180.0);
        lbl_80266A4C.y = 600.0f;
        lbl_80266A4C.z = (f32)(800.0 +
                         (200.0 *
                          sin((M_PI * (f64)lbl_1_bss_424.orbitAngle) /
                              180.0)));
        lbl_802C028C = 700.0f;
        lbl_80266A40.x = -25.0f;
        lbl_80266A40.y = -90.0f - lbl_1_bss_424.orbitAngle;
        for (modelIndex = 0x50, effectIndex = 0; modelIndex <= 0x55;
             modelIndex++, effectIndex++) {
            lbl_1_bss_750[modelIndex].position.y = lbl_1_bss_0[effectIndex].position.y - 200.0f;
            lbl_1_bss_750[modelIndex].position.z = lbl_1_bss_0[effectIndex].position.z - 450.0f;
            lbl_1_bss_750[modelIndex].rotation.x = -9.0f;
        }
        fn_1_6018(0, 5, NULL);
        lbl_1_bss_424.phase++;
        /* fall through */
    }
    case 1: {
        if (lbl_1_bss_424.phaseTimer++ > 0x3C) {
            lbl_1_bss_424.phaseTimer = 0;
            lbl_1_bss_424.phase++;
        }
        break;
    }
    case 2:
        lbl_80266A4C.x = (f32)(200.0 *
                         cos((M_PI * (f64)lbl_1_bss_424.orbitAngle) /
                             180.0));
        lbl_80266A4C.z = (f32)(800.0 +
                         (200.0 *
                          sin((M_PI * (f64)lbl_1_bss_424.orbitAngle) /
                              180.0)));
        lbl_80266A40.y = -90.0f - lbl_1_bss_424.orbitAngle;
        lbl_1_bss_424.orbitAngle += 0.4f;
        if (lbl_1_bss_424.orbitAngle > 290.0f) {
            lbl_80266A40.y += 360.0f;
            lbl_1_bss_424.phase++;
        }
        break;
    case 3: {
        if (lbl_1_bss_424.phaseTimer++ > 0xA) {
            lbl_1_bss_424.phaseTimer = 0;
            lbl_1_bss_424.phase++;
        }
        break;
    }
    case 4: {
        f32 step = 40.0f;

        motionFrames = fn_1_38D4(&lbl_80266A4C.x, step, 68.0f, 0.0f);
        motionFrames += fn_1_38D4(&lbl_80266A4C.y, step, 600.0f, 166.0f);
        motionFrames += fn_1_38D4(&lbl_80266A4C.z, step, 612.0f, 25.0f);
        motionFrames += fn_1_38D4(&lbl_80266A40.x, step, -25.0f, -31.7f);
        motionFrames += fn_1_38D4(&lbl_80266A40.y, step, -20.0f, 0.0f);
        motionFrames += fn_1_38D4(&lbl_80266A40.z, step, 0.0f, 0.0f);
        motionFrames += fn_1_38D4(&lbl_802C028C, step, 700.0f, 2483.3f);
        for (modelIndex = 0x50, effectIndex = 0; modelIndex <= 0x55;
             modelIndex++, effectIndex++) {
            motionFrames += fn_1_38D4(&lbl_1_bss_750[modelIndex].position.y, step,
                                     lbl_1_bss_0[effectIndex].position.y - 200.0f,
                                     lbl_1_bss_0[effectIndex].position.y);
            motionFrames += fn_1_38D4(&lbl_1_bss_750[modelIndex].position.z, step,
                                     lbl_1_bss_0[effectIndex].position.z - 450.0f,
                                     lbl_1_bss_0[effectIndex].position.z);
            motionFrames += fn_1_38D4(&lbl_1_bss_750[modelIndex].rotation.x, step, -9.0f,
                                     -19.0f);
        }
        if ((s16)motionFrames == 0) {
            lbl_1_bss_424.phase++;
        }
        break;
    }
    case 5:
        result = 1;
        break;
    }
    return result;
}

s32 lbl_1_data_160[33] = {
    DATANUM(0x7a0000, 0),
    DATANUM(0x7a0000, 1),
    DATANUM(0x7a0000, 2),
    DATANUM(0x7a0000, 3),
    DATANUM(0x7a0000, 4),
    DATANUM(0x7a0000, 5),
    DATANUM(0x7a0000, 6),
    DATANUM(0x7a0000, 7),
    DATANUM(0x7a0000, 8),
    DATANUM(0x7a0000, 9),
    DATANUM(0x7a0000, 10),
    DATANUM(0x7a0000, 11),
    DATANUM(0x7a0000, 12),
    DATANUM(0x7a0000, 13),
    DATANUM(0x7a0000, 14),
    DATANUM(0x7a0000, 15),
    DATANUM(0x7a0000, 16),
    DATANUM(0x7a0000, 17),
    DATANUM(0x7a0000, 18),
    DATANUM(0x7a0000, 19),
    DATANUM(0x7a0000, 20),
    DATANUM(0x7a0000, 21),
    DATANUM(0x7a0000, 22),
    DATANUM(0x7a0000, 23),
    DATANUM(0x7a0000, 24),
    DATANUM(0x7a0000, 25),
    DATANUM(0x7a0000, 26),
    DATANUM(0x7a0000, 27),
    DATANUM(0x7a0000, 28),
    DATANUM(0x7a0000, 29),
    DATANUM(0x7a0000, 30),
    DATANUM(0x7a0000, 31),
    DATANUM(0x7a0000, 32)
};
char lbl_1_data_1E4[5][32] = {
    "nop_A_body", "nop_B_body", "nop_C_body", "nop_D_body", "nop_E_body"
};
