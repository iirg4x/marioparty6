#include "dolphin/math.h"
#include "dolphin.h"
#include "game/gamework.h"
#include "game/flag.h"
#include "game/memory.h"
#include "game/charman.h"
#include "game/object.h"
#include "game/process.h"
#include "game/board/audio.h"
#include "game/board/branch.h"
#include "game/board/camera.h"
#include "game/board/capsule.h"
#include "game/board/coin.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/player.h"
#include "msm_se.h"


#define CAP_WORK_MAX 64

enum {
    CAPTRAP_DATA_BOBLE = 61,
    CAPTRAP_DATA_TUMUJIKUN_MODEL = 63,
    CAPTRAP_DATA_TUMUJIKUN_MOTION_A = 64,
    CAPTRAP_DATA_TUMUJIKUN_MOTION_B = 65,
    CAPTRAP_DATA_TUMUJIKUN_EFFECT = 66,
    CAPTRAP_DATA_BIRIQ = 67,
    CAPTRAP_DATA_CAMERA_TARGET_MODEL = 68,
    CAPTRAP_DATA_DOSSUN_MODEL = 62,
    CAPTRAP_DATA_BOMHEI_BODY = 58,
    CAPTRAP_DATA_BOMHEI_ATTACHMENT = 60,
    CAPTRAP_DATA_BOMHEI_EFFECT = 59,
    CAPTRAP_DATA_TUMUJIKUN_TRAP_MOTION = DATANUM(DATA_mario, 33),
    CAPTRAP_DATA_BOMHEI_TRAP_MOTION = DATANUM(DATA_mario, 37),
    CAPTRAP_DATA_BOMHEI_NONMETAL_MOTION_A = DATANUM(DATA_mariomot, 70),
    CAPTRAP_DATA_BOMHEI_NONMETAL_MOTION_B = DATANUM(DATA_mariomot, 71),
    CAPTRAP_SE_BOBLE = 1047,
    CAPTRAP_RANDOM_MODULUS = 32768,
    CAPTRAP_BOBLE_OBJ_PRIORITY = -32768,
};

#define CAPTRAP_EFF_RAND_NEXT() \
    do { \
        if (++mbCapEffNum >= 1024) { \
            mbCapEffNum = 0; \
        } \
    } while (0)

typedef struct EvCapWork {
    int motId[CAP_WORK_MAX][GW_PLAYER_MAX];
    int objId[CAP_WORK_MAX];
    int sprId[CAP_WORK_MAX];
    void *mem[CAP_WORK_MAX];
    int masuId[CAP_WORK_MAX];
    HuVecF objPos[CAP_WORK_MAX];
    int playerMasuId[GW_PLAYER_MAX];
    HuVecF playerPos[GW_PLAYER_MAX];
    int bgId;
    OMOBJ *obj;
} EVCAPWORK;

typedef struct CapWorkFlag {
    u8 _flag00 : 1;
    u8 _flag01 : 1;
    u8 _flag02 : 1;
    u8 _flag03 : 1;
    u8 _flag04 : 1;
    u8 _flag05 : 1;
    u8 _flag06 : 1;
    u8 _flag07 : 1;
    u8 _flag08 : 1;
    u8 _flag09 : 1;
    u8 _flag10 : 1;
    u8 _flag11 : 1;
    u8 _flag12 : 1;
    u8 _flag13 : 1;
    u8 _flag14 : 1;
    u8 _flag15 : 1;
    u8 _flag16 : 1;
    u8 _flag17 : 1;
    u8 _flag18 : 1;
    u8 _flag19 : 1;
    u8 _flag20 : 1;
    u8 _flag21 : 1;
    u8 _flag22 : 1;
    u8 _flag23 : 1;
    u8 _flag24 : 1;
    u8 _flag25 : 1;
    u8 _flag26 : 1;
    u8 _flag27 : 1;
    u8 _flag28 : 1;
    u8 _flag29 : 1;
    u8 _flag30 : 1;
    u8 _flag31 : 1;
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
    OMOBJ *starManObj;
    OMOBJ *capLoseObj;
} CAPWORK;

typedef struct CapBobleWork {
    int playerNo;
    int bobleNo;
    int modelId;
    int state;
    int time;
    float arcHeight;
    float angle;
    BOOL metalF;
    BOOL finishedF;
    HuVecF pos;
    HuVecF startPos;
    HuVecF endPos;
    OMOBJ **objP;
} CAPBOBLEWORK;

extern s16 mbCoinDispCapsuleCreate(HuVecF *pos, int coinNum);
extern int mbev_CapObjCreate(EVCAPWORK *work, int dataNum, int *motFile,
    BOOL linkF, int delay, BOOL closeDir);
extern int mbev_CapBiriQShockDelayGet(int playerNo);
extern void mbev_CapBiriQShockCreate(int playerNo);
extern void mbev_CapBiriQMetalShockCreate(int playerNo);
float mbSinDeg(float deg);
float mbCosDeg(float deg);
extern int mbev_CapPlayerSquishSet(int *playerNo, int masuId);
extern void mbev_CapPlayerStunSet(int *playerNo, int playerNum, BOOL type);
extern void mbev_CapPlayerIdleWait(void);
extern OMOBJ *mbev_CapEffExplodeCreate(void);
extern void mbev_CapEffDustHeavyAdd(OMOBJ *obj, HuVecF *pos);
extern int mbev_CapEffExplodeAnimGet(OMOBJ *obj);
extern OMOBJ *mbev_CapEffRingHitCreate(void);
extern OMOBJ *mbev_CapEffGlowFireCreate(void);
extern OMOBJ *mbev_CapEffElectricCreate(void);
extern int mbev_CapEffRingAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rot,
    HuVecF *scale, int kind, int time, int bank, GXColor *color);
extern int mbev_CapEffElectricAdd(OMOBJ *obj, HuVecF *pos, int time,
    int bank);
extern void mbev_CapEffElectricModelSet(OMOBJ *obj, MBMODELID modelId,
    int effectId, HuVecF *offset);
extern int mbev_CapEffGlowAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel,
    int time, float scale, float gravity, float rotStep, GXColor *color);
extern void mbev_CapEffColorSet(GXColor *color, int colorNo);
extern int mbev_CapEffRingDispGet(OMOBJ *obj);
extern int mbev_CapEffGlowDispGet(OMOBJ *obj);
extern void mbev_CapEffRingKill(OMOBJ *obj);
extern void mbev_CapEffGlowKill(OMOBJ *obj);
extern void mbev_CapEffElectricKill(OMOBJ *obj);
extern void mbev_CapPlayerMotShiftWait(int playerNo, int motionNo, int attr,
    BOOL waitF);
extern s16 mbev_CapPlayerMotionCreate(EVCAPWORK *work, int playerNo,
    int dataNum);
extern void mbev_CapPlayerPosSet(EVCAPWORK *work, int playerNo, int masuId,
    HuVecF *pos);
extern void mbev_CapPlayerRotate(int playerNo, float angle);
extern void mbev_CapVecChase(float weight, HuVecF *src, HuVecF *target,
    HuVecF *out);
extern OMOBJ *mbev_CapEffGlowCreate(void);
extern void mbev_CapEffGlowCoinAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rot);
extern void mbev_CapEffRingHitAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rot,
    HuVecF *scale);
extern void mbWipeDissolveFadeOutTime(int time);
extern void mbWipeDissolveFadeIn(void);
extern u32 mbCapEffNum;
extern s16 *mbCapEffData;

static HuVecF biriQEffectOfs = { 0.0f, 100.0f, 0.0f };
static char captrapBomheiItemHook[] = "itemhook_c";
static int bomheiMode[GW_PLAYER_MAX];
static float bomheiRotY[GW_PLAYER_MAX];
static int ev_CapMasuNumGet(int playerNo);
static void ev_CapBobleOMExec(OMOBJ *obj);

void mbev_CapBobleMove(int playerNo);
void mbev_CapBomheiMove(int playerNo);

void mbev_CapBoble(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    OMOBJ *obj[3];
    CAPBOBLEWORK *boble;
    OMOBJ *bobleObj;
    HuVecF masuPos;
    HuVecF pos;
    HuVecF vel;
    HuVecF rot;
    int modelId[3];
    HuVecF glowPosArg;
    HuVecF glowVelArg;
    GXColor color;
    GXColor glowColorArg;
    int frame;
    int i;
    int bobleNo;
    int masuNumPrev;
    int masuNumCur;
    MBMODELID model;
    GXColor *glowColorP;
    HuVecF *glowVelP;
    HuVecF *glowPosP;
    float angle;
    float radius;
    float masuNum;
    float baseAngle;
    float time;
    float sinAngle;
    float cosAngle;
    float sinYResult;
    float sinY;
    float sinX;
    float cosX;
    float cosYResult;
    float cosY;
    float sinX2;

    work->glowObj = mbev_CapEffGlowFireCreate();
    HuPrcVSleep();
    mbMasuPosGet(work->masuIdNext, &masuPos);
    baseAngle = 0.0f;
    for (i = 0; i < 3; i++) {
        modelId[i] = mbev_CapObjCreate(&work->objWork,
            DATANUM(DATA_capsule, CAPTRAP_DATA_BOBLE), NULL,
            TRUE, FALSE, FALSE);
        mbObjLayerSet(modelId[i], 3);
        model = modelId[i];
        mbObjAttrSet(model, HU3D_MOTATTR_LOOP);
        mbObjDispSet(modelId[i], FALSE);
        angle = baseAngle + 120.0f * (float)i;
        bobleObj = obj[i] = omAddObjEx(mbObjMan, CAPTRAP_BOBLE_OBJ_PRIORITY,
            0, 0, OM_GRP_NONE, ev_CapBobleOMExec);
        boble = bobleObj->data =
            HuMemDirectMallocNum(HEAP_HEAP, sizeof(*boble), HU_MEMNUM_OVL);
        memset(boble, 0, sizeof(*boble));
        boble->playerNo = work->playerNo;
        boble->bobleNo = i;
        boble->modelId = modelId[i];
        boble->state = 0;
        boble->time = 0;
        boble->metalF = GwPlayer[work->playerNo].metalF;
        boble->finishedF = FALSE;
        boble->arcHeight = 100.0f * (2.0f + MBCapsuleEffRandF());
        boble->angle = angle;
        boble->pos.x = masuPos.x
            + 2.0 * (100.0 * sin((M_PI * angle) / 180.0f));
        boble->pos.y = masuPos.y + 100.0f;
        boble->pos.z = masuPos.z
            + 2.0 * (100.0 * cos((M_PI * angle) / 180.0f));
        boble->startPos = boble->pos;
        boble->endPos.x = masuPos.x + 50.0f * (-0.5f + MBCapsuleEffRandF());
        boble->endPos.y = masuPos.y + 100.0f;
        boble->endPos.z = masuPos.z + 50.0f * (-0.5f + MBCapsuleEffRandF());
        boble->objP = &obj[i];
        HuPrcVSleep();
    }
    while (ev_CapMasuNumGet(work->playerNo) < 0 || ev_CapMasuNumGet(work->playerNo) > 60) {
        HuPrcVSleep();
    }
    bobleNo = frame = 0;
    do {
        HuPrcVSleep();
        if (++frame > 1 && bobleNo < 3) {
            mbAudFXPlay(CAPTRAP_SE_BOBLE);
            boble = obj[bobleNo]->data;
            boble->state++;
            frame = 0;
            bobleNo++;
        }
        masuNumPrev = ev_CapMasuNumGet(work->playerNo);
    } while (masuNumPrev < 0 || masuNumPrev > 20 || bobleNo < 3);

    masuNum = (float)ev_CapMasuNumGet(work->playerNo);
    if (masuNum <= 0.0f) {
        masuNum = 1.0f;
    }
    bobleNo = frame = 0;
    do {
        time = 1.0f
            - ((float)(ev_CapMasuNumGet(work->playerNo) - 1) / masuNum);
        masuNumCur = ev_CapMasuNumGet(work->playerNo);
        if (++frame > 1 && bobleNo < 3) {
            boble = obj[bobleNo]->data;
            boble->state++;
            frame = 0;
            bobleNo++;
        }
        HuPrcVSleep();
    } while (time < 1.0f && ev_CapMasuNumGet(work->playerNo) <= masuNumCur);

    for (i = 0; i < 128; i++) {
        rot.x = 45.0f * MBCapsuleEffRandF();
        rot.y = 360.0f * MBCapsuleEffRandF();
        radius = 100.0f * ((0.3f * 0.7f) * MBCapsuleEffRandF());
        pos.x = masuPos.x
            + 0.5f * (100.0f * (-0.5f + MBCapsuleEffRandF()));
        pos.y = masuPos.y
            + 0.5f * (100.0f * (-0.5f + MBCapsuleEffRandF()))
            + 100.0f;
        pos.z = masuPos.z
            + 0.5f * (100.0f * (-0.5f + MBCapsuleEffRandF()));
        sinAngle = rot.y;
        sinYResult = mbSinDeg(sinAngle);
        sinY = sinYResult;
        sinX = mbSinDeg(rot.x);
        vel.x = (radius * sinX) * sinY;
        cosX = mbCosDeg(rot.x);
        vel.y = radius * cosX;
        cosAngle = rot.y;
        cosYResult = mbCosDeg(cosAngle);
        cosY = cosYResult;
        sinX2 = mbSinDeg(rot.x);
        vel.z = (radius * sinX2) * cosY;
        color.r = (u8)(128.0f + 127.0f * MBCapsuleEffRandF());
        color.g = (u8)(64.0f + 63.0f * MBCapsuleEffRandF());
        color.b = 32;
        color.a = (u8)(192.0f + 63.0f * MBCapsuleEffRandF());
        glowColorArg = color;
        glowColorP = &glowColorArg;
        glowVelArg = vel;
        glowVelP = &glowVelArg;
        glowPosArg = pos;
        glowPosP = &glowPosArg;
        mbev_CapEffGlowAdd(work->glowObj, glowPosP, glowVelP,
            (int)(60.0f * (0.8f + 0.3f * MBCapsuleEffRandF())),
            100.0f * (0.2f + 0.1f * MBCapsuleEffRandF()),
            3.0f * (-0.5f + MBCapsuleEffRandF()),
            0.8166667f, glowColorP);
        if (i == 64) {
            HuPrcVSleep();
        }
    }
    while (bobleNo < 3) {
        if (++frame > 1 && bobleNo < 3) {
            boble = obj[bobleNo]->data;
            boble->state++;
            frame = 0;
            bobleNo++;
        }
        HuPrcVSleep();
    }
    do {
        for (i = 0; i < 3; i++) {
            if (obj[i] != NULL) {
                break;
            }
        }
        HuPrcVSleep();
    } while (i < 3);
    HuPrcEnd();
}

void mbev_CapBobleKill(void)
{
}

void mbev_CapBobleTrap(void *workP)
{
    CAPWORK *work = workP;
    HuVecF pos;
    HuVecF rot;
    HuVecF movePos;
    int coinNum;
    int frame;
    float time;

    if (!GwPlayer[work->playerNo].metalF) {
        mbPlayerMoveHookSet(work->playerNo, mbev_CapBobleMove);
        coinNum = mbPlayerCoinGet(work->playerNo);
        if (coinNum > 10) {
            coinNum = 10;
        }
        mbCoinAddDispExec(work->playerNo, -coinNum, FALSE, TRUE);
        mbPlayerPosGet(work->playerNo, &pos);
        pos.y += 250.0f;
        if (coinNum != 0) {
            mbCoinDispCapsuleCreate(&pos, -coinNum);
        }
        omVibrate((s16)work->playerNo, 20, 20, 0);
        if (GwPlayer[work->playerNo].moveNum <= 1 && !_CheckFlag(FLAG_BOARD_DEBUG)) {
            mbPlayerMotionShiftSet(work->playerNo, 15, 0.0f, 8.0f, 0);
            mbPlayerColSnapPlayerSet(work->playerNo, FALSE);
            mbMoveNumDispSet(work->playerNo, FALSE);
            mbPlayerRotGet(work->playerNo, &rot);
            for (frame = 1; (float)frame <= 36.0f; frame++) {
                time = (float)frame / 36.0f;
                mbMasuPosGet(GwPlayer[work->playerNo].masuId, &pos);
                movePos.x = pos.x;
                movePos.y = pos.y + 3.0f * (100.0f *
                    sin((M_PI * (180.0f * time)) / 180.0f));
                movePos.z = pos.z;
                mbPlayerPosSetV(work->playerNo, &movePos);
                mbPlayerRotSet(work->playerNo, 0.0f,
                    rot.y + 720.0f * time, 0.0f);
                HuPrcVSleep();
            }
            mbPlayerColSnapPlayerSet(work->playerNo, TRUE);
            mbPlayerMotionShiftSet(work->playerNo, 1, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
        }
    }
}

void mbev_CapBobleMove(int playerNo)
{
    HuVecF posStart;
    HuVecF posEnd;
    HuVecF pos;
    HuVecF rot;
    HuVecF rotStart;
    HuVecF focusPos;
    HuVecF dir;
    int masuId;
    int masuIdNext;
    int frame;
    int focusObj;
    int frameMax;
    float time;
    float dirMagnitude;

    frameMax = 1.75f * mbPlayerWalkSpeedGet();
    masuId = GwPlayer[playerNo].masuId;
    masuIdNext = GwPlayer[playerNo].masuIdNext;
    mbPlayerPosGet(playerNo, &posStart);
    mbMasuPosGet(masuIdNext, &posEnd);
    PSVECSubtract(&posEnd, &posStart, &dir);
    rot.y = 180.0 * (atan2(dir.x, dir.z) / M_PI);
    rot.x = rot.z = 0.0f;
    rotStart = rot;
    dirMagnitude = PSVECMag(&dir);
    focusObj = mbObjCreate(
        DATANUM(DATA_capsule, CAPTRAP_DATA_CAMERA_TARGET_MODEL), NULL, FALSE);
    focusPos = posStart;
    focusPos.y += 100.0f;
    mbObjPosSetV(focusObj, &focusPos);
    mbObjDispSet(focusObj, FALSE);
    mbCameraFocusObjSet(focusObj);
    mbPlayerMotionSet(playerNo, 15, HU3D_MOTATTR_LOOP);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbAudFXPlay(MSM_SE_BRD00_20);
    for (frame = 1; frame < frameMax; frame++) {
        time = (float)frame / (float)frameMax;
        mbMasuPosGet(masuIdNext, &posEnd);
        pos.x = posStart.x + time * (posEnd.x - posStart.x);
        pos.y = posStart.y + time * (posEnd.y - posStart.y)
            + 300.0 * sin((M_PI * (180.0f * time)) / 180.0f);
        pos.z = posStart.z + time * (posEnd.z - posStart.z);
        mbPlayerPosSetV(playerNo, &pos);
        mbPlayerRotSetV(playerNo, &rot);
        mbPlayerWorkGet(playerNo)->_unk08 = frameMax - frame;
        focusPos.x = posStart.x + time * (posEnd.x - posStart.x);
        focusPos.y = posStart.y + time * (posEnd.y - posStart.y) + 100.0f;
        focusPos.z = posStart.z + time * (posEnd.z - posStart.z);
        mbObjPosSetV(focusObj, &focusPos);
        HuPrcVSleep();
    }
    pos = posEnd;
    rot = rot;
    mbPlayerPosSetV(playerNo, &pos);
    mbPlayerRotSetV(playerNo, &rot);
    mbPlayerWorkGet(playerNo)->_unk08 = 0;
    mbCameraFocusPlayerSet(playerNo);
    mbObjKill(focusObj);
}

static void ev_CapBobleOMExec(OMOBJ *obj)
{
    CAPBOBLEWORK *work = obj->data;
    HuVecF playerPos;
    HuVecF dir;
    float time;
    float scale;
    float angle;

    if (mbExitCheck() || work->finishedF) {
        if (!mbExitCheck()) {
            *work->objP = NULL;
        }
        omDelObjEx(mbObjMan, obj);
        return;
    }
    switch (work->state) {
        case 1:
            work->time++;
            time = (float)work->time / 3.0f;
            scale = mbSinDeg(90.0f * time);
            mbObjPosSetV(work->modelId, &work->pos);
            mbObjScaleSet(work->modelId, scale, scale, scale);
            mbObjDispSet(work->modelId, TRUE);
            if (time >= 1.0f) {
                mbObjScaleSet(work->modelId, 1.0f, 1.0f, 1.0f);
                work->state++;
                work->time = 0;
            }
            break;

        case 3:
            work->time++;
            time = (float)work->time / 15.0f;
            mbPlayerPosGet(work->playerNo, &work->endPos);
            work->endPos.x += 0.5 * (100.0 * sin((M_PI * work->angle) / 180.0f));
            work->endPos.z += 0.5 * (100.0 * cos((M_PI * work->angle) / 180.0f));
            work->pos.x = work->startPos.x + time * (work->endPos.x - work->startPos.x);
            work->pos.y = work->startPos.y + time * (work->endPos.y - work->startPos.y)
                + work->arcHeight * mbSinDeg(180.0f * time);
            work->pos.z = work->startPos.z + time * (work->endPos.z - work->startPos.z);
            mbObjPosSetV(work->modelId, &work->pos);
            mbObjScaleSet(work->modelId, 1.0f, 1.0f, 1.0f);
            if (time >= 1.0f) {
                if (!work->metalF) {
                    if (work->bobleNo == 0) {
                        work->state++;
                    } else {
                        mbObjDispSet(work->modelId, FALSE);
                        work->state = 99;
                        work->time = 0;
                        work->finishedF = TRUE;
                    }
                } else {
                    work->state = 10;
                }
                work->time = 0;
            }
            break;

        case 4:
            mbPlayerPosGet(work->playerNo, &work->endPos);
            work->endPos.x += 0.5 * (100.0 * sin((M_PI * work->angle) / 180.0f));
            work->endPos.z += 0.5 * (100.0 * cos((M_PI * work->angle) / 180.0f));
            mbObjPosSetV(work->modelId, &work->endPos);
            work->time++;
            time = (float)work->time / 10.0f;
            scale = 1.0f + mbSinDeg(90.0f * time);
            mbObjScaleSet(work->modelId, scale, scale, scale);
            if (time >= 1.0f) {
                mbObjScaleSet(work->modelId, 1.0f, 1.0f, 1.0f);
                work->state++;
                work->time = 0;
            }
            break;

        case 5:
            mbPlayerPosGet(work->playerNo, &work->endPos);
            work->endPos.x += 0.5 * (100.0 * sin((M_PI * work->angle) / 180.0f));
            work->endPos.z += 0.5 * (100.0 * cos((M_PI * work->angle) / 180.0f));
            mbObjPosSetV(work->modelId, &work->endPos);
            work->time++;
            time = (float)work->time / 30.0f;
            scale = 2.0 * cos((M_PI * (90.0f * time)) / 180.0f);
            mbObjScaleSet(work->modelId, scale, scale, scale);
            if (time >= 1.0f) {
                mbObjDispSet(work->modelId, FALSE);
                work->state++;
                work->time = 0;
                work->finishedF = TRUE;
            }
            break;

        case 10:
            work->time++;
            time = (float)work->time / 30.0f;
            work->pos.x = work->endPos.x + time * (work->startPos.x - work->endPos.x);
            work->pos.y = work->endPos.y + time * (work->startPos.y - work->endPos.y)
                + work->arcHeight * mbSinDeg(180.0f * time);
            work->pos.z = work->endPos.z + time * (work->startPos.z - work->endPos.z);
            mbObjPosSetV(work->modelId, &work->pos);
            scale = cos((M_PI * (90.0f * time)) / 180.0f);
            mbObjScaleSet(work->modelId, scale, scale, scale);
            PSVECSubtract(&work->startPos, &work->endPos, &dir);
            angle = 180.0f * (atan2(dir.x, dir.z) / M_PI);
            mbObjRotSet(work->modelId, 180.0f * mbSinDeg(angle) * time,
                0.0f, 180.0f * mbCosDeg(angle) * time);
            if (time >= 1.0f) {
                mbObjDispSet(work->modelId, FALSE);
                work->state++;
                work->time = 0;
                work->finishedF = TRUE;
            }
            break;
    }
}

void mbev_CapBiriQ(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    HuVecF pos[3];
    HuVecF vel[3];
    HuVecF masuPos;
    HuVecF rot;
    int modelId[3];
    float angle[3];
    int masuNum;
    int masuNumPrev;
    int frame;
    int i;
    float shrinkScale;
    float masuNumF;
    float approachScale;
    float angleCur;
    float time;
    float metalValue;

    for (i = 0; i < 3; i++) {
        MBMODELID model;

        modelId[i] = mbev_CapObjCreate(&work->objWork,
            DATANUM(DATA_capsule, CAPTRAP_DATA_BIRIQ), NULL,
            TRUE, FALSE, FALSE);
        mbObjDispSet(modelId[i], FALSE);
        model = modelId[i];
        mbObjAttrSet(model, HU3D_MOTATTR_LOOP);
        mbObjLayerSet(modelId[i], 3);
    }
    mbMasuPosGet(GwPlayer[work->playerNo].masuIdNext, &masuPos);
    do {
        HuPrcVSleep();
        masuNum = ev_CapMasuNumGet(work->playerNo);
    } while (masuNum < 0 || masuNum > 20);

    angleCur = 360.0f * MBCapsuleEffRandF();
    for (i = 0; i < 3; i++) {
        angle[i] = angleCur + 120.0f * (float)i;
    }
    masuNumF = (float)ev_CapMasuNumGet(work->playerNo);
    if (masuNumF <= 0.0f) {
        masuNumF = 1.0f;
    }
    do {
        time = 1.0f
            - (float)(ev_CapMasuNumGet(work->playerNo) - 1) / masuNumF;
        masuNumPrev = ev_CapMasuNumGet(work->playerNo);
        for (i = 0; i < 3; i++) {
            angleCur = (angle[i] += 6.0f);
            pos[i].x = masuPos.x
                + (100.0f + 2.0f * (100.0f * (1.0f - time)))
                    * sin((M_PI * angleCur) / 180.0f);
            pos[i].y = masuPos.y + 100.0f
                + 2.0f * (100.0f * (1.0f - time));
            pos[i].z = masuPos.z
                + (100.0f + 2.0f * (100.0f * (1.0f - time)))
                    * cos((M_PI * angleCur) / 180.0f);
            approachScale = time;
            mbObjPosSetV(modelId[i], &pos[i]);
            mbObjScaleSet(modelId[i], approachScale, approachScale, approachScale);
            mbObjDispSet(modelId[i], TRUE);
        }
        HuPrcVSleep();
    } while (time < 1.0f
        && ev_CapMasuNumGet(work->playerNo) <= masuNumPrev);

    if (!GwPlayer[work->playerNo].metalF) {
        for (frame = 1; frame <= 30; frame++) {
            time = (float)frame / 30.0f;
            for (i = 0; i < 3; i++) {
                angleCur = (angle[i] += 6.0f);
                mbPlayerPosGet(work->playerNo, &masuPos);
                pos[i].x = masuPos.x
                    + 100.0f * sin((M_PI * angleCur) / 180.0f);
                pos[i].y = masuPos.y + 100.0f;
                pos[i].z = masuPos.z
                    + 100.0f * cos((M_PI * angleCur) / 180.0f);
                mbObjPosSetV(modelId[i], &pos[i]);
            }
            HuPrcVSleep();
        }
        for (frame = 1; frame <= 20; frame++) {
            time = (float)frame / 20.0f;
            shrinkScale = cos((M_PI * (90.0f * time)) / 180.0f);
            for (i = 0; i < 3; i++) {
                angleCur = (angle[i] += 6.0f);
                mbPlayerPosGet(work->playerNo, &masuPos);
                pos[i].x = masuPos.x
                    + 100.0f
                        * (sin((M_PI * angleCur) / 180.0f)
                            * cos((M_PI * (90.0f * time)) / 180.0f));
                pos[i].y = masuPos.y + 100.0f;
                pos[i].z = masuPos.z
                    + 100.0f
                        * (cos((M_PI * angleCur) / 180.0f)
                            * cos((M_PI * (90.0f * time)) / 180.0f));
                mbObjPosSetV(modelId[i], &pos[i]);
                mbObjScaleSet(modelId[i], shrinkScale, shrinkScale, shrinkScale);
            }
            HuPrcVSleep();
        }
    } else {
        for (i = 0; i < 3; i++) {
            angleCur = angle[i];
            rot.x = 20.0f + 15.0f * MBCapsuleEffRandF();
            rot.y = angleCur;
            metalValue = 100.0f * ((0.4f * 0.4f) * MBCapsuleEffRandF());
            if (GwPlayer[work->playerNo].metalF) {
                2.0f * metalValue;
            }
            vel[i].x = metalValue * sin((M_PI * rot.x) / 180.0f)
                * sin((M_PI * rot.y) / 180.0f);
            vel[i].y = metalValue * cos((M_PI * rot.x) / 180.0f);
            vel[i].z = metalValue * sin((M_PI * rot.x) / 180.0f)
                * cos((M_PI * rot.y) / 180.0f);
        }
        for (frame = 0; (float)frame < 60.0f; frame++) {
            metalValue = (float)frame / 60.0f;
            for (i = 0; i < 3; i++) {
                PSVECAdd(&pos[i], &vel[i], &pos[i]);
                vel[i].y -= 0.40833336f;
                vel[i].y *= 0.98f;
                mbObjPosSetV(modelId[i], &pos[i]);
                if (GwPlayer[work->playerNo].metalF) {
                    mbObjRotGet(modelId[i], &rot);
                    rot.z = metalValue * (180.0f * -(0.25f * vel[i].x));
                    mbObjRotSetV(modelId[i], &rot);
                }
                mbObjAlphaSet(modelId[i], 255.0f * (1.0f - metalValue));
            }
            HuPrcVSleep();
        }
    }
    HuPrcEnd();
}

void mbev_CapBiriQKill(void)
{
}

void mbev_CapBiriQTrap(void *workP)
{
    CAPWORK *work = workP;

    if (!GwPlayer[work->playerNo].metalF) {
        mbPlayerBiriQSet(work->playerNo, TRUE);
    }
    if (GwPlayer[work->playerNo].metalF) {
        mbev_CapBiriQMetalShockCreate(work->playerNo);
    } else {
        mbev_CapBiriQShockCreate(work->playerNo);
    }
}

void mbev_CapBiriQMetalShock(void *workP)
{
    CAPWORK *work;
    OMOBJ *ringObj;
    OMOBJ *glowObj;
    OMOBJ *electricObj;
    HuVecF playerPos;
    HuVecF pos;
    HuVecF scale;
    HuVecF vel;
    HuVecF rot;
    HuVecF ringPosArg;
    HuVecF ringRotArg;
    HuVecF ringScaleArg;
    HuVecF electricPosArg0;
    HuVecF electricPosArg1;
    HuVecF electricPosArg2;
    HuVecF glowPosArg;
    HuVecF glowVelArg;
    GXColor color;
    GXColor ringColorArg;
    GXColor glowColorArg;
    int coinNum;
    int motionId;
    int effectId;
    int i;
    BOOL metalF;
    s16 randColor;
    s16 randRotX;
    s16 randRotY;
    s16 randRotZ;
    s16 randScale;
    s16 randTime;
    s16 randRotStepX;
    s16 randRotStepY;
    s16 randRotStepZ;
    s16 randAngleX;
    s16 randAngleY;
    s16 randSpeed;
    s16 randColorR;
    s16 randColorG;
    s16 randColorB;
    s16 randColorA;
    s16 randGlowTime;
    s16 randGlowScale;
    s16 randPosX;
    s16 randPosY;
    s16 randPosZ;
    float sinAngle;
    float cosAngle;
    float randRotXF;
    float randRotYF;
    float randRotZF;
    float randScaleF;
    float randTimeF;
    float randRotStepXF;
    float randRotStepYF;
    float randRotStepZF;
    float randAngleXF;
    float randAngleYF;
    float randSpeedF;
    float speed;
    float randColorRF;
    float randColorGF;
    float randColorBF;
    float randColorAF;
    float randGlowTimeF;
    float randGlowScaleF;
    float randPosXF;
    float posOffsetX;
    float randPosYF;
    float posOffsetY;
    float randPosZF;
    float posOffsetZ;
    HuVecF *electricPosP0;
    HuVecF *electricPosP1;
    HuVecF *electricPosP2;
    float sinYResult;
    float sinY;
    float sinX;
    float cosX;
    float cosYResult;
    float cosY;
    float sinX2;

    work = workP;
    motionId = -1;
    metalF = GwPlayer[work->playerNo].metalF;
    ringObj = mbev_CapEffRingHitCreate();
    HuPrcVSleep();
    glowObj = mbev_CapEffGlowFireCreate();
    HuPrcVSleep();
    electricObj = mbev_CapEffElectricCreate();
    HuPrcVSleep();
    mbPlayerPosGet(work->playerNo, &playerPos);
    HuPrcVSleep();

    CAPTRAP_EFF_RAND_NEXT();
    randRotX = mbCapEffData[mbCapEffNum];
    randRotXF = (1.0f / 32767.0f) * (float)randRotX;
    rot.x = 360.0f * randRotXF;
    CAPTRAP_EFF_RAND_NEXT();
    randRotY = mbCapEffData[mbCapEffNum];
    randRotYF = (1.0f / 32767.0f) * (float)randRotY;
    rot.y = 360.0f * randRotYF;
    CAPTRAP_EFF_RAND_NEXT();
    randRotZ = mbCapEffData[mbCapEffNum];
    randRotZF = (1.0f / 32767.0f) * (float)randRotZ;
    rot.z = 360.0f * randRotZF;
    for (i = 0; i < 3; i++) {
        pos.x = playerPos.x;
        pos.y = 100.0f + playerPos.y;
        pos.z = playerPos.z;
        scale.x = 0.5f;
        scale.y = 2.5f;
        CAPTRAP_EFF_RAND_NEXT();
        randScale = mbCapEffData[mbCapEffNum];
        randScaleF = (1.0f / 32767.0f) * (float)randScale;
        scale.z = 100.0f * (2.0f + (1.5f * randScaleF));
        CAPTRAP_EFF_RAND_NEXT();
        randColor = mbCapEffData[mbCapEffNum];
        mbev_CapEffColorSet(&color, randColor);
        ringPosArg = pos;
        ringRotArg = rot;
        ringScaleArg = scale;
        CAPTRAP_EFF_RAND_NEXT();
        randTime = mbCapEffData[mbCapEffNum];
        randTimeF = (1.0f / 32767.0f) * (float)randTime;
        ringColorArg = color;
        mbev_CapEffRingAdd(ringObj, &ringPosArg, &ringRotArg, &ringScaleArg,
            1, 60.0 * (0.3f + (0.15 * (double)randTimeF)), i,
            &ringColorArg);
        CAPTRAP_EFF_RAND_NEXT();
        randRotStepX = mbCapEffData[mbCapEffNum];
        randRotStepXF = (1.0f / 32767.0f) * (float)randRotStepX;
        rot.x += 45.0f + (45.0f * randRotStepXF);
        CAPTRAP_EFF_RAND_NEXT();
        randRotStepY = mbCapEffData[mbCapEffNum];
        randRotStepYF = (1.0f / 32767.0f) * (float)randRotStepY;
        rot.y += 45.0f + (45.0f * randRotStepYF);
        CAPTRAP_EFF_RAND_NEXT();
        randRotStepZ = mbCapEffData[mbCapEffNum];
        randRotStepZF = (1.0f / 32767.0f) * (float)randRotStepZ;
        rot.z += 45.0f + (45.0f * randRotStepZF);
    }

    PSVECAdd(&playerPos, &biriQEffectOfs, &pos);
    electricPosArg0 = pos;
    electricPosP0 = &electricPosArg0;
    effectId = mbev_CapEffElectricAdd(electricObj, electricPosP0, 12, 1);
    mbev_CapEffElectricModelSet(electricObj,
        (s16)mbPlayerObjIDGet(work->playerNo), effectId, &biriQEffectOfs);
    electricPosArg1 = pos;
    electricPosP1 = &electricPosArg1;
    effectId = mbev_CapEffElectricAdd(electricObj, electricPosP1, 15, 1);
    mbev_CapEffElectricModelSet(electricObj,
        (s16)mbPlayerObjIDGet(work->playerNo), effectId, &biriQEffectOfs);
    electricPosArg2 = pos;
    electricPosP2 = &electricPosArg2;
    effectId = mbev_CapEffElectricAdd(electricObj, electricPosP2, 18, 1);
    mbev_CapEffElectricModelSet(electricObj,
        (s16)mbPlayerObjIDGet(work->playerNo), effectId, &biriQEffectOfs);
    HuPrcVSleep();

    for (i = 0; i < 128; i++) {
        CAPTRAP_EFF_RAND_NEXT();
        randAngleX = mbCapEffData[mbCapEffNum];
        randAngleXF = (1.0f / 32767.0f) * (float)randAngleX;
        rot.x = (180.0f * randAngleXF) - 90.0f;
        CAPTRAP_EFF_RAND_NEXT();
        randAngleY = mbCapEffData[mbCapEffNum];
        randAngleYF = (1.0f / 32767.0f) * (float)randAngleY;
        rot.y = 360.0f * randAngleYF;
        CAPTRAP_EFF_RAND_NEXT();
        randSpeed = mbCapEffData[mbCapEffNum];
        randSpeedF = (1.0f / 32767.0f) * (float)randSpeed;
        speed = 100.0f * (0.21000001f * randSpeedF);

        CAPTRAP_EFF_RAND_NEXT();
        randPosX = mbCapEffData[mbCapEffNum];
        randPosXF = (1.0f / 32767.0f) * (float)randPosX;
        posOffsetX = randPosXF - 0.5f;
        scale.x = playerPos.x + (0.5f * (100.0f * posOffsetX));
        CAPTRAP_EFF_RAND_NEXT();
        randPosY = mbCapEffData[mbCapEffNum];
        randPosYF = (1.0f / 32767.0f) * (float)randPosY;
        posOffsetY = randPosYF - 0.5f;
        scale.y = 100.0f
            + (playerPos.y + (0.5f * (100.0f * posOffsetY)));
        CAPTRAP_EFF_RAND_NEXT();
        randPosZ = mbCapEffData[mbCapEffNum];
        randPosZF = (1.0f / 32767.0f) * (float)randPosZ;
        posOffsetZ = randPosZF - 0.5f;
        scale.z = playerPos.z + (0.5f * (100.0f * posOffsetZ));

        sinAngle = rot.y;
        sinYResult = mbSinDeg(sinAngle);
        sinY = sinYResult;
        sinX = mbSinDeg(rot.x);
        vel.x = (speed * sinX) * sinY;
        cosX = mbCosDeg(rot.x);
        vel.y = speed * cosX;
        cosAngle = rot.y;
        cosYResult = mbCosDeg(cosAngle);
        cosY = cosYResult;
        sinX2 = mbSinDeg(rot.x);
        vel.z = (speed * sinX2) * cosY;

        CAPTRAP_EFF_RAND_NEXT();
        randColorR = mbCapEffData[mbCapEffNum];
        randColorRF = (1.0f / 32767.0f) * (float)randColorR;
        color.r = 64.0f + (63.0f * randColorRF);
        CAPTRAP_EFF_RAND_NEXT();
        randColorG = mbCapEffData[mbCapEffNum];
        randColorGF = (1.0f / 32767.0f) * (float)randColorG;
        color.g = 127.0f + (63.0f * randColorGF);
        CAPTRAP_EFF_RAND_NEXT();
        randColorB = mbCapEffData[mbCapEffNum];
        randColorBF = (1.0f / 32767.0f) * (float)randColorB;
        color.b = 192.0f + (63.0f * randColorBF);
        CAPTRAP_EFF_RAND_NEXT();
        randColorA = mbCapEffData[mbCapEffNum];
        randColorAF = (1.0f / 32767.0f) * (float)randColorA;
        color.a = 192.0f + (63.0f * randColorAF);

        glowPosArg = scale;
        glowVelArg = vel;
        CAPTRAP_EFF_RAND_NEXT();
        randGlowTime = mbCapEffData[mbCapEffNum];
        randGlowTimeF = (1.0f / 32767.0f) * (float)randGlowTime;
        CAPTRAP_EFF_RAND_NEXT();
        randGlowScale = mbCapEffData[mbCapEffNum];
        randGlowScaleF = (1.0f / 32767.0f) * (float)randGlowScale;
        glowColorArg = color;
        mbev_CapEffGlowAdd(glowObj, &glowPosArg, &glowVelArg,
            60.0f * (0.4f + (0.2f * randGlowTimeF)),
            100.0f * (0.2f + (0.1f * randGlowScaleF)),
            3.0f * (-0.5f + MBCapsuleEffRandF()),
            0.0f, &glowColorArg);
        if (i == 64) {
            HuPrcVSleep();
        }
    }

    mbAudFXPlay(1053);
    if (!metalF) {
        mbCameraShakeSet(18, 50.0f);
        HuPrcVSleep();
        omVibrate((s16)work->playerNo, 20, 7, 3);
        coinNum = mbPlayerCoinGet(work->playerNo);
        if (coinNum > 5) {
            coinNum = 5;
        }
        mbPlayerPosGet(work->playerNo, &scale);
        scale.y += 250.0f;
        mbCoinAddDispExec(work->playerNo, -coinNum, FALSE, TRUE);
        mbCoinDispCreate(&scale, -coinNum, -1, TRUE);
        HuPrcVSleep();
        motionId = mbPlayerMotionCreate(work->playerNo,
            CHARMOT_HSF_c000m1_333);
        mbPlayerMotionShiftSet(work->playerNo, motionId, 50.0f, 8.0f, 0);
        while (mbObjMotionShiftIDGet(mbPlayerObjIDGet(work->playerNo)) != -1) {
            HuPrcVSleep();
        }
        while (!mbObjMotionEndCheck(mbPlayerObjIDGet(work->playerNo))) {
            HuPrcVSleep();
        }
        mbev_CapPlayerMotShiftWait(work->playerNo, 1,
            HU3D_MOTATTR_LOOP, TRUE);
        CharMotionUpdateSet(GwPlayer[work->playerNo].charNo,
            CHARMOT_HSF_c000m1_324, TRUE);
    }
    while (mbev_CapEffRingDispGet(ringObj) != 0) {
        HuPrcVSleep();
    }
    while (mbev_CapEffGlowDispGet(glowObj) != 0) {
        HuPrcVSleep();
    }
    if (!(u8)(*(u8 *)&work->flags & 1)) {
        while (mbev_CapBiriQShockDelayGet(work->playerNo) > 0) {
            HuPrcVSleep();
        }
    }
    if (mbPlayerCoinGet(work->playerNo) <= 0
        && GwPlayer[work->playerNo].biriQF) {
        mbPlayerBiriQSet(work->playerNo, FALSE);
    }
    if (motionId != -1) {
        mbPlayerMotionKill(work->playerNo, motionId);
    }
    mbev_CapEffRingKill(ringObj);
    mbev_CapEffGlowKill(glowObj);
    mbev_CapEffElectricKill(electricObj);
}

static int ev_CapMasuNumGet(int playerNo)
{
    int masuNum = mbPlayerWorkGet(playerNo)->_unk08;

    return masuNum + mbev_CapBiriQShockDelayGet(playerNo);
}

void mbev_CapTumujikun(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    int motFile[] = {
        DATANUM(DATA_capsule, CAPTRAP_DATA_TUMUJIKUN_MOTION_A),
        DATANUM(DATA_capsule, CAPTRAP_DATA_TUMUJIKUN_MOTION_B),
        -1,
    };
    HuVecF pos;
    HuVecF posTop;
    HuVecF effectPos;
    int playerNo;
    int capObj;
    int effectObj;
    int soundId;
    int frame;
    int masuNum;
    int masuNumPrev;
    float time;
    float masuNumStart;

    playerNo = work->playerNo;
    capObj = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsule, CAPTRAP_DATA_TUMUJIKUN_MODEL), motFile,
        FALSE, 0, FALSE);
    mbObjDispSet(capObj, FALSE);
    mbObjLayerSet(capObj, 3);
    effectObj = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsule, CAPTRAP_DATA_TUMUJIKUN_EFFECT), NULL,
        FALSE, 5, FALSE);
    mbObjDispSet(effectObj, FALSE);
    mbObjLayerSet(effectObj, 3);
    mbMasuPosGet(work->masuId, &pos);
    posTop = pos;
    posTop.y += 1000.0f;
    do {
        HuPrcVSleep();
        masuNum = ev_CapMasuNumGet(playerNo);
    } while (masuNum < 0 || masuNum > 20);
    mbAudFXPlay(MSM_SE_BRD00_75);
    mbObjMotionTimeSet(effectObj, 0.0f);
    mbObjMotionSpeedSet(effectObj, 0.5f);
    mbObjPosSetV(effectObj, &pos);
    mbObjScaleSet(effectObj, 2.0f, 2.0f, 2.0f);
    mbObjDispSet(effectObj, TRUE);
    HuPrcSleep(15);
    masuNumStart = ev_CapMasuNumGet(playerNo);
    if (masuNumStart <= 0.0f) {
        masuNumStart = 1.0f;
    }
    do {
        masuNum = ev_CapMasuNumGet(playerNo);
        time = 1.0f - (float)(masuNum - 1) / masuNumStart;
        masuNumPrev = ev_CapMasuNumGet(playerNo);
        HuPrcVSleep();
        if (time >= 1.0f) {
            break;
        }
    } while (ev_CapMasuNumGet(playerNo) <= masuNumPrev);
    soundId = mbAudFXPlay(MSM_SE_BRD00_75);
    mbObjMotionSet(capObj, 1, HU3D_MOTATTR_LOOP);
    mbObjPosSetV(capObj, &pos);
    mbObjDispSet(capObj, TRUE);
    for (frame = 0; frame < 30; frame++) {
        time = (float)frame / 30.0f;
        mbObjScaleSet(capObj, 1.5f,
            1.5f * sin((M_PI * (90.0f * time)) / 180.0f), 1.5f);
        HuPrcVSleep();
    }
    if (!GwPlayer[playerNo].metalF) {
        HuPrcSleep(30);
        mbAudFXPlay(MSM_SE_BRD00_77);
        for (frame = 0; frame < 60; frame++) {
            time = (float)frame / 60.0f;
            time = sin((M_PI * (90.0f * time)) / 180.0f);
            effectPos.x = pos.x + time * (posTop.x - pos.x);
            effectPos.y = pos.y + time * (posTop.y - pos.y);
            effectPos.z = pos.z + time * (posTop.z - pos.z);
            mbObjPosSetV(capObj, &effectPos);
            mbObjAlphaSet(capObj, 255.0f * (1.0f - time));
            HuPrcVSleep();
        }
        mbObjDispSet(capObj, FALSE);
        if (soundId != -1) {
            mbAudFXStop(soundId);
        }
    } else {
        work->explodeObj = mbev_CapEffExplodeCreate();
        for (frame = 0; frame < 12; frame++) {
            time = (float)frame / 12.0f;
            mbObjScaleSet(capObj, 1.5f,
                1.5f * cos((M_PI * (90.0f * time)) / 180.0f), 1.5f);
            mbObjAlphaSet(effectObj, 255.0f * (1.0f - time));
            HuPrcVSleep();
        }
        mbMasuPosGet(work->masuId, &effectPos);
        mbev_CapEffDustHeavyAdd(work->explodeObj, &effectPos);
        if (soundId != -1) {
            mbAudFXStop(soundId);
        }
        mbAudFXPlay(MSM_SE_BRD00_78);
        while (mbev_CapEffExplodeAnimGet(work->explodeObj) > 0) {
            HuPrcVSleep();
        }
    }
    HuPrcEnd();
}

void mbev_CapTumujikunKill(void)
{
}

void mbev_CapTumujikunTrap(void *workP)
{
    CAPWORK *work = workP;
    HuVecF playerPos;
    HuVecF playerRot;
    HuVecF masuPos;
    HuVecF targetPos;
    HuVecF initialPos;
    HuVecF focusPos;
    int focusObj;
    int motionId;
    int masuNum;
    int branchAttr;
    int randomStart;
    int candidate;
    int masuId;
    int frame;
    float weight;

    masuId = GwPlayer[work->playerNo].masuId;
    mbPlayerPosGet(work->playerNo, &playerPos);
    mbPlayerRotGet(work->playerNo, &playerRot);
    if (GwPlayer[work->playerNo].metalF) {
        motionId = mbPlayerMotionCreate(work->playerNo,
            CAPTRAP_DATA_TUMUJIKUN_TRAP_MOTION);
        mbMoveNumDispSet(work->playerNo, FALSE);
        mbPlayerMotionShiftSet(work->playerNo, motionId, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        mbPlayerColSnapPlayerSet(work->playerNo, FALSE);
        for (frame = 0; frame < 30.0f; frame++) {
            weight = (float)frame / 30.0f;
            weight = sin((M_PI * (90.0f * weight)) / 180.0f);
            mbMasuPosGet(masuId, &masuPos);
            targetPos = masuPos;
            targetPos.y += 400.0f;
            playerPos = masuPos;
            playerPos.y = masuPos.y
                + (weight * (targetPos.y - masuPos.y));
            mbPlayerPosSetV(work->playerNo, &playerPos);
            HuPrcVSleep();
        }
        for (frame = 0; frame < 9.0f; frame++) {
            mbMasuPosGet(masuId, &masuPos);
            targetPos = masuPos;
            targetPos.y += 400.0f;
            mbPlayerPosSetV(work->playerNo, &targetPos);
            HuPrcVSleep();
        }
        for (frame = 0; frame < 12.0f; frame++) {
            weight = (float)frame / 12.0f;
            weight = sin((M_PI * (90.0f * weight)) / 180.0f);
            mbMasuPosGet(masuId, &masuPos);
            targetPos = masuPos;
            targetPos.y += 400.0f;
            playerPos = targetPos;
            playerPos.y = targetPos.y
                + (weight * (masuPos.y - targetPos.y));
            mbPlayerPosSetV(work->playerNo, &playerPos);
            if (frame == 6) {
                mbPlayerMotionShiftSet(work->playerNo, 5, 0.0f, 8.0f,
                    HU3D_MOTATTR_NONE);
            }
            HuPrcVSleep();
        }
        mbPlayerColSnapPlayerSet(work->playerNo, TRUE);
        while (!mbObjMotionEndCheck(mbPlayerObjIDGet(work->playerNo))
            || mbObjMotionShiftIDGet(
                mbPlayerObjIDGet(work->playerNo)) != -1) {
            HuPrcVSleep();
        }
        mbMoveNumDispSet(work->playerNo, TRUE);
        mbPlayerMotionKill(work->playerNo, motionId);
        return;
    }

    focusObj = mbObjCreate(
        DATANUM(DATA_capsule, CAPTRAP_DATA_CAMERA_TARGET_MODEL), NULL,
        FALSE);
    focusPos = playerPos;
    focusPos.y += 100.0f;
    mbObjPosSetV(focusObj, &focusPos);
    mbObjDispSet(focusObj, FALSE);
    mbCameraFocusObjSet(focusObj);
    motionId = mbPlayerMotionCreate(work->playerNo,
        CHARMOT_HSF_c000m1_344);
    initialPos = playerPos;
    targetPos = initialPos;
    masuPos = targetPos;
    targetPos.y += 200.0f;
    initialPos.y += 1000.0f;
    mbMoveNumDispSet(work->playerNo, FALSE);
    mbPlayerMotionShiftSet(work->playerNo, motionId, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    mbPlayerColSnapPlayerSet(work->playerNo, FALSE);
    omVibrate(work->playerNo, 20, 4, 4);

    for (frame = 0; frame < 30.0f; frame++) {
        weight = (float)frame / 30.0f;
        weight = sin((M_PI * (90.0f * weight)) / 180.0f);
        mbMasuPosGet(masuId, &masuPos);
        targetPos = masuPos;
        targetPos.y += 200.0f;
        playerPos.x = masuPos.x + (weight * (targetPos.x - masuPos.x));
        playerPos.y = masuPos.y + (weight * (targetPos.y - masuPos.y));
        playerPos.z = masuPos.z + (weight * (targetPos.z - masuPos.z));
        if ((playerRot.y += 10.0f * weight) > 360.0f) {
            playerRot.y -= 360.0f;
        }
        mbPlayerPosSetV(work->playerNo, &playerPos);
        mbPlayerRotSetV(work->playerNo, &playerRot);
        HuPrcVSleep();
    }
    omVibrate(work->playerNo, 90, 7, 3);
    for (frame = 0; frame < 30.0f; frame++) {
        weight = (float)frame / 30.0f;
        mbMasuPosGet(masuId, &masuPos);
        targetPos = masuPos;
        targetPos.y += 200.0f;
        playerPos = targetPos;
        playerPos.y += 0.2f * (100.0
            * sin((M_PI * (360.0f * weight)) / 180.0f));
        if ((playerRot.y += 10.0f) > 360.0f) {
            playerRot.y -= 360.0f;
        }
        mbPlayerPosSetV(work->playerNo, &playerPos);
        mbPlayerRotSetV(work->playerNo, &playerRot);
        HuPrcVSleep();
    }
    for (frame = 0; frame < 45.0f; frame++) {
        weight = (float)frame / 45.0f;
        weight = sin((M_PI * (90.0f * weight)) / 180.0f);
        mbMasuPosGet(masuId, &masuPos);
        targetPos = masuPos;
        targetPos.y += 200.0f;
        playerPos.x = targetPos.x
            + (weight * (initialPos.x - targetPos.x));
        playerPos.y = targetPos.y
            + (weight * (initialPos.y - targetPos.y));
        playerPos.z = targetPos.z
            + (weight * (initialPos.z - targetPos.z));
        if ((playerRot.y += 10.0f + (10.0f * weight)) > 360.0f) {
            playerRot.y -= 360.0f;
        }
        mbPlayerPosSetV(work->playerNo, &playerPos);
        mbPlayerRotSetV(work->playerNo, &playerRot);
        HuPrcVSleep();
    }
    mbPlayerDispSet(work->playerNo, FALSE);
    mbWipeDissolveFadeOutTime(1);

    masuNum = mbMasuNumGet();
    randomStart = mbRandMod(masuNum);
    for (frame = 0; frame < masuNum; frame++) {
        candidate = randomStart + frame;
        if (candidate >= masuNum) {
            candidate -= masuNum;
        }
        if (candidate == 0) {
            candidate++;
        }
        if (candidate == masuId) {
            continue;
        }
        branchAttr = mbBranchAttrGet();
        if ((mbMasuMAttrGet(candidate) & branchAttr) != 0
            || mbCapMasuDispTypeGet(candidate) == 2
            || (mbMasuTypeGet(candidate) != 1
                && mbMasuTypeGet(candidate) != 2)) {
            continue;
        }
        break;
    }
    if (frame >= masuNum) {
        for (frame = 0; frame < masuNum; frame++) {
            candidate = randomStart + frame;
            if (candidate >= masuNum) {
                candidate -= masuNum;
            }
            if (candidate == 0) {
                candidate++;
            }
            if (candidate == masuId) {
                continue;
            }
            branchAttr = mbBranchAttrGet();
            if ((mbMasuMAttrGet(candidate) & branchAttr) != 0
                || (mbCapMasuDispTypeGet(candidate) == 2
                    && mbCapMasuPlayerGet(candidate) != work->playerNo)
                || (mbMasuTypeGet(candidate) != 1
                    && mbMasuTypeGet(candidate) != 2)) {
                continue;
            }
            break;
        }
    }
    if (frame >= masuNum) {
        for (frame = 0; frame < masuNum; frame++) {
            candidate = randomStart + frame;
            if (candidate >= masuNum) {
                candidate -= masuNum;
            }
            if (candidate == 0) {
                candidate++;
            }
            if (candidate == masuId) {
                continue;
            }
            branchAttr = mbBranchAttrGet();
            if ((mbMasuMAttrGet(candidate) & branchAttr) != 0
                || (mbMasuTypeGet(candidate) != 1
                    && mbMasuTypeGet(candidate) != 2)) {
                continue;
            }
            break;
        }
    }

    mbMasuPosGet(candidate, &masuPos);
    focusPos = masuPos;
    focusPos.y += 100.0f;
    mbObjPosSetV(focusObj, &focusPos);
    mbCameraMoveOnSet(FALSE);
    mbCameraMoveWait();
    mbCameraMoveOnSet(TRUE);
    mbev_PlayerColMasuSet(work->playerNo, candidate, TRUE);
    mbWipeDissolveFadeIn();
    mbPlayerDispSet(work->playerNo, TRUE);
    for (frame = 0; frame < 60.0f; frame++) {
        weight = (float)frame / 60.0f;
        weight = sin((M_PI * (90.0f * weight)) / 180.0f);
        mbMasuPosGet(candidate, &masuPos);
        targetPos = masuPos;
        targetPos.y += 2000.0f;
        playerPos.x = targetPos.x
            + (weight * (masuPos.x - targetPos.x));
        playerPos.y = targetPos.y
            + (weight * (masuPos.y - targetPos.y));
        playerPos.z = targetPos.z
            + (weight * (masuPos.z - targetPos.z));
        if ((playerRot.y += 30.0f) > 360.0f) {
            playerRot.y -= 360.0f;
        }
        mbPlayerPosSetV(work->playerNo, &playerPos);
        mbPlayerRotSetV(work->playerNo, &playerRot);
        HuPrcVSleep();
    }
    GwPlayer[work->playerNo].masuId = candidate;
    mbPlayerColSnapPlayerSet(work->playerNo, TRUE);
    GwPlayer[work->playerNo].masuIdNext = candidate;
    mbPlayerMotionShiftSet(work->playerNo, 6, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    HuPrcSleep(60);
    mbPlayerMotionShiftSet(work->playerNo, 1, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    if (GwPlayer[work->playerNo].moveNum > 1) {
        mbMoveNumDispSet(work->playerNo, TRUE);
    }
    mbCameraFocusPlayerSet(work->playerNo);
    mbObjKill(focusObj);
    mbPlayerMotionKill(work->playerNo, motionId);
}

void mbev_CapDossun(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    HuVecF masuPos;
    HuVecF targetPos;
    HuVecF pos;
    HuVecF ringPos;
    HuVecF ringRot;
    HuVecF ringScale;
    GXColor colorTemp;
    GXColor color;
    int model;
    int masuNumCur;
    int frame;
    float masuNum;
    float time;
    float weight;
    float radius;
    float angle;
    float rotX;
    float rotZ;

    work->explodeObj = mbev_CapEffExplodeCreate();
    work->ringObj = mbev_CapEffRingHitCreate();
    model = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsule, CAPTRAP_DATA_DOSSUN_MODEL), NULL,
        FALSE, FALSE, FALSE);
    mbObjAttrSet(model, HU3D_ATTR_DIE | HU3D_ATTR_DISPOFF);
    mbObjDispSet(model, FALSE);
    mbObjLayerSet(model, 3);
    mbMasuPosGet(work->masuId, &masuPos);
    targetPos = masuPos;
    targetPos.y += 1000.0f;
    if (GwPlayer[work->playerNo].metalF) {
        masuPos.y += 150.0f;
    }
    do {
        HuPrcVSleep();
        masuNumCur = ev_CapMasuNumGet(work->playerNo);
    } while (masuNumCur < 0 || masuNumCur > 20);
    masuNumCur = ev_CapMasuNumGet(work->playerNo);
    masuNum = masuNumCur;
    if (masuNum <= 0.0f) {
        masuNum = 1.0f;
    }
    do {
        time = 1.0f
            - ((float)(ev_CapMasuNumGet(work->playerNo) - 1) / masuNum);
        weight = sin((M_PI * (90.0f * (1.0f - time))) / 180.0f);
        masuNumCur = ev_CapMasuNumGet(work->playerNo);
        mbMasuPosGet(work->masuId, &masuPos);
        pos.x = masuPos.x + (weight * (targetPos.x - masuPos.x));
        pos.y = masuPos.y + (weight * (targetPos.y - masuPos.y));
        pos.z = masuPos.z + (weight * (targetPos.z - masuPos.z));
        mbObjPosSetV(model, &pos);
        mbObjDispSet(model, TRUE);
        HuPrcVSleep();
    } while (time < 1.0f
        && ev_CapMasuNumGet(work->playerNo) <= masuNumCur);

    if (!GwPlayer[work->playerNo].metalF) {
        mbAudFXPlay(MSM_SE_BRD00_50);
        mbAudFXPlay(MSM_SE_GUIDE_15);
        pos = masuPos;
        mbev_CapEffDustHeavyAdd(work->explodeObj, &pos);
        mbObjPosSetV(model, &masuPos);
        for (frame = 0; frame < 60.0f; frame++) {
            mbMasuPosGet(work->masuId, &masuPos);
            mbObjPosSetV(model, &masuPos);
            HuPrcVSleep();
        }
        for (frame = 0; frame < 120.0f; frame++) {
            weight = sin((M_PI
                * (90.0f * ((float)frame / 120.0f))) / 180.0f);
            mbMasuPosGet(work->masuId, &masuPos);
            pos.x = masuPos.x + (weight * (targetPos.x - masuPos.x));
            pos.y = masuPos.y + (weight * (targetPos.y - masuPos.y));
            pos.z = masuPos.z + (weight * (targetPos.z - masuPos.z));
            mbObjPosSetV(model, &pos);
            mbObjDispSet(model, TRUE);
            HuPrcVSleep();
        }
        mbObjDispSet(model, FALSE);
    } else {
        mbPlayerPosGet(work->playerNo, &ringPos);
        ringPos.y += 150.0f;
        ringScale.x = 0.5f;
        ringScale.y = 3.0f;
        ringScale.z = 100.0f
            * (1.0f + (0.25f * MBCapsuleEffRandF()));
        ringRot.x = 90.0f
            + (20.0f * (-0.5f + MBCapsuleEffRandF()));
        ringRot.y = 0.0f;
        ringRot.z = 20.0f * (-0.5f + MBCapsuleEffRandF());
        colorTemp.r = 255;
        colorTemp.g = 255;
        colorTemp.b = 127;
        colorTemp.a = 255;
        color = colorTemp;
        mbev_CapEffRingAdd(work->ringObj, &ringPos, &ringRot, &ringScale,
            1, 12, 2, &color);
        radius = 100.0f * (3.0f + (2.0f * MBCapsuleEffRandF()));
        angle = 360.0f * MBCapsuleEffRandF();
        rotX = 180.0f
            * sin((M_PI * (90.0f * angle)) / 180.0f);
        rotZ = 180.0f
            * cos((M_PI * (90.0f * angle)) / 180.0f);
        targetPos.x = masuPos.x
            + (radius * sin((M_PI * angle) / 180.0f));
        targetPos.z = masuPos.z
            + (radius * sin((M_PI * angle) / 180.0f));
        targetPos.y = masuPos.y
            + (100.0f * (1.0f + MBCapsuleEffRandF()));
        for (frame = 0; frame < 60.0f; frame++) {
            weight = sin((M_PI
                * (90.0f * ((float)frame / 60.0f))) / 180.0f);
            pos.x = masuPos.x + (weight * (targetPos.x - masuPos.x));
            pos.y = masuPos.y + (weight * (targetPos.y - masuPos.y));
            pos.y += 5.0 * (100.0
                * sin((M_PI * (180.0f * weight)) / 180.0f));
            pos.z = masuPos.z + (weight * (targetPos.z - masuPos.z));
            mbObjPosSetV(model, &pos);
            mbObjRotSet(model, rotX * weight, 0.0f, rotZ * weight);
            mbObjAlphaSet(model, (int)(255.0f - (255.0f * weight)));
            HuPrcVSleep();
        }
    }
    HuPrcEnd();
}

void mbev_CapDossunKill(void)
{
}

void mbev_CapDossunTrap(void *workP)
{
    CAPWORK *work = workP;
    int playerNo[GW_PLAYER_MAX];
    int playerNum;
    int i;

    if (!GwPlayer[work->playerNo].metalF) {
        GwPlayer[work->playerNo].moveNum = 0;
        mbMoveNumDispSet(work->playerNo, FALSE);
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (GwPlayer[i].masuId == GwPlayer[work->playerNo].masuId) {
                omVibrate(i, 20, 7, 3);
            }
        }
        playerNum = mbev_CapPlayerSquishSet(playerNo,
            GwPlayer[work->playerNo].masuId);
        HuPrcSleep(60);
        mbCameraPlayerViewSet(work->playerNo, 0);
        HuPrcSleep(60);
        mbev_CapPlayerStunSet(playerNo, playerNum, FALSE);
        HuPrcSleep(60);
        mbev_CapPlayerIdleWait();
    }
}

void mbev_CapBomhei(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    HuVecF masuPos;
    HuVecF effectPos;
    HuVecF finalPos;
    HuVecF playerPos;
    HuVecF bodyPos;
    HuVecF ringPos;
    HuVecF ringRot;
    HuVecF ringScale;
    HuVecF glowPos;
    HuVecF glowRot;
    int body;
    int attachment;
    int effect;
    char *itemHook;
    int playerNo;
    int frame;
    float time;

    playerNo = work->playerNo;
    bomheiMode[playerNo] = 0;
    bomheiRotY[playerNo] = 180.0f
        * (-0.5f + MBCapsuleEffRandF());
    work->glowObj = mbev_CapEffGlowCreate();
    HuPrcVSleep();
    work->ringObj = mbev_CapEffRingHitCreate();
    HuPrcVSleep();
    body = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsule, CAPTRAP_DATA_BOMHEI_BODY), NULL,
        FALSE, FALSE, FALSE);
    mbObjLayerSet(body, 3);
    mbObjAttrSet(body, HU3D_ATTR_DIE | HU3D_ATTR_DISPOFF);
    mbObjDispSet(body, FALSE);
    HuPrcVSleep();
    attachment = mbev_CapObjCreate(&work->objWork,
        DATANUM(DATA_capsule, CAPTRAP_DATA_BOMHEI_ATTACHMENT), NULL,
        FALSE, FALSE, FALSE);
    mbObjAttrSet(attachment, HU3D_ATTR_DIE | HU3D_ATTR_DISPOFF);
    mbObjLayerSet(attachment, 5);
    mbObjHookSet(body, captrapBomheiItemHook, attachment);
    HuPrcVSleep();
    while (bomheiMode[playerNo] < 1) {
        HuPrcVSleep();
    }

    mbMasuPosGet(work->masuId, &masuPos);
    effectPos = masuPos;
    finalPos = masuPos;
    masuPos.y += 150.0f;
    effectPos.y += 500.0f;
    finalPos.y += 50.0f;
    if (!GwPlayer[playerNo].metalF) {
        mbObjDispSet(body, TRUE);
        mbObjDispSet(attachment, TRUE);
        for (frame = 0; frame < 40; frame++) {
            time = (float)frame / 40.0f;
            mbPlayerPosGet(playerNo, &playerPos);
            bodyPos.x = playerPos.x;
            bodyPos.y = playerPos.y + 100.0f
                + (8.0 * (100.0
                    * cos((M_PI * (90.0f * time)) / 180.0f)));
            bodyPos.z = playerPos.z + 50.0f;
            mbObjPosSet(body, bodyPos.x, bodyPos.y, bodyPos.z);
            HuPrcVSleep();
        }
        mbObjPosSet(body, 0.0f, 0.0f, 0.0f);
        itemHook = CharModelItemHookGet(GwPlayer[playerNo].charNo, 4, 0);
        mbObjHookSet(mbPlayerObjIDGet(playerNo), itemHook, body);
        CharFXPlay(GwPlayer[playerNo].charNo, CHARVOICEID(8));
        mbAudFXPlay(MSM_SE_GUIDE_05);
        bomheiMode[playerNo] = 2;
        while (bomheiMode[playerNo] < 3) {
            HuPrcVSleep();
        }
        mbObjHookReset(mbPlayerObjIDGet(playerNo));
        mbObjHookReset(body);
        mbObjDispSet(body, FALSE);
        mbObjDispSet(attachment, FALSE);
        effect = mbev_CapObjCreate(&work->objWork,
            DATANUM(DATA_capsule, CAPTRAP_DATA_BOMHEI_EFFECT), NULL,
            FALSE, FALSE, FALSE);
        mbObjLayerSet(effect, 3);
        mbObjScaleSet(effect, 1.2f, 1.2f, 1.2f);
        mbObjMotionTimeSet(effect, 5.0f);
        mbObjMotionSpeedSet(effect, 1.0f);
        mbObjPosSetV(effect, &finalPos);
        mbAudFXPlay(MSM_SE_BRD00_68);
        HuPrcSleep(60);
    } else {
        HuPrcSleep(26);
        mbObjDispSet(body, TRUE);
        mbObjDispSet(attachment, TRUE);
        for (frame = 1; frame <= 40; frame++) {
            time = (float)frame / 40.0f;
            mbPlayerPosGet(playerNo, &playerPos);
            bodyPos.x = playerPos.x;
            bodyPos.y = playerPos.y + 230.0f
                + (8.0 * (100.0
                    * cos((M_PI * (90.0f * time)) / 180.0f)));
            bodyPos.z = playerPos.z;
            mbObjPosSet(body, bodyPos.x, bodyPos.y, bodyPos.z);
            HuPrcVSleep();
        }
        mbObjPosGet(body, &bodyPos);
        ringPos = bodyPos;
        ringPos.y += 10.0f;
        ringRot.x = 90.0f;
        ringRot.y = 0.0f;
        ringRot.z = 0.0f;
        ringScale.x = 0.5f;
        ringScale.y = 3.0f;
        ringScale.z = 100.0f
            * (1.0f + (0.25f * MBCapsuleEffRandF()));
        mbev_CapEffRingHitAdd(work->ringObj, &ringPos, &ringRot,
            &ringScale);
        glowPos = bodyPos;
        glowPos.y += 20.0f;
        glowRot.x = 0.0f;
        glowRot.y = 0.0f;
        glowRot.z = 0.0f;
        mbev_CapEffGlowCoinAdd(work->glowObj, &glowPos, &glowRot);
        mbev_CapEffGlowCoinAdd(work->glowObj, &glowPos, &glowRot);
        for (frame = 1; frame <= 22; frame++) {
            time = (float)frame / 22.0f;
            effectPos.x = bodyPos.x + (time * (100.0
                * sin((M_PI * bomheiRotY[playerNo]) / 180.0f)));
            effectPos.y = bodyPos.y
                + (1.5 * (100.0
                    * sin((M_PI * (90.0f * time)) / 180.0f)))
                + (100.0
                    * sin((M_PI * (180.0f * time)) / 180.0f));
            effectPos.z = bodyPos.z + (time * (100.0
                * cos((M_PI * bomheiRotY[playerNo]) / 180.0f)));
            mbObjPosSetV(body, &effectPos);
            mbObjRotSet(body, 0.0f, 360.0f * time, 0.0f);
            HuPrcVSleep();
        }
        mbObjPosGet(body, &bodyPos);
        ringPos.x = bodyPos.x + (0.5 * (100.0
            * sin((M_PI * (180.0f + bomheiRotY[playerNo])) / 180.0f)));
        ringPos.y = bodyPos.y + 100.0f;
        ringPos.z = bodyPos.z + (0.5 * (100.0
            * cos((M_PI * (180.0f + bomheiRotY[playerNo])) / 180.0f)));
        ringRot.x = 0.0f;
        ringRot.y = bomheiRotY[playerNo];
        ringRot.z = 0.0f;
        ringScale.x = 0.5f;
        ringScale.y = 3.0f;
        ringScale.z = 100.0f
            * (1.0f + (0.25f * MBCapsuleEffRandF()));
        mbev_CapEffRingHitAdd(work->ringObj, &ringPos, &ringRot,
            &ringScale);
        glowPos.x = bodyPos.x + (0.5 * (100.0
            * sin((M_PI * (180.0f + bomheiRotY[playerNo])) / 180.0f)));
        glowPos.y = bodyPos.y + 100.0f;
        glowPos.z = bodyPos.z + (0.5 * (100.0
            * cos((M_PI * (180.0f + bomheiRotY[playerNo])) / 180.0f)));
        glowRot.x = 90.0f;
        glowRot.y = 0.0f;
        glowRot.z = bomheiRotY[playerNo];
        mbev_CapEffGlowCoinAdd(work->glowObj, &glowPos, &glowRot);
        mbev_CapEffGlowCoinAdd(work->glowObj, &glowPos, &glowRot);
        for (frame = 1; frame <= 20; frame++) {
            time = (float)frame / 20.0f;
            effectPos.x = bodyPos.x + (time * (3.0 * (100.0
                * sin((M_PI * bomheiRotY[playerNo]) / 180.0f))));
            effectPos.y = bodyPos.y
                + (100.0
                    * sin((M_PI * (90.0f * time)) / 180.0f))
                + (1.5 * (100.0
                    * sin((M_PI * (180.0f * time)) / 180.0f)));
            effectPos.z = bodyPos.z + (time * (3.0 * (100.0
                * cos((M_PI * bomheiRotY[playerNo]) / 180.0f))));
            mbObjPosSetV(body, &effectPos);
            mbObjRotSet(body, 0.0f, 720.0f * time, 0.0f);
            HuPrcVSleep();
        }
        mbObjPosGet(body, &bodyPos);
        mbObjDispSet(body, FALSE);
        mbObjDispSet(attachment, FALSE);
        effect = mbev_CapObjCreate(&work->objWork,
            DATANUM(DATA_capsule, CAPTRAP_DATA_BOMHEI_EFFECT), NULL,
            FALSE, FALSE, FALSE);
        mbObjLayerSet(effect, 3);
        mbObjScaleSet(effect, 1.2f, 1.2f, 1.2f);
        mbObjMotionTimeSet(effect, 5.0f);
        mbObjMotionSpeedSet(effect, 1.0f);
        mbObjPosSetV(effect, &bodyPos);
        mbAudFXPlay(MSM_SE_BRD00_68);
        HuPrcSleep(60);
    }
    HuPrcEnd();
}

void mbev_CapBomheiKill(void)
{
}

void mbev_CapBomheiTrap(void *workP)
{
    CAPWORK *work = workP;
    HuVecF startPos;
    HuVecF endPos;
    HuVecF playerRot;
    HuVecF playerPos;
    int motionId[16];
    int playerNo;
    int masuId;
    int frame;
    int moveNum;
    int moveNumOdd;
    float time;

    playerNo = work->playerNo;
    masuId = GwPlayer[playerNo].masuId;
    mbev_CapPlayerPosSet(&work->objWork, playerNo, masuId, NULL);
    mbev_CapPlayerRotate(playerNo, 0.0f);
    if (!GwPlayer[playerNo].metalF) {
        motionId[0] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
            CAPTRAP_DATA_BOMHEI_NONMETAL_MOTION_A);
        motionId[1] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
            CAPTRAP_DATA_BOMHEI_NONMETAL_MOTION_B);
        mbPlayerMotionShiftSet(playerNo, motionId[0], 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
        bomheiMode[playerNo] = 1;
        while (bomheiMode[playerNo] < 2) {
            HuPrcVSleep();
        }
        while (!mbPlayerMotionEndCheck(playerNo)) {
            HuPrcVSleep();
        }
        bomheiMode[playerNo] = 3;
        omVibrate(playerNo, 20, 20, 0);
        mbMoveNumDispSet(playerNo, FALSE);
    } else {
        motionId[0] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
            CHARMOT_HSF_c000m1_325);
        HuPrcVSleep();
        motionId[1] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
            CHARMOT_HSF_c000m1_311);
        HuPrcVSleep();
        motionId[2] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
            CAPTRAP_DATA_BOMHEI_TRAP_MOTION);
        HuPrcVSleep();
        motionId[3] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
            CHARMOT_HSF_c000m1_367);
        HuPrcVSleep();
        motionId[4] = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
            CHARMOT_HSF_c000m1_308);
        HuPrcVSleep();
        mbMoveNumDispSet(playerNo, FALSE);
        mbPlayerMotionShiftSet(playerNo, motionId[0], 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
        bomheiMode[playerNo] = 1;
        HuPrcSleep(40);
        mbPlayerMotionShiftSet(playerNo, motionId[1], 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
        HuPrcSleep(30);
        mbev_CapPlayerPosSet(&work->objWork, playerNo, -1, NULL);
        mbPlayerColSnapPlayerSet(playerNo, FALSE);
        mbPlayerMotionShiftSet(playerNo, 4, 0.0f, 4.0f,
            HU3D_MOTATTR_NONE);
        mbMasuPosGet(masuId, &startPos);
        endPos.x = startPos.x;
        endPos.y = startPos.y + 500.0f;
        endPos.z = startPos.z;
        for (frame = 1; frame <= 26; frame++) {
            time = (float)frame / 26.0f;
            mbev_CapVecChase(time, &startPos, &endPos, &playerPos);
            playerPos.y += 100.0
                * sin((M_PI * (180.0f * time)) / 180.0f);
            mbPlayerPosSetV(playerNo, &playerPos);
            mbPlayerRotSet(playerNo, 0.0f,
                bomheiRotY[playerNo]
                    * sin((M_PI * (90.0f * time)) / 180.0f),
                0.0f);
            if (frame == 10) {
                mbPlayerMotionShiftSet(playerNo, motionId[3], 0.0f, 4.0f,
                    HU3D_MOTATTR_NONE);
            }
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(playerNo, motionId[4], 999.0f, 8.0f,
            HU3D_MOTATTR_REV);
        for (frame = 1; frame <= 30.0f; frame++) {
            time = (float)frame / 30.0f;
            mbMasuPosGet(masuId, &startPos);
            mbev_CapVecChase(time, &endPos, &startPos, &playerPos);
            playerPos.y += 100.0
                * sin((M_PI * (180.0f * time)) / 180.0f);
            mbPlayerPosSetV(playerNo, &playerPos);
            mbPlayerRotSet(playerNo, 0.0f,
                bomheiRotY[playerNo]
                    * cos((M_PI * (90.0f * time)) / 180.0f),
                0.0f);
            if (frame == 21) {
                mbPlayerMotionShiftSet(playerNo, 5, 0.0f, 8.0f,
                    HU3D_MOTATTR_NONE);
            }
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        mbPlayerColSnapPlayerSet(playerNo, TRUE);
        if (GwPlayer[playerNo].moveNum > 1
            || _CheckFlag(FLAG_BOARD_DEBUG)) {
            mbMoveNumDispSet(playerNo, TRUE);
        }
    }

    mbev_CapPlayerPosSet(&work->objWork, playerNo, -1, NULL);
    if (!GwPlayer[playerNo].metalF) {
        mbPlayerMoveHookSet(playerNo, mbev_CapBomheiMove);
        moveNumOdd = GwPlayer[playerNo].moveNum & 1;
        moveNum = moveNumOdd + (GwPlayer[playerNo].moveNum / 2);
        if (!_CheckFlag(FLAG_BOARD_DEBUG)) {
            if (moveNum < 0) {
                moveNum = 0;
            }
            GwPlayer[playerNo].moveNum = moveNum;
        } else {
            if (moveNum < 1) {
                moveNum = 1;
            }
            GwPlayer[playerNo].moveNum = moveNum;
        }
        if (GwPlayer[playerNo].moveNum <= 1
            && !_CheckFlag(FLAG_BOARD_DEBUG)) {
            mbPlayerMotionShiftSet(playerNo, 15, 0.0f, 8.0f,
                HU3D_MOTATTR_NONE);
            mbPlayerColSnapPlayerSet(playerNo, FALSE);
            mbMoveNumDispSet(playerNo, FALSE);
            mbPlayerRotGet(playerNo, &playerRot);
            for (frame = 1; frame <= 36.0f; frame++) {
                time = (float)frame / 36.0f;
                mbMasuPosGet(GwPlayer[playerNo].masuId, &startPos);
                playerPos.x = startPos.x;
                playerPos.y = startPos.y + 3.0 * (100.0
                    * sin((M_PI * (180.0f * time)) / 180.0f));
                playerPos.z = startPos.z;
                mbPlayerPosSetV(playerNo, &playerPos);
                mbPlayerRotSet(playerNo, 0.0f,
                    playerRot.y + (720.0f * time), 0.0f);
                HuPrcVSleep();
            }
            mbPlayerColSnapPlayerSet(playerNo, TRUE);
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
                HU3D_MOTATTR_LOOP);
        }
    }
}

void mbev_CapBomheiMove(int playerNo)
{
    HuVecF posStart;
    HuVecF posEnd;
    HuVecF pos;
    HuVecF rot;
    HuVecF dir;
    HuVecF focusPos;
    MBMODELID focusObj;
    int frame;
    int frameMax;
    float time;
    float spinDir;

    frameMax = 1.75f * mbPlayerWalkSpeedGet();
    mbPlayerPosGet(playerNo, &posStart);
    mbMasuPosGet(GwPlayer[playerNo].masuId, &posEnd);
    PSVECSubtract(&posEnd, &posStart, &dir);
    rot.x = 0.0f;
    rot.y = 180.0 * (atan2(dir.x, dir.z) / M_PI);
    rot.z = 0.0f;
    PSVECMag(&dir);
    spinDir = (mbRandMod(CAPTRAP_RANDOM_MODULUS) & 1) ? 1.0f : -1.0f;
    focusObj = mbObjCreate(
        DATANUM(DATA_capsule, CAPTRAP_DATA_CAMERA_TARGET_MODEL), NULL,
        FALSE);
    focusPos = posStart;
    focusPos.y += 100.0f;
    mbObjPosSetV(focusObj, &focusPos);
    mbObjDispSet(focusObj, FALSE);
    mbCameraFocusObjSet(focusObj);
    mbPlayerMotionSet(playerNo, 15, HU3D_MOTATTR_LOOP);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    for (frame = 1; frame < frameMax; frame++) {
        time = (float)frame / (float)frameMax;
        mbMasuPosGet(GwPlayer[playerNo].masuId, &posEnd);
        pos.x = posStart.x + time * (posEnd.x - posStart.x);
        pos.y = posStart.y + time * (posEnd.y - posStart.y)
            + 300.0f * sin((M_PI * (180.0f * time)) / 180.0f);
        pos.z = posStart.z + time * (posEnd.z - posStart.z);
        rot.y += 720.0f * time * spinDir;
        mbPlayerPosSetV(playerNo, &pos);
        mbPlayerRotSetV(playerNo, &rot);
        mbPlayerWorkGet(playerNo)->_unk08 = frameMax - frame;
        focusPos.x = posStart.x + time * (posEnd.x - posStart.x);
        focusPos.y = posStart.y + 100.0f + time * (posEnd.y - posStart.y);
        focusPos.z = posStart.z + time * (posEnd.z - posStart.z);
        mbObjPosSetV(focusObj, &focusPos);
        HuPrcVSleep();
    }
    mbPlayerPosSetV(playerNo, &posEnd);
    mbPlayerRotSetV(playerNo, &rot);
    mbPlayerWorkGet(playerNo)->_unk08 = 0;
    if (GwPlayer[playerNo].moveNum > 1 || _CheckFlag(FLAG_BOARD_DEBUG)) {
        mbMoveNumDispSet(playerNo, TRUE);
    }
    if (!mbMasuDispCheck(GwPlayer[playerNo].masuId)) {
        mbMoveNumDispSet(playerNo, TRUE);
    }
    mbCameraFocusPlayerSet(playerNo);
    mbObjKill(focusObj);
}
