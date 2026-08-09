#define Hu3DModelLightInfoSet Hu3DModelLightInfoSet_Header
#define Hu3DModelObjMtxGet Hu3DModelObjMtxGet_Header

#include "dolphin.h"
#include "math.h"
#include "game/gamework.h"
#include "game/object.h"
#include "game/board/camera.h"
#include "game/board/effect.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/opening.h"
#include "game/board/pause.h"
#include "game/board/player.h"
#include "game/board/audio.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/process.h"

#undef Hu3DModelLightInfoSet
#undef Hu3DModelObjMtxGet

typedef void (*VoidFunc)(void);

typedef struct S02PairModelIds {
    s32 modelId[4];
} S02PairModelIds;

typedef struct S02DataB4 {
    char hook[12];
    s32 modelId[2];
} S02DataB4;

typedef struct S02DataC8 {
    s32 modelId[2];
    HuVecF pos[2];
} S02DataC8;

typedef struct S02Work {
    s32 modelId[3];
    s32 modelIdC;
    s32 eventModelId;
    s32 modelId14;
    s32 modelId18;
    s32 mapObjId[12];
    s32 modelId4C;
    s32 unk_50;
    s32 unk_54;
    s32 modelId58;
    s32 modelId5C;
    s32 effectModelId;
    s32 modelId64;
    S02PairModelIds pair[2];
    s32 modelId88;
    s32 modelId8C;
} S02Work;

enum {
    S02_OBJECT_PRIORITY = 8204,
    S02_RANDOM_MASK = 65535,
};

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern char lbl_1_data_20[4][16];
extern HuVecF lbl_1_data_60;
extern HuVecF lbl_1_data_6C;
extern HuVecF lbl_1_data_78;
extern float lbl_1_data_84;
extern HuVecF lbl_1_data_88;
extern HuVecF lbl_1_data_94;
extern HuVecF lbl_1_data_A0;
extern s32 lbl_1_data_AC[2];
extern S02DataB4 lbl_1_data_B4;
extern S02DataC8 lbl_1_data_C8;
extern HuVecF s02MapObjectInitialPositions[12];
extern HuVecF lbl_1_data_178[12];
extern HuVecF lbl_1_data_208[2];
extern HuVecF lbl_1_data_220[3];
extern char lbl_1_data_244[8];
extern s32 lbl_1_data_24C[2];
extern s32 lbl_1_data_254[3];
extern HuVecF lbl_1_data_260[2];
extern HuVecF lbl_1_data_278[2];
extern HuVecF lbl_1_data_290[2];
extern s16 lbl_1_bss_0;
extern HuVecF s02MapScrollDelta;
extern s32 lbl_1_bss_10[3];
extern S02Work s02Work;
void S02OverlayInitialize(void);
void fn_1_F4(void);
void S02ObjectClose(OMOBJ *obj);
void S02MapObjectScrollUpdate(OMOBJ *obj);
int S02MasuAttr16Handler(int playerNo, s16 id);
int fn_1_3EC(int playerNo, s16 id);
int S02MasuAttr5Handler(int playerNo, s16 id);
void fn_1_450(int playerNo);
void S02PairModelsResetEvent(int playerNo);
void S02PrimaryModelLightInfoEnable(void);
void fn_1_4B0(void);
void fn_1_4B4(BOOL enterF);
void fn_1_4B8(int playerNo, s16 id);
void S02SceneModelsCreate(void);
void fn_1_1120(int playerNo, s16 id);
void S02PairModelsReset(void);
void fn_1_1DC0(void);
void fn_1_1DC4(void);
void fn_1_1DC8(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx);
void fn_1_22C8(void);
void fn_1_22CC(void);

void mbObjectSetup(s32 boardNo, void (*init)(void), void (*close)(OMOBJ *));
void mbLightFuncSet(void (*setHook)(void), void (*resetHook)(void));
void mbMapCameraSet(const HuVecF *rot, const HuVecF *pos, float zoom);
void mbMapHookSet(void (*hook)(BOOL enterF));
void mbScrollInit(int dataNum);
s16 mbObjCreate(int dataNum, const int *motDataNum, BOOL linkF);
void mbObjAttrSet(s16 modelId, u32 attr);
int mbObjModelIDGet(s16 modelId);
void Hu3DModelLightInfoSet(int modelId, BOOL lightInfoF);
void Hu3DModelObjMtxGet(int modelId, char *objName, Mtx mtx);
void mbObjPosGet(s16 modelId, HuVecF *pos);
void mbObjPosSet(s16 modelId, float x, float y, float z);
void mbObjPosSetV(s16 modelId, const HuVecF *pos);
void mbObjRotSet(s16 modelId, float x, float y, float z);
void mbObjRotSetV(s16 modelId, const HuVecF *rot);
void mbObjScaleSet(s16 modelId, float x, float y, float z);
void mbObjDispSet(s16 modelId, BOOL dispF);
void mbObjMotionTimeSet(s16 modelId, float time);
float mbObjMotionTimeGet(s16 modelId);
BOOL mbObjMotionEndCheck(s16 modelId);
void mbObjMotionSpeedSet(s16 modelId, float speed);
void mbObjMotionStartEndSet(s16 modelId, s16 start, s16 end);
void mbObjMtxSet(s16 modelId, Mtx *mtx);
void mbObjAlphaSet(s16 modelId, u8 alpha);
void mbObjHookSet(s16 modelId, char *objName, s16 hookModelId);
void mbPos2Dto3D(HuVecF *src, HuVecF *dst);
void mbWipeSpecialCreate(int state, int type, int time);
void mbWipeSpecialWait(void);
void mbWipeFadeOutTime(int time);
void mbWipeSpecialKill(void);
void mbWipeFadeOut(void);
void mbSingleReturnWrite(void);
int mbBoardDataNumGet(int dataNum);

int _prolog(void)
{
    const VoidFunc *ctors = _ctors;

    while (*ctors != 0) {
        (**ctors)();
        ctors++;
    }
    S02OverlayInitialize();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}

void S02OverlayInitialize(void)
{
    GwSystem.partyF = FALSE;
    mbObjectSetup(7, fn_1_F4, S02ObjectClose);
}

void fn_1_F4(void)
{
    s16 *modelId = &lbl_1_bss_0;
    s32 boardNo[1];

    boardNo[0] = MBBoardNoGet();

    mbMasuInit(DATANUM(DATA_s02, 0));
    modelId[0] = mbObjCreate(DATANUM(DATA_s02, 2), NULL, FALSE);
    mbObjAttrSet(modelId[0], HU3D_MOTATTR_LOOP);
    mbScrollInit(DATANUM(DATA_s02, 1));
    mbLightFuncSet(S02PrimaryModelLightInfoEnable, fn_1_4B0);
    mbPlayerTurnInitHookSet(fn_1_450);
    mbPlayerTurnCloseHookSet(S02PairModelsResetEvent);
    mbev_MasuMoveEndSet(fn_1_3EC);
    mbev_MasuMoveStartSet(S02MasuAttr16Handler);
    mbev_MasuHatenaSet(S02MasuAttr5Handler);
    mbMapCameraSet(NULL, &lbl_1_data_60, 9800.0f);
    mbMapHookSet(fn_1_4B4);
    omAddObjEx(mbObjMan, S02_OBJECT_PRIORITY, 0, 0, -1, S02MapObjectScrollUpdate);
    S02SceneModelsCreate();
    mbCameraNearFarSet(100.0f, 80000.0f);
    mbOpeningViewSet(&lbl_1_data_6C, &lbl_1_data_78, lbl_1_data_84);
    HuAudSndGrpSet(29);
    HuDataDirClose(DATANUM(DATA_s02, 0));
    (void)boardNo;
}

void S02ObjectClose(OMOBJ *obj)
{
}

void S02MapObjectScrollUpdate(OMOBJ *obj)
{
    HuVecF pos;
    s32 i;

    for (i = 0; i < 12; i++) {
        mbObjPosGet((s16)s02Work.mapObjId[i], &pos);
        pos.x += s02MapScrollDelta.x;
        pos.z += s02MapScrollDelta.z;
        if (pos.x >= 1500.0f + lbl_1_data_88.x) {
            pos = lbl_1_data_94;
            pos.x += 1500.0f;
            pos.z += 200.0f;
        }
        mbObjPosSetV((s16)s02Work.mapObjId[i], &pos);
    }
}

int S02MasuAttr16Handler(int playerNo, s16 id)
{
    u32 attr = mbMasuMAttrGet(id);

    if (attr & 16) {
        fn_1_4B8(playerNo, id);
    }
    return 0;
}

int fn_1_3EC(int playerNo, s16 id)
{
    return 0;
}

int S02MasuAttr5Handler(int playerNo, s16 id)
{
    u32 attr = mbMasuMAttrGet(id);

    if (attr & 5) {
        fn_1_1120(playerNo, id);
    }
    return 0;
}

void fn_1_450(int playerNo)
{
}

void S02PairModelsResetEvent(int playerNo)
{
    S02PairModelsReset();
}

void S02PrimaryModelLightInfoEnable(void)
{
    s16 *modelId = &lbl_1_bss_0;

    Hu3DModelLightInfoSet(mbObjModelIDGet(modelId[0]), TRUE);
}

void fn_1_4B0(void)
{
}

void fn_1_4B4(BOOL enterF)
{
    (void)enterF;
}

void fn_1_4B8(int playerNo, s16 id)
{
    s32 motionId[2];
    s32 i;
    s32 state[1];
    s32 yOffset;
    s32 objectModelId;
    s32 sound;
    Mtx objectMtx;
    Mtx playerMtx;

    yOffset = 0;
    state[0] = 0;
    (void)state;
    mbPauseDisableSet(TRUE);
    for (i = 0; i < 2; i++) {
        motionId[i] = mbPlayerMotionCreate(playerNo, lbl_1_data_AC[i]);
    }
    mbMoveNumDispSet(playerNo, FALSE);
    omVibrate((s16)playerNo, 20, 7, 3);
    mbPlayerMasuMove(playerNo, TRUE);
    mbPlayerRotateStart(playerNo, 0, 15);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    HuPrcSleep(30);
    mbPlayerMotionShiftSet(playerNo, motionId[0], 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);
    mbCameraMovePlayer((s16)playerNo, &lbl_1_data_A0, NULL,
        2400.0f, -1.0f, 90);
    mbCameraMoveWait();
    mbObjMotionSpeedSet((s16)s02Work.effectModelId, 2.0f);
    HuPrcSleep(10);
    mbObjMotionSpeedSet((s16)s02Work.eventModelId, 1.5f);
    sound = mbAudFXPlay(1553);
    omVibrate((s16)playerNo, 200, 4, 4);

    while (mbObjMotionTimeGet((s16)s02Work.eventModelId)
        <= 500.0f) {
        if (mbObjMotionTimeGet((s16)s02Work.eventModelId)
            == 300.0f) {
            omVibrate((s16)playerNo, 80, 7, 3);
        }
        if (mbObjMotionTimeGet((s16)s02Work.eventModelId)
            == 381.0f) {
            mbPlayerRotateStart(playerNo, -180, 7);
            while (!mbPlayerRotateCheck(playerNo)) {
                HuPrcVSleep();
            }
            mbPlayerMotionShiftSet(playerNo, 9, 0.0f,
                5.0f, HU3D_MOTATTR_NONE);
            mbCameraFocusPlayerSet(-1);
        } else if (mbObjMotionTimeGet((s16)s02Work.eventModelId)
            == 420.0f) {
            mbWipeSpecialCreate(1, 6, 108);
            mbPlayerMotionShiftSet(playerNo, motionId[1], 0.0f,
                8.0f, HU3D_MOTATTR_LOOP);
            mbPlayerRotSet(playerNo, 45.0f, 180.0f,
                0.0f);
            mbCameraShakeSet(18, 100.0f);
            omVibrate((s16)playerNo, 20, 0, 0);
            mbMusFadeOutSpeed(0, 1000);
        } else if (mbObjMotionTimeGet((s16)s02Work.eventModelId)
            >= 420.0f) {
            objectModelId = mbObjModelIDGet((s16)s02Work.eventModelId);

            Hu3DModelObjMtxGet(objectModelId, lbl_1_data_B4.hook, objectMtx);
            PSMTXTransApply(objectMtx, playerMtx, 0.0f,
                -60.0f + (float)yOffset, 130.0f);
            mbPlayerPosSet(playerNo, playerMtx[0][3], playerMtx[1][3],
                playerMtx[2][3]);
            yOffset += 2;
        }
        HuPrcVSleep();
    }
    mbCameraShakeReset();
    mbAudFXStop(sound);
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();
    for (i = 0; i < 2; i++) {
        mbPlayerMotionKill(playerNo, motionId[i]);
    }
    mbSingleReturnWrite();
}

void S02SceneModelsCreate(void)
{
    HuVecF pos;
    s32 modelId;
    s32 i;

    for (i = 0; i < 3; i++) {
        modelId = (s16)mbObjCreate(13107203 + i, NULL, FALSE);
        s02Work.modelId[i] = modelId;
        mbObjMotionTimeSet(modelId, 0.0f);
        mbObjMotionSpeedSet(modelId, 1.0f);
        mbObjAttrSet(modelId, 1073741825);
    }

    modelId = (s16)mbObjCreate(13107211, NULL, FALSE);
    s02Work.modelIdC = modelId;
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjMotionSpeedSet(modelId, 1.0f);
    mbObjAttrSet(modelId, 1073741825);

    modelId = (s16)mbObjCreate(13107212, NULL, FALSE);
    s02Work.eventModelId = modelId;
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjMotionSpeedSet(modelId, 0.0f);

    modelId = (s16)mbObjCreate(13107213, NULL, FALSE);
    s02Work.modelId14 = modelId;
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjMotionSpeedSet(modelId, 1.0f);
    mbObjAttrSet(modelId, 1073741825);

    modelId = (s16)mbObjCreate(13107214, NULL, FALSE);
    s02Work.modelId5C = modelId;
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjMotionSpeedSet(modelId, 1.0f);
    mbObjAttrSet(modelId, 1073741825);

    modelId = (s16)mbObjCreate(13107215, NULL, FALSE);
    s02Work.effectModelId = modelId;
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjHookSet((s16)s02Work.modelId5C, lbl_1_data_244,
        (s16)s02Work.effectModelId);

    modelId = (s16)mbObjCreate(13107216, NULL, FALSE);
    s02Work.modelId64 = modelId;
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjMotionSpeedSet(modelId, 0.0f);
    mbObjAttrSet(modelId, 1073741825);

    modelId = (s16)mbObjCreate(13107217, NULL, FALSE);
    s02Work.modelId18 = modelId;
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjMotionSpeedSet(modelId, 1.0f);
    mbObjAttrSet(modelId, 1073741825);
    mbObjDispSet(modelId, FALSE);

    for (i = 0; i < 12; i++) {
        modelId = (s16)mbObjCreate(13107218, NULL, TRUE);
        s02Work.mapObjId[i] = modelId;
        mbObjMotionTimeSet(modelId, 0.0f);
        mbObjMotionSpeedSet(modelId, 1.0f);
        mbObjAttrSet(modelId, 1073741825);
        pos = s02MapObjectInitialPositions[i];
        pos.x += 1500.0f;
        pos.z += 200.0f;
        mbObjPosSetV(modelId, &pos);
        mbObjRotSetV(modelId, &lbl_1_data_178[i]);
    }

    s02MapScrollDelta.x = (1500.0f
        + ((1500.0f + s02MapObjectInitialPositions[0].x) - s02MapObjectInitialPositions[11].x))
        / 6000.0f;
    s02MapScrollDelta.z = (200.0f
        + ((200.0f + s02MapObjectInitialPositions[0].z) - s02MapObjectInitialPositions[11].z))
        / 6000.0f;

    modelId = (s16)mbObjCreate(13107219, NULL, FALSE);
    s02Work.modelId4C = modelId;
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjMotionSpeedSet(modelId, 1.0f);
    mbObjAttrSet(modelId, 1073741825);

    modelId = (s16)mbObjCreate(13107221, NULL, FALSE);
    s02Work.modelId58 = modelId;
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjMotionSpeedSet(modelId, 1.0f);
    mbObjAttrSet(modelId, 1073741825);

    for (i = 0; i < 2; i++) {
        modelId = (s16)mbObjCreate(13107222 + i, NULL, FALSE);
        s02Work.pair[i].modelId[0] = modelId;
        mbObjMotionTimeSet(modelId, 0.0f);
        mbObjMotionSpeedSet(modelId, 0.0f);
        mbObjPosSetV(modelId, &lbl_1_data_208[i]);
        mbObjRotSetV(modelId, &lbl_1_data_220[i]);

        modelId = (s16)mbObjCreate(lbl_1_data_C8.modelId[i], NULL, FALSE);
        s02Work.pair[i].modelId[2] = modelId;
        mbObjMotionTimeSet(modelId, 0.0f);
        mbObjMotionSpeedSet(modelId, 1.0f);
        mbObjAttrSet(modelId, 1073741825);

        modelId = (s16)mbObjCreate(13107210, NULL, TRUE);
        s02Work.pair[i].modelId[3] = modelId;
        mbObjMotionTimeSet(modelId, 0.0f);
        mbObjMotionSpeedSet(modelId, 0.0f);
        mbObjHookSet((s16)s02Work.pair[i].modelId[0], lbl_1_data_20[i],
            (s16)s02Work.pair[i].modelId[3]);
    }

    modelId = (s16)mbObjCreate(mbBoardDataNumGet(327771), NULL, FALSE);
    s02Work.modelId88 = modelId;
    mbObjPosSet(modelId, 0.0f, 0.0f, 0.0f);
    mbObjDispSet(modelId, FALSE);

    modelId = (s16)mbObjCreate(13107206, NULL, FALSE);
    s02Work.modelId8C = modelId;
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjMotionSpeedSet(modelId, 1.0f);
    mbObjAttrSet(modelId, 1073741825);
}

void fn_1_1120(int playerNo, s16 id)
{
    ANIMDATA *anim;
    MBCAMERA *cameraP;
    s32 playerMotion[3];
    HuVecF delta;
    HuVecF rise;
    HuVecF objectPos;
    HuVecF particlePos;
    HuVecF masuPos;
    HuVecF pos2D;
    HuVecF pos3D;
    S02PairModelIds pairObj;
    Mtx lookAt;
    HU3D_MODELID particleModel0;
    HU3D_MODELID particleModel1;
    s16 targetMasu;
    s32 cameraNo;
    s32 i;
    u32 mAttr;
    float scale;
    s16 pairEffect;

    cameraP = mbCameraGet();
    mAttr = mbMasuMAttrGet(id);
    if (mAttr & 1) {
        cameraNo = 0;
    } else if (mAttr & 4) {
        cameraNo = 1;
    }
    pairObj = s02Work.pair[cameraNo];
    targetMasu = mbMasuFind_MAttrIdGet(-1, lbl_1_data_24C[cameraNo]);

    for (i = 0; i < 3; i++) {
        playerMotion[i] = mbPlayerMotionCreate(playerNo,
            lbl_1_data_254[i]);
    }
    anim = HuSprAnimRead(HuDataReadNum(DATANUM(DATA_effect, 3), HU_MEMNUM_OVL));
    lbl_1_bss_10[0] = 0;
    mbPlayerRotateStart(playerNo, 0, 15);
    while (!mbPlayerRotateCheck(playerNo)) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, playerMotion[2], 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);
    HuPrcSleep(30);
    mbObjMotionSpeedSet((s16)pairObj.modelId[0], 1.0f);
    mbPlayerMotionShiftSet(playerNo, 9, 0.0f,
        8.0f, HU3D_MOTATTR_NONE);
    while (!mbPlayerMotionEndCheck(playerNo)) {
        HuPrcVSleep();
    }
    while (!mbObjMotionEndCheck((s16)pairObj.modelId[0])) {
        HuPrcVSleep();
    }
    mbMasuDispMAttrSet(mAttr);
    mbMasuNextSet(targetMasu);

    particleModel0 = mbParticleCreate(anim, 128);
    mbParticleHookSet(particleModel0, fn_1_1DC8);
    Hu3DModelLayerSet(particleModel0, 5);
    particlePos = lbl_1_data_260[cameraNo];
    particlePos.y += 50.0f;
    Hu3DModelPosSetV(particleModel0, &particlePos);
    lbl_1_bss_10[0] = 1;
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);
    mbPlayerDispSet(playerNo, FALSE);
    mbObjMotionSpeedSet((s16)pairObj.modelId[3], 1.0f);
    mbCameraShakeSet(66, 50.0f);
    omVibrate((s16)playerNo, 90, 4, 4);
    mbAudFXPlay(1552);
    HuPrcSleep(6);

    particleModel1 = mbParticleCreate(anim, 128);
    mbParticleHookSet(particleModel1, fn_1_1DC8);
    Hu3DModelLayerSet(particleModel1, 5);
    Hu3DModelPosSetV(particleModel1, &particlePos);
    HuPrcSleep(60);
    rise.x = rise.y = rise.z = 0.0f;
    mbObjPosGet((s16)pairObj.modelId[0], &objectPos);
    omVibrate((s16)playerNo, 90, 7, 3);
    for (i = 0; (u32)i < 120; i++) {
        if (mbObjMotionEndCheck((s16)pairObj.modelId[3])) {
            mbObjMotionTimeSet((s16)pairObj.modelId[3], 120.0f);
            mbObjMotionStartEndSet((s16)pairObj.modelId[3], 120, 150);
            pairEffect = (s16)pairObj.modelId[3];
            mbObjAttrSet(pairEffect, HU3D_MOTATTR_LOOP);
        }
        rise.y += -16.333334f;
        objectPos.y -= 0.016666668f * rise.y;
        mbObjPosSetV((s16)pairObj.modelId[0], &objectPos);
        Hu3DModelPosSetV(particleModel0, &particlePos);
        Hu3DModelPosSetV(particleModel1, &particlePos);
        if ((u32)i == 60) {
            lbl_1_bss_10[0] = 0;
        }
        HuPrcVSleep();
    }
    mbCameraShakeReset();
    mbCameraMoveMasu(targetMasu, NULL, NULL, -1.0f,
        -1.0f, 60);
    mbCameraMoveWait();
    mbMasuPosGet(targetMasu, &masuPos);
    GwPlayer[playerNo].masuIdPrev = id;
    GwPlayer[playerNo].masuId = targetMasu;
    mbPlayerPosSetV(playerNo, &masuPos);
    mbPlayerRotYSet(playerNo, 0.0f);
    particlePos = masuPos;
    particlePos.x -= 1000.0f;
    particlePos.y += 1000.0f;
    mbObjPosSetV((s16)pairObj.modelId[0], &particlePos);
    mbObjRotSet((s16)pairObj.modelId[0], 0.0f, 0.0f,
        225.0f);
    mbObjScaleSet((s16)pairObj.modelId[0], 0.8f, 0.8f,
        0.8f);
    delta.x = masuPos.x - 100.0f - particlePos.x;
    delta.y = 100.0f + masuPos.y - particlePos.y;
    delta.z = masuPos.z - particlePos.z;

    for (i = 0; (u32)i < 32; i++) {
        float ratio;

        if ((u32)i == 27) {
            omVibrate((s16)playerNo, 30, 20, 0);
        }
        ratio = (float)i / 42.0f;
        mbObjPosSet((s16)pairObj.modelId[0],
            particlePos.x + delta.x
                * sin((M_PI * (90.0f * ratio))
                    / 180.0),
            particlePos.y + delta.y
                * sin((M_PI * (90.0f * ratio))
                    / 180.0),
            particlePos.z + delta.z
                * sin((M_PI * (90.0f * ratio))
                    / 180.0));
        HuPrcVSleep();
    }
    mbObjDispSet((s16)pairObj.modelId[0], FALSE);
    mbPlayerDispSet(playerNo, TRUE);
    mbPlayerMotionShiftSet(playerNo, 6, 0.0f,
        1.0f, HU3D_MOTATTR_LOOP);

    pos2D.x = 288.0f;
    pos2D.y = 240.0f;
    pos2D.z = 500.0f;
    mbPos2Dto3D(&pos2D, &pos3D);
    C_MTXLookAt(lookAt, &cameraP->eye, &cameraP->up,
        (Point3d *)&pos3D);
    PSMTXInverse(lookAt, lookAt);
    lookAt[0][3] = lookAt[1][3] = lookAt[2][3] = 0.0f;
    mbObjMtxSet((s16)s02Work.modelId88, &lookAt);
    mbObjPosSetV((s16)s02Work.modelId88, &pos3D);
    mbObjAlphaSet((s16)s02Work.modelId88, 255);
    mbObjDispSet((s16)s02Work.modelId88, TRUE);
    mbAudFXPlay(1113);
    mbCameraShakeSet(30, 30.000002f);
    for (i = 0; i < 30; i++) {
        float ratio;

        ratio = (float)i / 30.0f;
        scale = 1.0f + 5.0f * ratio
            + 0.5
                * sin((M_PI
                    * (16.0f * (360.0f * ratio)))
                    / 180.0);
        scale *= 0.4f;
        mbObjScaleSet((s16)s02Work.modelId88, scale, scale, scale);
        if (i >= 25) {
            mbObjAlphaSet((s16)s02Work.modelId88,
                (u8)(255.0
                    * cos(M_PI
                        * (90.0f * ((float)(i - 25)
                            / 5.0f)) / 180.0)));
        }
        HuPrcVSleep();
    }
    mbCameraShakeReset();
    mbObjDispSet((s16)s02Work.modelId88, FALSE);
    HuPrcSleep(120);
    for (i = 0; i < 3; i++) {
        mbPlayerMotionKill(playerNo, playerMotion[i]);
    }
    mbMasuDispMAttrReset(mAttr);
    mbParticleKill(particleModel0);
    mbParticleKill(particleModel1);
    HuDataDirClose(DATA_effect);
    mbWipeFadeOut();
}

void S02PairModelsReset(void)
{
    s32 i;

    for (i = 0; i < 2; i++) {
        mbObjDispSet((s16)s02Work.pair[i].modelId[0], TRUE);
        mbObjMotionTimeSet((s16)s02Work.pair[i].modelId[0], 0.0f);
        mbObjMotionSpeedSet((s16)s02Work.pair[i].modelId[0], 0.0f);
        mbObjPosSetV((s16)s02Work.pair[i].modelId[0], &lbl_1_data_278[i]);
        mbObjRotSetV((s16)s02Work.pair[i].modelId[0], &lbl_1_data_290[i]);
        mbObjScaleSet((s16)s02Work.pair[i].modelId[0], 1.0f,
            1.0f, 1.0f);
        mbObjMotionTimeSet((s16)s02Work.pair[i].modelId[3], 0.0f);
        mbObjMotionSpeedSet((s16)s02Work.pair[i].modelId[3], 0.0f);
        mbObjMotionStartEndSet((s16)s02Work.pair[i].modelId[3], 0, 150);
    }
}

void fn_1_1DC0(void)
{
}

void fn_1_1DC4(void)
{
}

void fn_1_1DC8(HU3D_MODEL *modelP, MBPARTICLE *particleP, Mtx mtx)
{
    MBPARTICLEDATA *data;
    s32 i;
    s32 activeCount;
    float angle;
    float radius;
    GXColor color = { 255, 255, 255, 192 };
    if (particleP->mode == 0) {
        data = particleP->data;
        for (i = 0; i < particleP->num; i++, data++) {
            data->activeF = (s16)(60.0f
                * (0.2f
                    + 0.1f
                        * (0.000015258789f * (frand() & S02_RANDOM_MASK))));
            angle = 360.0f * frandf();
            radius = 100.0f * frandf();
            data->pos.x = radius * cos(
                M_PI * (double)angle / 180.0);
            data->pos.y = 0.0f;
            data->pos.z = radius * sin(
                M_PI * (double)angle / 180.0);
            data->vel.x = 100.0 * (10.0
                * cos(M_PI * (double)angle
                    / 180.0));
            data->vel.y = 0.0f;
            data->vel.z = 100.0 * (10.0
                * sin(M_PI * (double)angle
                    / 180.0));
            data->scale = 100.0f
                + 20.0f * frandf();
            data->color.r = color.r;
            data->color.g = color.g;
            data->color.b = color.b;
            data->color.a = color.a;
            data->color.a = 20.0f
                + 80.0f
                    * ((float)(frand() & S02_RANDOM_MASK) * 0.000015258789f);
        }
        particleP->mode = 1;
    }
    activeCount = 0;
    data = particleP->data;
    for (i = 0; i < particleP->num; i++, data++) {
        if (data->activeF != 0) {
            data->pos.x += 0.016666668f * data->vel.x;
            data->pos.y += 0.016666668f * data->vel.y;
            data->pos.z += 0.016666668f * data->vel.z;
            data->vel.x *= 0.9f;
            data->vel.y *= 0.9f;
            data->vel.z *= 0.9f;
            data->color.a -= 10;
            if (data->color.a <= 0) {
                data->color.a = 0;
            }
            data->scale -= 0.7f;
            if (data->scale <= 0.0f) {
                data->scale = 0.0f;
            }
            activeCount++;
            data->activeF--;
            if (data->activeF <= 0) {
                data->time = 0;
                data->scale = 0.0f;
                data->color.a = 0;
            } else {
                if (data->activeF < 10 && data->color.a >= 10) {
                    data->color.a -= 10;
                }
            }
            if (data->color.a >= 10) {
                data->color.a -= 2;
            }
        }
    }
    if (lbl_1_bss_10[0] != 0 && activeCount == 0) {
        particleP->mode = 0;
    }
}

void fn_1_22C8(void)
{
}

void fn_1_22CC(void)
{
}

static char s02PairItemHookNameTbl[2][16] = {
    "itemhook_6",
    "itemhook_7"
};
char lbl_1_data_20[4][16] = {
    "itemhook",
    "itemhook_2",
    "itemhook_6",
    "itemhook_7"
};
HuVecF lbl_1_data_60 = { 0.0f, 0.0f, 300.0f };
HuVecF lbl_1_data_6C = { -46.0f, 0.0f, 0.0f };
HuVecF lbl_1_data_78 = { 1000.0f, 1500.0f, 3100.0f };
float lbl_1_data_84 = 7500.0f;
HuVecF lbl_1_data_88 = { 4906.0f, -3125.0f, -2167.0f };
HuVecF lbl_1_data_94 = { -5880.0f, -3125.0f, 2640.0f };
HuVecF lbl_1_data_A0 = { -10.0f, 0.0f, 0.0f };
s32 lbl_1_data_AC[2] = {
    CHARMOT_HSF_c000m1_323,
    CHARMOT_HSF_c000m1_344
};
S02DataB4 lbl_1_data_B4 = {
    "itemhook_8",
    { DATANUM(DATA_s02, 6), DATANUM(DATA_s02, 8) }
};
S02DataC8 lbl_1_data_C8 = {
    { DATANUM(DATA_s02, 7), DATANUM(DATA_s02, 9) },
    {
        { -230.0f, 100.0f, -800.0f },
        { 990.0f, 100.0f, 2860.0f }
    }
};
HuVecF s02MapObjectInitialPositions[12] = {
    { 4906.0f, -3125.0f, -2167.0f },
    { 4000.0f, -3125.0f, -1700.0f },
    { 3232.0f, -3125.0f, -1215.0f },
    { 2283.0f, -3125.0f, -500.0f },
    { 1310.0f, -3125.0f, -300.0f },
    { 460.0f, -3125.0f, 190.0f },
    { -520.0f, -3125.0f, 595.0f },
    { -1382.0f, -3125.0f, 1006.0f },
    { -2311.0f, -3125.0f, 1480.0f },
    { -3335.0f, -3125.0f, 1700.0f },
    { -4012.0f, -3125.0f, 1992.0f },
    { -5013.0f, -3125.0f, 2255.0f }
};
HuVecF lbl_1_data_178[12] = {
    { 0.0f, 0.0f, 0.0f },
    { 0.0f, 10.0f, 0.0f },
    { 0.0f, -65.0f, 0.0f },
    { 0.0f, -238.0f, 0.0f },
    { 0.0f, -375.0f, 0.0f },
    { -95.0f, -333.0f, 0.0f },
    { -65.0f, -375.0f, -60.0f },
    { 17.0f, -375.0f, -75.0f },
    { 22.0f, -318.0f, -40.0f },
    { 63.0f, -366.0f, -10.0f },
    { 120.0f, 0.0f, 150.0f },
    { 0.0f, -300.0f, 0.0f }
};
HuVecF lbl_1_data_208[2] = {
    { 612.6f, -143.0f, -355.0f },
    { 1409.6f, -143.0f, 1449.0f }
};
HuVecF lbl_1_data_220[3] = {
    { 0.0f, -59.0f, 0.0f },
    { 0.0f, -59.0f, 0.0f },
    { 55.1f, 0.0f, 25.0f }
};
char lbl_1_data_244[8] = "m12hook";
s32 lbl_1_data_24C[2] = { 2, 8 };
s32 lbl_1_data_254[3] = {
    CHARMOT_HSF_c000m1_318,
    CHARMOT_HSF_c000m1_345,
    CHARMOT_HSF_c000m1_323
};
HuVecF lbl_1_data_260[2] = {
    { 612.6f, -143.0f, -355.0f },
    { 1409.6f, -143.0f, 1449.0f }
};
HuVecF lbl_1_data_278[2] = {
    { 612.6f, -143.0f, -355.0f },
    { 1409.6f, -143.0f, 1449.0f }
};
HuVecF lbl_1_data_290[2] = {
    { 0.0f, -59.0f, 0.0f },
    { 0.0f, -59.0f, 0.0f }
};

S02Work s02Work;
s32 lbl_1_bss_10[3];
HuVecF s02MapScrollDelta;
s16 lbl_1_bss_0;
