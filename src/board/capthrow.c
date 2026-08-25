#include "dolphin.h"
#include "dolphin/math.h"
#include "datanum/charmot.h"
#include "game/charman.h"
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
#include "game/window_enum.h"
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
    OMOBJ *coinManObj;
    OMOBJ *_unkBEC;
    OMOBJ *capLoseObj;
} CAPWORK;

typedef struct CapTogezoOMWork {
    int modelId;
    int state;
    int mode;
    int time;
    int timeMax;
    HuVecF pos;
    HuVecF velocity;
    HuVecF rot;
    HuVecF targetPos;
} CAPTOGEZOOMWORK;

#define CAPTHROW_TOGEZO_GRAVITY 1.633333357671897

enum {
    CAPTHROW_DATA_TOGEZO = 0,
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
extern OMOBJ *mbev_CapEffGlowCreate(void);
extern OMOBJ *mbev_CapEffRingHitCreate(void);
extern OMOBJ *mbev_CapEffExplodeCreate(void);
extern OMOBJ *mbev_CapEffSnowCreate(void);
extern OMOBJ *mbev_CapEffCapLoseCreate(void);
extern OMOBJ *mbev_CapEffCoinCreate(void);
extern int mbev_CapEffCoinAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel,
    float scale, float gravity, int time, int arg);
extern void mbev_CapEffCoinGlowSet(OMOBJ *obj, OMOBJ *glowObj);
extern void mbev_CapEffGlowCoinAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rot);
extern void mbev_CapEffRingHitAdd(OMOBJ *obj, HuVecF pos, HuVecF rot,
    HuVecF scale);
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
extern float mbev_CapAngleSumLerp(float t, float a, float b);
extern void mbev_CapVecChase(float weight, HuVecF *src, HuVecF *target,
    HuVecF *out);
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
extern BOOL mbev_CapCullCheck(int playerNo, int masuId);
extern int mbev_CapEffGlowAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel,
    int time, float scale, float gravity, float rotStep, GXColor *color);
extern int mbev_CapEffGlowKinokoTimeSet(OMOBJ *obj, int index, int time,
    int delay);
extern void mbev_CapPlayerMotShiftSet(int modelId, int motionNo, u32 attr,
    BOOL shiftF);
extern int mbCapObjCreate(int capsuleNo, BOOL flag);
extern void mbCapObjKill(int objId);
extern int mbCapUseMesGet(int capsuleNo);
extern void mbCapCapsuleGet(int playerNo, int capsuleNo);
extern OMOBJ *mbev_CapCoinManCreate(void);
extern int mbev_CapCoinManAdd(OMOBJ *obj, HuVecF *from, HuVecF *to,
    int targetPlayerNo, BOOL highF);
extern int mbev_CapCoinManNumGet(OMOBJ *obj);
extern int mbDiceExec(int playerNo, int diceType, s8 *valueTbl,
    int tutorialVal, BOOL padWinF, BOOL waitF, HuVecF *pos, int color);
extern void mbDiceNumKill(int playerNo);
extern s16 mbCoinDispCapsuleCreate(HuVecF *pos, int coinNum);
extern void mbev_CapBezierGetV(float time, HuVecF *a, HuVecF *b,
    HuVecF *c, HuVecF *out);
extern void mbev_CapBezierNormGetV(float time, HuVecF *a, HuVecF *b,
    HuVecF *c, HuVecF *out);
extern void mbev_CapVecRotGet(HuVecF *vec, HuVecF *rot);
extern void mbev_PlayerColMasuSet(int playerNo, int masuId, BOOL waitF);

static void ev_CapTogezoOMExec(OMOBJ *obj);
static void KokamekkuObjUpdate(float t, float angle, int *objId, int objNum,
    int scaleNo, int skipNo, HuVecF *ofs, HuVecF *startPos);

void mbev_CapTogezo(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    CAPTOGEZOOMWORK *objWork;
    OMOBJ *obj;
    HU3D_MODEL *model;
    int i;
    int j;
    int modelId;
    int playerNo;
    int targetPlayerNo;
    int playerMasuId;
    int targetMasuId;
    int coinNum;
    int coinNumBase;
    int coinNumTotal;
    HuVecF togezoPos[3];
    int motionId[16];
    float time;
    float angle;
    float angleX;
    float radius;
    HuVecF targetPlayerPos;
    HuVecF playerPos;
    HuVecF coinPos;
    HuVecF coinVel;
    HuVecF playerPosNext;
    HuVecF playerRot;
    HuVecF playerPosCur;
    HuVecF ringScale;

    mbev_CapWait(work);
    work->glowObj = mbev_CapEffGlowCreate();
    work->ringObj = mbev_CapEffRingHitCreate();
    work->coinObj = mbev_CapEffCoinCreate();
    mbev_CapEffCoinGlowSet(work->coinObj, work->glowObj);
    mbev_CapPlayerMoveObjInit();
    playerNo = work->playerNo;
    targetPlayerNo = work->targetPlayerNo;
    playerMasuId = GwPlayer[playerNo].masuId;
    targetMasuId = GwPlayer[targetPlayerNo].masuId;
    mbPlayerPosGet(targetPlayerNo, &targetPlayerPos);
    mbPlayerPosGet(playerNo, &playerPos);
    motionId[0] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
        CHARMOT_HSF_c000m1_323);
    motionId[1] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
        CHARMOT_HSF_c000m1_325);
    motionId[2] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
        CHARMOT_HSF_c000m1_344);
    modelId = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsulechar2, CAPTHROW_DATA_TOGEZO), NULL, FALSE, 5,
        FALSE);
    mbObjDispSet(modelId, FALSE);

    obj = omAddObj(mbObjMan, -32768, 0, 0, ev_CapTogezoOMExec);
    objWork = obj->data = HuMemDirectMallocNum(HEAP_HEAP,
        sizeof(CAPTOGEZOOMWORK), HU_MEMNUM_OVL);
    memset(obj->data, 0, sizeof(CAPTOGEZOOMWORK));
    objWork->modelId = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsulechar2, CAPTHROW_DATA_TOGEZO), NULL, TRUE, 0,
        FALSE);
    mbObjDispSet(objWork->modelId, FALSE);
    objWork->state = 0;
    objWork->mode = 1;
    objWork->time = 0;
    objWork->timeMax = 138;
    objWork->pos = playerPos;
    objWork->pos.y += 350.0f;
    objWork->velocity.x = objWork->velocity.y =
        objWork->velocity.z = 0.0f;
    objWork->rot.x = objWork->rot.y = 0.0f;
    objWork->rot.z = 0.0f;
    objWork->targetPos = playerPos;
    objWork->targetPos.y += 150.0f;
    mbAudFXPlay(MSM_SE_BRD00_51);
    mbPlayerMotionShiftSet(playerNo, motionId[0], 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);
    i = mbPlayerObjIDGet(playerNo);
    i = mbObjModelIDGet(i);
    model = &Hu3DData[i];
    model->motShiftWork.speed = 3.0f;
    HuPrcSleep(120);
    mbPlayerMotionShiftSet(playerNo, motionId[1], 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);

    angle = 360.0f * MBCapsuleEffRandF();
    for (i = 0; i < 3; i++) {
        radius = 100.0f * (1.0f + (1.5f * MBCapsuleEffRandF()));
        togezoPos[i].x = (float)(playerPos.x
            + (radius * sin((M_PI * angle) / 180.0)));
        togezoPos[i].y = playerPos.y
            + (100.0f * (2.5f + (2.0f * MBCapsuleEffRandF())));
        togezoPos[i].z = (float)(playerPos.z
            + (radius * cos((M_PI * angle) / 180.0)));
        angle += 60.0f + (60.0f * MBCapsuleEffRandF());
    }
    togezoPos[0] = playerPos;
    coinNumTotal = coinNumBase = coinNum = 0;
    for (i = 0; i < 3; i++) {
        if (i == 0) {
            HuPrcSleep(24);
            mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            mbPlayerPosGet(playerNo, &playerPosCur);
            mbPlayerColSnapPlayerSet(playerNo, FALSE);
        } else {
            obj = omAddObj(mbObjMan, -32768, 0, 0, ev_CapTogezoOMExec);
            objWork = obj->data = HuMemDirectMallocNum(HEAP_HEAP,
                sizeof(CAPTOGEZOOMWORK), HU_MEMNUM_OVL);
            memset(obj->data, 0, sizeof(CAPTOGEZOOMWORK));
            objWork->modelId = mbev_CapObjCreate(&work->objWork,
                DATANUM(DATA_capsulechar2, CAPTHROW_DATA_TOGEZO), NULL,
                TRUE, 0, FALSE);
            mbObjDispSet(objWork->modelId, FALSE);
            objWork->state = 1;
            objWork->mode = 0;
            objWork->time = 0;
            objWork->timeMax = 24;
            objWork->pos = togezoPos[i];
            objWork->pos.y += 500.0f;
            objWork->velocity.x = objWork->velocity.y =
                objWork->velocity.z = 0.0f;
            objWork->rot.x = 90.0f;
            objWork->rot.y = 0.0f;
            objWork->rot.z = 60.0f * (-0.5f + MBCapsuleEffRandF());
            objWork->targetPos = togezoPos[i];
            objWork->targetPos.y += 150.0f;
            for (j = 0; (float)j <= 24.0f; j++) {
                time = (float)j / 24.0f;
                playerRot.x = playerRot.z = 0.0f;
                if (i & 1) {
                    playerRot.y = 360.0f * time;
                } else {
                    playerRot.y = 360.0f * -time;
                }
                mbev_CapVecChase(time, &playerPosCur, &togezoPos[i],
                    &playerPosNext);
                playerPosNext.y = (float)(playerPosNext.y
                    + (1.5 * (100.0
                    * sin((M_PI * (180.0f * time)) / 180.0))));
                mbPlayerPosSetV(playerNo, &playerPosNext);
                mbPlayerRotSetV(playerNo, &playerRot);
                HuPrcVSleep();
            }
            mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_NONE);
            mbPlayerMotionShiftSet(playerNo, 9, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            mbPlayerPosGet(playerNo, &playerPosCur);
        }
        mbAudFXPlay(MSM_SE_BRD00_52);
        if (i < 2) {
            coinNum = 3;
            coinNumBase += coinNum;
        } else {
            coinNum = 10 - coinNumBase;
        }
        if (mbPlayerCoinGet(playerNo) < coinNum) {
            coinNum = mbPlayerCoinGet(playerNo);
        }
        mbCoinAddDispExec(playerNo, -coinNum, FALSE, TRUE);
        coinNumTotal += coinNum;
        angle = 360.0f * MBCapsuleEffRandF();
        for (j = 0; j < coinNum; j++) {
            angle += 360.0f / (float)coinNum;
            coinPos = playerPosCur;
            coinPos.y += 100.0f;
            angleX = 70.0f + (15.0f * MBCapsuleEffRandF());
            time = 65.0f * (0.8f + (0.3f * MBCapsuleEffRandF()));
            coinVel.x = (float)(time
                * (sin((M_PI * angle) / 180.0)
                * cos((M_PI * angleX) / 180.0)));
            coinVel.z = (float)(time
                * (cos((M_PI * angle) / 180.0)
                * cos((M_PI * angleX) / 180.0)));
            coinVel.y = (float)(time * sin((M_PI * angleX) / 180.0));
            mbev_CapEffCoinAdd(work->coinObj, &coinPos, &coinVel, 0.75f,
                4.9f, 30, 0);
        }
        mbPlayerPosGet(playerNo, &playerPosNext);
        playerPosNext.y += 150.0f;
        playerRot.x = 45.0f * MBCapsuleEffRandF();
        playerRot.y = 360.0f * MBCapsuleEffRandF();
        playerRot.z = 0.0f;
        mbev_CapEffGlowCoinAdd(work->glowObj, &playerPosNext, &playerRot);
        ringScale.x = 0.5f;
        ringScale.y = 3.0f;
        ringScale.z = 100.0f * (1.0f + (0.25f * MBCapsuleEffRandF()));
        mbev_CapEffRingHitAdd(work->ringObj, playerPosNext, playerRot,
            ringScale);
        omVibrate(playerNo, 20, 7, 3);
    }
    mbPlayerMotionShiftSet(playerNo, motionId[2], 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    for (j = 0; (float)j <= 36.0f; j++) {
        time = (float)j / 36.0f;
        playerRot.z = 0.0f;
        playerRot.x = 360.0f * time;
        playerRot.y = 360.0f * time;
        mbMasuPosGet(playerMasuId, &playerPos);
        mbev_CapVecChase(time, &playerPosCur, &playerPos, &playerPosNext);
        playerPosNext.y = (float)(playerPosNext.y
            + (5.0 * (100.0
            * sin((M_PI * (180.0f * time)) / 180.0))));
        mbPlayerPosSetV(playerNo, &playerPosNext);
        mbPlayerRotSetV(playerNo, &playerRot);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 6, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
    if (coinNumTotal > 0) {
        mbev_CapCoinDisp(playerNo, -coinNumTotal, FALSE, TRUE);
    } else {
        HuPrcSleep(60);
    }
    if (coinNumTotal > 0) {
        mbWipeDissolveFadeOutTime(1);
        mbCameraPlayerViewSetFast(targetPlayerNo, 0);
        mbCameraMoveWait();
        mbWipeDissolveFadeIn();
        mbev_CapCoinAdd(work->coinObj, targetPlayerNo, coinNumTotal, TRUE);
    }
    mbev_CapPlayerMotShiftWait(targetPlayerNo, 1, HU3D_MOTATTR_LOOP, TRUE);
    mbev_CapPlayerMotShiftWait(playerNo, 1, HU3D_MOTATTR_LOOP, TRUE);
    HuPrcEnd();
}

void mbev_CapTogezoKill(void)
{
}

static void ev_CapTogezoOMExec(OMOBJ *obj)
{
    CAPTOGEZOOMWORK *work = obj->data;
    HuVecF nextPos;
    float time;
    float angleX;
    float angleY;
    float speed;

    if (mbExitCheck() || obj->work[3] != 0) {
        omDelObjEx(mbObjMan, obj);
        return;
    }
    switch (work->state) {
        case 0:
            time = (float)++work->time / (float)work->timeMax;
            work->rot.y = 720.0f * time;
            mbObjPosSetV(work->modelId, &work->pos);
            mbObjRotSetV(work->modelId, &work->rot);
            mbObjScaleSet(work->modelId, time, time, time);
            mbObjDispSet(work->modelId, TRUE);
            if (time >= 1.0f) {
                work->state++;
                work->time = 0;
                work->timeMax = 12;
            }
            break;
        case 1:
            time = (float)++work->time / (float)work->timeMax;
            angleX = time * time;
            work->rot.x = mbev_CapAngleSumLerp(time, work->rot.x, 90.0f);
            if (work->mode == 0) {
                work->rot.y = 360.0f * time;
            }
            mbev_CapVecChase(angleX, &work->pos, &work->targetPos,
                &nextPos);
            mbObjPosSetV(work->modelId, &nextPos);
            mbObjRotSetV(work->modelId, &work->rot);
            mbObjDispSet(work->modelId, TRUE);
            if (time >= 1.0f) {
                angleY = (mbRandMod(1 << 15) & 1) ? 90.0f : 270.0f;
                angleY += 30.0f * (0.5f - MBCapsuleEffRandF());
                angleX = 45.0f + (10.0f * MBCapsuleEffRandF());
                speed = 35.0f;
                work->velocity.x = (float)(speed
                    * (sin((M_PI * angleY) / 180.0)
                    * cos((M_PI * angleX) / 180.0)));
                work->velocity.z = (float)(speed
                    * (cos((M_PI * angleY) / 180.0)
                    * cos((M_PI * angleX) / 180.0)));
                work->velocity.y =
                    (float)(time * sin((M_PI * angleX) / 180.0));
                work->time = 0;
                work->timeMax = 30;
                mbObjPosGet(work->modelId, &work->pos);
                work->pos.y += 100.0f;
                work->state++;
                obj->work[1] = 0;
            }
            break;
        default:
            time = (float)++work->time / (float)work->timeMax;
            PSVECAdd(&work->pos, &work->velocity, &work->pos);
            work->velocity.y =
                (float)((double)work->velocity.y - CAPTHROW_TOGEZO_GRAVITY);
            work->rot.z += 2.5f * MBCapsuleEffRandF();
            mbObjPosSetV(work->modelId, &work->pos);
            mbObjRotSetV(work->modelId, &work->rot);
            if (time >= 1.0f) {
                mbObjDispSet(work->modelId, FALSE);
                omDelObjEx(mbObjMan, obj);
            }
            break;
    }
}

enum {
    CAPTHROW_DATA_KURIBO = 1,
    CAPTHROW_DATA_KURIBO_MOTION_02 = 2,
    CAPTHROW_DATA_KURIBO_MOTION_03 = 3,
    CAPTHROW_DATA_KURIBO_MODEL_26 = 26,
    CAPTHROW_DATA_KURIBO_MODEL_27 = 27,
    CAPTHROW_DATA_JANGO = 8,
    CAPTHROW_DATA_JANGO_MOTION_09 = 9,
    CAPTHROW_DATA_JANGO_MOTION_10 = 10,
    CAPTHROW_MESSAGE_KURIBO_00 = 0,
    CAPTHROW_MESSAGE_KURIBO_01 = 1,
    CAPTHROW_MESSAGE_KURIBO_02 = 2,
    CAPTHROW_MESSAGE_KURIBO_03 = 3,
    CAPTHROW_DICE_TYPE_KURIBO = 13,
    CAPTHROW_PLAYER_MOTION_IDLE = 1,
    CAPTHROW_PLAYER_MOTION_WIN = 12,
    CAPTHROW_PLAYER_MOTION_LOSE = 13,
    CAPTHROW_RANDOM_RANGE = 1 << 15,
};

static int kuriboCoinTbl[] = { 3, 5, 10, 20, 30, 40 };
static s8 diceKuriboNumTbl[] = { 0, 1, 2, 3, -1 };

void mbev_CapKuribo(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    int motFile[8];
    HuVecF sourcePlayerPos;
    HuVecF modelPos;
    HuVecF playerPos;
    HuVecF playerRot;
    HuVecF targetPlayerPos;
    HuVecF coinFromPos;
    HuVecF coinToPos;
    HuVecF coinDispPos;
    int modelId;
    int baseModelId;
    int carryModelId;
    int playerNo;
    int diceValue;
    char coinNumStr[16];
    int coinNum[3];
    int coinDelay;
    int coinDelayCount;
    int coinAddNum;
    int targetPlayerNo;
    int i;
    int coinDispWin;
    int coinDispLose;
    float time;

    mbev_CapWait(work);
    work->coinManObj = mbev_CapCoinManCreate();
    playerNo = work->playerNo;
    targetPlayerNo = work->targetPlayerNo;
    coinNum[1] = mbPlayerCoinGet(targetPlayerNo);
    coinNum[2] = mbPlayerCoinGet(playerNo);
    mbPlayerPosGet(targetPlayerNo, &targetPlayerPos);
    mbPlayerPosGet(playerNo, &sourcePlayerPos);
    motFile[0] = DATANUM(DATA_capsulechar2,
        CAPTHROW_DATA_KURIBO_MOTION_02);
    motFile[1] = DATANUM(DATA_capsulechar2,
        CAPTHROW_DATA_KURIBO_MOTION_03);
    motFile[2] = -1;
    modelId = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsulechar2, CAPTHROW_DATA_KURIBO), motFile, FALSE,
        5, FALSE);
    mbObjMotionSet(modelId, 1, HU3D_MOTATTR_LOOP);
    mbObjDispSet(modelId, FALSE);
    baseModelId = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsulechar2, CAPTHROW_DATA_KURIBO_MODEL_26), NULL,
        TRUE, 5, FALSE);
    mbObjDispSet(baseModelId, FALSE);
    carryModelId = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsulechar2, CAPTHROW_DATA_KURIBO_MODEL_27), NULL,
        TRUE, 5, FALSE);
    mbObjDispSet(carryModelId, FALSE);
    mbObjDispSet(modelId, TRUE);
    mbObjDispSet(baseModelId, TRUE);
    mbAudFXPlay(MSM_SE_BRD00_32);
    for (i = 0; (float)i <= 60.0f; i++) {
        time = (float)i / 60.0f;
        modelPos.x = sourcePlayerPos.x;
        modelPos.y = sourcePlayerPos.y + 250.0f
            + (500.0 * cos((M_PI * (90.0f * time)) / 180.0f));
        modelPos.z = sourcePlayerPos.z;
        mbObjPosSetV(modelId, &modelPos);
        mbObjPosSetV(baseModelId, &modelPos);
        HuPrcVSleep();
    }
    mbObjMotionShiftSet(modelId, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbAudFXPlay(MSM_SE_GUIDE_57);
    mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, CAPTHROW_MESSAGE_KURIBO_00),
        2);
    mbWinTopInsertMesSet(mbPlayerNameMesGet(targetPlayerNo), 0);
    mbWinTopWait();
    mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);

    if (mbPlayerCoinGet(playerNo) > 0) {
        for (i = 1; (float)i < 60.0f; i++) {
            time = (float)i / 60.0f;
            time = (float)sin((M_PI * (90.0f * time)) / 180.0f);
            modelPos.x = sourcePlayerPos.x
                - (1.5f * (100.0f * time));
            modelPos.y = sourcePlayerPos.y + 250.0f;
            modelPos.z = sourcePlayerPos.z;
            mbObjPosSetV(modelId, &modelPos);
            mbObjPosSetV(baseModelId, &modelPos);
            HuPrcVSleep();
        }
        switch (mbRandMod(CAPTHROW_RANDOM_RANGE) % 10) {
            case 0:
            case 1:
                diceValue = 0;
                break;

            case 2:
            case 3:
            case 4:
                diceValue = 1;
                break;

            case 5:
            case 6:
            case 7:
                diceValue = 2;
                break;

            default:
                diceValue = 3;
                break;
        }
        diceValue = kuriboCoinTbl[
            (diceValue = mbDiceExec(playerNo,
                CAPTHROW_DICE_TYPE_KURIBO, diceKuriboNumTbl,
                diceValue, TRUE, TRUE, NULL, 0)) - 1];
        HuPrcSleep(30);
        mbWipeDissolveFadeOutTime(1);
        mbDiceNumKill(playerNo);
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (i != playerNo) {
                mbPlayerDispSet(i, FALSE);
            }
        }
        modelPos.x = sourcePlayerPos.x - 100.0f;
        modelPos.y = sourcePlayerPos.y + 250.0f;
        modelPos.z = sourcePlayerPos.z;
        mbObjPosSetV(modelId, &modelPos);
        mbObjPosSetV(baseModelId, &modelPos);
        mbWipeDissolveFadeIn();
        mbAudFXPlay(MSM_SE_BRD00_32);
        mbPlayerDispSet(targetPlayerNo, TRUE);
        mbPlayerColSnapPlayerSet(targetPlayerNo, FALSE);
        mbObjDispSet(carryModelId, TRUE);
        for (i = 0; (float)i <= 60.0f; i++) {
            time = (float)i / 60.0f;
            playerPos.x = sourcePlayerPos.x + 100.0f;
            playerPos.y = sourcePlayerPos.y + 250.0f
                + (500.0 * cos((M_PI * (90.0f * time)) / 180.0f));
            playerPos.z = sourcePlayerPos.z;
            playerRot.x = playerRot.z = 0.0f;
            playerRot.y = (720.0
                * sin((M_PI * (90.0f * time)) / 180.0f));
            mbPlayerPosSetV(targetPlayerNo, &playerPos);
            mbPlayerRotSetV(targetPlayerNo, &playerRot);
            mbObjPosSetV(carryModelId, &playerPos);
            HuPrcVSleep();
        }
        mbObjMotionShiftSet(modelId, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        sprintf(coinNumStr, "%d", diceValue);
        mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02,
            CAPTHROW_MESSAGE_KURIBO_01), 2);
        mbWinTopInsertMesSet((u32)coinNumStr, 0);
        mbWinTopInsertMesSet(mbPlayerNameMesGet(targetPlayerNo), 1);
        mbWinTopWait();
        mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        coinNum[1] = diceValue;
        if (coinNum[1] > mbPlayerCoinGet(playerNo)) {
            diceValue = mbPlayerCoinGet(playerNo);
            coinNum[1] = diceValue;
        }
        if (coinNum[1] > 30) {
            coinDelay = 0;
        } else if (coinNum[1] > 20) {
            coinDelay = 1;
        } else if (coinNum[1] > 10) {
            coinDelay = 2;
        } else {
            coinDelay = 3;
        }
        coinNum[0] = coinDelayCount = 0;
        do {
            if (coinNum[1] > 0) {
                coinDelayCount++;
                if (coinDelayCount > coinDelay) {
                    mbPlayerPosGet(playerNo, &coinFromPos);
                    mbPlayerPosGet(targetPlayerNo, &coinToPos);
                    coinFromPos.y += 150.0f;
                    coinToPos.y += 150.0f;
                    coinAddNum = mbev_CapCoinManAdd(work->coinManObj,
                        &coinFromPos, &coinToPos, targetPlayerNo,
                        TRUE);
                    if (coinAddNum != 0) {
                        mbPlayerCoinAdd(playerNo, -coinAddNum);
                        coinNum[1] -= coinAddNum;
                        coinDelayCount = 0;
                    }
                }
            }
            HuPrcVSleep();
        } while (coinNum[1] > 0 || mbev_CapCoinManNumGet(work->coinManObj) > 0);
        mbAudFXPlay(MSM_SE_CMN_16);
        mbPlayerWinLoseVoicePlay(targetPlayerNo,
            CAPTHROW_PLAYER_MOTION_WIN, CHARVOICEID(6));
        mbPlayerMotionShiftSet(targetPlayerNo,
            CAPTHROW_PLAYER_MOTION_WIN, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        mbPlayerPosGet(targetPlayerNo, &coinDispPos);
        coinDispPos.y += 250.0f;
        coinDispWin = mbCoinDispCapsuleCreate(&coinDispPos, diceValue);
        mbPlayerMotionShiftSet(playerNo, CAPTHROW_PLAYER_MOTION_LOSE,
            0.0f, 8.0f, HU3D_MOTATTR_NONE);
        mbPlayerPosGet(playerNo, &coinDispPos);
        coinDispPos.y += 250.0f;
        coinDispLose = mbCoinDispCapsuleCreate(&coinDispPos, -diceValue);
        while (!mbCoinDispKillCheck(coinDispWin)
            || !mbCoinDispKillCheck(coinDispLose)) {
            HuPrcVSleep();
        }
        mbWipeDissolveFadeOutTime(1);
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            mbPlayerMotionShiftSet(i, CAPTHROW_PLAYER_MOTION_IDLE, 0.0f,
                8.0f, HU3D_MOTATTR_LOOP);
            mbPlayerDispSet(i, TRUE);
        }
        mbObjDispSet(carryModelId, FALSE);
        mbPlayerPosSetV(targetPlayerNo, &targetPlayerPos);
        mbPlayerColSnapPlayerSet(targetPlayerNo, TRUE);
        modelPos.x = sourcePlayerPos.x;
        modelPos.y = sourcePlayerPos.y + 250.0f;
        modelPos.z = sourcePlayerPos.z;
        mbObjPosSetV(modelId, &modelPos);
        mbObjPosSetV(baseModelId, &modelPos);
        mbWipeDissolveFadeIn();
    } else {
        mbAudFXPlay(MSM_SE_GUIDE_58);
        mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02,
            CAPTHROW_MESSAGE_KURIBO_03), 2);
        mbWinTopWait();
    }
    mbObjMotionShiftSet(modelId, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbAudFXPlay(MSM_SE_GUIDE_57);
    mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, CAPTHROW_MESSAGE_KURIBO_02),
        2);
    mbWinTopWait();
    mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbAudFXPlay(MSM_SE_BRD00_32);
    for (i = 0; (float)i <= 60.0f; i++) {
        time = (float)i / 60.0f;
        modelPos.y = sourcePlayerPos.y + 250.0f
            + (500.0 * sin((M_PI * (90.0f * time)) / 180.0f));
        mbObjPosSetV(modelId, &modelPos);
        mbObjPosSetV(baseModelId, &modelPos);
        HuPrcVSleep();
    }
    mbObjDispSet(modelId, FALSE);
    mbObjDispSet(baseModelId, FALSE);
    HuPrcEnd();
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

static HuVecF jangoPlayerOfs = { 0.0f, -120.0f, 0.0f };

void mbev_CapJango(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    Mtx hookMtx;
    int motionId[16];
    HuVecF playerPos;
    HuVecF modelRot;
    HuVecF curveEnd;
    HuVecF curveControl;
    HuVecF playerHookPos;
    HuVecF targetPos;
    HuVecF playerRotStart;
    HuVecF modelRotStart;
    HuVecF curvePos;
    int modelId;
    int playerMasuId;
    int startMasuId;
    int soundId;
    int i;
    float time;
    float angle;
    float radius;
    float weight;

    mbev_CapWait(work);
    playerMasuId = GwPlayer[work->playerNo].masuId;
    for (i = 1; i < mbMasuNumGet(); i++) {
        if (mbMasuAttrGet(i) & MASU_FLAG_START) {
            break;
        }
    }
    if (i < mbMasuNumGet()) {
        startMasuId = i;
    } else {
        startMasuId = playerMasuId;
    }
    motionId[0] = DATANUM(DATA_capsulechar2,
        CAPTHROW_DATA_JANGO_MOTION_09);
    motionId[1] = DATANUM(DATA_capsulechar2,
        CAPTHROW_DATA_JANGO_MOTION_10);
    motionId[2] = -1;
    modelId = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsulechar2, CAPTHROW_DATA_JANGO), motionId, FALSE,
        5, FALSE);
    mbObjMotionSet(modelId, 1, HU3D_MOTATTR_LOOP);
    mbObjScaleSet(modelId, 0.5f, 0.5f, 0.5f);
    mbObjDispSet(modelId, FALSE);
    curvePos.x = curvePos.z = 0.0f;
    curvePos.y = 100.0f;
    mbCameraMoveMasu(playerMasuId, NULL, &curvePos, -1.0f, -1.0f, 30);
    motionId[0] = mbev_CapPlayerMotionCreate(&work->objWork, work->playerNo,
        CHARMOT_HSF_c000m1_376);
    motionId[1] = mbev_CapPlayerMotionCreate(&work->objWork, work->playerNo,
        CHARMOT_HSF_c000m1_323);
    mbPlayerMotionShiftSet(work->playerNo, motionId[1], 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    HuPrcSleep(60);
    mbObjDispSet(modelId, TRUE);
    mbAudFXPlay(MSM_SE_BRD00_41);
    soundId = mbAudFXPlay(MSM_SE_BRD00_42);
    for (i = 1; (float)i <= 120.0f; i++) {
        time = (float)i / 120.0f;
        radius = (float)(4.0 * (100.0
            * cos((M_PI * (90.0f * time)) / 180.0f)));
        angle = 990.0f * time;
        mbPlayerPosGet(work->playerNo, &playerPos);
        curvePos.x = (float)(playerPos.x + (radius
            * sin((M_PI * angle) / 180.0f)));
        curvePos.y = playerPos.y + 150.0f + (8.0 * (100.0
            * cos((M_PI * (90.0f * time)) / 180.0f)));
        curvePos.z = (float)(playerPos.z + (radius
            * cos((M_PI * angle) / 180.0f)));
        modelRot.x = 0.0f;
        modelRot.y = 90.0f + angle;
        modelRot.z = 0.0f;
        mbObjPosSetV(modelId, &curvePos);
        mbObjRotSetV(modelId, &modelRot);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(work->playerNo, motionId[0], 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    mbPlayerColSnapPlayerSet(work->playerNo, FALSE);
    mbObjMotionShiftSet(modelId, 2, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    omVibrate(work->playerNo, 20, 7, 3);
    for (i = 0; (float)i < 18.0f; i++) {
        time = (float)i / 18.0f;
        modelRot.x = 0.0f;
        modelRot.y = 0.0f;
        modelRot.z = 0.0f;
        mbObjRotSetV(modelId, &modelRot);
        Hu3DModelObjMtxGet(mbObjModelIDGet(modelId), "itemhook_oya", hookMtx);
        curveEnd.x = hookMtx[0][3] + jangoPlayerOfs.x;
        curveEnd.y = hookMtx[1][3] + jangoPlayerOfs.y;
        curveEnd.z = hookMtx[2][3] + jangoPlayerOfs.z;
        mbPlayerPosGet(work->playerNo, &playerPos);
        mbev_CapVecChase(time, &playerPos, &curveEnd, &curvePos);
        mbPlayerPosSetV(work->playerNo, &curvePos);
        HuPrcVSleep();
    }
    mbObjPosGet(modelId, &playerPos);
    curveControl.x = playerPos.x;
    curveControl.y = playerPos.y;
    curveControl.z = playerPos.z + 500.0f;
    curveEnd.x = playerPos.x;
    curveEnd.y = playerPos.y + 1000.0f;
    curveEnd.z = playerPos.z + 1000.0f;
    for (i = 1; (float)i <= 72.0f; i++) {
        time = (float)i / 72.0f;
        mbev_CapBezierGetV(time, &playerPos, &curveControl, &curveEnd,
            &curvePos);
        mbev_CapBezierNormGetV(time, &playerPos, &curveControl, &curveEnd,
            &modelRot);
        mbObjPosSetV(modelId, &curvePos);
        mbObjRotSetV(modelId, &modelRot);
        Hu3DMotionCalc(mbObjModelIDGet(modelId));
        Hu3DModelObjMtxGet(mbObjModelIDGet(modelId), "itemhook_oya", hookMtx);
        curvePos.x = hookMtx[0][3] + jangoPlayerOfs.x;
        curvePos.y = hookMtx[1][3] + jangoPlayerOfs.y;
        curvePos.z = hookMtx[2][3] + jangoPlayerOfs.z;
        mbPlayerPosSetV(work->playerNo, &curvePos);
        mbPlayerRotSetV(work->playerNo, &modelRot);
        HuPrcVSleep();
    }
    mbWipeDissolveFadeOutTime(1);
    mbMasuPosGet(startMasuId, &curveEnd);
    curveEnd.y += 2000.0f;
    mbObjPosSetV(modelId, &curveEnd);
    mbPlayerPosSetV(work->playerNo, &curveEnd);
    mbev_PlayerColMasuSet(work->playerNo, startMasuId, TRUE);
    curvePos.x = curvePos.z = 0.0f;
    curvePos.y = 100.0f;
    mbCameraMoveMasu(startMasuId, NULL, &curvePos, -1.0f, -1.0f, -1);
    mbCameraMoveWait();
    mbWipeDissolveFadeIn();
    mbMasuPosGet(startMasuId, &curveEnd);
    curveEnd.y += 150.0f;
    curveControl.x = curveEnd.x + 200.0f;
    curveControl.y = curveEnd.y + 300.0f;
    curveControl.z = curveEnd.z + 200.0f;
    playerPos.x = curveEnd.x - 200.0f;
    playerPos.y = curveEnd.y + 800.0f;
    playerPos.z = curveEnd.z + 600.0f;
    for (i = 1; (float)i <= 60.0f; i++) {
        time = (float)i / 60.0f;
        mbev_CapBezierGetV(time, &playerPos, &curveControl, &curveEnd,
            &curvePos);
        mbev_CapBezierNormGetV(time, &playerPos, &curveControl, &curveEnd,
            &modelRot);
        mbev_CapVecRotGet(&modelRot, &modelRot);
        mbObjPosSetV(modelId, &curvePos);
        mbObjRotSetV(modelId, &modelRot);
        Hu3DMotionCalc(mbObjModelIDGet(modelId));
        Hu3DModelObjMtxGet(mbObjModelIDGet(modelId), "itemhook_oya", hookMtx);
        curvePos.x = hookMtx[0][3] + jangoPlayerOfs.x;
        curvePos.y = hookMtx[1][3] + jangoPlayerOfs.y;
        curvePos.z = hookMtx[2][3] + jangoPlayerOfs.z;
        mbPlayerPosSetV(work->playerNo, &curvePos);
        mbPlayerRotSetV(work->playerNo, &modelRot);
        if (i == 52) {
            mbPlayerMotionShiftSet(work->playerNo, 6, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
        }
        HuPrcVSleep();
    }
    Hu3DMotionCalc(mbObjModelIDGet(modelId));
    Hu3DModelObjMtxGet(mbObjModelIDGet(modelId), "itemhook_oya", hookMtx);
    playerHookPos.x = hookMtx[0][3] + jangoPlayerOfs.x;
    playerHookPos.y = hookMtx[1][3] + jangoPlayerOfs.y;
    playerHookPos.z = hookMtx[2][3] + jangoPlayerOfs.z;
    mbMasuPosGet(startMasuId, &targetPos);
    mbPlayerRotGet(work->playerNo, &playerRotStart);
    mbObjRotGet(modelId, &modelRotStart);
    mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbMasuPosGet(startMasuId, &playerPos);
    playerPos.y += 150.0f;
    curveControl.x = playerPos.x - 200.0f;
    curveControl.y = playerPos.y;
    curveControl.z = playerPos.z + 200.0f;
    curveEnd.x = playerPos.x + 200.0f;
    curveEnd.y = playerPos.y + 800.0f;
    curveEnd.z = playerPos.z + 800.0f;
    for (i = 1; (float)i <= 60.0f; i++) {
        time = (float)i / 60.0f;
        mbev_CapBezierGetV(time, &playerPos, &curveControl, &curveEnd,
            &curvePos);
        mbev_CapBezierNormGetV(time, &playerPos, &curveControl, &curveEnd,
            &modelRot);
        mbev_CapVecRotGet(&modelRot, &modelRot);
        weight = 2.0f * time;
        if (weight > 1.0f) {
            weight = 1.0f;
        }
        modelRot.x = mbev_CapAngleSumLerp(weight,
            modelRotStart.x, modelRot.x);
        modelRot.y = mbev_CapAngleSumLerp(weight,
            modelRotStart.y, modelRot.y);
        modelRot.z = mbev_CapAngleSumLerp(weight,
            modelRotStart.z, modelRot.z);
        mbObjPosSetV(modelId, &curvePos);
        mbObjRotSetV(modelId, &modelRot);
        weight = 5.0f * time;
        if (weight > 1.0f) {
            weight = 1.0f;
        }
        mbMasuPosGet(startMasuId, &targetPos);
        mbev_CapVecChase(weight, &playerHookPos, &targetPos,
            &curvePos);
        modelRot.x = mbev_CapAngleSumLerp(weight,
            playerRotStart.x, 0.0f);
        modelRot.y = mbev_CapAngleSumLerp(weight,
            playerRotStart.y, 0.0f);
        modelRot.z = mbev_CapAngleSumLerp(weight,
            playerRotStart.z, 0.0f);
        mbPlayerPosSetV(work->playerNo, &curvePos);
        mbPlayerRotSetV(work->playerNo, &modelRot);
        HuPrcVSleep();
    }
    mbObjDispSet(modelId, FALSE);
    if (soundId != -1) {
        mbAudFXStop(soundId);
    }
    mbMasuPosGet(startMasuId, &curveEnd);
    mbPlayerPosSetV(work->playerNo, &curveEnd);
    mbPlayerRotSet(work->playerNo, 0.0f, 0.0f, 0.0f);
    GwPlayer[work->playerNo].masuId = startMasuId;
    mbPlayerColSnapPlayerSet(work->playerNo, TRUE);
    HuPrcSleep(60);
    mbPlayerMotionShiftSet(work->playerNo, CAPTHROW_PLAYER_MOTION_IDLE, 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);
    HuPrcEnd();
}
void mbev_CapJangoKill(void)
{
}

static HuVecF patapataPlayerOfs[14] = {
    { 0.0f, -140.0f, 0.0f }, { 0.0f, -160.0f, 0.0f }, { 0.0f, -140.0f, 0.0f }, { 0.0f, -150.0f, 0.0f },
    { 0.0f, -140.0f, 0.0f }, { 0.0f, -140.0f, 0.0f }, { 0.0f, -160.0f, 0.0f }, { 0.0f, -140.0f, 0.0f },
    { 0.0f, -140.0f, 0.0f }, { 0.0f, -150.0f, 0.0f }, { 0.0f, -140.0f, 0.0f }, { 0.0f, -150.0f, 0.0f },
    { 0.0f, -150.0f, 0.0f }, { 0.0f, -150.0f, 0.0f },
};
static GXColor kokamekkuColorTbl[2] = {{ 255, 127, 127, 255 }, { 127, 127, 255, 255 }};

void mbev_CapPatapata(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    Mtx hookMtx;
    int motFile[16];
    HuVecF playerPos[2];
    HuVecF modelPos[2];
    HuVecF direction[2];
    HuVecF tempPos;
    HuVecF landingPos;
    int motionId[2][2];
    int modelId[2];
    int playerNo[2];
    int playerMasu[2];
    int destinationMasu[2];
    int soundId[2];
    int objectCount;
    int i;
    int j;
    float t;
    float weight;

    mbMoveNumDispSet(work->playerNo, FALSE);
    if (GwPlayer[work->playerNo].masuId
        != GwPlayer[work->targetPlayerNo].masuId) {
        while (GwPlayer[work->targetPlayerNo].moveF) {
            HuPrcVSleep();
        }
        mbev_PlayerColMasu(work->targetPlayerNo,
            GwPlayer[work->targetPlayerNo].masuId, FALSE);
        while (GwPlayer[work->targetPlayerNo].moveF) {
            HuPrcVSleep();
        }
    }
    mbev_CapWait(work);

    playerNo[0] = work->playerNo;
    playerNo[1] = work->targetPlayerNo;
    playerMasu[0] = GwPlayer[playerNo[0]].masuId;
    playerMasu[1] = GwPlayer[playerNo[1]].masuId;
    destinationMasu[0] = playerMasu[1];
    destinationMasu[1] = playerMasu[0];
    mbPlayerPosGet(playerNo[0], &playerPos[0]);
    mbMasuPosGet(playerMasu[0], &tempPos);
    PSVECSubtract(&playerPos[0], &tempPos, &direction[0]);
    direction[0].y = 0.0f;
    mbPlayerPosGet(playerNo[1], &playerPos[1]);
    mbMasuPosGet(playerMasu[1], &tempPos);
    PSVECSubtract(&playerPos[1], &tempPos, &direction[1]);
    direction[1].y = 0.0f;
    tempPos.x = tempPos.z = 0.0f;
    tempPos.y = 100.0f;
    mbCameraMoveMasu(playerMasu[0], NULL, &tempPos, -1.0f, -1.0f, 30);

    for (i = 0; i < 2; i++) {
        motFile[0] = DATANUM(DATA_capsulechar2, 12);
        motFile[1] = DATANUM(DATA_capsulechar2, 13);
        motFile[2] = DATANUM(DATA_capsulechar2, 14);
        motFile[3] = -1;
        modelId[i] = mbev_CapObjCreate(&work->objWork,
            DATANUM(DATA_capsulechar2, 11), motFile, TRUE, 5, FALSE);
        mbObjMotionSet(modelId[i], 1, HU3D_MOTATTR_LOOP);
        mbObjMotionTimeSet(modelId[i],
            mbObjMotionMaxTimeGet(modelId[i]) * MBCapsuleEffRandF());
        mbObjDispSet(modelId[i], FALSE);
    }
    for (i = 0; i < 2; i++) {
        motionId[i][0] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo[i],
            CHARMOT_HSF_c000m1_376);
    }
    if (GwPlayer[playerNo[0]].metalF || playerMasu[0] == playerMasu[1]) {
        objectCount = 1;
    } else {
        objectCount = 2;
    }
    for (i = 0; i < objectCount; i++) {
        soundId[i] = mbAudFXPlay(MSM_SE_BRD00_53);
        mbObjDispSet(modelId[i], TRUE);
    }
    if (!mbev_CapCullCheck(playerNo[1], playerMasu[1])) {
        mbPlayerDispSet(playerNo[1], FALSE);
        mbObjDispSet(modelId[1], FALSE);
    }

    for (j = 0; (float)j < 90.0f; j++) {
        for (i = 0; i < objectCount; i++) {
            t = (float)j / 90.0f;
            mbPlayerPosGet(playerNo[i], &playerPos[i]);
            modelPos[i] = playerPos[i];
            modelPos[i].y += 150.0f
                + (100.0 * cos((M_PI * (90.0f * t)) / 180.0f) * 6.0);
            modelPos[i].z -= 100.0f;
            mbObjPosSetV(modelId[i], &modelPos[i]);
        }
        HuPrcVSleep();
    }
    mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, 6), HUWIN_SPEAKER_NOKONOKO_START);
    mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo[1]), 0);
    mbWinTopWait();

    if (GwPlayer[playerNo[0]].masuId
        == GwPlayer[playerNo[1]].masuId) {
        mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, 8), HUWIN_SPEAKER_NOKONOKO_START);
        mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo[1]), 0);
        mbWinTopWait();
    } else {
        for (i = 0; i < objectCount; i++) {
            mbObjMotionShiftSet(modelId[i], 2, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
            mbObjPosGet(modelId[i], &modelPos[i]);
            mbPlayerPosGet(playerNo[i], &playerPos[i]);
        }
        omVibrate(playerNo[0], 20, 7, 3);
        omVibrate(playerNo[1], 20, 7, 3);
        for (j = 0; (float)j < 30.0f; j++) {
            for (i = 0; i < objectCount; i++) {
                t = (float)j / 30.0f;
                tempPos = modelPos[i];
                tempPos.y += 100.0
                    * -sin((M_PI * (90.0f * t)) / 180.0f);
                mbObjPosSetV(modelId[i], &tempPos);
                Hu3DMotionCalc(mbObjModelIDGet(modelId[i]));
                Hu3DModelObjMtxGet(mbObjModelIDGet(modelId[i]),
                    "itemhook_C", hookMtx);
                tempPos.x = hookMtx[0][3]
                    + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].x;
                tempPos.y = hookMtx[1][3]
                    + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].y;
                tempPos.z = hookMtx[2][3]
                    + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].z;
                if (t < 0.5f) {
                    weight = 0.0f;
                } else {
                    weight = 2.0f * (t - 0.5f);
                }
                if (j == 15) {
                    mbPlayerMotionShiftSet(playerNo[i], motionId[i][0], 0.0f,
                        8.0f, HU3D_MOTATTR_LOOP);
                    mbPlayerColSnapPlayerSet(playerNo[i], FALSE);
                }
                mbev_CapVecChase(weight, &playerPos[i], &tempPos, &tempPos);
                mbPlayerPosSetV(playerNo[i], &tempPos);
            }
            HuPrcVSleep();
        }
        for (i = 0; i < objectCount; i++) {
            mbObjPosGet(modelId[i], &modelPos[i]);
        }

        if (GwPlayer[playerNo[0]].metalF) {
            work->explodeObj = mbev_CapEffExplodeCreate();
            for (i = 0, j = 0; (float)j < 90.0f; j++) {
                t = (float)j / 90.0f;
                tempPos = modelPos[i];
                tempPos.y += 2.0 *
                    (100.0 * sin((M_PI * (90.0f * (t * t))) / 180.0f));
                mbObjPosSetV(modelId[i], &tempPos);
                Hu3DMotionCalc(mbObjModelIDGet(modelId[i]));
                Hu3DModelObjMtxGet(mbObjModelIDGet(modelId[i]),
                    "itemhook_C", hookMtx);
                tempPos.x = hookMtx[0][3]
                    + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].x;
                tempPos.y = hookMtx[1][3]
                    + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].y;
                tempPos.z = hookMtx[2][3]
                    + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].z;
                mbPlayerPosSetV(playerNo[i], &tempPos);
                HuPrcVSleep();
            }
            for (i = 0, j = 0; (float)j < 60.0f; j++) {
                t = (float)j / 60.0f;
                tempPos = modelPos[i];
                tempPos.y += 200.0
                    + (100.0 * sin((M_PI * (720.0f * t)) / 180.0f)
                        * 0.1f);
                mbObjPosSetV(modelId[i], &tempPos);
                Hu3DMotionCalc(mbObjModelIDGet(modelId[i]));
                Hu3DModelObjMtxGet(mbObjModelIDGet(modelId[i]),
                    "itemhook_C", hookMtx);
                tempPos.x = hookMtx[0][3]
                    + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].x;
                tempPos.y = hookMtx[1][3]
                    + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].y;
                tempPos.z = hookMtx[2][3]
                    + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].z;
                mbPlayerPosSetV(playerNo[i], &tempPos);
                HuPrcVSleep();
            }
            mbObjPosGet(modelId[i], &modelPos[i]);
            mbPlayerPosGet(playerNo[i], &landingPos);
            mbObjMotionShiftSet(modelId[i], 1, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
            mbPlayerMotionShiftSet(playerNo[i], 4, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            for (i = 0, j = 1; (float)j <= 30.0f; j++) {
                t = (float)j / 30.0f;
                t = cos((M_PI * (90.0f * t)) / 180.0f);
                mbMasuPosGet(playerMasu[i], &tempPos);
                mbev_CapVecChase(t, &tempPos, &landingPos, &tempPos);
                mbPlayerPosSetV(playerNo[i], &tempPos);
                if (i == 10) {
                    mbPlayerMotionShiftSet(playerNo[i], 5, 0.0f, 8.0f,
                        HU3D_MOTATTR_NONE);
                }
                HuPrcVSleep();
            }
            mbPlayerColSnapPlayerSet(playerNo[i], TRUE);
            mbMasuPosGet(playerMasu[i], &tempPos);
            mbev_CapEffDustHeavyAdd(work->explodeObj, tempPos);
            mbPlayerMotionShiftSet(playerNo[i], 1, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
            mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, 9), HUWIN_SPEAKER_NOKONOKO_START);
            mbWinTopWait();
            for (i = 0, j = 0; (float)j < 60.0f; j++) {
                t = (float)j / 60.0f;
                tempPos = modelPos[i];
                tempPos.y += 100.0
                    * sin((M_PI * (90.0f * (t * t))) / 180.0f) * 3.0;
                mbObjPosSetV(modelId[i], &tempPos);
                HuPrcVSleep();
            }
            mbObjDispSet(modelId[i], FALSE);
            goto cleanup;
        } else {
            for (j = 0; (float)j < 90.0f; j++) {
                for (i = 0; i < objectCount; i++) {
                    t = (float)j / 90.0f;
                    tempPos = modelPos[i];
                    tempPos.y += 100.0
                        * sin((M_PI * (90.0f * (t * t))) / 180.0f) * 6.0;
                    mbObjPosSetV(modelId[i], &tempPos);
                    Hu3DMotionCalc(mbObjModelIDGet(modelId[i]));
                    Hu3DModelObjMtxGet(mbObjModelIDGet(modelId[i]),
                        "itemhook_C", hookMtx);
                    tempPos.x = hookMtx[0][3]
                        + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].x;
                    tempPos.y = hookMtx[1][3]
                        + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].y;
                    tempPos.z = hookMtx[2][3]
                        + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].z;
                    mbPlayerPosSetV(playerNo[i], &tempPos);
                }
                HuPrcVSleep();
            }
            mbWipeDissolveFadeOutTime(1);
            for (i = 0; i < 2; i++) {
                mbPlayerDispSet(playerNo[i], FALSE);
                mbObjDispSet(modelId[i], FALSE);
            }
            tempPos.x = tempPos.z = 0.0f;
            tempPos.y = 100.0f;
            mbCameraMoveMasu(destinationMasu[0], NULL, &tempPos,
                -1.0f, -1.0f, -1);
            mbCameraMoveWait();
            mbWipeDissolveFadeIn();
            for (i = 0; i < 2; i++) {
                mbPlayerDispSet(playerNo[i], TRUE);
                mbObjDispSet(modelId[i], TRUE);
            }
            if (!mbev_CapCullCheck(playerNo[1], destinationMasu[1])) {
                mbPlayerDispSet(playerNo[1], FALSE);
                mbObjDispSet(modelId[1], FALSE);
            }
            for (j = 0; (float)j < 90.0f; j++) {
                for (i = 0; i < 2; i++) {
                    t = (float)j / 90.0f;
                    mbMasuPosGet(destinationMasu[i], &modelPos[i]);
                    PSVECAdd(&modelPos[i], &direction[i ^ 1], &modelPos[i]);
                    modelPos[i].y += 150.0f;
                    modelPos[i].z -= 100.0f;
                    tempPos = modelPos[i];
                    tempPos.y += 100.0
                        * cos((M_PI * (90.0f * t)) / 180.0f) * 6.0;
                    mbObjPosSetV(modelId[i], &tempPos);
                    Hu3DMotionCalc(mbObjModelIDGet(modelId[i]));
                    Hu3DModelObjMtxGet(mbObjModelIDGet(modelId[i]),
                        "itemhook_C", hookMtx);
                    tempPos.x = hookMtx[0][3]
                        + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].x;
                    tempPos.y = hookMtx[1][3]
                        + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].y;
                    tempPos.z = hookMtx[2][3]
                        + patapataPlayerOfs[GwPlayer[playerNo[i]].charNo].z;
                    mbPlayerPosSetV(playerNo[i], &tempPos);
                }
                HuPrcVSleep();
            }
            for (i = 0; i < 2; i++) {
                mbObjMotionShiftSet(modelId[i], 1, 0.0f, 8.0f,
                    HU3D_MOTATTR_LOOP);
                mbPlayerMotionShiftSet(playerNo[i], 1, 0.0f, 8.0f,
                    HU3D_MOTATTR_LOOP);
                mbObjPosGet(modelId[i], &modelPos[i]);
                mbPlayerPosGet(playerNo[i], &playerPos[i]);
            }
            for (j = 0; (float)j < 18.0f; j++) {
                for (i = 0; i < 2; i++) {
                    t = (float)j / 18.0f;
                    tempPos = modelPos[i];
                    tempPos.y += 100.0
                        * -sin((M_PI * (180.0f * t)) / 180.0f) * 0.5;
                    mbObjPosSetV(modelId[i], &tempPos);
                    mbPlayerPosGet(playerNo[i], &playerPos[i]);
                    mbMasuPosGet(destinationMasu[i], &tempPos);
                    PSVECAdd(&tempPos, &direction[i ^ 1], &tempPos);
                    mbev_CapVecChase(t, &playerPos[i], &tempPos, &tempPos);
                    mbPlayerPosSetV(playerNo[i], &tempPos);
                }
                HuPrcVSleep();
            }
            for (i = 0; i < 2; i++) {
                mbMasuPosGet(destinationMasu[i], &playerPos[i]);
                PSVECAdd(&playerPos[i], &direction[i ^ 1], &playerPos[i]);
                mbPlayerPosSetV(playerNo[i], &playerPos[i]);
                GwPlayer[playerNo[i]].masuId = (s16)destinationMasu[i];
                mbPlayerColSnapPlayerSet(playerNo[i], TRUE);
                GwPlayer[playerNo[i]].masuIdNext = (s16)destinationMasu[i];
                mbPlayerDispSet(playerNo[i], TRUE);
            }
        }
    }

    for (i = 0; i < 2; i++) {
        mbPlayerDispSet(playerNo[i], TRUE);
    }
    mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, 7), HUWIN_SPEAKER_NOKONOKO_START);
    mbWinTopWait();
    for (j = 0; (float)j < 90.0f; j++) {
        for (i = 0; i < 2; i++) {
            t = (float)j / 90.0f;
            tempPos = modelPos[i];
            tempPos.y += 100.0
                * sin((M_PI * (90.0f * t)) / 180.0f) * 6.0;
            mbObjPosSetV(modelId[i], &tempPos);
        }
        HuPrcVSleep();
    }
cleanup:
    for (i = 0; i < 2; i++) {
        mbPlayerDispSet(playerNo[i], TRUE);
    }
    for (i = 0; i < objectCount; i++) {
        if (soundId[i] != -1) {
            mbAudFXStop(soundId[i]);
        }
    }
    if (GwPlayer[work->playerNo].moveNum > 1) {
        mbMoveNumDispSet(work->playerNo, TRUE);
    }
    HuPrcEnd();
}

void mbev_CapPatapataKill(void)
{
}

void mbev_CapKokamekku(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    s8 moveDir[20];
    int capsuleObjId[5];
    HuVecF playerPos;
    HuVecF modelPos;
    HuVecF effectVel;
    HuVecF effectPos;
    HuVecF orbitBase;
    HuVecF capsuleStart;
    int motFile[16];
    int modelId;
    int soundId;
    int playerNo;
    int targetPlayerNo;
    int capsuleNum;
    int capsuleIndex;
    int currentObj;
    int j;
    int nextObj;
    int pathCount;
    int glowIndex;
    int i;
    int capsuleNo;
    float t;
    float angle;
    float angleStep;
    float nextAngle;
    float rotation;
    float radius;
    float speed;
    float scale;
    float blend;
    float unusedValue3;
    float unusedValue2;
    GXColor color0;
    GXColor color1;
    GXColor color2;

    mbev_CapWait(work);
    work->glowObj = mbev_CapEffGlowCreate();
    playerNo = work->playerNo;
    targetPlayerNo = work->targetPlayerNo;
    mbPlayerPosGet(playerNo, &playerPos);
    motFile[0] = DATANUM(DATA_capsulechar2, 16);
    motFile[1] = DATANUM(DATA_capsulechar2, 17);
    motFile[2] = DATANUM(DATA_capsulechar2, 18);
    motFile[3] = -1;
    modelId = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsulechar2, 15), motFile, FALSE, 5, FALSE);
    mbObjMotionSet(modelId, 1, HU3D_MOTATTR_LOOP);
    mbObjDispSet(modelId, FALSE);
    modelPos = playerPos;
    modelPos.y += 300.0f;
    mbObjPosSetV(modelId, &modelPos);
    soundId = mbAudFXPlay(MSM_SE_BRD00_70);
    omVibrate(work->playerNo, 90, 7, 3);

    for (i = 1; (float)i <= 90.0f; i++) {
        t = (float)i / 90.0f;
        angle = 2160.0f
            * sin((M_PI * (90.0f * t)) / 180.0f);
        speed = 150.0f
            + (100.0f
                * sin((M_PI * (720.0f * (1.0f - t))) / 180.0f));
        unusedValue2 = (150.0f * t)
            + (100.0f * sin((M_PI * (1440.0f * t)) / 180.0f));
        unusedValue3 = (150.0f * t)
            + (100.0f
                * -sin((M_PI * (1440.0f * t)) / 180.0f));
        for (j = 0; (float)j < 1.0f + (3.0f * t); j++) {
            effectVel.x = effectVel.y = effectVel.z = 0.0f;
            effectPos.x = modelPos.x
                + (2.0f * (100.0f * (-0.5f + MBCapsuleEffRandF())));
            effectPos.y = modelPos.y
                + (2.0f * (100.0f * MBCapsuleEffRandF()));
            effectPos.z = modelPos.z
                + (2.0f * (100.0f * (-0.5f + MBCapsuleEffRandF())));
            {
                HuVecF effectPosArg;
                HuVecF effectVelArg;
                GXColor *colorP;
                HuVecF *effectVelP;
                HuVecF *effectPosP;

                color0 = kokamekkuColorTbl[0];
                colorP = &color0;
                effectVelArg = effectVel;
                effectVelP = &effectVelArg;
                effectPosArg = effectPos;
                effectPosP = &effectPosArg;
                glowIndex = mbev_CapEffGlowAdd(work->glowObj, effectPosP,
                    effectVelP,
                    (int)(60.0f * (1.0f + (0.3f * MBCapsuleEffRandF()))),
                    100.0f * (0.15f + (0.05f * MBCapsuleEffRandF())),
                    0.05f + (0.02f * MBCapsuleEffRandF()), -0.08166666f,
                    colorP);
                mbev_CapEffGlowKinokoTimeSet(work->glowObj, glowIndex,
                    mbRandMod(2) + 2, (int)(60.0f
                        * (0.5f + (0.5f * MBCapsuleEffRandF()))));
            }

            effectPos.x = modelPos.x
                + (2.0f * (100.0f * (-0.5f + MBCapsuleEffRandF())));
            effectPos.y = modelPos.y
                + (2.0f * (100.0f * MBCapsuleEffRandF()));
            effectPos.z = modelPos.z
                + (2.0f * (100.0f * (-0.5f + MBCapsuleEffRandF())));
            {
                HuVecF effectPosArg;
                HuVecF effectVelArg;
                GXColor *colorP;
                HuVecF *effectVelP;
                HuVecF *effectPosP;

                color1 = kokamekkuColorTbl[1];
                colorP = &color1;
                effectVelArg = effectVel;
                effectVelP = &effectVelArg;
                effectPosArg = effectPos;
                effectPosP = &effectPosArg;
                glowIndex = mbev_CapEffGlowAdd(work->glowObj, effectPosP,
                    effectVelP,
                    (int)(60.0f * (1.0f + (0.3f * MBCapsuleEffRandF()))),
                    100.0f * (0.15f + (0.05f * MBCapsuleEffRandF())),
                    0.05f + (0.02f * MBCapsuleEffRandF()), -0.08166666f,
                    colorP);
                mbev_CapEffGlowKinokoTimeSet(work->glowObj, glowIndex,
                    mbRandMod(2) + 2, (int)(60.0f
                        * (0.5f + (0.5f * MBCapsuleEffRandF()))));
            }
        }
        if (t > 0.5f && t <= 1.0f) {
            blend = 2.0f * (t - 0.5f);
            if (blend > 1.0f) {
                blend = 1.0f;
            }
            mbObjRotSet(modelId, 0.0f,
                720.0f
                    * sin((M_PI * (90.0f * (blend * blend))) / 180.0f),
                0.0f);
            mbObjScaleSet(modelId, 0.0001f + blend, 1.0f,
                0.0001f + blend);
            mbObjDispSet(modelId, TRUE);
        }
        HuPrcVSleep();
    }
    if (soundId != -1) {
        mbAudFXStop(soundId);
    }
    mbAudFXPlay(MSM_SE_BRD00_71);

    for (i = 0; i < 512; i++) {
        effectPos.x = modelPos.x
            + (2.0f * (100.0f * (-0.5f + MBCapsuleEffRandF())));
        effectPos.y = modelPos.y
            + (2.0f * (100.0f * (-0.5f + MBCapsuleEffRandF())));
        effectPos.z = modelPos.z
            + (2.0f * (100.0f * (-0.5f + MBCapsuleEffRandF())));
        angle = 360.0f * MBCapsuleEffRandF();
        angleStep = 30.0f * MBCapsuleEffRandF();
        speed = 100.0f * (0.08f + (0.02f * MBCapsuleEffRandF()));
        effectVel.x = speed
            * (sin((M_PI * angleStep) / 180.0f)
                * sin((M_PI * angle) / 180.0f));
        effectVel.y = speed * cos((M_PI * angleStep) / 180.0f);
        effectVel.z = speed
            * (sin((M_PI * angleStep) / 180.0f)
                * cos((M_PI * angle) / 180.0f));
        {
            HuVecF effectPosArg;
            HuVecF effectVelArg;
            GXColor *colorP;
            HuVecF *effectVelP;
            HuVecF *effectPosP;

            color2 = kokamekkuColorTbl[mbRandMod(2)];
            colorP = &color2;
            effectVelArg = effectVel;
            effectVelP = &effectVelArg;
            effectPosArg = effectPos;
            effectPosP = &effectPosArg;
            mbev_CapEffGlowAdd(work->glowObj, effectPosP, effectVelP,
                (int)(60.0f * (1.0f + (0.5f * MBCapsuleEffRandF()))),
                100.0f * (0.15f + (0.05f * MBCapsuleEffRandF())),
                0.05f + (0.02f * MBCapsuleEffRandF()), 0.08166666f,
                colorP);
        }
    }
    mbObjRotSet(modelId, 0.0f, 0.0f, 0.0f);
    mbObjScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    HuPrcSleep(30);
    mbAudFXPlay(MSM_SE_GUIDE_07);
    mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, 10),
        HUWIN_SPEAKER_KOKAMEKKU);
    mbWinTopInsertMesSet(mbPlayerNameMesGet(targetPlayerNo), 0);
    mbWinTopWait();

    capsuleNum = mbPlayerCapsuleNumGet(playerNo);
    if (capsuleNum <= 0) {
        mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, 11),
            HUWIN_SPEAKER_KOKAMEKKU);
        mbWinTopWait();
        for (i = 1; (float)i < 60.0f; i++) {
            t = (float)i / 60.0f;
            effectPos.x = modelPos.x;
            effectPos.y = modelPos.y
                + (500.0f
                    * sin((M_PI * (90.0f * (t * t))) / 180.0f));
            effectPos.z = modelPos.z;
            mbObjPosSetV(modelId, &effectPos);
            HuPrcVSleep();
        }
    } else {
        if (capsuleNum > 1) {
            capsuleIndex = mbRandMod(capsuleNum);
        } else {
            capsuleIndex = 0;
        }
        capsuleObjId[0] = mbCapObjCreate(
            mbPlayerCapsuleGet(playerNo, capsuleIndex), FALSE);
        mbObjCameraSet(capsuleObjId[0], HU3D_CAM1);
        mbObjAttrSet(capsuleObjId[0], HU3D_MOTATTR_LOOP);
        mbObjMotionSpeedSet(capsuleObjId[0], 0.0f);
        capsuleNum = 1;
        orbitBase.x = playerPos.x;
        orbitBase.y = playerPos.y + 100.0f;
        orbitBase.z = playerPos.z - 125.0f;
        rotation = nextAngle = 0.0f;
        radius = 360.0f / (float)capsuleNum;
        currentObj = nextObj = 0;
        for (i = 1; (float)i <= 12.0f; i++) {
            t = (float)i / 12.0f;
            KokamekkuObjUpdate(t, rotation, capsuleObjId, capsuleNum, currentObj,
                -1, &orbitBase, &playerPos);
            HuPrcVSleep();
        }

        if (capsuleNum > 1) {
            pathCount = 0;
            switch (mbRandMod(3)) {
                case 0:
                    moveDir[pathCount] = 1;
                    pathCount++;
                    moveDir[pathCount] = -1;
                    pathCount++;
                    break;
                case 1:
                    moveDir[pathCount] = -1;
                    pathCount++;
                    moveDir[pathCount] = 1;
                    pathCount++;
                    break;
            }
            glowIndex = mbRandMod(capsuleNum);
            if ((mbRandMod(1 << 15) & 1) != 0) {
                for (i = 0; i < glowIndex; i++) {
                    moveDir[pathCount] = 1;
                    pathCount++;
                }
            } else {
                for (i = 0; i < glowIndex; i++) {
                    moveDir[pathCount] = -1;
                    pathCount++;
                }
            }
            glowIndex = mbRandMod(capsuleNum);
            if ((mbRandMod(1 << 15) & 1) != 0) {
                for (i = 0; i < glowIndex; i++) {
                    moveDir[pathCount] = 1;
                    pathCount++;
                }
            } else {
                for (i = 0; i < glowIndex; i++) {
                    moveDir[pathCount] = -1;
                    pathCount++;
                }
            }
            moveDir[pathCount] = 0;
            pathCount++;
            for (glowIndex = 0; glowIndex < pathCount; glowIndex++) {
                mbObjMotionSpeedSet(capsuleObjId[currentObj], 1.0f);
                if (moveDir[glowIndex] != 0) {
                    HuPrcSleep(mbRandMod(20) + 10);
                    if (moveDir[glowIndex] > 0) {
                        nextAngle = rotation + radius;
                        nextObj = currentObj + 1;
                    } else {
                        nextAngle = rotation - radius;
                        nextObj = currentObj - 1;
                    }
                    if (nextAngle > 360.0f) {
                        nextAngle -= 360.0f;
                    } else if (nextAngle < 0.0f) {
                        nextAngle += 360.0f;
                    }
                    if (nextObj >= capsuleNum) {
                        nextObj -= capsuleNum;
                    } else if (nextObj < 0) {
                        nextObj += capsuleNum;
                    }
                    mbObjMotionSpeedSet(capsuleObjId[currentObj], 0.0f);
                    mbObjMotionTimeSet(capsuleObjId[currentObj], 0.0f);
                    for (i = 1; (float)i <= 12.0f; i++) {
                        t = (float)i / 12.0f;
                        KokamekkuObjUpdate(1.0f,
                            mbev_CapAngleSumLerp(t, rotation, nextAngle),
                            capsuleObjId, capsuleNum, -1, -1,
                            &orbitBase, &playerPos);
                        for (j = 0; j < capsuleNum; j++) {
                            if (j == currentObj) {
                                scale = (float)(1.0 + (-0.25f * t));
                            } else if (j == nextObj) {
                                scale = 0.75f + (0.25f * t);
                            } else {
                                scale = 0.75f;
                            }
                            mbObjScaleSet(capsuleObjId[j], scale, scale,
                                scale);
                        }
                        HuPrcVSleep();
                    }
                    rotation = nextAngle;
                    currentObj = nextObj;
                } else {
                    break;
                }
            }
            HuPrcSleep(mbRandMod(20) + 10);
            capsuleNo = mbPlayerCapsuleGet(playerNo, capsuleIndex);
            mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, 12),
                HUWIN_SPEAKER_KOKAMEKKU);
            mbWinTopInsertMesSet(mbCapUseMesGet(capsuleNo), 0);
            mbWinTopWait();
        } else {
            capsuleNo = mbPlayerCapsuleGet(playerNo, capsuleIndex);
            mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, 12),
                HUWIN_SPEAKER_KOKAMEKKU);
            mbWinTopInsertMesSet(mbCapUseMesGet(capsuleNo), 0);
            mbWinTopWait();
        }
        mbev_CapPlayerMotShiftSet(modelId, 2, HU3D_MOTATTR_NONE, TRUE);
        mbev_CapPlayerMotShiftSet(modelId, 3, HU3D_MOTATTR_LOOP, TRUE);
        mbObjPosGet(capsuleObjId[currentObj], &capsuleStart);
        PSVECSubtract(&capsuleStart, &modelPos, &effectPos);
        angleStep = PSVECMag(&effectPos);
        mbAudFXPlay(MSM_SE_BRD00_72);
        for (i = 1; (float)i <= 45.0f; i++) {
            t = (float)i / 45.0f;
            angle = 360.0f
                * sin((M_PI * (90.0f * (t * t))) / 180.0f);
            speed = angleStep
                * (cos((M_PI * (90.0f * t)) / 180.0f)
                    + (2.0f * sin((M_PI * (180.0f * t)) / 180.0f)));
            scale = 1.0f - t;
            effectPos.x = modelPos.x
                + (speed * sin((M_PI * angle) / 180.0f));
            effectPos.z = modelPos.z
                + (speed * cos((M_PI * angle) / 180.0f));
            effectPos.y = capsuleStart.y
                + (t * ((modelPos.y + 100.0f) - capsuleStart.y));
            if (t < 0.2f) {
                blend = 5.0f * t;
                if (blend > 1.0f) {
                    blend = 1.0f;
                }
                effectPos.x = capsuleStart.x
                    + (blend * (effectPos.x - capsuleStart.x));
                effectPos.z = capsuleStart.z
                    + (blend * (effectPos.z - capsuleStart.z));
            }
            mbObjPosSetV(capsuleObjId[currentObj], &effectPos);
            mbObjScaleSet(capsuleObjId[currentObj], scale, scale, scale);
            HuPrcVSleep();
        }
        mbev_CapPlayerMotShiftWait(playerNo, 13, HU3D_MOTATTR_NONE,
            TRUE);
        mbWipeDissolveFadeOutTime(1);
        mbPlayerCapsuleRemove(playerNo, capsuleIndex);
        for (i = 0; i < capsuleNum; i++) {
            mbCapObjKill(capsuleObjId[i]);
        }
        effectPos.x = 0.0f;
        effectPos.y = 100.0f;
        effectPos.z = 0.0f;
        mbCameraMoveMasu(GwPlayer[targetPlayerNo].masuId, NULL,
            &effectPos, -1.0f, -1.0f, -1);
        mbPlayerPosGet(targetPlayerNo, &modelPos);
        modelPos.y += 300.0f;
        mbObjPosSetV(modelId, &modelPos);
        mbObjMotionSet(modelId, 1, HU3D_MOTATTR_LOOP);
        mbWipeDissolveFadeIn();

        mbAudFXPlay(MSM_SE_GUIDE_07);
        mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, 13),
            HUWIN_SPEAKER_KOKAMEKKU);
        mbWinTopInsertMesSet(mbPlayerNameMesGet(playerNo), 0);
        mbWinTopInsertMesSet(mbCapUseMesGet(capsuleNo), 1);
        mbWinTopPlayerDisable(targetPlayerNo);
        mbWinTopWait();
        mbev_CapPlayerMotShiftSet(modelId, 2, HU3D_MOTATTR_NONE, TRUE);
        mbev_CapPlayerMotShiftSet(modelId, 3, HU3D_MOTATTR_LOOP, TRUE);

        if (mbPlayerCapsuleNumGet(targetPlayerNo)
            < mbPlayerCapsuleMaxGet()) {
            mbCapCapsuleGet(targetPlayerNo, capsuleNo);
            mbPlayerCapsuleAdd(targetPlayerNo, capsuleNo);
            mbPlayerWinLoseVoicePlay(targetPlayerNo, 12,
                CHARVOICEID(6));
            mbPlayerMotionShiftSet(targetPlayerNo, 12, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, 14), -1);
            mbWinTopInsertMesSet(mbCapUseMesGet(capsuleNo), 0);
            mbWinTopPlayerDisable(targetPlayerNo);
            mbWinTopWait();
            mbev_CapPlayerMotShiftSet(modelId, 1, HU3D_MOTATTR_LOOP, TRUE);
            mbPlayerMotionShiftSet(targetPlayerNo, 1, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
        } else {
            mbev_CapPlayerMotShiftSet(modelId, 1, HU3D_MOTATTR_LOOP, TRUE);
            mbAudFXPlay(MSM_SE_GUIDE_08);
            mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, 15),
                HUWIN_SPEAKER_KOKAMEKKU);
            mbWinTopPlayerDisable(targetPlayerNo);
            mbWinTopWait();
            mbev_CapPlayerMotShiftWait(targetPlayerNo, 13,
                HU3D_MOTATTR_NONE, TRUE);
            mbPlayerMotionShiftSet(targetPlayerNo, 1, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
        }
        mbAudFXPlay(MSM_SE_GUIDE_07);
        mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02, 16),
            HUWIN_SPEAKER_KOKAMEKKU);
        mbWinTopPlayerDisable(targetPlayerNo);
        mbWinTopWait();
        mbAudFXPlay(MSM_SE_BRD00_47);
        for (i = 1; (float)i < 60.0f; i++) {
            t = (float)i / 60.0f;
            effectPos.x = modelPos.x;
            effectPos.y = modelPos.y
                + (500.0f
                    * sin((M_PI * (90.0f * (t * t))) / 180.0f));
            effectPos.z = modelPos.z;
            mbObjPosSetV(modelId, &effectPos);
            HuPrcVSleep();
        }
        mbev_CapPlayerMotShiftWait(playerNo, 1, HU3D_MOTATTR_LOOP,
            TRUE);
    }
    HuPrcEnd();
}

void mbev_CapKokamekkuKill(void)
{
}

static void KokamekkuObjUpdate(float t, float angle, int *objId, int objNum,
    int scaleNo, int skipNo, HuVecF *ofs, HuVecF *startPos)
{
    HuVecF destination;
    HuVecF objectPos;
    float objectAngle;
    float scale;
    int i;

    for (i = 0; i < objNum; i++) {
        if (i == skipNo) {
            continue;
        }
        objectAngle = 360.0f - ((float)i * (360.0f / (float)objNum));
        destination.x = ofs->x
            + (125.0f
                * sin((M_PI * (objectAngle + angle)) / 180.0f));
        destination.y = ofs->y + 150.0f;
        destination.z = ofs->z
            + (125.0f
                * cos((M_PI * (objectAngle + angle)) / 180.0f));
        if (t < 1.0f) {
            objectPos.y = startPos->y
                + ((destination.y - startPos->y)
                    * sin((M_PI * (90.0f * t)) / 180.0f));
            objectPos.x = startPos->x
                + (t * (destination.x - startPos->x));
            objectPos.z = startPos->z
                + (t * (destination.z - startPos->z));
        } else {
            objectPos = destination;
        }
        if (scaleNo != -1) {
            if (i == scaleNo) {
                scale = 1.0f;
            } else {
                scale = 0.75f;
            }
            scale *= t;
            mbObjScaleSet(objId[i], scale, scale, scale);
        }
        mbObjPosSetV(objId[i], &objectPos);
    }
}

static GXColor kamekkuColorTbl[6] = {
    { 127, 255, 255, 255 }, { 255, 127, 255, 255 }, { 255, 255, 127, 255 },
    { 127, 127, 255, 255 }, { 127, 255, 127, 255 }, { 255, 127, 127, 255 },
};

typedef struct CapKamekkuOMWork {
    int modelId;
    int state;
    int posIndex;
    int time;
    int ready;
    int burstDone;
    int _unk18;
    float modelAlpha[16];
    HuVecF modelPos[16];
    CAPWORK *capWork;
} CAPKAMEKKUOMWORK;

enum {
    CAPTHROW_DATA_KAMEKKU_HIDE = 27,
    CAPTHROW_DATA_KAMEKKU = 19,
    CAPTHROW_DATA_KAMEKKU_MOTION_A = 20,
    CAPTHROW_DATA_KAMEKKU_MOTION_B = 21,
    CAPTHROW_DATA_KAMEKKU_MOTION_C = 22,
    CAPTHROW_DATA_KAMEKKU_MOTION_D = 23,
    CAPTHROW_DATA_KAMEKKU_MOTION_E = 24,
    CAPTHROW_DATA_KAMEKKU_TRAIL = 31,
    CAPTHROW_MESSAGE_KAMEKKU_INTRO = 17,
    CAPTHROW_MESSAGE_KAMEKKU_EMPTY = 18,
    CAPTHROW_MESSAGE_KAMEKKU_RESULT = 19,
};

extern void *mbev_CapMalloc(EVCAPWORK *work, int size);
extern BOOL mbev_CapPlayerCheck(int playerNo1, int playerNo2);
extern void mbev_CapObjMotionSet(int modelId, int time, int motNo,
    int nextMotNo, u32 attr, u32 unk18, BOOL shiftF, BOOL nextAttr);
extern void mbev_CapPlayerMotionSet(int playerNo, int time, int motNo,
    int nextMotNo, u32 attr, u32 unk18, BOOL shiftF, BOOL nextAttr);
extern void mbev_CapEffColorSet(GXColor *color, int colorNo);
extern s16 mbCapValueTypeGet(s16 value);

static void ev_CapKamekkuOMExec(OMOBJ *obj);

void mbev_CapKamekku(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    CAPKAMEKKUOMWORK *omWork;
    OMOBJ *obj;
    HSF_DATA *hsf;
    HuVecF playerPos;
    HuVecF modelRot;
    HuVecF modelPos;
    HuVecF glowVel;
    HuVecF masuPos;
    HuVecF glowPos;
    HuVecF posArg;
    HuVecF velArg;
    int glowId;
    int modelIdHide;
    int soundId;
    HU3D_MODEL *model;
    HSF_MATERIAL *material;
    GXColor color;
    GXColor *colorP;
    HuVecF *velP;
    HuVecF *posP;
    int motionData[16];
    int modelId;
    int playerNo;
    int teammatePlayerNo;
    int targetPlayerNo;
    int targetMasuId;
    int j;
    s8 *capsuleMasuList;
    s8 *characterMasuList;
    int capsuleMasuNum;
    int characterMasuNum;
    int i;
    float time;
    float angle;
    float radius;
    float glowGravity;
    float glowScale;

    mbev_CapWait(work);
    work->glowObj = mbev_CapEffGlowCreate();
    modelIdHide = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsulechar2, CAPTHROW_DATA_KAMEKKU_HIDE), NULL, FALSE,
        5, FALSE);
    mbObjDispSet(modelIdHide, FALSE);

    playerNo = work->playerNo;
    targetPlayerNo = work->targetPlayerNo;
    for (i = 0, teammatePlayerNo = -1; i < GW_PLAYER_MAX; i++) {
        if (playerNo != i && mbev_CapPlayerCheck(i, playerNo)) {
            teammatePlayerNo = i;
        }
    }

    mbPlayerPosGet(playerNo, &playerPos);
    motionData[0] = DATANUM(DATA_capsulechar2,
        CAPTHROW_DATA_KAMEKKU_MOTION_A);
    motionData[1] = DATANUM(DATA_capsulechar2,
        CAPTHROW_DATA_KAMEKKU_MOTION_B);
    motionData[2] = DATANUM(DATA_capsulechar2,
        CAPTHROW_DATA_KAMEKKU_MOTION_C);
    motionData[3] = DATANUM(DATA_capsulechar2,
        CAPTHROW_DATA_KAMEKKU_MOTION_D);
    motionData[4] = DATANUM(DATA_capsulechar2,
        CAPTHROW_DATA_KAMEKKU_MOTION_E);
    motionData[5] = -1;
    modelId = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsulechar2, CAPTHROW_DATA_KAMEKKU), motionData,
        FALSE, 5, FALSE);
    mbObjMotionSet(modelId, 1, HU3D_MOTATTR_LOOP);
    mbObjDispSet(modelId, FALSE);
    modelPos = playerPos;
    modelPos.y += 200.0f;
    mbObjPosSetV(modelId, &modelPos);

    obj = omAddObjEx(mbObjMan, -32768, 16, 0, OM_GRP_NONE,
        ev_CapKamekkuOMExec);
    obj->stat |= OM_STAT_MODELPAUSE;
    omWork = obj->data = HuMemDirectMallocNum(HEAP_HEAP,
        sizeof(CAPKAMEKKUOMWORK), HU_MEMNUM_OVL);
    memset(omWork, 0, sizeof(CAPKAMEKKUOMWORK));
    omWork->modelId = modelId;
    omWork->state = 0;
    omWork->posIndex = 0;
    omWork->ready = FALSE;
    omWork->burstDone = FALSE;
    omWork->_unk18 = 0;
    omWork->capWork = work;

    for (i = 0; i < 16; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_capsulechar2, CAPTHROW_DATA_KAMEKKU_TRAIL),
            HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(obj->mdlId[i], 1);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_MOTATTR_LOOP);
        time = 0.25f * ((float)(16 - i) / 16.0f);
        Hu3DModelTPLvlSet(obj->mdlId[i], time);
        Hu3DModelLayerSet(obj->mdlId[i], 3);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet(obj->mdlId[i], 1.2f, 1.2f, 1.2f);
        Hu3DMotionTimeSet(obj->mdlId[i],
            Hu3DMotionMaxTimeGet(obj->mdlId[i]) - (0.35f * (float)i));
        model = &Hu3DData[obj->mdlId[i]];
        hsf = model->hsf;
        material = hsf->material;
        for (j = 0; j < hsf->materialNum; j++, material++) {
            material->flags |= HSF_MATERIAL_ADDCOL;
        }
        omWork->modelAlpha[i] = time;
    }

    soundId = mbAudFXPlay(MSM_SE_BRD00_60);
    for (i = 1; (float)i <= 180.0f; i++) {
        time = (float)i / 180.0f;
        radius = (float)(4.0
            * (100.0 * cos((M_PI * (90.0f * time)) / 180.0f)));
        angle = 630.0f * time;
        mbPlayerPosGet(work->playerNo, &playerPos);
        masuPos.x = modelPos.x
            + (radius * sin((M_PI * angle) / 180.0f));
        masuPos.y = (float)(modelPos.y + (4.5
            * (100.0 * cos((M_PI * (90.0f * time)) / 180.0f))));
        masuPos.z = modelPos.z
            + (radius * cos((M_PI * angle) / 180.0f));
        modelRot.x = 0.0f;
        modelRot.y = 90.0f + angle;
        modelRot.z = 0.0f;
        mbObjPosSetV(modelId, &masuPos);
        mbObjRotSetV(modelId, &modelRot);
        mbObjDispSet(modelId, TRUE);

        for (j = 0; j < 4; j++) {
            glowPos.x = masuPos.x
                + ((-0.5f + MBCapsuleEffRandF()) * 100.0f * 0.75f);
            glowPos.y = masuPos.y
                + ((-0.5f + MBCapsuleEffRandF()) * 100.0f * 0.75f);
            glowPos.z = masuPos.z
                + ((-0.5f + MBCapsuleEffRandF()) * 100.0f * 0.75f);
            glowVel.x = glowVel.y = glowVel.z = 0.0f;
            color = kamekkuColorTbl[mbRandMod(6)];
            colorP = &color;
            velArg = glowVel;
            velP = &velArg;
            posArg = glowPos;
            posP = &posArg;
            glowGravity = 0.05f + (0.02f * MBCapsuleEffRandF());
            glowScale = 100.0f * (0.15f + (0.05f * MBCapsuleEffRandF()));
            glowId = mbev_CapEffGlowAdd(work->glowObj, posP, velP,
                (int)(60.0f * (1.0f + (0.3f * MBCapsuleEffRandF()))),
                glowScale, glowGravity, 0.08166666f, colorP);
            mbev_CapEffGlowKinokoTimeSet(work->glowObj, glowId, 1, 90);
        }
        HuPrcVSleep();
    }
    if (soundId != -1) {
        mbAudFXStop(soundId);
    }
    mbAudFXPlay(MSM_SE_GUIDE_43);
    mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02,
        CAPTHROW_MESSAGE_KAMEKKU_INTRO), HUWIN_SPEAKER_KAMEKKU);
    mbWinTopInsertMesSet(mbPlayerNameMesGet(targetPlayerNo), 0);
    mbWinTopWait();

    capsuleMasuList = mbev_CapMalloc(&work->objWork, 256);
    memset(capsuleMasuList, 0, 256);
    for (i = 1, capsuleMasuNum = 0; i < mbMasuNumGet(); i++) {
        if (mbCapMasuDispTypeGet(i) == 1
            && mbCapMasuPlayerGet(i) == playerNo) {
            capsuleMasuList[capsuleMasuNum] = i;
            capsuleMasuNum++;
        }
    }
    if (GWTeamFGet() && capsuleMasuNum <= 0 && teammatePlayerNo >= 0) {
        for (i = 1, capsuleMasuNum = 0; i < mbMasuNumGet(); i++) {
            if (mbCapMasuDispTypeGet(i) == 1
                && mbCapMasuPlayerGet(i) == teammatePlayerNo) {
                capsuleMasuList[capsuleMasuNum] = i;
                capsuleMasuNum++;
            }
        }
    }

    characterMasuList = mbev_CapMalloc(&work->objWork, 256);
    memset(characterMasuList, 0, 256);
    for (i = 1, characterMasuNum = 0; i < mbMasuNumGet(); i++) {
        if (mbCapMasuDispTypeGet(i) == 2
            && mbCapMasuPlayerGet(i) == playerNo) {
            characterMasuList[characterMasuNum] = i;
            characterMasuNum++;
        }
    }
    if (GWTeamFGet() && characterMasuNum <= 0 && teammatePlayerNo >= 0) {
        for (i = 1, characterMasuNum = 0; i < mbMasuNumGet(); i++) {
            if (mbCapMasuDispTypeGet(i) == 2
                && mbCapMasuPlayerGet(i) == teammatePlayerNo) {
                characterMasuList[characterMasuNum] = i;
                characterMasuNum++;
            }
        }
    }

    if (capsuleMasuNum <= 0 && characterMasuNum <= 0) {
        mbAudFXPlay(MSM_SE_GUIDE_44);
        mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02,
            CAPTHROW_MESSAGE_KAMEKKU_EMPTY), HUWIN_SPEAKER_KAMEKKU);
        mbWinTopWait();
        for (i = 1; (float)i < 60.0f; i++) {
            time = (float)i / 60.0f;
            masuPos.x = modelPos.x;
            masuPos.y = modelPos.y + (500.0f * sin((M_PI
                * (90.0f * (time * time))) / 180.0f));
            masuPos.z = modelPos.z;
            mbObjPosSetV(modelId, &masuPos);
            HuPrcVSleep();
        }
    } else {
        if (capsuleMasuNum > 0) {
        targetMasuId = capsuleMasuList[mbRandMod(capsuleMasuNum)];
    } else {
        targetMasuId = characterMasuList[mbRandMod(characterMasuNum)];
    }
    mbWipeDissolveFadeOutTime(1);
    masuPos.x = masuPos.z = 0.0f;
    masuPos.y = 100.0f;
    mbCameraMoveMasu(targetMasuId, NULL, &masuPos, -1.0f, -1.0f, -1);
    mbCameraMoveWait();
    mbMasuPosGet(targetMasuId, &masuPos);
    modelPos.x = masuPos.x;
    modelPos.y = masuPos.y + 200.0f;
    modelPos.z = masuPos.z;
    mbObjPosSetV(modelId, &modelPos);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, FALSE);
    }
    mbWipeDissolveFadeIn();
    mbAudFXPlay(MSM_SE_BRD00_61);
    mbObjMotionShiftSet(modelId, 2, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    while (mbObjMotionShiftIDGet(modelId) != -1) {
        HuPrcVSleep();
    }
    omWork->state++;
    while (!mbObjMotionEndCheck(modelId)) {
        HuPrcVSleep();
    }
    omWork->ready = TRUE;
    mbev_CapObjMotionSet(modelId, 0, 3, 1, HU3D_MOTATTR_NONE,
        HU3D_MOTATTR_LOOP, TRUE, TRUE);
    while (mbObjMotionTimeGet(modelId)
        < (glowGravity = mbObjMotionMaxTimeGet(modelId) / 2.0f)
        || mbObjMotionShiftIDGet(modelId) != -1) {
        HuPrcVSleep();
    }
    omVibrate(playerNo, 20, 7, 3);
    omVibrate(targetPlayerNo, 20, 7, 3);
    mbMasuPosGet(targetMasuId, &masuPos);
    mbCapAutoThrow(&masuPos, &masuPos, &masuPos, targetPlayerNo,
        targetMasuId, mbCapValueTypeGet(mbMasuCapsuleGet(targetMasuId)),
        TRUE, 1.0f);
    HuPrcSleep(30);

    mbWipeDissolveFadeOutTime(1);
    masuPos.x = masuPos.z = 0.0f;
    masuPos.y = 100.0f;
    mbCameraMoveMasu(GwPlayer[playerNo].masuId, NULL, &masuPos, -1.0f,
        -1.0f, -1);
    mbCameraMoveWait();
    mbPlayerPosGet(playerNo, &masuPos);
    modelPos.x = masuPos.x;
    modelPos.y = masuPos.y + 200.0f;
    modelPos.z = masuPos.z;
    mbObjPosSetV(modelId, &modelPos);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, TRUE);
    }
    mbWipeDissolveFadeIn();
    mbObjMotionShiftSet(modelId, 5, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    mbev_CapPlayerMotionSet(playerNo, 0, 13, 1, HU3D_MOTATTR_NONE,
        HU3D_MOTATTR_LOOP, TRUE, TRUE);
    mbAudFXPlay(MSM_SE_GUIDE_43);
    mbWinCreate(2, MESSNUM(MESS_CAPSULE_EX02,
        CAPTHROW_MESSAGE_KAMEKKU_RESULT), HUWIN_SPEAKER_KAMEKKU);
    mbWinTopInsertMesSet(mbPlayerNameMesGet(targetPlayerNo), 0);
    mbWinTopWait();
    mbObjMotionShiftSet(modelId, 1, 5.0f, 8.0f, HU3D_MOTATTR_LOOP);
    obj->work[3] = TRUE;
    mbPlayerMotionSet(targetPlayerNo, 1, HU3D_MOTATTR_LOOP);
    mbObjPosGet(modelId, &modelPos);
    for (i = 1; (float)i <= 60.0f; i++) {
        time = (float)i / 60.0f;
        masuPos.x = modelPos.x;
        masuPos.y = modelPos.y + (500.0f * sin((M_PI
            * (90.0f * (time * time))) / 180.0f));
        masuPos.z = modelPos.z;
        mbObjPosSetV(modelId, &masuPos);
        HuPrcVSleep();
    }
    }
    HuPrcEnd();
}

void mbev_CapKamekkuKill(void)
{
}

static void ev_CapKamekkuOMExec(OMOBJ *obj)
{
    CAPKAMEKKUOMWORK *work = obj->data;
    CAPWORK *capWork = work->capWork;
    HuVecF hookPos;
    HuVecF glowPos;
    HuVecF glowVel;
    HuVecF posArg;
    HuVecF velArg;
    Mtx hookMtx;
    GXColor color;
    GXColor colorArg;
    GXColor *colorP;
    HuVecF *velP;
    HuVecF *posP;
    int i;
    int index;
    int posIndex;
    float time;
    float angle;
    float glowScale;
    float radius;

    if (work->state >= 5 || obj->work[3] != 0 || mbExitCheck()) {
        for (i = 0; i < obj->mdlcnt; i++) {
            if (obj->mdlId[i] != HU3D_MODELID_NONE) {
                Hu3DModelKill(obj->mdlId[i]);
            }
            obj->mdlId[i] = HU3D_MODELID_NONE;
        }
        omDelObjEx(mbObjMan, obj);
        return;
    }

    if (!work->burstDone && work->ready
        && mbObjMotionShiftIDGet(work->modelId) == -1
        && mbObjMotionTimeGet(work->modelId) > 29.0f) {
        Hu3DModelObjMtxGet(mbObjModelIDGet(work->modelId), "itemhook_T",
            hookMtx);
        hookPos.x = hookMtx[0][3];
        hookPos.y = hookMtx[1][3];
        hookPos.z = hookMtx[2][3];
        for (i = 0, angle = 0.0f; i < 192; i++) {
            angle += 10.0f * (1.0f + MBCapsuleEffRandF());
            glowPos.x = hookPos.x
                + (25.0f * (-0.5f + MBCapsuleEffRandF()));
            glowPos.y = hookPos.y
                + (25.0f * (-0.5f + MBCapsuleEffRandF()));
            glowPos.z = hookPos.z + 25.0f;
            radius = 2.5f * (0.2f + (1.8f * MBCapsuleEffRandF()));
            glowVel.x = radius * sin((M_PI * angle) / 180.0f);
            glowVel.y = radius * cos((M_PI * angle) / 180.0f);
            glowVel.z = 0.0f;
            /* Retail advances the effect RNG once before choosing the color. */
            time = MBCapsuleEffRandF();
            mbev_CapEffColorSet(&color, mbRandMod(32768));
            colorArg = color;
            colorP = &colorArg;
            velArg = glowVel;
            velP = &velArg;
            posArg = glowPos;
            posP = &posArg;
            glowScale = 60.0f * (0.5f + MBCapsuleEffRandF());
            mbev_CapEffGlowAdd(capWork->glowObj, posP, velP,
                (int)(100.0f * (0.3f + (0.1f * MBCapsuleEffRandF()))),
                glowScale, 0.0f, 0.0f, colorP);
        }
        work->burstDone = TRUE;
        work->state = 4;
    }

    switch (work->state) {
        case 0:
            break;

        case 1:
            Hu3DModelObjMtxGet(mbObjModelIDGet(work->modelId),
                "itemhook_T", hookMtx);
            for (i = 0; i < obj->mdlcnt; i++) {
                work->modelPos[i].x = hookMtx[0][3];
                work->modelPos[i].y = hookMtx[1][3];
                work->modelPos[i].z = hookMtx[2][3];
                Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            }
            work->state++;
            work->time = 0;
            /* fall through */

        case 2:
            time = (float)++work->time / 60.0f;
            if (time > 1.0f) {
                time = 1.0f;
            }
            Hu3DModelObjMtxGet(mbObjModelIDGet(work->modelId),
                "itemhook_T", hookMtx);
            work->modelPos[work->posIndex].x = hookMtx[0][3];
            work->modelPos[work->posIndex].y = hookMtx[1][3];
            work->modelPos[work->posIndex].z = hookMtx[2][3];
            index = work->posIndex;
            if (++work->posIndex >= obj->mdlcnt) {
                work->posIndex = 0;
            }
            for (i = 0; i < obj->mdlcnt; i++) {
                posIndex = index - i;

                if (posIndex < 0) {
                    posIndex += obj->mdlcnt;
                }
                Hu3DModelPosSet(obj->mdlId[i], work->modelPos[posIndex].x,
                    work->modelPos[posIndex].y, work->modelPos[posIndex].z);
                Hu3DModelTPLvlSet(obj->mdlId[i], work->modelAlpha[i] * time);
            }
            if (time >= 1.0f) {
                work->state++;
            }
            break;

        case 3:
            Hu3DModelObjMtxGet(mbObjModelIDGet(work->modelId),
                "itemhook_T", hookMtx);
            work->modelPos[work->posIndex].x = hookMtx[0][3];
            work->modelPos[work->posIndex].y = hookMtx[1][3];
            work->modelPos[work->posIndex].z = hookMtx[2][3];
            index = work->posIndex;
            if (++work->posIndex >= obj->mdlcnt) {
                work->posIndex = 0;
            }
            for (i = 0; i < obj->mdlcnt; i++) {
                posIndex = index - i;

                if (posIndex < 0) {
                    posIndex += obj->mdlcnt;
                }
                Hu3DModelPosSet(obj->mdlId[i], work->modelPos[posIndex].x,
                    work->modelPos[posIndex].y, work->modelPos[posIndex].z);
            }
            break;

        case 4:
            Hu3DModelObjMtxGet(mbObjModelIDGet(work->modelId),
                "itemhook_T", hookMtx);
            work->modelPos[work->posIndex].x = hookMtx[0][3];
            work->modelPos[work->posIndex].y = hookMtx[1][3];
            work->modelPos[work->posIndex].z = hookMtx[2][3];
            index = work->posIndex;
            if (++work->posIndex >= obj->mdlcnt) {
                work->posIndex = 0;
            }
            for (i = 0; i < obj->mdlcnt; i++) {
                posIndex = index - i;

                if (posIndex < 0) {
                    posIndex += obj->mdlcnt;
                }
                if ((work->modelAlpha[i] *= 0.9f) < 0.01f) {
                    work->modelAlpha[i] = 0.0f;
                }
                Hu3DModelPosSet(obj->mdlId[i], work->modelPos[posIndex].x,
                    work->modelPos[posIndex].y, work->modelPos[posIndex].z);
                Hu3DModelTPLvlSet(obj->mdlId[i], work->modelAlpha[i]);
            }
            break;

        case 5:
            obj->work[3] = TRUE;
            break;
    }
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
