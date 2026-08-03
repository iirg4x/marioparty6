#include "dolphin.h"
#include "dolphin/math.h"
#include "datanum/charmot.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/process.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/capsule.h"
#include "game/board/coin.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/wipe.h"
#include "messdir_enum.h"
#include "msm_se.h"

typedef struct EvCapWork {
    int motId[64][GW_PLAYER_MAX];
    int objId[64];
    int sprId[64];
    void *mem[64];
    int masuId[64];
    HuVecF objPos[64];
    int playerMasuId[GW_PLAYER_MAX];
    HuVecF playerPos[GW_PLAYER_MAX];
    int bgId;
    OMOBJ *obj;
} EVCAPWORK;

typedef struct CapWorkFlag {
    u8 _flag00 : 1;
    u8 _flag01 : 1;
    u8 _unused : 6;
    u8 _pad[3];
} CAPWORKFLAG;

typedef struct CapWork {
    int playerNo;
    int targetPlayerNo;
    int capsuleNo;
    int masuId;
    int masuIdNext;
    int _unk14;
    int _unk18;
    int _unk1C;
    EVCAPWORK objWork;
    CAPWORKFLAG flags;
    int _unkB6C;
    u8 _unkB70[92];
    int processNo;
    OMOBJ *explodeObj;
    OMOBJ *boostObj;
    OMOBJ *snowObj;
    OMOBJ *glowObj;
    OMOBJ *ringObj;
    OMOBJ *coinObj;
    OMOBJ *starManObj;
    OMOBJ *_unkBEC;
    OMOBJ *capLoseObj;
} CAPWORK;

enum {
    CAPTHROW_DATA_PAKKUN = 4,
    CAPTHROW_DATA_PAKKUN_MOTION_A = 5,
    CAPTHROW_DATA_PAKKUN_MOTION_B = 6,
    CAPTHROW_DATA_PAKKUN_MOTION_C = 7,
    CAPTHROW_DATA_THROWMAN = 25,
    CAPTHROW_MESSAGE_PAKKUN_RESULT = 4,
    CAPTHROW_MESSAGE_PAKKUN_NONE = 5,
    CAPTHROW_MESSAGE_THROWMAN_RESULT = 20,
};

extern void mbev_CapWait(CAPWORK *work);
extern OMOBJ *mbev_CapEffExplodeCreate(void);
extern OMOBJ *mbev_CapEffSnowCreate(void);
extern OMOBJ *mbev_CapEffCapLoseCreate(void);
extern OMOBJ *mbev_CapEffCoinCreate(void);
extern int mbev_CapEffExplodeAdd(OMOBJ *obj, HuVecF pos, HuVecF vel,
    float active, float angleStep, float fadeStep, GXColor color);
extern int mbev_CapEffSnowAdd(OMOBJ *obj, HuVecF *pos, int time);
extern void mbev_CapEffDustHeavyAdd(OMOBJ *obj, HuVecF pos);
extern void mbev_CapEffCapLoseAdd(OMOBJ *obj, int playerNo, float height,
    int count);
extern int mbev_CapEffCapLoseNumGet(OMOBJ *obj);
extern s16 mbev_CapPlayerMotionCreate(EVCAPWORK *work, int playerNo,
    int dataNum);
extern void mbev_CapPlayerMotShiftWait(int playerNo, int motionNo, u32 attr,
    BOOL shiftF);
extern void mbev_CapPlayerMoveObjInit(void);
extern void mbev_CapPlayerMoveHitCreate(int playerNo, BOOL useMotF,
    BOOL useShiftF);
extern void mbev_CapPlayerMoveEjectCreate(int playerNo, BOOL useShiftF);
extern void mbev_CapPlayerMoveMinYSet(int playerNo, float minY);
extern void mbev_CapPlayerMoveVelSet(int playerNo, float vel,
    HuVecF moveDir);
extern BOOL mbev_CapPlayerMoveObjCheck(int playerNo);
extern void mbev_CapPlayerIdleWait(void);
extern int mbev_CapObjCreate(EVCAPWORK *work, int dataNum, int *motFile,
    BOOL linkF, int delay, BOOL closeDir);
extern void mbev_CapObjPosSet(EVCAPWORK *work, int objId, int masuId,
    HuVecF *pos);
extern void mbev_CapCoinAdd(OMOBJ *obj, int playerNo, int coinNum,
    BOOL highF);
extern s16 mbev_CapCoinDisp(int playerNo, int coinNum, BOOL winMotF,
    BOOL waitF);
extern void mbWipeDissolveFadeOutTime(int time);
extern void mbWipeDissolveFadeIn(void);

void mbev_CapTogezoKill(void)
{
}

void mbev_CapKuriboKill(void)
{
}

void mbev_CapPakkun(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    int i;
    int modelId;
    int j;
    int coinNum;
    float time;
    Mtx hookMtx;
    int motFile[8];
    HuVecF playerPos;
    HuVecF tempVec;
    HuVecF playerVel;
    HuVecF effectVel;
    HuVecF playerPosOld;
    HuVecF modelPos;
    int motionId[4];
    GXColor color;
    int masuId;
    int soundId;
    float angle;
    float radius;
    float speed;

    mbev_CapWait(work);
    work->explodeObj = mbev_CapEffExplodeCreate();
    work->coinObj = mbev_CapEffCoinCreate();
    mbev_CapPlayerMoveObjInit();
    masuId = GwPlayer[work->playerNo].masuId;
    mbPlayerPosGet(work->playerNo, &playerPos);
    playerPosOld = playerPos;
    motFile[0] = DATANUM(DATA_capsulechar2,
        CAPTHROW_DATA_PAKKUN_MOTION_A);
    motFile[1] = DATANUM(DATA_capsulechar2,
        CAPTHROW_DATA_PAKKUN_MOTION_B);
    motFile[2] = DATANUM(DATA_capsulechar2,
        CAPTHROW_DATA_PAKKUN_MOTION_C);
    motFile[3] = -1;
    modelId = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsulechar2, CAPTHROW_DATA_PAKKUN), motFile, FALSE,
        5, FALSE);
    mbev_CapObjPosSet(&work->objWork, modelId,
        GwPlayer[work->playerNo].masuId, NULL);
    modelPos = playerPos;
    mbObjMotionSet(modelId, 1, HU3D_MOTATTR_NONE);
    mbObjDispSet(modelId, FALSE);
    motionId[0] = mbev_CapPlayerMotionCreate(&work->objWork, work->playerNo,
        CHARMOT_HSF_c000m1_385);
    motionId[1] = mbev_CapPlayerMotionCreate(&work->objWork, work->playerNo,
        CHARMOT_HSF_c000m1_381);
    motionId[2] = mbev_CapPlayerMotionCreate(&work->objWork, work->playerNo,
        CHARMOT_HSF_c000m1_382);
    motionId[3] = mbev_CapPlayerMotionCreate(&work->objWork, work->playerNo,
        CHARMOT_HSF_c000m1_323);
    tempVec.x = 0.0f;
    tempVec.y = 100.0f;
    tempVec.z = 0.0f;
    mbCameraMoveMasu(masuId, NULL, &tempVec, 2000.0f, -1.0f, 21);
    mbPlayerMotionShiftSet(work->playerNo, motionId[3], 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);
    HuPrcSleep(60);
    mbev_CapPlayerMotShiftWait(work->playerNo, motionId[0],
        HU3D_MOTATTR_NONE, TRUE);
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 32; j++) {
            radius = MBCapsuleEffRandF() * 100.0f;
            angle = 360.0f * MBCapsuleEffRandF();
            speed = (MBCapsuleEffRandF() * 100.0f) * 0.05f;
            tempVec.x = playerPos.x
                + (radius * sin((M_PI * angle) / 180.0f));
            tempVec.y = playerPos.y
                + ((100.0f * MBCapsuleEffRandF()) * 0.5f) + 50.0f;
            tempVec.z = playerPos.z
                + (radius * cos((M_PI * angle) / 180.0f));
            effectVel.x = speed * sin((M_PI * angle) / 180.0f);
            effectVel.y = 0.0f;
            effectVel.z = speed * cos((M_PI * angle) / 180.0f);
            time = MBCapsuleEffRandF();
            color.r = 192.0f + (63.0f * time);
            color.g = 192.0f + (63.0f * time);
            color.b = 192.0f + (63.0f * time);
            color.a = 127.0f + (63.0f * MBCapsuleEffRandF());
            mbev_CapEffExplodeAdd(work->explodeObj, tempVec, effectVel,
                ((MBCapsuleEffRandF() * 0.5f) + 1.0f) * 100.0f,
                -0.5f + MBCapsuleEffRandF(),
                (MBCapsuleEffRandF() * 0.2f) + 0.33f, color);
        }
        HuPrcVSleep();
    }
    mbAudFXPlay(MSM_SE_BRD00_65);
    mbObjDispSet(modelId, TRUE);
    mbObjMotionSet(modelId, 1, HU3D_MOTATTR_NONE);
    mbObjMotionTimeSet(modelId, 0.0f);
    mbObjMotionSpeedSet(modelId, 0.5f);
    mbObjScaleSet(modelId, 0.0f, 0.0f, 0.0f);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (i != work->playerNo && masuId == GwPlayer[i].masuId) {
            mbev_CapPlayerMoveHitCreate(i, TRUE, FALSE);
        }
    }
    mbPlayerMotionShiftSet(work->playerNo, motionId[1], 0.0f, 5.0f,
        HU3D_MOTATTR_NONE);
    mbPlayerColSnapPlayerSet(work->playerNo, FALSE);
    for (i = 0; i < 10; i++) {
        time = (float)i / 9.0f;
        if (time > 1.0f) {
            time = 1.0f;
        }
        mbObjScaleSet(modelId, time, time, time);
        Hu3DMotionCalc(mbObjModelIDGet(modelId));
        Hu3DModelObjMtxGet(mbObjModelIDGet(modelId), "itemhook_C", hookMtx);
        tempVec.x = hookMtx[0][3];
        tempVec.y = hookMtx[1][3];
        tempVec.z = hookMtx[2][3];
        if (tempVec.y > playerPos.y) {
            tempVec.y = playerPos.y;
        }
        PSVECSubtract(&tempVec, &playerPos, &playerVel);
        PSVECScale(&playerVel, &playerVel, (float)i / 9.0f);
        PSVECAdd(&playerPos, &playerVel, &tempVec);
        mbPlayerPosSetV(work->playerNo, &tempVec);
        for (j = 0; j < 2; j++) {
            radius = MBCapsuleEffRandF() * 100.0f;
            angle = 360.0f * MBCapsuleEffRandF();
            speed = (MBCapsuleEffRandF() * 100.0f) * 0.05f;
            tempVec.x = playerPos.x
                + (radius * sin((M_PI * angle) / 180.0f));
            tempVec.y = playerPos.y
                + ((100.0f * MBCapsuleEffRandF()) * 0.5f);
            tempVec.z = playerPos.z
                + (radius * cos((M_PI * angle) / 180.0f));
            effectVel.x = speed * sin((M_PI * angle) / 180.0f);
            effectVel.y = 0.0f;
            effectVel.z = speed * cos((M_PI * angle) / 180.0f);
            time = MBCapsuleEffRandF();
            color.r = 192.0f + (63.0f * time);
            color.g = 192.0f + (63.0f * time);
            color.b = 192.0f + (63.0f * time);
            color.a = 192.0f + (63.0f * MBCapsuleEffRandF());
            mbev_CapEffExplodeAdd(work->explodeObj, tempVec, effectVel,
                (MBCapsuleEffRandF() + 2.0f) * 100.0f,
                (-0.5f + MBCapsuleEffRandF()) * 2.0f,
                (MBCapsuleEffRandF() * 0.2f) + 0.33f, color);
        }
        HuPrcVSleep();
    }
    mbObjDispSet(mbPlayerObjIDGet(work->playerNo), FALSE);
    omVibrate(work->playerNo, 20, 7, 3);
    do {
        HuPrcVSleep();
    } while (!mbObjMotionEndCheck(modelId)
        || mbObjMotionShiftIDGet(modelId) != -1);
    coinNum = (mbPlayerCoinGet(work->playerNo) + 1) / 2;
    soundId = mbAudFXPlay(MSM_SE_BRD00_66);
    if (coinNum >= 1) {
        mbObjMotionShiftSet(modelId, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        HuPrcSleep(120);
    }
    if (soundId != -1) {
        mbAudFXStop(soundId);
    }
    mbAudFXPlay(MSM_SE_BRD00_67);
    mbObjMotionShiftSet(modelId, 3, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    HuPrcSleep(38);
    Hu3DMotionCalc(mbObjModelIDGet(modelId));
    Hu3DModelObjMtxGet(mbObjModelIDGet(modelId), "itemhook_C", hookMtx);
    mbPlayerMotionSet(work->playerNo, motionId[2], HU3D_MOTATTR_NONE);
    mbObjDispSet(mbPlayerObjIDGet(work->playerNo), TRUE);
    tempVec.x = playerPosOld.x;
    tempVec.y = hookMtx[1][3];
    tempVec.z = playerPosOld.z;
    mbPlayerPosSetV(work->playerNo, &tempVec);
    mbev_CapPlayerMoveEjectCreate(work->playerNo, TRUE);
    mbev_CapPlayerMoveMinYSet(work->playerNo, playerPosOld.y);
    tempVec.x = 0.0f;
    tempVec.y = 35.0f;
    tempVec.z = 0.0f;
    mbev_CapPlayerMoveVelSet(work->playerNo, 3.266667f, tempVec);
    mbCoinAddDispExec(work->playerNo, -coinNum, FALSE, TRUE);
    do {
        HuPrcVSleep();
    } while (!mbObjMotionEndCheck(modelId)
        || mbObjMotionShiftIDGet(modelId) != -1);
    for (i = 0; i < 15.0f; i++) {
        time = (float)i / 15.0f;
        mbObjScaleSet(modelId, 1.0f - time, 1.0f - time, 1.0f - time);
        HuPrcVSleep();
    }
    mbObjDispSet(modelId, FALSE);
    do {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (!mbev_CapPlayerMoveObjCheck(i)) {
                break;
            }
        }
        HuPrcVSleep();
    } while (i < GW_PLAYER_MAX);
    if (coinNum != 0) {
        mbev_CapCoinDisp(work->playerNo, -coinNum, FALSE, TRUE);
    }
    if (coinNum != 0) {
        mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02,
            CAPTHROW_MESSAGE_PAKKUN_RESULT), -1);
    } else {
        mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02,
            CAPTHROW_MESSAGE_PAKKUN_NONE), -1);
    }
    mbWinTopWait();
    if (coinNum != 0) {
        mbWipeDissolveFadeOutTime(1);
        tempVec.x = 0.0f;
        tempVec.y = 100.0f;
        tempVec.z = 0.0f;
        mbCameraMoveMasu(GwPlayer[work->targetPlayerNo].masuId, NULL,
            &tempVec, 2000.0f, -1.0f, -1);
        mbCameraMoveWait();
        mbWipeDissolveFadeIn();
        mbev_CapCoinAdd(work->coinObj, work->targetPlayerNo, coinNum, TRUE);
    }
    mbev_CapPlayerIdleWait();
    HuPrcEnd();
}

void mbev_CapPakkunKill(void)
{
}

void mbev_CapJangoKill(void)
{
}

void mbev_CapPatapataKill(void)
{
}

void mbev_CapKokamekkuKill(void)
{
}

void mbev_CapKamekkuKill(void)
{
}

void mbev_CapThrowman(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    int i;
    int modelId;
    float time;
    float alphaTime;
    HuVecF playerPos;
    HuVecF effectPos;
    HuVecF modelPos;
    HuVecF modelVel;
    int motionId[3];
    HU3D_MODEL *model;
    int masuId;
    int soundId;
    float angle;

    mbev_CapWait(work);
    work->explodeObj = mbev_CapEffExplodeCreate();
    work->snowObj = mbev_CapEffSnowCreate();
    work->capLoseObj = mbev_CapEffCapLoseCreate();
    motionId[0] = mbev_CapPlayerMotionCreate(&work->objWork, work->playerNo,
        CHARMOT_HSF_c000m1_323);
    motionId[1] = mbev_CapPlayerMotionCreate(&work->objWork, work->playerNo,
        CHARMOT_HSF_c000m1_325);
    motionId[2] = mbev_CapPlayerMotionCreate(&work->objWork, work->playerNo,
        CHARMOT_HSF_c000m1_357);
    masuId = GwPlayer[work->playerNo].masuId;
    mbPlayerPosGet(work->playerNo, &playerPos);
    soundId = mbAudFXPlay(MSM_SE_BRD00_136);
    for (i = 0; i < 120.0f; i++) {
        if ((i & 3) == 0) {
            effectPos.x = playerPos.x
                + ((MBCapsuleEffRandF() - 0.5f) * 200.0f);
            effectPos.y = playerPos.y
                + ((MBCapsuleEffRandF() * 0.2f) + 1.0f) * 600.0f;
            effectPos.z = playerPos.z
                + ((MBCapsuleEffRandF() - 0.5f) * 200.0f);
            mbev_CapEffSnowAdd(work->snowObj, &effectPos,
                (MBCapsuleEffRandF() + 3.5f) * 60.0f);
        }
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(work->playerNo, motionId[0], 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);
    i = mbPlayerObjIDGet(work->playerNo);
    i = mbObjModelIDGet(i);
    model = &Hu3DData[i];
    model->motShiftWork.speed = 3.0f;
    i = 0;
    do {
        HuPrcVSleep();
        i++;
        if ((i & 3) == 0) {
            effectPos.x = playerPos.x
                + ((MBCapsuleEffRandF() - 0.5f) * 200.0f);
            effectPos.y = playerPos.y
                + ((MBCapsuleEffRandF() * 0.2f) + 1.0f) * 600.0f;
            effectPos.z = playerPos.z
                + ((MBCapsuleEffRandF() - 0.5f) * 200.0f);
            mbev_CapEffSnowAdd(work->snowObj, &effectPos,
                (MBCapsuleEffRandF() + 3.5f) * 60.0f);
        }
    } while (!mbev_CapPlayerMotShiftCheck(work->playerNo)
        || !mbPlayerMotionEndCheck(work->playerNo));
    mbPlayerMotionShiftSet(work->playerNo, motionId[1], 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);
    modelId = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsulechar2, CAPTHROW_DATA_THROWMAN), NULL, FALSE, 5,
        FALSE);
    modelPos = playerPos;
    modelPos.y += 800.0f;
    mbObjPosSet(modelId, modelPos.x, modelPos.y, modelPos.z);
    mbObjLayerSet(modelId, 5);
    modelVel.x = modelVel.y = modelVel.z = 0.0f;
    do {
        HuPrcVSleep();
        mbPlayerPosGet(work->playerNo, &playerPos);
        PSVECAdd(&modelPos, &modelVel, &modelPos);
        modelVel.y -= 2.4500003f;
        mbObjPosSet(modelId, modelPos.x, modelPos.y, modelPos.z);
    } while (modelPos.y > playerPos.y);
    modelPos.y = playerPos.y + 10.0f;
    mbObjPosSet(modelId, modelPos.x, modelPos.y, modelPos.z);
    if (soundId != -1) {
        mbAudFXStop(soundId);
    }
    mbAudFXPlay(MSM_SE_BRD00_63);
    mbev_CapEffDustHeavyAdd(work->explodeObj, playerPos);
    modelPos = playerPos;
    mbObjPosSet(modelId, modelPos.x, modelPos.y, modelPos.z);
    mbPlayerMotionSet(work->playerNo, 10, HU3D_MOTATTR_NONE);
    mbPlayerMotionSpeedSet(work->playerNo, 0.0f);
    mbPlayerMotionTimeSet(work->playerNo, 20.0f);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (i != work->playerNo && masuId == GwPlayer[i].masuId) {
            mbPlayerMotionShiftSet(i, 6, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        if (masuId == GwPlayer[i].masuId) {
            omVibrate(i, 20, 7, 3);
        }
    }
    if (mbPlayerCapsuleNumGet(work->playerNo) > 0) {
        mbev_CapEffCapLoseAdd(work->capLoseObj, work->playerNo, 100.0f,
            mbPlayerCapsuleMaxGet());
        for (i = 0; i < mbPlayerCapsuleMaxGet(); i++) {
            mbPlayerCapsuleRemove(work->playerNo, 0);
        }
        do {
            HuPrcVSleep();
        } while (mbev_CapEffCapLoseNumGet(work->capLoseObj) > 0);
        HuPrcSleep(60);
        mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02,
            CAPTHROW_MESSAGE_THROWMAN_RESULT), -1);
        mbWinTopWait();
    } else {
        HuPrcSleep(60);
    }
    omVibrate(work->playerNo, 120, 4, 4);
    HuAudFXPlay(MSM_SE_BRD00_78);
    for (i = 0; i < 120.0f; i++) {
        time = (float)i / 120.0f;
        angle = sin((M_PI * (90.0f * time)) / 180.0f);
        if (time < 0.5f) {
            alphaTime = 0.0f;
        } else {
            alphaTime = 2.0f * (time - 0.5f);
        }
        mbObjScaleSet(modelId, 1.0f + time, 1.0f - time, 1.0f + time);
        mbObjAlphaSet(modelId, 255.0f * (1.0f - alphaTime));
        HuPrcVSleep();
    }
    mbPlayerMotionSpeedSet(work->playerNo, 1.0f);
    do {
        HuPrcVSleep();
    } while (!mbPlayerMotionEndCheck(work->playerNo));
    mbPlayerMotionShiftSet(work->playerNo, 6, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    for (i = 0; i < 60.0f; i++) {
        HuPrcVSleep();
    }
    mbev_CapPlayerIdleWait();
    HuPrcEnd();
}

void mbev_CapThrowmanKill(void)
{
}
