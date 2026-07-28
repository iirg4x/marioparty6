#include "dolphin.h"
#include "game/gamework.h"
#include "game/flag.h"
#include "game/charman.h"
#include "game/object.h"
#include "game/process.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/coin.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/player.h"

#include "math.h"

#define CAP_WORK_MAX 64

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
    u8 _unkB70[0x5C];
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

extern s16 mbCoinDispCapsuleCreate(HuVecF *pos, int coinNum);
extern int mbev_CapObjCreate(EVCAPWORK *work, int dataNum, int *motFile,
    BOOL linkF, int delay, BOOL closeDir);
extern int mbev_CapBiriQShockDelayGet(int playerNo);
extern void mbev_CapBiriQShockCreate(int playerNo);
extern void mbev_CapBiriQMetalShockCreate(int playerNo);
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
extern u32 mbCapEffNum;
extern s16 *mbCapEffData;

static HuVecF biriQEffectOfs = { 0.0f, 100.0f, 0.0f };

void mbev_CapBobleMove(int playerNo);

void mbev_CapBobleKill(void)
{
}

void mbev_CapBobleTrap(CAPWORK *work)
{
    HuVecF pos;
    HuVecF movePos;
    HuVecF rot;
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
        if (GwPlayer[work->playerNo].moveNum < 2 && !_CheckFlag(0x20001)) {
            mbPlayerMotionShiftSet(work->playerNo, 15, 0.0f, 8.0f, 0);
            mbPlayerColSnapPlayerSet(work->playerNo, FALSE);
            mbMoveNumDispSet(work->playerNo, FALSE);
            mbPlayerRotGet(work->playerNo, &rot);
            for (frame = 1; (float)frame <= 36.0f; frame++) {
                time = (float)frame / 36.0f;
                mbMasuPosGet(GwPlayer[work->playerNo].masuId, &pos);
                movePos.x = pos.x;
                movePos.y = pos.y + 3.0f * 100.0f *
                    sin((M_PI * (180.0f * time)) / 180.0f);
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
    HuVecF dir;
    HuVecF focusPos;
    MBMODELID focusObj;
    int frame;
    int frameMax;
    float time;

    frameMax = 1.75f * mbPlayerWalkSpeedGet();
    mbPlayerPosGet(playerNo, &posStart);
    mbMasuPosGet(GwPlayer[playerNo].masuId, &posEnd);
    PSVECSubtract(&posEnd, &posStart, &dir);
    rot.x = 0.0f;
    rot.y = 180.0 * (atan2(dir.x, dir.z) / M_PI);
    rot.z = 0.0f;
    PSVECMag(&dir);
    focusObj = mbObjCreate(0xC0044, NULL, FALSE);
    focusPos = posStart;
    focusPos.y += 100.0f;
    mbObjPosSetV(focusObj, &focusPos);
    mbObjDispSet(focusObj, FALSE);
    mbCameraFocusObjSet(focusObj);
    mbPlayerMotionSet(playerNo, 15, HU3D_MOTATTR_LOOP);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbAudFXPlay(0x400);
    for (frame = 1; frame < frameMax; frame++) {
        time = (float)frame / (float)frameMax;
        mbMasuPosGet(GwPlayer[playerNo].masuId, &posEnd);
        pos.x = posStart.x + time * (posEnd.x - posStart.x);
        pos.y = posStart.y + time * (posEnd.y - posStart.y)
            + 300.0f * sin((M_PI * (180.0f * time)) / 180.0f);
        pos.z = posStart.z + time * (posEnd.z - posStart.z);
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
    mbCameraFocusPlayerSet(playerNo);
    mbObjKill(focusObj);
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

void mbev_CapBiriQMetalShock(CAPWORK *work)
{
    OMOBJ *ringObj;
    OMOBJ *glowObj;
    OMOBJ *electricObj;
    HuVecF playerPos;
    HuVecF ringPos;
    HuVecF ringRot;
    HuVecF ringScale;
    HuVecF electricPos;
    HuVecF electricPos0;
    HuVecF electricPos1;
    HuVecF electricPos2;
    HuVecF glowPos;
    HuVecF glowVel;
    GXColor color;
    int motionId;
    int effectId;
    int coinNum;
    int frame;
    int i;
    int time;
    float randF;
    float angleX;
    float angleY;
    float speed;
    float glowRand;
    float glowScaleRand;

    motionId = -1;
    ringObj = mbev_CapEffRingHitCreate();
    HuPrcVSleep();
    glowObj = mbev_CapEffGlowFireCreate();
    HuPrcVSleep();
    electricObj = mbev_CapEffElectricCreate();
    HuPrcVSleep();
    mbPlayerPosGet(work->playerNo, &playerPos);
    HuPrcVSleep();

    CAPTRAP_EFF_RAND_NEXT();
    ringRot.x = 360.0f * ((float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f));
    CAPTRAP_EFF_RAND_NEXT();
    ringRot.y = 360.0f * ((float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f));
    CAPTRAP_EFF_RAND_NEXT();
    ringRot.z = 360.0f * ((float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f));
    for (i = 0; i < 3; i++) {
        ringPos = playerPos;
        ringPos.y += 100.0f;
        ringScale.x = 0.5f;
        ringScale.y = 2.5f;
        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        ringScale.z = 100.0f * (2.0f + (1.5f * randF));
        CAPTRAP_EFF_RAND_NEXT();
        mbev_CapEffColorSet(&color, mbCapEffData[mbCapEffNum]);
        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        time = 60.0f * (0.3f + (0.15f * randF));
        mbev_CapEffRingAdd(ringObj, &ringPos, &ringRot, &ringScale,
            1, time, i, &color);
        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        ringRot.x += 45.0f + (45.0f * randF);
        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        ringRot.y += 45.0f + (45.0f * randF);
        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        ringRot.z += 45.0f + (45.0f * randF);
    }

    PSVECAdd(&playerPos, &biriQEffectOfs, &electricPos);
    electricPos0 = electricPos;
    effectId = mbev_CapEffElectricAdd(electricObj, &electricPos0, 12, 1);
    mbev_CapEffElectricModelSet(electricObj,
        mbPlayerObjIDGet(work->playerNo), effectId, &biriQEffectOfs);
    electricPos1 = electricPos;
    effectId = mbev_CapEffElectricAdd(electricObj, &electricPos1, 15, 1);
    mbev_CapEffElectricModelSet(electricObj,
        mbPlayerObjIDGet(work->playerNo), effectId, &biriQEffectOfs);
    electricPos2 = electricPos;
    effectId = mbev_CapEffElectricAdd(electricObj, &electricPos2, 18, 1);
    mbev_CapEffElectricModelSet(electricObj,
        mbPlayerObjIDGet(work->playerNo), effectId, &biriQEffectOfs);
    HuPrcVSleep();

    for (frame = 0; frame < 128; frame++) {
        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        angleX = (180.0f * randF) - 90.0f;
        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        angleY = 360.0f * randF;
        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        speed = 100.0f * 0.21f * randF;

        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        glowPos.x = playerPos.x + (0.5f * 100.0f * (randF - 0.5f));
        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        glowPos.y = playerPos.y + 100.0f
            + (0.5f * 100.0f * (randF - 0.5f));
        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        glowPos.z = playerPos.z + (0.5f * 100.0f * (randF - 0.5f));

        glowVel.x = speed * mbSinDeg(angleY) * mbSinDeg(angleX);
        glowVel.y = speed * mbCosDeg(angleX);
        glowVel.z = speed * mbSinDeg(angleY) * mbCosDeg(angleX);

        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        color.r = 64.0f + (63.0f * randF);
        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        color.g = 127.0f + (63.0f * randF);
        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        color.b = 192.0f + (63.0f * randF);
        CAPTRAP_EFF_RAND_NEXT();
        randF = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        color.a = 192.0f + (63.0f * randF);

        CAPTRAP_EFF_RAND_NEXT();
        glowRand = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        CAPTRAP_EFF_RAND_NEXT();
        glowScaleRand = (float)mbCapEffData[mbCapEffNum] * (1.0f / 32767.0f);
        time = 60.0f * (0.4f + (0.2f * glowRand));
        mbev_CapEffGlowAdd(glowObj, &glowPos, &glowVel, time,
            100.0f * (0.2f + (0.1f * glowScaleRand)),
            3.0f * (-0.5f
                + (3.725290298461914e-09f * mbRandMod(0x10000000))),
            0.0f, &color);
        if (frame == 64) {
            HuPrcVSleep();
        }
    }

    mbAudFXPlay(0x41D);
    if (!GwPlayer[work->playerNo].metalF) {
        mbCameraShakeSet(50.0f, 18);
        HuPrcVSleep();
        omVibrate((s16)work->playerNo, 20, 7, 3);
        coinNum = mbPlayerCoinGet(work->playerNo);
        if (coinNum > 5) {
            coinNum = 5;
        }
        mbPlayerPosGet(work->playerNo, &glowPos);
        glowPos.y += 250.0f;
        mbCoinAddDispExec(work->playerNo, -coinNum, FALSE, TRUE);
        mbCoinDispCreate(&glowPos, -coinNum, -1, TRUE);
        HuPrcVSleep();
        motionId = mbPlayerMotionCreate(work->playerNo, 0x93001E);
        mbPlayerMotionShiftSet(work->playerNo, motionId, 50.0f, 8.0f, 0);
        while (mbObjMotionShiftIDGet(mbPlayerObjIDGet(work->playerNo)) != -1) {
            HuPrcVSleep();
        }
        while (!mbObjMotionEndCheck(mbPlayerObjIDGet(work->playerNo))) {
            HuPrcVSleep();
        }
        mbev_CapPlayerMotShiftWait(work->playerNo, 1,
            HU3D_MOTATTR_LOOP, TRUE);
        CharMotionUpdateSet(GwPlayer[work->playerNo].charNo, 0x930018, TRUE);
    }
    while (mbev_CapEffRingDispGet(ringObj) != 0) {
        HuPrcVSleep();
    }
    while (mbev_CapEffGlowDispGet(glowObj) != 0) {
        HuPrcVSleep();
    }
    if (!work->flags._flag00) {
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
    int motFile[] = { 0xC0040, 0xC0041, -1 };
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
    capObj = mbev_CapObjCreate(&work->objWork, 0xC003F, motFile,
        FALSE, 0, FALSE);
    mbObjDispSet(capObj, FALSE);
    mbObjLayerSet(capObj, 3);
    effectObj = mbev_CapObjCreate(&work->objWork, 0xC0042, NULL,
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
    mbAudFXPlay(0x437);
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
    soundId = mbAudFXPlay(0x437);
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
        mbAudFXPlay(0x439);
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
        mbAudFXPlay(0x43A);
        while (mbev_CapEffExplodeAnimGet(work->explodeObj) > 0) {
            HuPrcVSleep();
        }
    }
    HuPrcEnd();
}

void mbev_CapTumujikunKill(void)
{
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

void mbev_CapBomheiKill(void)
{
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
    spinDir = (mbRandMod(0x8000) & 1) ? 1.0f : -1.0f;
    focusObj = mbObjCreate(0xC0044, NULL, FALSE);
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
    if (GwPlayer[playerNo].moveNum > 1 || _CheckFlag(0x20001)) {
        mbMoveNumDispSet(playerNo, TRUE);
    }
    if (!mbMasuDispCheck(GwPlayer[playerNo].masuId)) {
        mbMoveNumDispSet(playerNo, TRUE);
    }
    mbCameraFocusPlayerSet(playerNo);
    mbObjKill(focusObj);
}
