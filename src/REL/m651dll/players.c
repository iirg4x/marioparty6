/* Black Hole updates its two duel characters and animates the meteor scene. */
#include "REL/m651dll.h"
#include "game/main.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/pad.h"
#include "datadir_enum.h"
#include "math.h"

M651Player lbl_1_bss_A8[2];
M651Work64 lbl_1_bss_64;
M651Work14 lbl_1_bss_14;
/* Shared speed used to move a character toward the central scene target. */
float lbl_1_bss_10;
/* Character motion resource IDs passed to CharModelMotListCreate, terminated by zero. */
unsigned int lbl_1_data_70[6] = {
    DATANUM(0x8e0000, 33), DATANUM(0x930000, 26), DATANUM(0x8e0000, 118),
    DATANUM(0x930000, 27), DATANUM(0x930000, 34), 0
};
/* Base CPU input delay by configured difficulty, in frames; each reset may subtract one frame. */
int lbl_1_data_88[4] = { 15, 10, 8, 6 };
/* Per-frame movement toward the finish depth, increased gradually during the sequence. */
float lbl_1_data_98 = 0.020000001f;
/* Scene-space target reached by a character selected as the winner. */
HuVecF lbl_1_data_9C = { 0.0f, 40.0f, 2300.0f };
/* Starting motion speed for the central scene model's animation. */
float lbl_1_data_A8 = 1.0f;
/* Handle for the sustained scene sound effect, played during the opening and faded when a player
 * reaches the result point. */
s32 lbl_1_data_AC = -1;
/* Pitch for the sustained scene sound effect, raised by 10 each scene update until it reaches
 * zero. */
s32 lbl_1_data_B0 = -8192;

/* Called by the duel-phase callback in lbl_1_data_14; raises approach speeds and decreases the
 * rotation offset after a finish. */
void fn_1_4A0(void)
{
    lbl_1_data_98 += 0.016666668f;
    lbl_1_bss_14.meteorMotionSpeed += 0.02f;
    if (lbl_1_bss_14.meteorMotionSpeed > 2.6f) {
        lbl_1_bss_14.meteorMotionSpeed = 2.6f;
    }
    /* This accumulator advances, but the scene update never reads it. */
    lbl_1_bss_64.unusedSceneSpeedAccumulator += 0.002;
    if (lbl_1_bss_A8[0].reachedFinish == 1 || lbl_1_bss_A8[1].reachedFinish == 1) {
        lbl_1_bss_64.sceneRotationSpeedOffset -= 0.01;
        if (lbl_1_bss_64.sceneRotationSpeedOffset > 0.0f) {
            lbl_1_bss_64.sceneRotationSpeedOffset = 0.0f;
        }
    } else {
        lbl_1_bss_64.sceneRotationSpeedOffset += 0.0001;
        if (lbl_1_bss_64.sceneRotationSpeedOffset > 1.2f) {
            lbl_1_bss_64.sceneRotationSpeedOffset = 1.2f;
        }
    }
}

/* Called by the result-phase callback in lbl_1_data_14 at frame 0; hides all meteor and display
 * models. */
void fn_1_62C(void)
{
    int meteorIndex;
    for (meteorIndex = 0; meteorIndex < 5; meteorIndex++) {
        fn_800269E8(lbl_1_bss_14.meteorModels[meteorIndex], (1 << 0));
        fn_800269E8(lbl_1_bss_14.meteorDisplayModels[meteorIndex], (1 << 0));
    }
}

/* Replaces the meteor update callback after all five motions end during result cleanup; hides each
 * pair as its motion ends. */
void fn_1_69C(OMOBJ *obj)
{
    int meteorIndex;
    for (meteorIndex = 0; meteorIndex < 5; meteorIndex++) {
        if (fn_8002D75C(lbl_1_bss_14.meteorModels[meteorIndex]) == 1) {
            fn_800269E8(lbl_1_bss_14.meteorModels[meteorIndex], (1 << 0));
            fn_800269E8(lbl_1_bss_14.meteorDisplayModels[meteorIndex], (1 << 0));
        }
    }
}

/* Meteor object callback created by fn_1_D28; runs falling motions and approach effects, then
 * switches to result cleanup after all five motions end. */
void fn_1_72C(OMOBJ *obj)
{
    int meteorIndex;
    int endedMeteorCount = 0;
    int motionStartTime;

    for (meteorIndex = 0; meteorIndex < 5; meteorIndex++) {
        if (fn_8002D75C(lbl_1_bss_14.meteorModels[meteorIndex]) == 1) {
            if (lbl_1_bss_14.resultStarted == 1) {
                endedMeteorCount++;
                if (endedMeteorCount == 5) {
                    obj->objFunc = fn_1_69C;
                    return;
                }
                continue;
            }
            fn_800269E8(lbl_1_bss_14.meteorModels[meteorIndex], (1 << 0));
            fn_800269E8(lbl_1_bss_14.meteorDisplayModels[meteorIndex], (1 << 0));
            lbl_1_bss_14.meteorDelayFrames[meteorIndex]--;
            if (lbl_1_bss_14.meteorDelayFrames[meteorIndex] == 0) {
                motionStartTime = rand8() % 64;
                lbl_1_bss_14.approachEffectTriggered[meteorIndex] = 0;
                if (meteorIndex != 4) {
                    lbl_1_bss_14.meteorDelayFrames[meteorIndex] = rand8() % 256 + 1;
                    fn_80026A44(lbl_1_bss_14.meteorModels[meteorIndex], (1 << 0));
                    fn_80026A44(lbl_1_bss_14.meteorDisplayModels[meteorIndex],
                                       (1 << 0));
                    fn_8002D86C(lbl_1_bss_14.meteorModels[meteorIndex],
                                       lbl_1_bss_14.meteorMotionSpeed);
                    fn_8002D414(lbl_1_bss_14.meteorModels[meteorIndex], motionStartTime);
                    fn_8002D86C(lbl_1_bss_14.meteorDisplayModels[meteorIndex],
                                       lbl_1_bss_14.meteorMotionSpeed);
                    fn_8002D414(lbl_1_bss_14.meteorDisplayModels[meteorIndex],
                                      motionStartTime);
                }
            }
        } else {
            Mtx mtx;
            char *names[5] = { "meteo-astreS", "meteo-astreM", "meteo-astreLL", "meteo-astreL",
                               "kuriboo_null" };
            HuVecF pos;
            HuVecF scale;

            fn_80023398(lbl_1_bss_14.meteorModels[meteorIndex], names[meteorIndex], mtx);
            Hu3DMtxTransGet(mtx, &pos);
            Hu3DMtxScaleGet(mtx, &scale);
            if (80.0f * lbl_1_bss_64.firstBackgroundScale > pos.z &&
                lbl_1_bss_14.approachEffectTriggered[meteorIndex] == 0 && meteorIndex != 4) {
                fn_80026A44(lbl_1_bss_14.meteorApproachModels[meteorIndex],
                                   ((1 << 30) | (1 << 1)));
                fn_80026790(lbl_1_bss_14.meteorApproachModels[meteorIndex], &pos);
                fn_800268B0(lbl_1_bss_14.meteorApproachModels[meteorIndex], 1.0f, 1.0f, 1.0f);
                fn_800269E8(lbl_1_bss_14.meteorModels[meteorIndex], (1 << 0));
                fn_800269E8(lbl_1_bss_14.meteorDisplayModels[meteorIndex], (1 << 0));
                fn_8002D414(lbl_1_bss_14.meteorApproachModels[meteorIndex], 0.0f);
                lbl_1_bss_14.approachEffectTriggered[meteorIndex] = 1;
            }
        }
        if (meteorIndex == 4 &&
            (lbl_1_bss_A8[0].pos.z < 1700.0f || lbl_1_bss_A8[1].pos.z < 1700.0f) &&
            lbl_1_bss_64.finalMeteorCheckDone == 0) {
            lbl_1_bss_64.finalMeteorCheckDone = 1;
            if (rand8() % 100 < 40) {
                fn_80026A44(lbl_1_bss_14.meteorModels[4], ((1 << 30) | (1 << 1)));
                fn_8002D86C(lbl_1_bss_14.meteorModels[4], lbl_1_bss_14.meteorMotionSpeed);
            }
        }
    }
}

/* Layer-6 draw hook registered by fn_1_D28; copies the first four meteor transforms to their linked
 * display models. */
void fn_1_C20(s16 layerNo)
{
    int meteorIndex;
    for (meteorIndex = 0; meteorIndex < 4; meteorIndex++) {
        Mtx mtx;
        char *meteorJointNames[5] = { "meteo-astreS", "meteo-astreM", "meteo-astreLL",
                                      "meteo-astreL", "kuriboo_null" };
        HuVecF pos;
        HuVecF rot;
        HuVecF scale;

        fn_80023398(lbl_1_bss_14.meteorModels[meteorIndex], meteorJointNames[meteorIndex],
                           mtx);
        Hu3DMtxTransGet(mtx, &pos);
        Hu3DMtxRotGet(mtx, &rot);
        Hu3DMtxScaleGet(mtx, &scale);
        fn_80026790(lbl_1_bss_14.meteorDisplayModels[meteorIndex], &pos);
        fn_80026838(lbl_1_bss_14.meteorDisplayModels[meteorIndex], &rot);
        fn_800268E0(lbl_1_bss_14.meteorDisplayModels[meteorIndex], &scale);
    }
}

/* Called during scene setup by fn_1_3F30; loads meteor, approach-effect, linked-display, and hooked
 * models, assigns motions, then creates the meteor update object. */
void fn_1_D28(void)
{
    int meteorIndex;
    OMOBJ *obj;
    HU3D_MOTIONID motionId;
    u32 meteorModelDataIds[5] = { DATANUM(0x6e0000, 8), DATANUM(0x6e0000, 9),
                                  DATANUM(0x6e0000, 10), DATANUM(0x6e0000, 11),
                                  DATANUM(0x6e0000, 16) };
    u32 meteorMotionDataIds[5] = { DATANUM(0x6e0000, 18), DATANUM(0x6e0000, 19),
                                   DATANUM(0x6e0000, 20), DATANUM(0x6e0000, 21),
                                   DATANUM(0x6e0000, 17) };

    for (meteorIndex = 0; meteorIndex < 5; meteorIndex++) {
        if (meteorIndex == 0) {
            lbl_1_bss_14.meteorDisplayModels[0] = fn_80025A04(
                fn_80008DD0(DATANUM(0x6e0000, 14), 0x10000000, 2));
        } else {
            lbl_1_bss_14.meteorDisplayModels[meteorIndex] =
                fn_80025D4C(lbl_1_bss_14.meteorDisplayModels[0]);
        }
        fn_80026B8C(lbl_1_bss_14.meteorDisplayModels[meteorIndex], 6);
        fn_800269E8(lbl_1_bss_14.meteorDisplayModels[meteorIndex], ((1 << 30) | (1 << 0)));
        fn_80025918(6, fn_1_C20);
    }
    for (meteorIndex = 0; meteorIndex < 5; meteorIndex++) {
        if (meteorIndex == 0) {
            lbl_1_bss_14.meteorApproachModels[0] = fn_80025A04(
                fn_80008DD0(DATANUM(0x6e0000, 15), 0x10000000, 2));
        } else {
            lbl_1_bss_14.meteorApproachModels[meteorIndex] =
                fn_80025D4C(lbl_1_bss_14.meteorApproachModels[0]);
        }
        fn_80026B8C(lbl_1_bss_14.meteorApproachModels[meteorIndex], 5);
        fn_80026A44(lbl_1_bss_14.meteorApproachModels[meteorIndex], ((1 << 30) | (1 << 0)));
        fn_800269E8(lbl_1_bss_14.meteorApproachModels[meteorIndex], ((1 << 30) | (1 << 1)));
        lbl_1_bss_14.approachEffectTriggered[meteorIndex] = 0;
    }
    lbl_1_bss_14.hookedMeteorModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x6e0000, 12), 0x10000000, 2));
    motionId =
        fn_80030458(lbl_1_bss_14.hookedMeteorModel,
                        fn_80008DD0(DATANUM(0x6e0000, 13), 0x10000000, 2));
    fn_8002C95C(lbl_1_bss_14.hookedMeteorModel, motionId);
    fn_800269E8(lbl_1_bss_14.hookedMeteorModel, ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_14.meteorDisplayModels[4], (1 << 0));
    for (meteorIndex = 0; meteorIndex < 5; meteorIndex++) {
        lbl_1_bss_14.meteorModels[meteorIndex] = fn_80025A04(
            fn_80008DD0(meteorModelDataIds[meteorIndex], 0x10000000, 2));
        fn_80026B8C(lbl_1_bss_14.meteorModels[meteorIndex], 5);
        lbl_1_bss_14.meteorMotions[meteorIndex] = fn_80030458(
            lbl_1_bss_14.meteorModels[meteorIndex],
            fn_80008DD0(meteorMotionDataIds[meteorIndex], 0x10000000, 2));
        lbl_1_bss_14.meteorDelayFrames[meteorIndex] = rand8() % 4 + 1;
        fn_80026A44(lbl_1_bss_14.meteorModels[meteorIndex], ((1 << 30) | (1 << 0)));
        fn_80026A44(lbl_1_bss_14.meteorModels[meteorIndex], (1 << 0));
        fn_8002C95C(lbl_1_bss_14.meteorModels[meteorIndex],
                      lbl_1_bss_14.meteorMotions[meteorIndex]);
        fn_8002D414(lbl_1_bss_14.meteorModels[meteorIndex], rand8() % 64);
    }
    fn_800269E8(lbl_1_bss_14.meteorModels[4], ((1 << 30) | (1 << 1)));
    fn_80026A44(lbl_1_bss_14.meteorModels[4], ((1 << 30) | (1 << 0)));
    fn_8002741C(lbl_1_bss_14.meteorModels[4], "kuriboo_null", lbl_1_bss_14.hookedMeteorModel);
    obj = fn_8003AB38(fn_1_A0(), 336, 0, 0, 0, fn_1_72C);
    obj->data = &lbl_1_bss_14;
    lbl_1_bss_14.resultStarted = 0;
    lbl_1_bss_14.meteorMotionSpeed = 1.0f;
}

/* Called by fn_1_3F30 during scene setup; creates the duel camera and sets its perspective,
 * position, and 640 by 480 viewport. */
void fn_1_1220(void)
{
    Hu3DCameraCreate((1 << 0));
    Hu3DCameraPerspectiveSet((1 << 0), 45.0f, 20.0f, 15000.0f, 1.2f);
    Hu3DCameraPosSet((1 << 0), 0.0f, 300.0f, 3100.0f, 0.0f, 1.0f, 0.0f, 0.0f, -100.0f, 0.0f);
    Hu3DCameraViewportSet((1 << 0), 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
}

/* Called by fn_1_3F30 during scene setup; creates the static white light used by the meteor and
 * character models. */
void fn_1_1344(void)
{
    HU3D_LIGHTID lightId;

    lightId = Hu3DGLightCreate(0.0f, 1000.0f, 1000.0f, 0.0f, -1.0f, -1.0f, 255, 255, 255);
    Hu3DGLightStaticSet(lightId, TRUE);
    Hu3DGLightInfinitytSet(lightId);
}

/* Called during scene setup by fn_1_3F30; creates one character object for each configured player
 * assigned to group 0 or 1. */
void fn_1_13D8(void)
{
    int groupNo;
    int playerNo;
    OMOBJ *obj;
    int zeroReserved = 0; /* Initialized but not used by character setup. */
    u32 modelData[2] = { DATANUM(0x6e0000, 22), DATANUM(0x6e0000, 24) };
    u32 motionData[2] = { DATANUM(0x6e0000, 23), DATANUM(0x6e0000, 25) };

    for (playerNo = 0; playerNo < 4; playerNo++) {
        groupNo = lbl_80265728[playerNo].grpNo;
        if (groupNo == 0 || groupNo == 1) {
            fn_1_10C(playerNo);
            lbl_1_bss_A8[groupNo].groupNo = groupNo;
            lbl_1_bss_A8[groupNo].playerNo = playerNo;
            lbl_1_bss_A8[groupNo].characterNo = lbl_80265728[playerNo].charNo;
            lbl_1_bss_A8[groupNo].padNo = lbl_80265728[playerNo].padNo;
            lbl_1_bss_A8[groupNo].modelId =
                fn_8005C4E4(lbl_1_bss_A8[groupNo].characterNo, 1, lbl_1_data_70,
                                       lbl_1_bss_A8[groupNo].motionIds);
            fn_8005BE80(lbl_1_bss_A8[groupNo].characterNo, lbl_1_bss_A8[groupNo].motionIds[1]);
            fn_80026B64(lbl_1_bss_A8[groupNo].modelId, (1 << 0));
            fn_800269E8(lbl_1_bss_A8[groupNo].modelId, ((1 << 30) | (1 << 0)));
            lbl_1_bss_A8[groupNo].resultHookModel = fn_80025A04(
                fn_80008DD0(modelData[groupNo], 0x10000000, 2));
            lbl_1_bss_A8[groupNo].resultHookMotion = fn_80030458(
                lbl_1_bss_A8[groupNo].resultHookModel,
                fn_80008DD0(motionData[groupNo], 0x10000000, 2));
            fn_8002C95C(lbl_1_bss_A8[groupNo].resultHookModel,
                          lbl_1_bss_A8[groupNo].resultHookMotion);
            fn_80026A44(lbl_1_bss_A8[groupNo].resultHookModel, ((1 << 30) | (1 << 0)));
            fn_800269E8(lbl_1_bss_A8[groupNo].resultHookModel, ((1 << 30) | (1 << 1)));
            obj = fn_8003AB38(fn_1_A0(), 400, 0, 0, 0, fn_1_18DC);
            obj->data = &lbl_1_bss_A8[groupNo];
            lbl_1_bss_A8[groupNo].pos.x = -100.0f + 200.0f * groupNo;
            lbl_1_bss_A8[groupNo].pos.y = 0.0f;
            lbl_1_bss_A8[groupNo].pos.z = 2000.0f;
            fn_80026790(lbl_1_bss_A8[groupNo].modelId, &lbl_1_bss_A8[groupNo].pos);
            lbl_1_bss_A8[groupNo].movementBoostFrames = 0;
            lbl_1_bss_A8[groupNo].reachedFinish = 0;
            lbl_1_bss_A8[groupNo].finishResolved = 0;
            lbl_1_bss_A8[groupNo].inputReceived = 0;
            lbl_1_bss_A8[groupNo].crossingYaw = 0.0f;
            lbl_1_bss_A8[groupNo].isCpu = lbl_80265728[playerNo].type;
            lbl_1_bss_A8[groupNo].cpuDifficulty = lbl_80265728[playerNo].comDif;
            lbl_1_bss_A8[groupNo].cpuInputDelayFrames =
                (rand8() % 2 - 1) + lbl_1_data_88[lbl_1_bss_A8[groupNo].cpuDifficulty];
            lbl_1_bss_A8[groupNo].effectHandle = -1;
        }
    }
}

/* Initial callback assigned when a player object is created; hands it to the sequence-wait callback
 * on its first update. */
void fn_1_18DC(OMOBJ *obj)
{
    obj->objFunc = fn_1_18EC;
}

/* Player-object callback after fn_1_18DC; waits for duel mode 5 before enabling input and movement
 * updates. */
void fn_1_18EC(OMOBJ *obj)
{
    if (fn_80071A8C() == 5) {
        obj->objFunc = fn_1_192C;
    }
}

/* Active player-object callback after fn_1_18EC; reads input, moves the character, and handles its
 * finish sequence. */
void fn_1_192C(OMOBJ *obj)
{
    M651Player *work = obj->data;
    int input = 0;

    if (work->isCpu) {
        work->cpuInputDelayFrames--;
        if (work->cpuInputDelayFrames == 0) {
            work->cpuInputDelayFrames = (rand8() % 2 - 1) + lbl_1_data_88[work->cpuDifficulty];
            input = 1;
        }
    } else if (lbl_802C0008[work->padNo] & PAD_BUTTON_A) {
        input = 1;
    }
    if (input == 1) {
        work->inputReceived = 1;
        work->pos.z += 20.0f;
        /* When z is below 1700, the input adds a second 20 to the character's z position. */
        if (work->pos.z < 1700.0f) {
            work->pos.z += 20.0f;
        }
        work->movementBoostFrames += 6;
        if (work->movementBoostFrames > 30) {
            work->movementBoostFrames = 30;
        }
    }
    if (work->movementBoostFrames > 0) {
        work->movementBoostFrames--;
        fn_8002D86C(work->modelId, 1.8f);
    } else {
        fn_8002D86C(work->modelId, 1.0f);
    }
    work->pos.z -= lbl_1_data_98;
    if (work->pos.z > 2300.0f) {
        work->pos.z = 2300.0f;
    }
    if (work->pos.z < 1700.0f) {
        if (work->effectHandle == -1) {
            work->effectHandle = CharFXPlay(work->characterNo, MSM_SE_CHARVOICE_EXCITED_SHOUT);
        }
        if (work->motionIds[2] != fn_8002D3C4(work->modelId) &&
            fn_8002D3EC(work->modelId) == -1) {
            fn_8002CCC0(work->modelId, work->motionIds[2], 0.0f, 30.0f,
                ((1 << 30) | (1 << 0)));
        }
        if (fn_8002D3EC(work->modelId) != -1) {
            work->pos.y -= 2.7f;
            if (work->pos.y < -81.0f) {
                work->pos.y = -81.0f;
            }
        }
    }
    if (work->pos.z > 1700.0f) {
        if (work->motionIds[1] != fn_8002D3C4(work->modelId) &&
            fn_8002D3EC(work->modelId) == -1) {
            fn_8002CCC0(work->modelId, work->motionIds[1], 0.0f, 30.0f,
                ((1 << 30) | (1 << 0)));
        }
        if (fn_8002D3EC(work->modelId) != -1) {
            work->pos.y += 2.7f;
            if (work->pos.y > 0.0f) {
                work->pos.y = 0.0f;
            }
        }
    }
    fn_80026790(work->modelId, &work->pos);
    if (work->pos.z < 1000.0f && work->finishResolved != 1) {
        work->reachedFinish = 1;
    }
    if (work->finishResolved == 1) {
        if (work->reachedFinish == 1) {
            char *hooks[2] = { "P1", "P2" };
            fn_80026760(work->modelId, 0.0f, work->pos.y, 0.0f);
            fn_8002741C(work->resultHookModel, hooks[work->groupNo], work->modelId);
            fn_80026A44(work->resultHookModel, ((1 << 30) | (1 << 1)));
            fn_1_B0(work->playerNo);
            CharFXPlay(work->characterNo, MSM_SE_CHARVOICE_HIT_YELP);
            fn_8006A030(work->playerNo, 20, 20, 0);
        } else {
            HuVecF target = lbl_1_data_9C;
            HuVecF pos = work->pos;
            PSVECSubtract(&target, &pos, &work->crossingDirection);
            lbl_1_bss_10 = PSVECMag(&work->crossingDirection) / 120.0f;
            OSReport("winner speed %f\n", lbl_1_bss_10);
            PSVECNormalize(&work->crossingDirection, &work->crossingDirection);
            fn_8002D86C(work->modelId, 0.8f);
            fn_8002CCC0(work->modelId, work->motionIds[1], 0.0f, 60.0f,
                ((1 << 30) | (1 << 0)));
        }
        obj->objFunc = fn_1_2034;
    }
}

/* Called during the duel update; starts result handling when either player reaches the finish. If
 * both reach it, keeps a no-input finish as a draw or randomly selects one finisher when either
 * received input. */
void fn_1_1E60(void)
{
    int player;

    if (lbl_1_bss_A8[0].finishResolved == 1 || lbl_1_bss_A8[1].finishResolved == 1) {
        return;
    }
    if (lbl_1_bss_A8[0].reachedFinish != 0 || lbl_1_bss_A8[1].reachedFinish != 0) {
        OSReport("finish.\n");
        lbl_1_bss_14.resultStarted = 1;
        if (lbl_1_bss_A8[0].reachedFinish == 1 && lbl_1_bss_A8[1].reachedFinish == 1) {
            if (lbl_1_bss_A8[0].inputReceived == 0 && lbl_1_bss_A8[1].inputReceived == 0) {
                /* If neither side tapped, keep both finish flags set so the sequence shows a
                 * draw. */
                lbl_1_bss_A8[0].reachedFinish = 1;
                lbl_1_bss_A8[1].reachedFinish = 1;
                OSReport("draw.\n");
            } else {
                player = rand8() % 2;
                lbl_1_bss_A8[player].reachedFinish = 1;
                lbl_1_bss_A8[1 - player].reachedFinish = 0;
                OSReport("random.\n");
            }
        }
        OSReport("%d ( %d ) : %d ( %d )\n", lbl_1_bss_A8[0].groupNo, lbl_1_bss_A8[0].inputReceived,
                 lbl_1_bss_A8[1].groupNo, lbl_1_bss_A8[1].inputReceived);
        lbl_1_bss_A8[0].finishResolved = 1;
        lbl_1_bss_A8[1].finishResolved = 1;
    }
}

/* Player-object callback after fn_1_192C; holds a finisher at its hook or moves the other player to
 * the result target. */
void fn_1_2034(OMOBJ *obj)
{
    M651Player *work = obj->data;
    HuVecF target;
    HuVecF pos;

    if (work->reachedFinish == 1) {
        if (work->pos.z > 0.0f) {
            char *hooks[2] = { "P1", "P2" };
            Hu3DModelObjPosGet(work->resultHookModel, hooks[work->groupNo], &work->pos);
            return;
        }
        lbl_1_bss_64.resultAnimationActive = 1;
        lbl_1_bss_64.sceneFrame = 0;
        lbl_1_bss_64.resultScaleStep = lbl_1_bss_64.firstBackgroundScale / 120.0f;
        OSReport("void : %f\n", lbl_1_bss_64.resultScaleStep);
        fn_800269E8(work->modelId, (1 << 0));
        fn_80027508(work->resultHookModel);
        obj->objFunc = fn_1_242C;
        HuAudFXFadeOut(lbl_1_data_AC, 500);
        HuAudFXPlay(MSM_SE_MG651_RESULT_COLLAPSE);
        return;
    }
    target = lbl_1_data_9C;
    pos = work->pos;
    PSVECSubtract(&target, &pos, &work->crossingDirection);
    if (work->pos.x != lbl_1_data_9C.x || work->pos.y != lbl_1_data_9C.y ||
        work->pos.z != lbl_1_data_9C.z) {
        PSVECNormalize(&work->crossingDirection, &work->crossingDirection);
    }
    work->pos.x += lbl_1_bss_10 * work->crossingDirection.x;
    work->pos.y += lbl_1_bss_10 * work->crossingDirection.y;
    work->pos.z += lbl_1_bss_10 * work->crossingDirection.z;
    if (work->pos.x < 1.0f + lbl_1_data_9C.x && work->pos.x > lbl_1_data_9C.x - 1.0f) {
        work->pos.x = lbl_1_data_9C.x;
    }
    if (work->pos.y < 1.0f + lbl_1_data_9C.y && work->pos.y > lbl_1_data_9C.y - 1.0f) {
        work->pos.y = lbl_1_data_9C.y;
    }
    if (work->pos.z < 1.0f + lbl_1_data_9C.z && work->pos.z > lbl_1_data_9C.z - 1.0f) {
        work->pos.z = lbl_1_data_9C.z;
    }
    fn_80026790(work->modelId, &work->pos);
    if (fn_80071A8C() == 6) {
        fn_8002CCC0(work->modelId, work->motionIds[0], 0.0f, 40.0f, ((1 << 30) | (1 << 0)));
        obj->objFunc = fn_1_23D4;
    }
}

/* Player-object callback assigned by fn_1_2034; holds the completed transition motion at frame
 * 35. */
void fn_1_23D4(OMOBJ *obj)
{
    M651Player *work = obj->data;
    if (fn_8002D75C(work->modelId)) {
        fn_8002D414(work->modelId, 35.0f);
    }
}

/* Player-object callback after fn_1_2034; starts the finisher's final character motion when result
 * mode 7 begins. */
void fn_1_242C(OMOBJ *obj)
{
    M651Player *work = obj->data;
    if (fn_80071A8C() == 7 && work->reachedFinish == 1) {
        fn_8002C95C(work->modelId, work->motionIds[4]);
        fn_80026A44(work->modelId, ((1 << 30) | (1 << 1)));
        fn_800269E8(work->modelId, ((1 << 30) | (1 << 0)));
        fn_8005DFC0(work->characterNo, 0);
        obj->objFunc = fn_1_24BC;
    }
}

/* Player-object callback after fn_1_242C; moves the character along its result path and turns it
 * toward that path. */
void fn_1_24BC(OMOBJ *obj)
{
    M651Player *work = obj->data;
    work->crossingPosition.x += 12.0f * work->crossingDirection.x;
    work->crossingPosition.y += 12.0f * work->crossingDirection.y;
    work->crossingPosition.z += 12.0f * work->crossingDirection.z;
    fn_80026760(work->modelId, work->crossingPosition.x, work->crossingPosition.y,
                    work->crossingPosition.z);
    fn_80026808(work->modelId, 90.0f, 0.0f, work->crossingYaw);
    fn_80026A44(work->modelId, (1 << 0));
    OSReport("pos : %f, %f, %f\n", work->crossingDirection.x, work->crossingDirection.y,
             work->crossingDirection.z);
}

/* Called during scene setup by fn_1_3F30; loads the scene models and creates their frame-update
 * object. */
void fn_1_25B0(void)
{
    OMOBJ *obj;

    lbl_1_bss_64.sceneModels[0] =
        fn_80025A04(fn_80008DD0(DATANUM(0x6e0000, 0), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_64.sceneModels[0], ((1 << 30) | (1 << 0)));
    lbl_1_bss_64.sceneModels[2] =
        fn_80025A04(fn_80008DD0(DATANUM(0x6e0000, 2), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_64.sceneModels[2], ((1 << 30) | (1 << 0)));
    lbl_1_bss_64.sceneModels[1] =
        fn_80025A04(fn_80008DD0(DATANUM(0x6e0000, 1), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_64.sceneModels[1], ((1 << 30) | (1 << 6)));
    fn_800269E8(lbl_1_bss_64.sceneModels[1], ((1 << 30) | (1 << 0)));
    lbl_1_bss_64.sceneModels[3] =
        fn_80025A04(fn_80008DD0(DATANUM(0x6e0000, 4), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_64.sceneModels[3], ((1 << 30) | (1 << 1)));
    fn_8002D86C(lbl_1_bss_64.sceneModels[3], 2.0f);
    lbl_1_bss_64.sceneModels[4] =
        fn_80025A04(fn_80008DD0(DATANUM(0x6e0000, 3), 0x10000000, 2));
    fn_80026A44(lbl_1_bss_64.sceneModels[4], ((1 << 30) | (1 << 0)));
    lbl_1_bss_64.sceneRotationSpeedOffset = 0.0f;
    lbl_1_bss_64.animatedSceneModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x6e0000, 5), 0x10000000, 2));
    lbl_1_bss_64.unusedSceneSpeedAccumulator = 1.0f;
    fn_800269E8(lbl_1_bss_64.animatedSceneModel, ((1 << 30) | (1 << 0)));
    lbl_1_bss_64.approachEffectModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x6e0000, 6), 0x10000000, 2));
    lbl_1_bss_64.approachEffectOverlayModel =
        fn_80025A04(fn_80008DD0(DATANUM(0x6e0000, 7), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_64.approachEffectModel, ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_64.approachEffectOverlayModel, ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_64.approachEffectOverlayModel, (1 << 0));
    fn_80026B8C(lbl_1_bss_64.sceneModels[4], 0);
    fn_80026B8C(lbl_1_bss_64.sceneModels[0], 1);
    fn_80026B8C(lbl_1_bss_64.sceneModels[2], 2);
    fn_80026B8C(lbl_1_bss_64.sceneModels[1], 3);
    fn_80026B8C(lbl_1_bss_64.animatedSceneModel, 4);
    fn_80026B8C(lbl_1_bss_64.approachEffectModel, 4);
    fn_80026B8C(lbl_1_bss_64.approachEffectOverlayModel, 4);
    obj = fn_8003AB38(fn_1_A0(), 336, 0, 0, 0, fn_1_29A4);
    obj->data = &lbl_1_bss_64;
    lbl_1_bss_64.sceneFrame = 0;
    lbl_1_bss_64.mainSceneRotation = 0.0f;
    lbl_1_bss_64.mainSceneScale = 0.0f;
    lbl_1_bss_64.backgroundRotation = 0.0f;
    lbl_1_bss_64.firstBackgroundScale = 0.0f;
    lbl_1_bss_64.secondBackgroundScale = 0.0f;
    lbl_1_bss_64.closingModelRotation = 1.0f;
    lbl_1_bss_64.resultAnimationActive = 0;
    lbl_1_bss_64.finalMeteorCheckDone = 0;
}

/* Scene-object callback created by fn_1_25B0; animates the opening and hands off to the duel
 * callback at mode 5. */
void fn_1_29A4(OMOBJ *obj)
{
    M651Work64 *work = obj->data;
    if (lbl_1_bss_64.sceneFrame == 55) {
        HuAudFXPlay(MSM_SE_MG651_SCENE_START_IMPACT);
        HuAudFXPlay(MSM_SE_MG651_SCENE_START_RUMBLE);
        lbl_1_data_AC = HuAudFXPlay(MSM_SE_MG651_METEOR_APPROACH_LOOP);
        HuAudFXPitchSet(lbl_1_data_AC, lbl_1_data_B0);
    }
    lbl_1_bss_64.sceneFrame++;
    if (lbl_1_bss_64.sceneFrame >= 80 && lbl_1_bss_64.sceneFrame < 200) {
        lbl_1_bss_64.mainSceneScale += 0.00833;
        if (lbl_1_bss_64.mainSceneScale > 1.0f) lbl_1_bss_64.mainSceneScale = 1.0f;
    }
    lbl_1_bss_64.mainSceneRotation += 1.0f;
    fn_800268B0(lbl_1_bss_64.sceneModels[0], lbl_1_bss_64.mainSceneScale,
                      lbl_1_bss_64.mainSceneScale, lbl_1_bss_64.mainSceneScale);
    fn_80026808(lbl_1_bss_64.sceneModels[0], 0.0f, 0.0f, lbl_1_bss_64.mainSceneRotation);
    if (lbl_1_bss_64.sceneFrame < 120) {
        lbl_1_bss_64.firstBackgroundScale += 0.00833;
        lbl_1_bss_64.secondBackgroundScale += 0.00833;
        if (lbl_1_bss_64.firstBackgroundScale > 1.0f) lbl_1_bss_64.firstBackgroundScale = 1.0f;
        if (lbl_1_bss_64.secondBackgroundScale > 1.0f) lbl_1_bss_64.secondBackgroundScale = 1.0f;
    }
    lbl_1_bss_64.backgroundRotation += 2.0f;
    fn_800268B0(lbl_1_bss_64.sceneModels[1], lbl_1_bss_64.firstBackgroundScale,
                      lbl_1_bss_64.firstBackgroundScale, lbl_1_bss_64.firstBackgroundScale);
    fn_80026808(lbl_1_bss_64.sceneModels[1], 0.0f, 0.0f, lbl_1_bss_64.backgroundRotation);
    fn_800268B0(lbl_1_bss_64.sceneModels[2], lbl_1_bss_64.firstBackgroundScale,
                      lbl_1_bss_64.firstBackgroundScale, lbl_1_bss_64.firstBackgroundScale);
    fn_80026808(lbl_1_bss_64.sceneModels[2], 0.0f, 0.0f, lbl_1_bss_64.backgroundRotation);
    if (fn_8002D75C(lbl_1_bss_64.sceneModels[4]) == 1) {
        lbl_1_bss_64.closingModelRotation -= 2.0f;
        fn_80026808(lbl_1_bss_64.sceneModels[4], 0.0f, 0.0f, lbl_1_bss_64.closingModelRotation);
    }
    if (fn_80071A8C() == 5) obj->objFunc = fn_1_2D8C;
}

/* Scene-object callback after fn_1_29A4; animates duel cues and, after a result, shrinks the scene
 * before advancing the sequence. */
void fn_1_2D8C(OMOBJ *obj)
{
    M651Work64 *work = obj->data;
    if (lbl_1_bss_64.resultAnimationActive == 1) {
        fn_800269E8(lbl_1_bss_64.sceneModels[4], ((1 << 30) | (1 << 8)));
        lbl_1_bss_64.sceneFrame++;
        lbl_1_bss_64.mainSceneScale -= 0.00833;
        if (lbl_1_bss_64.mainSceneScale < 0.0f) lbl_1_bss_64.mainSceneScale = 0.0f;
        lbl_1_bss_64.firstBackgroundScale -= lbl_1_bss_64.resultScaleStep;
        lbl_1_bss_64.secondBackgroundScale -= lbl_1_bss_64.resultScaleStep;
        if (lbl_1_bss_64.firstBackgroundScale < 0.0f) lbl_1_bss_64.firstBackgroundScale = 0.0f;
        if (lbl_1_bss_64.secondBackgroundScale < 0.0f) lbl_1_bss_64.secondBackgroundScale = 0.0f;
        if (lbl_1_bss_64.firstBackgroundScale < 0.2f)
            fn_80026A44(lbl_1_bss_64.sceneModels[3], ((1 << 30) | (1 << 1)));
        if ((float)lbl_1_bss_64.sceneFrame == 90.0f) HuAudFXPlay(MSM_SE_MG651_RESULT_SHRINK);
        if ((float)lbl_1_bss_64.sceneFrame > 120.0f) {
            fn_1_190();
            obj->objFunc = NULL;
        }
    } else {
        lbl_1_bss_64.firstBackgroundScale += 0.004f;
        if (lbl_1_bss_64.firstBackgroundScale > 3.0f) lbl_1_bss_64.firstBackgroundScale = 3.0f;
        lbl_1_bss_64.secondBackgroundScale += 0.004f;
        if (lbl_1_bss_64.secondBackgroundScale > 1.5f) lbl_1_bss_64.secondBackgroundScale = 1.5f;
        fn_8002D86C(lbl_1_bss_64.sceneModels[0], lbl_1_data_A8);
        lbl_1_data_A8 += 0.05f;
        if (lbl_1_data_A8 > 4.0f) lbl_1_data_A8 = 4.0f;
        {
            float soundCueFrames[6] = { 405.0f, 426.0f, 494.0f, 540.0f, 590.0f, 630.0f };
            s16 soundCuePan[6] = { 32, 32, 96, 32, 32, 96 };
            int cueIndex = 0;
            float animatedSceneFrame = fn_8002D4E0(lbl_1_bss_64.animatedSceneModel);
            for (cueIndex = 0; cueIndex < 6; cueIndex++) {
                if (animatedSceneFrame == soundCueFrames[cueIndex])
                    HuAudFXPlayPan(MSM_SE_MG651_METEOR_FLYBY, soundCuePan[cueIndex]);
            }
        }
        {
            float sceneCueFrames[19] = { 386.0f,  490.0f,  580.0f,  666.0f,  737.0f,
                                         760.0f,  830.0f,  870.0f,  900.0f,  932.0f,
                                         955.0f,  1000.0f, 1024.0f, 1064.0f, 1096.0f,
                                         1130.0f, 1150.0f, 1170.0f, 1180.0f };
            int cueIndex = 0;
            float sceneMotionFrame = fn_8002D4E0(lbl_1_bss_64.sceneModels[1]);
            for (cueIndex = 0; cueIndex < 19; cueIndex++) {
                if (sceneMotionFrame == sceneCueFrames[cueIndex])
                    HuAudFXPlay(MSM_SE_MG651_METEOR_SCENE_BEAT);
            }
        }
    }
    if (lbl_1_data_B0 != 0) {
        lbl_1_data_B0 += 10;
        if (lbl_1_data_B0 > 0) lbl_1_data_B0 = 0;
        HuAudFXPitchSet(lbl_1_data_AC, lbl_1_data_B0);
    }
    lbl_1_bss_64.mainSceneRotation += 1.0f + lbl_1_bss_64.sceneRotationSpeedOffset;
    fn_800268B0(lbl_1_bss_64.sceneModels[0], lbl_1_bss_64.mainSceneScale,
                      lbl_1_bss_64.mainSceneScale, lbl_1_bss_64.mainSceneScale);
    fn_80026808(lbl_1_bss_64.sceneModels[0], 0.0f, 0.0f, lbl_1_bss_64.mainSceneRotation);
    lbl_1_bss_64.backgroundRotation += 2.0f + lbl_1_bss_64.sceneRotationSpeedOffset;
    fn_800268B0(lbl_1_bss_64.sceneModels[1], lbl_1_bss_64.firstBackgroundScale,
                      lbl_1_bss_64.firstBackgroundScale, lbl_1_bss_64.firstBackgroundScale);
    fn_80026808(lbl_1_bss_64.sceneModels[1], 0.0f, 0.0f, lbl_1_bss_64.backgroundRotation);
    fn_800268B0(lbl_1_bss_64.sceneModels[2], lbl_1_bss_64.secondBackgroundScale,
                      lbl_1_bss_64.secondBackgroundScale, lbl_1_bss_64.secondBackgroundScale);
    fn_80026808(lbl_1_bss_64.sceneModels[2], 0.0f, 0.0f, lbl_1_bss_64.backgroundRotation);
    lbl_1_bss_64.closingModelRotation -= 2.0f + lbl_1_bss_64.sceneRotationSpeedOffset;
    fn_80026808(lbl_1_bss_64.sceneModels[4], 0.0f, 0.0f, lbl_1_bss_64.closingModelRotation);
    if (lbl_1_bss_A8[0].pos.z < 1700.0f || lbl_1_bss_A8[1].pos.z < 1700.0f) {
        fn_80026A44(lbl_1_bss_64.approachEffectOverlayModel, (1 << 0));
        fn_800269E8(lbl_1_bss_64.approachEffectOverlayModel, ((1 << 30) | (1 << 0)));
    } else {
        fn_80026A44(lbl_1_bss_64.approachEffectOverlayModel, ((1 << 30) | (1 << 0)));
    }
    if (lbl_1_bss_A8[0].reachedFinish == 1 || lbl_1_bss_A8[1].reachedFinish == 1) {
        fn_80026A44(lbl_1_bss_64.approachEffectOverlayModel, ((1 << 30) | (1 << 0)));
        fn_80026A44(lbl_1_bss_64.approachEffectModel, ((1 << 30) | (1 << 0)));
        fn_800269E8(lbl_1_bss_64.animatedSceneModel, (1 << 0));
    }
}

/* Placeholder player-object callback; it reads the object's data but performs no per-frame gameplay
 * update. */
void fn_1_35D8(OMOBJ *obj)
{
    M651Player *work = obj->data;
}

/* Called by fn_1_36F8; sets crossing endpoints and direction, selects the crossing motion, and sets
 * the character's yaw. */
void fn_1_35EC(M651Player *playerState, float startY, float endY, float startX, float endX)
{
    playerState->crossingStart.x = startX;
    playerState->crossingStart.y = startY;
    playerState->crossingStart.z = 1000.0f;
    playerState->crossingEnd.x = endX;
    playerState->crossingEnd.y = endY;
    playerState->crossingEnd.z = 1000.0f;
    playerState->crossingPosition = playerState->crossingStart;
    PSVECSubtract(&playerState->crossingEnd, &playerState->crossingStart,
                  &playerState->crossingDirection);
    PSVECNormalize(&playerState->crossingDirection, &playerState->crossingDirection);
    fn_80026A44(playerState->modelId, ((1 << 30) | (1 << 0)));
    fn_8005BE80(playerState->characterNo, playerState->motionIds[0]);
    if (startX < endX) {
        playerState->crossingYaw = -90.0f;
    } else {
        playerState->crossingYaw = 90.0f;
    }
}

/* Called by the result-phase callback in lbl_1_data_14 at frame 90; assigns both draw paths or
 * pauses the winner and assigns only the other player's crossing path. */
void fn_1_36F8(void)
{
    int firstDrawPlayerIndex;
    int drawPathOrder;
    int winnerPathOrder;
    int winnerPathHeight;
    int winnerIndex;

    if (lbl_1_bss_A8[0].reachedFinish == 1 && lbl_1_bss_A8[1].reachedFinish == 1) {
        firstDrawPlayerIndex = rand8() % 2;
        drawPathOrder = rand8() % 2;
        {
            float pathHeights[2] = { 250.0f, -250.0f };
            fn_1_35EC(&lbl_1_bss_A8[firstDrawPlayerIndex], pathHeights[drawPathOrder],
                      pathHeights[1 - drawPathOrder], -1600.0f, 1600.0f);
            fn_1_35EC(&lbl_1_bss_A8[1 - firstDrawPlayerIndex], pathHeights[drawPathOrder],
                      pathHeights[1 - drawPathOrder], 1600.0f, -1600.0f);
        }
    } else {
        winnerIndex = lbl_1_bss_A8[0].reachedFinish == 1 ? 1 : 0;
        {
            HU3D_MODELID winnerModelId = lbl_1_bss_A8[winnerIndex].modelId;
            fn_800269E8(winnerModelId, ((1 << 30) | (1 << 1)));
            fn_8005C2C8(lbl_1_bss_A8[winnerIndex].characterNo,
                               lbl_1_bss_A8[winnerIndex].motionIds[3], 0.0f, 15.0f,
                               0);
        }
        winnerPathOrder = rand8() % 2;
        winnerPathHeight = rand8() % 2;
        {
            float pathHeights[2] = { 250.0f, -250.0f };
            float pathStartXs[2] = { -1600.0f, 1600.0f };
            fn_1_35EC(&lbl_1_bss_A8[1 - winnerIndex], pathHeights[winnerPathHeight],
                      pathHeights[1 - winnerPathHeight], pathStartXs[winnerPathOrder],
                      pathStartXs[1 - winnerPathOrder]);
        }
    }
}

/* Called by fn_1_40C; builds lighting, camera, player characters, scene models, and meteor objects
 * in setup order. */
void fn_1_3F30(void)
{
    fn_1_1344();
    fn_1_1220();
    fn_1_13D8();
    fn_1_25B0();
    fn_1_D28();
}
