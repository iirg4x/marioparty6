#include "dolphin/math.h"
#include "dolphin/mtx.h"
#include "datanum/charmot.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/board/camera.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/process.h"
#include "game/board/audio.h"
#include "game/board/capsule.h"
#include "game/board/main.h"
#include "game/board/player.h"
#include "game/board/roulette.h"
#include "game/board/window.h"
#include "datadir_enum.h"
#include "msm_se.h"

double sin(double);
double cos(double);

typedef struct {
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

typedef struct {
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

typedef struct {
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

#define CAPMOVE_DATA_KINOKO DATANUM(DATA_capsule, 68)
#define CAPMOVE_DATA_S_KINOKO DATANUM(DATA_capsule, 69)
#define CAPMOVE_DATA_N_KINOKO DATANUM(DATA_capsule, 70)
#define CAPMOVE_DATA_P_KINOKO DATANUM(DATA_capsule, 71)
#define CAPMOVE_DATA_M_KINOKO DATANUM(DATA_capsule, 72)
#define CAPMOVE_DATA_KILLER DATANUM(DATA_capsulechar0, 0)
#define CAPMOVE_DATA_KILLER_RIDE_START DATANUM(DATA_mariomot, 67)
#define CAPMOVE_DATA_KILLER_RIDE DATANUM(DATA_mariomot, 66)

enum {
    CAPMOVE_DATA_HANACHAN = DATANUM(DATA_capsulechar0, 1),
    CAPMOVE_DATA_HANACHAN_MOTION_A = DATANUM(DATA_capsulechar0, 2),
    CAPMOVE_DATA_HANACHAN_MOTION_B = DATANUM(DATA_capsulechar0, 3),
    CAPMOVE_DATA_DOKAN = DATANUM(DATA_capsulechar0, 4),
    CAPMOVE_DATA_CAMERA_TARGET = DATANUM(DATA_capsule, 68),
    CAPMOVE_MASU_TYPE_NONE = 0,
    CAPMOVE_MASU_TYPE_STAR = 7,
    CAPMOVE_PLAYER_MOT_JUMP = 4,
    CAPMOVE_EFFECT_COLOR_RANGE = 1 << 15,
    CAPMOVE_MESS_DOKAN_SAME_SPACE = 3473408,
    CAPMOVE_MESS_DOKAN_TARGET = 3473409,
    CAPMOVE_MESS_HANACHAN_ARRIVE = 3473411,
    CAPMOVE_MESS_HANACHAN_DEPART = 3473412,
    CAPMOVE_MESS_HANACHAN_STAR = 3473413,
    CAPMOVE_MESS_STAR_MAX_DAY = 2555914,
    CAPMOVE_MESS_STAR_MAX_NIGHT = 2555915,
};

static HuVecF capsuleCameraOfs = { 0.0f, 100.0f, 0.0f };
static HuVecF hanachanPlayerOfs[GW_CHARA_MAX] = {
    { 0.0f, -130.0f, 30.000002f },
    { 0.0f, -135.0f, 30.000002f },
    { -20.0f, -120.00001f, 80.0f },
    { 0.0f, -130.0f, 30.000002f },
    { 0.0f, -120.00001f, 40.0f },
    { -20.0f, -130.0f, 80.0f },
    { 0.0f, -140.0f, 30.000002f },
    { 0.0f, -100.0f, 50.0f },
    { 0.0f, -104.99999f, 70.0f },
    { -10.0f, -130.0f, 30.000002f },
    { 0.0f, -100.0f, 50.0f },
    { -10.0f, -130.0f, 30.000002f },
    { -10.0f, -130.0f, 30.000002f },
    { -10.0f, -130.0f, 30.000002f },
};

void mbev_CapWait(CAPWORK *work);
extern OMOBJMAN *mbObjMan;
BOOL mbCapEffUseCreate(int playerNo, int capsuleNo);
void mbev_CapRandomBonusCoin(int playerNo, int capsuleNo, BOOL waitF);
int mbCapEffUseModeGet(int playerNo);
BOOL mbev_CapBonusCoinCheck(int playerNo);
OMOBJ *mbev_CapEffExplodeCreate(void);
OMOBJ *mbev_CapEffGlowCreate(void);
void mbev_CapEffGlowBlendModeSet(OMOBJ *obj, int blendMode);
void mbev_CapEffGlowPatSet(OMOBJ *obj, int pat);
int mbev_CapEffGlowDispGet(OMOBJ *obj);
int mbev_CapEffGlowKinokoAdd(OMOBJ *obj, HuVecF *pos, int time, float scale,
    float xRange, float yRange, float zRange, int type, GXColor *color);
int mbev_CapEffGlowKinokoTimeSet(OMOBJ *obj, int index, int time, int delay);
OMOBJ *mbev_CapEffRingCreate(void);
int mbev_CapEffGlowAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel, int time,
    float scale, float gravity, float unk, GXColor *color);
void mbev_CapEffColorSet(GXColor *color, int colorNo);
void mbev_CapEffDustExplodeAdd(OMOBJ *obj, HuVecF *pos);
int mbev_CapEffExplodeAnimGet(OMOBJ *obj);
int mbev_CapPlayerMasuNumGet(int masuId);
s16 mbev_CapPlayerMotionCreate(EVCAPWORK *work, int playerNo, int dataNum);
void mbev_CapBezierGetV(float time, float *a, float *b, float *c, float *out);
void mbev_CapBubbleHookCall(int type, int modelId, BOOL flag1, BOOL flag2,
    BOOL flag3);
void mbev_CapPlayerMotShiftWait(int playerNo, int motNo, u32 attr, BOOL waitF);
void mbev_StarMasu(int playerNo);
extern s32 mbBGRead(s32 dataNum);
extern void mbBGReadWait(s32 statId);
extern OMOBJ *mbev_CapEffExhaustCreate(void);
extern OMOBJ *mbev_CapEffBoostCreate(void);
extern void mbev_CapEffBoostBlendModeSet(OMOBJ *obj, int blendMode);
extern void mbev_CapPlayerMoveObjInit(void);
extern void mbev_CapPlayerMoveIdleCreate(int playerNo);
extern void mbev_CapPlayerMoveHitCreate(int playerNo, BOOL useMotF,
    BOOL useShiftF);
extern BOOL mbev_CapPlayerMoveObjCheck(void);
extern void mbev_CapPlayerIdleWait(int playerNo);
extern int mbPlayerCoinGet(int playerNo);
extern s16 mbev_CapMasuLinkNextGet(s16 masuId, HuVecF *pos);
extern s16 mbev_CapMasuLinkNextRandomGet(s16 masuId, HuVecF *pos);
extern void mbev_CapEffExplodeKillerAdd(OMOBJ *obj, HuVecF *pos,
    HuVecF *vel, float active, float angleStep, float fadeStep,
    float scale, GXColor *color);
extern int mbev_CapEffExplodeAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel,
    float active, float angleStep, float fadeStep, GXColor *color);
extern int mbev_CapEffBoostAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel,
    float active, float angleStep, int time, GXColor *color);
extern void mbev_CapEffDustCloudAdd(OMOBJ *obj, HuVecF *pos);
extern int mbDiceResultGet(int playerNo);
extern void mbev_CapStatusDispSetAll(BOOL dispF, BOOL waitF);
extern s16 mbCoinDispCapsuleCreate(HuVecF *pos, int coinNum);
extern int mbCoinAddDispExec(int playerNo, int coinNum, BOOL dispF,
    BOOL fastF);
extern void mbMoveNumKill(int playerNo);
extern float mbev_CapAngleSumLerp(float t, float a, float b);
extern void mbev_CapHermiteGetV(float t, HuVecF *a, HuVecF *b, HuVecF *c,
    HuVecF *d, HuVecF *out);

static void ev_CapEffKinokoCreate(CAPWORK *work);
static void ev_CapEffKinokoOMExec(OMOBJ *obj);
static void ev_CapHanachanOMExec(OMOBJ *obj);
static void ev_CapEffKillerDustCreate(CAPWORK *work, HuVecF *pos,
    HuVecF *rot);
static void ev_CapEffKillerExplodeCreate(CAPWORK *work, HuVecF *pos,
    HuVecF *rot, int count);
static void ev_CapEffKillerBoostCreate(CAPWORK *work, HuVecF *pos,
    HuVecF *rot);

void mbev_CapKinoko(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    HuVecF playerPosNext;
    HuVecF playerPos;
    int modelId;
    int i;
    float time;
    float scale;
    float radius;
    float yOfs;

    mbev_CapWait(work);
    work->explodeObj = mbev_CapEffExplodeCreate();
    work->glowObj = mbev_CapEffGlowCreate();
    mbev_CapEffGlowBlendModeSet(work->glowObj, 1);
    mbPlayerPosGet(work->playerNo, &playerPos);
    playerPos.y += 250.0f;
    modelId = mbev_CapObjCreate(&work->objWork, CAPMOVE_DATA_KINOKO, NULL, FALSE, 0, FALSE);
    mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
    mbObjScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    mbObjDispSet(modelId, FALSE);
    mbCapEffUseCreate(work->playerNo, work->capsuleNo);
    while (mbCapEffUseModeGet(work->playerNo) < 2) {
        HuPrcVSleep();
    }
    mbObjDispSet(modelId, TRUE);
    work->_unkB6C = modelId;
    ev_CapEffKinokoCreate(work);
    for (i = 0; i < 30.0f; i++) {
        time = i / 30.0f;
        scale = sin((M_PI * (180.0f * time)) / 180.0f) + 1.0f;
        mbObjScaleSet(modelId, scale, scale, scale);
        HuPrcVSleep();
    }
    mbev_CapRandomBonusCoin(work->playerNo, work->capsuleNo, FALSE);
    for (i = 0; i < 60.0f || !mbev_CapBonusCoinCheck(work->playerNo); i++) {
        time = i / 60.0f;
        mbPlayerPosGet(work->playerNo, &playerPos);
        playerPos.y += (100.0f * sin((M_PI * (360.0f * time)) / 180.0f)) * 0.1f + 250.0f;
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        HuPrcVSleep();
    }
    for (i = 1; i <= 10; i++) {
        time = i / 10.0f;
        mbPlayerPosGet(work->playerNo, &playerPosNext);
        playerPos.y += time * ((playerPosNext.y + 250.0f) - playerPos.y);
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        HuPrcVSleep();
    }
    mbAudFXPlay(MSM_SE_BRD00_36);
    for (i = 0; i < 90.0f; i++) {
        time = 1.0f - (i / 90.0f);
        yOfs = 50.0f + (time * time * 200.0f);
        radius = sin((M_PI * (180.0f * (time * time))) / 180.0f);
        mbPlayerPosGet(work->playerNo, &playerPos);
        playerPos.x += (radius * cos((M_PI * (2.0f * (time * 360.0f))) / 180.0f)) * 100.0f * 1.5f;
        playerPos.z += (radius * sin((M_PI * (2.0f * (time * 360.0f))) / 180.0f)) * 100.0f * 1.5f;
        playerPos.y += yOfs;
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        mbObjScaleSet(modelId, time, time, time);
        HuPrcVSleep();
    }
    GwPlayer[work->playerNo].diceMode = 1;
    omVibrate(work->playerNo, 20, 4, 4);
    for (i = 0; i < 6.0f; i++) {
        HuPrcVSleep();
    }
    while (mbev_CapEffGlowDispGet(work->glowObj) > 0) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

void mbev_CapKinokoKill(void)
{
}

void mbev_CapSKinoko(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    HuVecF playerPosNext;
    HuVecF playerPos;
    int modelId;
    int i;
    float time;
    float scale;
    float radius;
    float yOfs;

    mbev_CapWait(work);
    work->explodeObj = mbev_CapEffExplodeCreate();
    work->glowObj = mbev_CapEffGlowCreate();
    mbev_CapEffGlowBlendModeSet(work->glowObj, 1);
    mbPlayerPosGet(work->playerNo, &playerPos);
    playerPos.y += 250.0f;
    modelId = mbev_CapObjCreate(&work->objWork, CAPMOVE_DATA_S_KINOKO, NULL, FALSE, 0, FALSE);
    mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
    mbObjScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    mbObjDispSet(modelId, FALSE);
    mbCapEffUseCreate(work->playerNo, work->capsuleNo);
    while (mbCapEffUseModeGet(work->playerNo) < 2) {
        HuPrcVSleep();
    }
    mbObjDispSet(modelId, TRUE);
    work->_unkB6C = modelId;
    ev_CapEffKinokoCreate(work);
    for (i = 0; i < 30.0f; i++) {
        time = i / 30.0f;
        scale = sin((M_PI * (180.0f * time)) / 180.0f) + 1.0f;
        mbObjScaleSet(modelId, scale, scale, scale);
        HuPrcVSleep();
    }
    mbev_CapRandomBonusCoin(work->playerNo, work->capsuleNo, FALSE);
    for (i = 0; i < 60.0f || !mbev_CapBonusCoinCheck(work->playerNo); i++) {
        time = i / 60.0f;
        mbPlayerPosGet(work->playerNo, &playerPos);
        playerPos.y += (100.0f * sin((M_PI * (360.0f * time)) / 180.0f)) * 0.1f + 250.0f;
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        HuPrcVSleep();
    }
    for (i = 1; i <= 10; i++) {
        time = i / 10.0f;
        mbPlayerPosGet(work->playerNo, &playerPosNext);
        playerPos.y += time * ((playerPosNext.y + 250.0f) - playerPos.y);
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        HuPrcVSleep();
    }
    mbAudFXPlay(MSM_SE_BRD00_36);
    for (i = 0; i < 90.0f; i++) {
        time = 1.0f - (i / 90.0f);
        yOfs = 50.0f + (time * time * 200.0f);
        radius = sin((M_PI * (180.0f * (time * time))) / 180.0f);
        mbPlayerPosGet(work->playerNo, &playerPos);
        playerPos.x += (radius * cos((M_PI * (2.0f * (time * 360.0f))) / 180.0f)) * 100.0f * 1.5f;
        playerPos.z += (radius * sin((M_PI * (2.0f * (time * 360.0f))) / 180.0f)) * 100.0f * 1.5f;
        playerPos.y += yOfs;
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        mbObjScaleSet(modelId, time, time, time);
        HuPrcVSleep();
    }
    GwPlayer[work->playerNo].diceMode = 2;
    omVibrate(work->playerNo, 20, 4, 4);
    for (i = 0; i < 6.0f; i++) {
        HuPrcVSleep();
    }
    while (mbev_CapEffGlowDispGet(work->glowObj) > 0) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

void mbev_CapSKinokoKill(void)
{
}

void mbev_CapPKinoko(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    HuVecF playerPosNext;
    HuVecF playerPos;
    int modelId;
    int i;
    float time;
    float scale;
    float radius;
    float yOfs;

    mbev_CapWait(work);
    work->explodeObj = mbev_CapEffExplodeCreate();
    work->glowObj = mbev_CapEffGlowCreate();
    mbev_CapEffGlowBlendModeSet(work->glowObj, 1);
    mbPlayerPosGet(work->playerNo, &playerPos);
    playerPos.y += 250.0f;
    modelId = mbev_CapObjCreate(&work->objWork, CAPMOVE_DATA_P_KINOKO, NULL, FALSE, 0, FALSE);
    mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
    mbObjScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    mbObjDispSet(modelId, FALSE);
    mbCapEffUseCreate(work->playerNo, work->capsuleNo);
    while (mbCapEffUseModeGet(work->playerNo) < 2) {
        HuPrcVSleep();
    }
    mbObjDispSet(modelId, TRUE);
    work->_unkB6C = modelId;
    ev_CapEffKinokoCreate(work);
    for (i = 0; i < 30.0f; i++) {
        time = i / 30.0f;
        scale = sin((M_PI * (180.0f * time)) / 180.0f) + 1.0f;
        mbObjScaleSet(modelId, scale, scale, scale);
        HuPrcVSleep();
    }
    mbev_CapRandomBonusCoin(work->playerNo, work->capsuleNo, FALSE);
    for (i = 0; i < 60.0f || !mbev_CapBonusCoinCheck(work->playerNo); i++) {
        time = i / 60.0f;
        mbPlayerPosGet(work->playerNo, &playerPos);
        playerPos.y += (100.0f * sin((M_PI * (360.0f * time)) / 180.0f)) * 0.1f + 250.0f;
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        HuPrcVSleep();
    }
    for (i = 1; i <= 10; i++) {
        time = i / 10.0f;
        mbPlayerPosGet(work->playerNo, &playerPosNext);
        playerPos.y += time * ((playerPosNext.y + 250.0f) - playerPos.y);
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        HuPrcVSleep();
    }
    mbAudFXPlay(MSM_SE_BRD00_36);
    for (i = 0; i < 90.0f; i++) {
        time = 1.0f - (i / 90.0f);
        yOfs = 50.0f + (time * time * 200.0f);
        radius = sin((M_PI * (180.0f * (time * time))) / 180.0f);
        mbPlayerPosGet(work->playerNo, &playerPos);
        playerPos.x += (radius * cos((M_PI * (2.0f * (time * 360.0f))) / 180.0f)) * 100.0f * 1.5f;
        playerPos.z += (radius * sin((M_PI * (2.0f * (time * 360.0f))) / 180.0f)) * 100.0f * 1.5f;
        playerPos.y += yOfs;
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        mbObjScaleSet(modelId, time, time, time);
        HuPrcVSleep();
    }
    GwPlayer[work->playerNo].diceMode = 3;
    omVibrate(work->playerNo, 20, 4, 4);
    for (i = 0; i < 6.0f; i++) {
        HuPrcVSleep();
    }
    while (mbev_CapEffGlowDispGet(work->glowObj) > 0) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

void mbev_CapPKinokoKill(void)
{
}

void mbev_CapMKinoko(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    HuVecF playerPosNext;
    HuVecF playerPos;
    int modelId;
    int i;
    float time;
    float scale;
    float radius;
    float yOfs;

    mbev_CapWait(work);
    work->explodeObj = mbev_CapEffExplodeCreate();
    work->glowObj = mbev_CapEffGlowCreate();
    mbev_CapEffGlowBlendModeSet(work->glowObj, 1);
    mbPlayerPosGet(work->playerNo, &playerPos);
    playerPos.y += 250.0f;
    modelId = mbev_CapObjCreate(&work->objWork, CAPMOVE_DATA_M_KINOKO, NULL, FALSE, 0, FALSE);
    mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
    mbObjScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    mbObjDispSet(modelId, FALSE);
    mbCapEffUseCreate(work->playerNo, work->capsuleNo);
    while (mbCapEffUseModeGet(work->playerNo) < 2) {
        HuPrcVSleep();
    }
    mbObjDispSet(modelId, TRUE);
    work->_unkB6C = modelId;
    ev_CapEffKinokoCreate(work);
    for (i = 0; i < 30.0f; i++) {
        time = i / 30.0f;
        scale = sin((M_PI * (180.0f * time)) / 180.0f) + 1.0f;
        mbObjScaleSet(modelId, scale, scale, scale);
        HuPrcVSleep();
    }
    mbev_CapRandomBonusCoin(work->playerNo, work->capsuleNo, FALSE);
    for (i = 0; i < 60.0f || !mbev_CapBonusCoinCheck(work->playerNo); i++) {
        time = i / 60.0f;
        mbPlayerPosGet(work->playerNo, &playerPos);
        playerPos.y += (100.0f * sin((M_PI * (360.0f * time)) / 180.0f)) * 0.1f + 250.0f;
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        HuPrcVSleep();
    }
    for (i = 1; i <= 10; i++) {
        time = i / 10.0f;
        mbPlayerPosGet(work->playerNo, &playerPosNext);
        playerPos.y += time * ((playerPosNext.y + 250.0f) - playerPos.y);
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        HuPrcVSleep();
    }
    mbAudFXPlay(MSM_SE_BRD00_36);
    for (i = 0; i < 90.0f; i++) {
        time = 1.0f - (i / 90.0f);
        yOfs = 50.0f + (time * time * 200.0f);
        radius = sin((M_PI * (180.0f * (time * time))) / 180.0f);
        mbPlayerPosGet(work->playerNo, &playerPos);
        playerPos.x += (radius * cos((M_PI * (2.0f * (time * 360.0f))) / 180.0f)) * 100.0f * 1.5f;
        playerPos.z += (radius * sin((M_PI * (2.0f * (time * 360.0f))) / 180.0f)) * 100.0f * 1.5f;
        playerPos.y += yOfs;
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        mbObjScaleSet(modelId, time, time, time);
        HuPrcVSleep();
    }
    GwPlayer[work->playerNo].diceMode = 4;
    mbPlayerMetalSet(work->playerNo, TRUE);
    omVibrate(work->playerNo, 20, 4, 4);
    mbAudFXPlay(MSM_SE_BRD00_69);
    while (mbev_CapEffGlowDispGet(work->glowObj) > 0) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

void mbev_CapMKinokoKill(void)
{
}

void mbev_CapKiller(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;

    mbev_CapWait(work);
    mbCapEffUseCreate(work->playerNo, work->capsuleNo);
    omVibrate(work->playerNo, 20, 4, 4);
    HuPrcSleep(20);
    mbev_CapRandomBonusCoin(work->playerNo, work->capsuleNo, TRUE);
    while (mbCapEffUseModeGet(work->playerNo) >= 0) {
        HuPrcVSleep();
    }
    GwPlayer[work->playerNo].diceMode = 5;
    HuPrcEnd();
}

void mbev_CapKillerKill(void)
{
}

void mbev_CapDokan(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    HuVecF masuPos[2];
    HuVecF playerPos;
    HuVecF playerPosSwap;
    int modelId[2];
    int playerNo[2];
    int masuId[2];
    int playerMotionId[2][2];
    int swapPlayer;
    int i;
    int j;
    float time;
    float scale;

    mbCapEffUseCreate(work->playerNo, work->capsuleNo);
    omVibrate(work->playerNo, 20, 4, 4);
    HuPrcSleep(20);
    mbev_CapRandomBonusCoin(work->playerNo, work->capsuleNo, TRUE);
    while (mbCapEffUseModeGet(work->playerNo) >= 0) {
        HuPrcVSleep();
    }
    mbWinCreate(2, CAPMOVE_MESS_DOKAN_TARGET, -1);
    mbWinTopWait();
    if (mbRouletteCreate(work->playerNo, 3)) {
        mbRouletteWait();
    }
    swapPlayer = mbRouletteResultGet();
    playerNo[0] = work->playerNo;
    playerNo[1] = swapPlayer;
    masuId[0] = GwPlayer[playerNo[0]].masuId;
    masuId[1] = GwPlayer[playerNo[1]].masuId;
    if (masuId[0] == masuId[1]) {
        mbWinCreate(2, CAPMOVE_MESS_DOKAN_SAME_SPACE, -1);
        mbWinTopWait();
        HuPrcEnd();
    }
    if (mbev_CapPlayerMasuNumGet(masuId[0]) > 1) {
        mbev_PlayerColCircleAdd(playerNo[0], masuId[0], FALSE, 125.0f);
        while (GwPlayer[playerNo[0]].moveF) {
            HuPrcVSleep();
        }
    }
    if (mbev_CapPlayerMasuNumGet(masuId[1]) > 1) {
        mbev_PlayerColCircleAdd(playerNo[1], masuId[1], FALSE, 125.0f);
        while (GwPlayer[playerNo[1]].moveF) {
            HuPrcVSleep();
        }
    }
    mbPlayerPosGet(playerNo[0], &playerPos);
    mbPlayerPosGet(playerNo[1], &playerPosSwap);
    playerMotionId[0][0] = mbev_CapPlayerMotionCreate(
        &work->objWork, playerNo[0], CHARMOT_HSF_c000m1_381);
    HuPrcVSleep();
    playerMotionId[0][1] = mbev_CapPlayerMotionCreate(
        &work->objWork, playerNo[0], CHARMOT_HSF_c000m1_382);
    HuPrcVSleep();
    playerMotionId[1][0] = mbev_CapPlayerMotionCreate(
        &work->objWork, playerNo[1], CHARMOT_HSF_c000m1_381);
    HuPrcVSleep();
    playerMotionId[1][1] = mbev_CapPlayerMotionCreate(
        &work->objWork, playerNo[1], CHARMOT_HSF_c000m1_382);
    HuPrcVSleep();
    mbev_CapWait(work);
    for (i = 0; i < 2; i++) {
        modelId[i] = mbev_CapObjCreate(&work->objWork, CAPMOVE_DATA_DOKAN,
            NULL, TRUE, 5, FALSE);
        mbMasuPosGet(masuId[i], &masuPos[i]);
        mbObjDispSet(modelId[i], FALSE);
        HuPrcVSleep();
    }
    mbAudFXPlay(MSM_SE_BRD00_39);
    for (i = 0; i < 30.0f; i++) {
        time = i / 30.0f;
        for (j = 0; j < 2; j++) {
            mbMasuPosGet(masuId[j], &masuPos[j]);
            mbObjPosSet(modelId[j], masuPos[j].x, masuPos[j].y,
                masuPos[j].z);
            mbObjScaleSet(modelId[j],
                time + (0.5f * sin((M_PI * (180.0f * time)) / 180.0f)),
                time + (0.5f * sin((M_PI * (180.0f * time)) / 180.0f)),
                time + (0.5f * sin((M_PI * (180.0f * time)) / 180.0f)));
            mbObjDispSet(modelId[j], TRUE);
            mbPlayerPosSet(playerNo[j], masuPos[j].x,
                masuPos[j].y + (100.0f *
                    (time + (0.5f * sin((M_PI * (180.0f * time)) /
                        180.0f)))),
                masuPos[j].z);
            mbPlayerColSnapPlayerSet(playerNo[j], FALSE);
        }
        HuPrcVSleep();
    }
    for (i = 0; i < 2; i++) {
        if (playerMotionId[i][0] != -1) {
            mbPlayerMotionShiftSet(playerNo[i], playerMotionId[i][0],
                0.0f, 8.0f, HU3D_MOTATTR_NONE);
        }
        omVibrate(playerNo[i], 20, 7, 3);
    }
    mbAudFXPlay(MSM_SE_BRD00_40);
    for (i = 0; i < 30.0f; i++) {
        time = 1.0f - (i / 30.0f);
        for (j = 0; j < 2; j++) {
            mbMasuPosGet(masuId[j], &masuPos[j]);
            mbObjPosSetV(modelId[j], &masuPos[j]);
            mbPlayerPosSet(playerNo[j], masuPos[j].x,
                masuPos[j].y + 100.0f, masuPos[j].z);
            mbPlayerRotSet(playerNo[j], 0.0f, 720.0f * time, 0.0f);
            scale = 0.5f + (0.5f * time);
        }
        HuPrcVSleep();
    }
    scale = 0.0f;
    mbPlayerScaleSet(playerNo[0], scale, scale, scale);
    mbPlayerScaleSet(playerNo[1], scale, scale, scale);
    for (i = 0; i < 30.0f; i++) {
        time = i / 30.0f;
        for (j = 0; j < 2; j++) {
            mbMasuPosGet(masuId[j], &masuPos[j]);
            mbObjPosSetV(modelId[j], &masuPos[j]);
            mbObjPosSet(modelId[j], masuPos[j].x, masuPos[j].y,
                masuPos[j].z);
            mbObjScaleSet(modelId[j], 1.0f, 1.0f - time, 1.0f);
        }
        HuPrcVSleep();
    }
    mbObjDispSet(modelId[0], FALSE);
    mbObjDispSet(modelId[1], FALSE);
    mbWipeDissolveFadeOutTime(1);
    mbPlayerPosSetV(playerNo[0], &playerPosSwap);
    GwPlayer[playerNo[0]].masuId = masuId[1];
    mbPlayerPosSetV(playerNo[1], &playerPos);
    GwPlayer[playerNo[1]].masuId = masuId[0];
    mbCameraMoveMasu(masuId[1], NULL, &capsuleCameraOfs, -1.0f, -1.0f,
        -1);
    mbCameraMoveWait();
    mbStarMasuDispSet(masuId[1], FALSE);
    mbWipeDissolveFadeIn();
    mbAudFXPlay(MSM_SE_BRD00_39);
    mbObjDispSet(modelId[0], TRUE);
    mbObjDispSet(modelId[1], TRUE);
    for (i = 0; i < 30.0f; i++) {
        time = i / 30.0f;
        for (j = 0; j < 2; j++) {
            mbMasuPosGet(masuId[j], &masuPos[j]);
            mbObjPosSetV(modelId[j], &masuPos[j]);
            mbObjScaleSet(modelId[j],
                time + (0.5f * sin((M_PI * (180.0f * time)) / 180.0f)),
                time + (0.5f * sin((M_PI * (180.0f * time)) / 180.0f)),
                time + (0.5f * sin((M_PI * (180.0f * time)) / 180.0f)));
        }
        HuPrcVSleep();
    }
    for (i = 0; i < 2; i++) {
        if (playerMotionId[i][1] != -1) {
            mbPlayerMotionSet(playerNo[i], playerMotionId[i][1],
                HU3D_MOTATTR_NONE);
            mbPlayerMotionSpeedSet(playerNo[i], 0.75f);
            mbPlayerScaleSet(playerNo[i], 1.0f, 1.0f, 1.0f);
        }
        omVibrate(playerNo[i], 20, 7, 3);
    }
    for (i = 0; i < 30.0f; i++) {
        time = i / 30.0f;
        for (j = 0; j < 2; j++) {
            mbMasuPosGet(masuId[j], &masuPos[j]);
            mbObjPosSetV(modelId[j], &masuPos[j]);
            mbPlayerPosSet(playerNo[j], masuPos[j ^ 1].x,
                masuPos[j ^ 1].y + 100.0f, masuPos[j ^ 1].z);
            mbPlayerRotSet(playerNo[j], 0.0f, 720.0f * -time, 0.0f);
            scale = 0.5f + (0.5f * time);
        }
        HuPrcVSleep();
    }
    for (j = 0; j < 2; j++) {
        mbPlayerMotionShiftSet(playerNo[j], 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
    }
    for (i = 0; i < 30.0f; i++) {
        time = i / 30.0f;
        for (j = 0; j < 2; j++) {
            mbMasuPosGet(masuId[j], &masuPos[j]);
            mbObjPosSetV(modelId[j], &masuPos[j]);
            mbObjScaleSet(modelId[j], 1.0f, 1.0f - time, 1.0f);
            mbPlayerPosSet(playerNo[j], masuPos[j ^ 1].x,
                masuPos[j ^ 1].y + (100.0f * (1.0f - time)),
                masuPos[j ^ 1].z);
            mbPlayerRotSet(playerNo[j], 0.0f, 0.0f, 0.0f);
        }
        HuPrcVSleep();
    }
    for (j = 0; j < 2; j++) {
        mbPlayerColSnapPlayerSet(playerNo[j], TRUE);
    }
    mbPlayerColFirstSet(playerNo[0]);
    for (j = 0; j < 2; j++) {
        GwPlayer[playerNo[j]].masuIdNext = masuId[j ^ 1];
    }
    mbObjDispSet(modelId[0], FALSE);
    mbObjDispSet(modelId[1], FALSE);
    HuPrcEnd();
}

void mbev_CapDokanKill(void)
{
}

void mbev_CapHanachan(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    HuVecF playerPos;
    HuVecF basePos;
    HuVecF starPos;
    HuVecF startPos;
    HuVecF controlPos;
    HuVecF endPos;
    HuVecF modelPos;
    HuVecF hookPos;
    HuVecF tempVec;
    Mtx hookMtx;
    OMOBJ *obj;
    int motFile[3];
    int fallMotionId;
    int hangMotionId;
    int modelId;
    int focusModelId;
    int starMasuId;
    int soundId;
    int cancelF;
    int i;
    float time;
    float angle;
    float radius;
    float hookYOfs;
    float rotX;
    float distance;
    float ease;

    work->explodeObj = mbev_CapEffExplodeCreate();
    HuPrcVSleep();
    work->glowObj = mbev_CapEffGlowCreate();
    HuPrcVSleep();
    work->ringObj = mbev_CapEffRingCreate();
    HuPrcVSleep();
    fallMotionId = mbev_CapPlayerMotionCreate(&work->objWork,
        work->playerNo, CHARMOT_HSF_c000m1_344);
    HuPrcVSleep();
    hangMotionId = mbev_CapPlayerMotionCreate(&work->objWork,
        work->playerNo, CHARMOT_HSF_c000m1_376);
    HuPrcVSleep();
    mbev_CapWait(work);
    motFile[0] = CAPMOVE_DATA_HANACHAN_MOTION_A;
    motFile[1] = CAPMOVE_DATA_HANACHAN_MOTION_B;
    motFile[2] = -1;
    modelId = mbev_CapObjCreate(&work->objWork, CAPMOVE_DATA_HANACHAN,
        motFile, FALSE, 5, FALSE);
    mbObjLayerSet(modelId, 3);
    mbObjMotionSet(modelId, 1, HU3D_MOTATTR_LOOP);
    mbObjDispSet(modelId, FALSE);
    focusModelId = mbev_CapObjCreate(&work->objWork,
        CAPMOVE_DATA_CAMERA_TARGET, NULL, FALSE, 5, FALSE);
    mbObjDispSet(focusModelId, FALSE);
    HuPrcVSleep();
    mbPlayerPosGet(work->playerNo, &playerPos);
    basePos = playerPos;
    for (i = 1; i <= mbMasuRawNumGet(); i++) {
        if (mbMasuTypeGet(i) == CAPMOVE_MASU_TYPE_STAR) {
            starMasuId = i;
            break;
        }
    }
    if (i > mbMasuRawNumGet()) {
        starMasuId = GwPlayer[work->playerNo].masuId;
    }
    mbMasuPosGet(starMasuId, &starPos);
    mbObjPosSetV(focusModelId, &playerPos);
    mbCameraMoveObj(focusModelId, NULL, &capsuleCameraOfs, -1.0f, -1.0f,
        21);
    mbCameraMoveWait();
    obj = omAddObjEx(
        mbObjMan, -32768, 0, 0, -1, ev_CapHanachanOMExec);
    obj->work[0] = 0;
    obj->work[1] = 0;
    work->_unkB6C = modelId;
    work->objWork.obj = obj;
    obj->data = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(CAPWORK), HU_MEMNUM_OVL);
    memcpy(obj->data, work, sizeof(CAPWORK));
    soundId = mbAudFXPlay(MSM_SE_BRD00_48);
    startPos.x = playerPos.x;
    startPos.y = playerPos.y + 250.0f;
    startPos.z = playerPos.z - 100.0f;
    modelPos = basePos;
    modelPos.y += 250.0f;
    controlPos = modelPos;
    controlPos.y -= 200.0f;
    controlPos.z += 500.0f;
    endPos = modelPos;
    endPos.y += 300.0f;
    tempVec = modelPos;
    tempVec.y += 200.0f;
    tempVec.z -= 300.0f;
    mbCapEffUseCreate(work->playerNo, work->capsuleNo);
    omVibrate(work->playerNo, 20, 4, 4);
    while (mbCapEffUseModeGet(work->playerNo) < 2) {
        HuPrcVSleep();
    }
    mbObjDispSet(modelId, TRUE);
    for (i = 0; i < 90.0f; i++) {
        time = i / 90.0f;
        mbObjRotSet(modelId, 45.0f - (405.0f * time), 0.0f, 0.0f);
        if (time < 0.5f) {
            mbev_CapBezierGetV(
                sin((M_PI * (90.0f * (2.0f * time))) / 180.0f),
                (float *)&modelPos, (float *)&controlPos,
                (float *)&endPos, (float *)&hookPos);
        } else {
            mbev_CapBezierGetV(cos((M_PI *
                (90.0f - (90.0f * (2.0f * (time - 0.5f))))) / 180.0f),
                (float *)&endPos, (float *)&tempVec, (float *)&startPos,
                (float *)&hookPos);
        }
        mbObjPosSetV(modelId, &hookPos);
        HuPrcVSleep();
    }
    mbev_CapRandomBonusCoin(work->playerNo, work->capsuleNo, TRUE);
    mbPlayerRotateStart(work->playerNo, 180, 15);
    while (!mbPlayerRotateCheck(work->playerNo)) {
        HuPrcVSleep();
    }
    HuPrcSleep(15);
    mbAudFXPlay(MSM_SE_GUIDE_32);
    mbWinCreate(2, CAPMOVE_MESS_HANACHAN_ARRIVE,
        HUWIN_SPEAKER_HANACHAN_STAR);
    mbWinTopWait();
    cancelF = FALSE;
    if (cancelF) {
        mbAudFXPlay(MSM_SE_BRD00_35);
        mbAudFXPlay(MSM_SE_BRD00_10);
        if (soundId != -1) {
            mbAudFXStop(soundId);
        }
        if (!mbExitCheck()) {
            obj->work[0]++;
        }
        modelPos = startPos;
        mbev_CapEffDustExplodeAdd(work->explodeObj, &modelPos);
        mbObjDispSet(modelId, FALSE);
        do {
            HuPrcVSleep();
        } while (mbev_CapEffExplodeAnimGet(work->explodeObj));
        mbPlayerRotateStart(work->playerNo, 0, 15);
        while (!mbPlayerRotateCheck(work->playerNo)) {
            HuPrcVSleep();
        }
        mbCameraPlayerViewSet(work->playerNo, 0);
        mbCameraMoveWait();
        mbMusBoardPlay();
    } else {
        mbev_CapBubbleHookCall(3, modelId, TRUE, FALSE, FALSE);
        mbWinCreate(2, CAPMOVE_MESS_HANACHAN_DEPART,
            HUWIN_SPEAKER_HANACHAN_STAR);
        mbWinTopWait();
        modelPos = startPos;
        PSVECSubtract(&startPos, &playerPos, &tempVec);
        if (PSVECMag(&tempVec) > 0.0f) {
            PSVECNormalize(&tempVec, &tempVec);
        }
        endPos.x = playerPos.x + (50.0f * tempVec.x);
        endPos.y = playerPos.y + 200.0f;
        endPos.z = playerPos.z + (50.0f * tempVec.z) - 100.0f;
        controlPos = startPos;
        controlPos.y -= 200.0f;
        controlPos.z += 50.0f;
        radius = hanachanPlayerOfs[GwPlayer[work->playerNo].charNo].z;
        hookYOfs = hanachanPlayerOfs[GwPlayer[work->playerNo].charNo].y;
        rotX = hanachanPlayerOfs[GwPlayer[work->playerNo].charNo].x;
        angle = 0.0f;
        mbPlayerMotionShiftSet(work->playerNo, 4, 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
        mbPlayerColSnapPlayerSet(work->playerNo, FALSE);
        for (i = 0; i < 30.0f; i++) {
            time = i / 30.0f;
            mbev_CapBezierGetV(time, (float *)&modelPos,
                (float *)&controlPos, (float *)&endPos,
                (float *)&startPos);
            mbObjPosSetV(modelId, &startPos);
            tempVec = basePos;
            tempVec.y += 200.0f *
                sin((M_PI * (180.0f * time)) / 180.0f);
            mbPlayerPosSetV(work->playerNo, &tempVec);
            mbPlayerRotSet(work->playerNo, 0.0f,
                180.0f - (180.0f * time), 0.0f);
            HuPrcVSleep();
        }
        startPos = endPos;
        mbObjPosSetV(modelId, &startPos);
        mbObjMotionShiftSet(modelId, 2, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        if (hangMotionId != -1) {
            mbPlayerMotionShiftSet(work->playerNo, hangMotionId, 0.0f,
                15.0f, HU3D_MOTATTR_LOOP);
        }
        omVibrate(work->playerNo, 20, 7, 3);
        mbPlayerPosGet(work->playerNo, &playerPos);
        for (i = 0; i < 24.0f; i++) {
            time = i / 24.0f;
            Hu3DModelObjMtxGet(
                mbObjModelIDGet(modelId), "itemhook_C", hookMtx);
            hookPos.x = hookMtx[0][3] +
                (radius * sin((M_PI * angle) / 180.0f));
            hookPos.y = hookMtx[1][3] + hookYOfs;
            hookPos.z = hookMtx[2][3] +
                (radius * cos((M_PI * angle) / 180.0f));
            tempVec.x = playerPos.x + (time * (hookPos.x - playerPos.x));
            tempVec.y = playerPos.y + (time * (hookPos.y - playerPos.y));
            tempVec.z = playerPos.z + (time * (hookPos.z - playerPos.z));
            tempVec.y -= 0.25f * (100.0f *
                sin((M_PI * (180.0f * time)) / 180.0f));
            mbPlayerPosSetV(work->playerNo, &tempVec);
            mbPlayerRotGet(work->playerNo, &tempVec);
            tempVec.x = time * rotX;
            mbPlayerRotSetV(work->playerNo, &tempVec);
            tempVec = startPos;
            tempVec.y -= 0.25f * (100.0f *
                sin((M_PI * (180.0f * time)) / 180.0f));
            mbObjPosSetV(modelId, &tempVec);
            HuPrcVSleep();
        }
        omVibrate(work->playerNo, 180, 4, 4);
        endPos = startPos;
        for (i = 0; i < 90.0f; i++) {
            time = i / 90.0f;
            angle = 180.0f * time;
            distance = sin((M_PI * (90.0f * time)) / 180.0f);
            startPos.x = endPos.x + (300.0f *
                (sin((M_PI * angle) / 180.0f) * distance));
            startPos.z = endPos.z + (300.0f *
                (cos((M_PI * angle) / 180.0f) * distance));
            startPos.y = endPos.y + (550.0f * time);
            mbObjPosSet(modelId, startPos.x, startPos.y, startPos.z);
            mbObjRotSet(modelId, 0.0f, angle, 0.0f);
            Hu3DModelObjMtxGet(
                mbObjModelIDGet(modelId), "itemhook_C", hookMtx);
            hookMtx[1][3] += hookYOfs;
            hookMtx[0][3] += radius *
                sin((M_PI * angle) / 180.0f);
            hookMtx[2][3] += radius *
                cos((M_PI * angle) / 180.0f);
            tempVec.x = hookMtx[0][3];
            tempVec.y = hookMtx[1][3];
            tempVec.z = hookMtx[2][3];
            mbPlayerPosSetV(work->playerNo, &tempVec);
            mbPlayerRotSet(work->playerNo, rotX, angle, 0.0f);
            HuPrcVSleep();
        }
        mbObjDispSet(modelId, FALSE);
        mbPlayerDispSet(work->playerNo, FALSE);
        mbWipeDissolveFadeOutTime(1);
        mbev_PlayerColMasuSet(work->playerNo, starMasuId, TRUE);
        GwPlayer[work->playerNo].masuId = starMasuId;
        startPos = starPos;
        startPos.y += 750.0f;
        mbObjPosSet(modelId, startPos.x, startPos.y, startPos.z);
        mbObjRotSet(modelId, 0.0f, 0.0f, 0.0f);
        playerPos = starPos;
        playerPos.y += 650.0f;
        mbCameraMoveMasu(starMasuId, NULL, &capsuleCameraOfs, -1.0f,
            -1.0f, -1);
        mbCameraMoveWait();
        mbWipeDissolveFadeIn();
        omVibrate(work->playerNo, 180, 4, 4);
        endPos = starPos;
        endPos.y += 100.0f;
        endPos.z -= 50.0f;
        mbObjDispSet(modelId, TRUE);
        mbPlayerDispSet(work->playerNo, TRUE);
        for (i = 0; i < 150.0f; i++) {
            time = i / 150.0f;
            angle = 180.0f + (360.0f * time);
            distance = 200.0f *
                sin((M_PI * (180.0f * time)) / 180.0f);
            startPos.x = endPos.x + (2.0f * distance *
                sin((M_PI * angle) / 180.0f));
            startPos.z = endPos.z + (distance *
                cos((M_PI * angle) / 180.0f));
            startPos.y = endPos.y + (650.0f * (1.0f - time));
            mbObjPosSet(modelId, startPos.x, startPos.y, startPos.z);
            mbObjRotSet(modelId, 0.0f, 180.0f + angle, 0.0f);
            playerPos.x = startPos.x + (50.0f *
                sin((M_PI * (180.0f + angle)) / 180.0f));
            playerPos.y = startPos.y - 100.0f;
            playerPos.z = startPos.z + (50.0f *
                cos((M_PI * (180.0f + angle)) / 180.0f));
            Hu3DModelObjMtxGet(
                mbObjModelIDGet(modelId), "itemhook_C", hookMtx);
            hookMtx[1][3] += hookYOfs;
            hookMtx[0][3] += radius *
                sin((M_PI * (180.0f + angle)) / 180.0f);
            hookMtx[2][3] += radius *
                cos((M_PI * (180.0f + angle)) / 180.0f);
            tempVec.x = hookMtx[0][3];
            tempVec.y = hookMtx[1][3];
            tempVec.z = hookMtx[2][3];
            mbPlayerPosSetV(work->playerNo, &tempVec);
            mbPlayerRotSet(work->playerNo, rotX, 180.0f + angle, 0.0f);
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(work->playerNo, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        mbMasuPosGet(starMasuId, &playerPos);
        for (i = 0; i < 8; i++) {
            time = 1.0f - (i / 7.0f);
            Hu3DModelObjMtxGet(
                mbObjModelIDGet(modelId), "itemhook_C", hookMtx);
            hookMtx[1][3] += hookYOfs;
            hookMtx[0][3] += radius *
                sin((M_PI * (180.0f + angle)) / 180.0f);
            hookMtx[2][3] += radius *
                cos((M_PI * (180.0f + angle)) / 180.0f);
            tempVec.x = playerPos.x +
                (time * (hookMtx[0][3] - playerPos.x));
            tempVec.y = playerPos.y +
                (time * (hookMtx[1][3] - playerPos.y));
            tempVec.z = playerPos.z +
                (time * (hookMtx[2][3] - playerPos.z));
            mbPlayerPosSetV(work->playerNo, &tempVec);
            mbPlayerRotSet(work->playerNo, time * rotX,
                180.0f + angle, 0.0f);
            HuPrcVSleep();
        }
        mbPlayerPosSet(work->playerNo, playerPos.x, playerPos.y,
            playerPos.z);
        mbPlayerRotSet(work->playerNo, 0.0f, 180.0f + angle, 0.0f);
        mbPlayerColSnapPlayerSet(work->playerNo, TRUE);
        mbObjMotionShiftSet(modelId, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        for (i = 0; i < 30.0f; i++) {
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(work->playerNo, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
        mbev_CapBubbleHookCall(3, modelId, TRUE, FALSE, FALSE);
        mbWinCreate(2, CAPMOVE_MESS_HANACHAN_STAR,
            HUWIN_SPEAKER_HANACHAN_STAR);
        mbWinTopWait();
        mbPlayerPosGet(work->playerNo, &playerPos);
        endPos = playerPos;
        distance = fabs(playerPos.z - startPos.z);
        ease = 0.0f;
        for (i = 0; i < 120.0f; i++) {
            time = i / 120.0f;
            angle = 180.0f + (360.0f * time);
            if (time < 0.2f) {
                ease = 5.0f * time;
            }
            if (ease > 1.0f) {
                ease = 1.0f;
            }
            startPos.x = endPos.x + ((distance +
                (2.0f * (3.0f * (100.0f * time)))) *
                sin((M_PI * angle) / 180.0f));
            startPos.z = endPos.z + ((distance +
                (3.0f * (100.0f * time))) *
                cos((M_PI * angle) / 180.0f));
            startPos.y = endPos.y + (time * 650.0f);
            mbObjPosSet(modelId, startPos.x, startPos.y, startPos.z);
            mbObjRotSet(modelId, 0.0f,
                (180.0f + angle) - (90.0f * ease), 0.0f);
            HuPrcVSleep();
        }
        mbObjDispSet(modelId, FALSE);
        if (!mbExitCheck()) {
            obj->work[0]++;
        }
        if (soundId != -1) {
            mbAudFXStop(soundId);
        }
        mbPlayerRotateStart(work->playerNo, 0, 15);
        while (!mbPlayerRotateCheck(work->playerNo)) {
            HuPrcVSleep();
        }
        mbCameraMoveWait();
        mbev_CapPlayerMotShiftWait(
            work->playerNo, 1, HU3D_MOTATTR_LOOP, TRUE);
        mbCameraPlayerViewSet(work->playerNo, 0);
        mbCameraMoveWait();
        if (mbPlayerStarGet(work->playerNo) >= 999) {
            if (!GwSystem.curTime) {
                mbWinCreate(0, CAPMOVE_MESS_STAR_MAX_DAY,
                    mbGuideSpeakerNoGet());
            } else {
                mbWinCreate(0, CAPMOVE_MESS_STAR_MAX_NIGHT,
                    mbGuideSpeakerNoGet());
            }
            mbWinTopWait();
        } else {
            mbev_StarMasu(work->playerNo);
        }
    }
    HuPrcEnd();
}

void mbev_CapHanachanKill(void)
{
}

static void ev_CapHanachanOMExec(OMOBJ *obj)
{
    CAPWORK *work = obj->data;
    HuVecF pos;
    HuVecF rot;
    HuVecF offset;
    HuVecF vel;
    GXColor color;
    HuVecF posArg;
    HuVecF velArg;
    GXColor colorArg;
    int particleNo;
    GXColor *colorP;
    HuVecF *velP;
    HuVecF *posP;
    Mtx matrix;

    if (mbExitCheck()) {
        omDelObjEx(mbObjMan, obj);
        return;
    }
    if ((int)obj->work[0] != 0) {
        omDelObjEx(mbObjMan, obj);
        return;
    } else if (++obj->work[1] & 1) {
        mbObjPosGet(work->_unkB6C, &pos);
        mbObjRotGet(work->_unkB6C, &rot);
        pos.y += 50.0f;
        offset.x = 100.0f * (1.5f * (-0.5f + MBCapsuleEffRandF()));
        offset.y = 100.0f * (-0.5f + MBCapsuleEffRandF());
        offset.z = -100.0f * (1.0f +
            (0.5f * MBCapsuleEffRandF()));
        mtxRot(matrix, rot.x, rot.y, rot.z);
        PSMTXMultVec(matrix, &offset, &offset);
        PSVECAdd(&pos, &offset, &pos);
        vel.x = 0.0f;
        vel.y = 0.0f;
        vel.z = 0.0f;
        mbev_CapEffColorSet(&color, mbRandMod(CAPMOVE_EFFECT_COLOR_RANGE));
        colorArg = color;
        colorP = &colorArg;
        velArg = vel;
        velP = &velArg;
        posArg = pos;
        posP = &posArg;
        particleNo = mbev_CapEffGlowAdd(work->glowObj, posP, velP,
            (int)(60.0f * (1.0f + (0.5f * MBCapsuleEffRandF()))),
            100.0f * (0.15f + (0.05f * MBCapsuleEffRandF())),
            0.05f + (0.02f * MBCapsuleEffRandF()), 0.08166666f,
            colorP);
        mbev_CapEffGlowKinokoTimeSet(
            work->glowObj, particleNo, 1, 90);
    }
}

void mbev_CapNKinoko(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    HuVecF playerPosNext;
    HuVecF playerPos;
    int modelId;
    int i;
    float time;
    float scale;
    float radius;
    float yOfs;

    mbev_CapWait(work);
    work->explodeObj = mbev_CapEffExplodeCreate();
    work->glowObj = mbev_CapEffGlowCreate();
    mbev_CapEffGlowBlendModeSet(work->glowObj, 1);
    mbev_CapEffGlowPatSet(work->glowObj, 0);
    mbPlayerPosGet(work->playerNo, &playerPos);
    playerPos.y += 250.0f;
    modelId = mbev_CapObjCreate(&work->objWork, CAPMOVE_DATA_N_KINOKO, NULL, FALSE, 0, FALSE);
    mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
    mbObjScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    mbObjDispSet(modelId, FALSE);
    mbCapEffUseCreate(work->playerNo, work->capsuleNo);
    while (mbCapEffUseModeGet(work->playerNo) < 2) {
        HuPrcVSleep();
    }
    mbObjDispSet(modelId, TRUE);
    work->_unkB6C = modelId;
    ev_CapEffKinokoCreate(work);
    for (i = 0; i < 30.0f; i++) {
        time = i / 30.0f;
        scale = sin((M_PI * (180.0f * time)) / 180.0f) + 1.0f;
        mbObjScaleSet(modelId, scale, scale, scale);
        HuPrcVSleep();
    }
    mbev_CapRandomBonusCoin(work->playerNo, work->capsuleNo, FALSE);
    for (i = 0; i < 60.0f || !mbev_CapBonusCoinCheck(work->playerNo); i++) {
        time = i / 60.0f;
        mbPlayerPosGet(work->playerNo, &playerPos);
        playerPos.y += (100.0f * sin((M_PI * (360.0f * time)) / 180.0f)) * 0.1f + 250.0f;
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        HuPrcVSleep();
    }
    for (i = 1; i <= 10; i++) {
        time = i / 10.0f;
        mbPlayerPosGet(work->playerNo, &playerPosNext);
        playerPos.y += time * ((playerPosNext.y + 250.0f) - playerPos.y);
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        HuPrcVSleep();
    }
    mbAudFXPlay(MSM_SE_BRD00_37);
    for (i = 0; i < 90.0f; i++) {
        time = 1.0f - (i / 90.0f);
        yOfs = 50.0f + (time * time * 200.0f);
        radius = sin((M_PI * (180.0f * (time * time))) / 180.0f);
        mbPlayerPosGet(work->playerNo, &playerPos);
        playerPos.x += (radius * cos((M_PI * (2.0f * (time * 360.0f))) / 180.0f)) * 100.0f * 1.5f;
        playerPos.z += (radius * sin((M_PI * (2.0f * (time * 360.0f))) / 180.0f)) * 100.0f * 1.5f;
        playerPos.y += yOfs;
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        mbObjScaleSet(modelId, time, time, time);
        HuPrcVSleep();
    }
    GwPlayer[work->playerNo].diceMode = 6;
    omVibrate(work->playerNo, 20, 4, 4);
    for (i = 0; i < 6.0f; i++) {
        HuPrcVSleep();
    }
    while (mbev_CapEffGlowDispGet(work->glowObj) > 0) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

void mbev_CapNKinokoKill(void)
{
}

void mbev_CapKillerMove(void)
{
    CAPWORK *work;
    HuVecF path[6];
    HuVecF playerPos;
    HuVecF playerRot;
    HuVecF killerPos;
    HuVecF killerRot;
    HuVecF targetRot;
    HuVecF offset;
    HuVecF direction;
    HuVecF velocity;
    HuVecF landingPos;
    GXColor color;
    s16 motionStart;
    s16 motionRide;
    s16 masuId;
    s16 nextMasuId;
    int modelId;
    int readStat;
    int coinNum;
    int moveNum;
    int stepNum;
    int i;
    int j;
    float t;

    work = HuPrcCurrentGet()->property;
    readStat = mbBGRead(CAPMOVE_DATA_KILLER);
    if (readStat != HU_DATA_STAT_NONE) {
        mbBGReadWait(readStat);
    }
    work->explodeObj = mbev_CapEffExhaustCreate();
    work->boostObj = mbev_CapEffBoostCreate();
    mbev_CapEffBoostBlendModeSet(work->boostObj, 1);
    mbev_CapPlayerMoveObjInit();

    mbPlayerPosGet(work->playerNo, &playerPos);
    mbPlayerRotGet(work->playerNo, &playerRot);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        masuId = GwPlayer[work->playerNo].masuId;
        mbMasuPosGet(masuId, &path[i]);
        path[i].y += 100.0f;
    }
    masuId = mbev_CapMasuLinkNextRandomGet(GwPlayer[work->playerNo].masuId,
        &path[4]);
    path[4].y += 250.0f;
    nextMasuId = mbev_CapMasuLinkNextRandomGet(masuId, &path[5]);
    path[5].y += 250.0f;

    modelId = mbev_CapObjCreate(&work->objWork, CAPMOVE_DATA_KILLER, NULL,
        FALSE, 5, FALSE);
    mbObjDispSet(modelId, FALSE);
    motionStart = mbev_CapPlayerMotionCreate(&work->objWork, work->playerNo,
        CAPMOVE_DATA_KILLER_RIDE_START);
    motionRide = mbev_CapPlayerMotionCreate(&work->objWork, work->playerNo,
        CAPMOVE_DATA_KILLER_RIDE);
    moveNum = mbDiceResultGet(work->playerNo);
    mbev_CapStatusDispSetAll(TRUE, FALSE);

    mbev_CapHermiteGetV(0.0f, &path[0], &path[1], &path[2], &path[3],
        &killerPos);
    mbev_CapHermiteGetV(1.0f, &path[0], &path[1], &path[2], &path[3],
        &landingPos);
    PSVECSubtract(&landingPos, &killerPos, &direction);
    PSVECNormalize(&direction, &direction);
    offset = direction;
    offset.y = 0.0f;
    killerRot.x = -atan2(direction.y, PSVECMag(&offset)) * 180.0f / M_PI;
    killerRot.y = atan2(direction.x, direction.z) * 180.0f / M_PI;
    killerRot.z = 0.0f;
    mbObjPosSetV(modelId, &killerPos);
    mbObjRotSetV(modelId, &killerRot);
    mbObjDispSet(modelId, TRUE);
    mbPlayerMotionShiftSet(work->playerNo, motionStart, 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);

    for (i = 0; i < 30; i++) {
        t = (float)i / 30.0f;
        mbev_CapHermiteGetV(t, &path[0], &path[1], &path[2], &path[3],
            &killerPos);
        killerRot.x = mbev_CapAngleSumLerp(t, killerRot.x, 0.0f);
        killerRot.y = mbev_CapAngleSumLerp(t, killerRot.y, playerRot.y);
        mbObjPosSetV(modelId, &killerPos);
        mbObjRotSetV(modelId, &killerRot);
        offset.x = 0.0f;
        offset.y = 25.0f;
        offset.z = -50.0f;
        mbPlayerPosSetV(work->playerNo, &killerPos);
        mbPlayerRotSetV(work->playerNo, &killerRot);
        ev_CapEffKillerDustCreate(work, &killerPos, &killerRot);
        HuPrcVSleep();
    }

    mbPlayerMotionShiftSet(work->playerNo, motionRide, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    for (i = 0; i < moveNum; i++) {
        masuId = GwPlayer[work->playerNo].masuId;
        nextMasuId = mbev_CapMasuLinkNextGet(masuId, NULL);
        if (nextMasuId <= 0) {
            break;
        }
        mbMasuPosGet(nextMasuId, &landingPos);
        landingPos.y += 100.0f;
        PSVECSubtract(&landingPos, &killerPos, &direction);
        stepNum = (int)(PSVECMag(&direction) / 20.0f);
        if (stepNum < 1) {
            stepNum = 1;
        }
        for (j = 0; j < stepNum; j++) {
            t = (float)j / (float)stepNum;
            mbev_CapHermiteGetV(t, &path[2], &path[3], &path[4],
                &landingPos, &killerPos);
            PSVECSubtract(&landingPos, &killerPos, &direction);
            PSVECNormalize(&direction, &direction);
            offset = direction;
            offset.y = 0.0f;
            targetRot.x = -atan2(direction.y, PSVECMag(&offset)) * 180.0f
                / M_PI;
            targetRot.y = atan2(direction.x, direction.z) * 180.0f / M_PI;
            killerRot.x = mbev_CapAngleSumLerp(0.5f, killerRot.x,
                targetRot.x);
            killerRot.y = mbev_CapAngleSumLerp(0.5f, killerRot.y,
                targetRot.y);
            mbObjPosSetV(modelId, &killerPos);
            mbObjRotSetV(modelId, &killerRot);
            mbObjScaleSet(modelId, 1.0f, 1.0f, 1.0f);
            mbPlayerPosSetV(work->playerNo, &killerPos);
            mbPlayerRotSetV(work->playerNo, &killerRot);
            ev_CapEffKillerDustCreate(work, &killerPos, &killerRot);
            ev_CapEffKillerBoostCreate(work, &killerPos, &killerRot);
            HuPrcVSleep();
        }
        if (mbMasuTypeGet(nextMasuId) != CAPMOVE_MASU_TYPE_NONE) {
            HuAudFXPlay(MSM_SE_BRD00_02);
        }
        for (j = 0; j < GW_PLAYER_MAX; j++) {
            if (j == work->playerNo || GwPlayer[j].masuId != nextMasuId) {
                continue;
            }
            mbAudFXPlay(MSM_SE_BRD00_55);
            mbev_CapPlayerMoveHitCreate(j, TRUE, TRUE);
            CharFXPlay(GwPlayer[j].charNo, CHARVOICEID(9));
            omVibrate(j, 12, 4, 2);
            coinNum = 20;
            if (mbPlayerCoinGet(j) < coinNum) {
                coinNum = mbPlayerCoinGet(j);
            }
            if (coinNum > 0) {
                mbPlayerPosGet(j, &playerPos);
                playerPos.y += 250.0f;
                mbCoinDispCapsuleCreate(&playerPos, -coinNum);
                mbCoinAddDispExec(j, -coinNum, FALSE, TRUE);
                mbCoinAddDispExec(work->playerNo, coinNum, FALSE, TRUE);
            }
        }
        GwPlayer[work->playerNo].masuIdPrev = masuId;
        GwPlayer[work->playerNo].masuId = nextMasuId;
        (void)mbMasuTypeGet(nextMasuId);
        (void)mbMasuAttrGet(nextMasuId);
        if (mbMasuDispCheck(nextMasuId)) {
            moveNum--;
        }
    }

    mbMoveNumKill(work->playerNo);
    mbObjDispSet(modelId, FALSE);
    mbev_CapEffDustCloudAdd(work->explodeObj, &killerPos);
    mbPlayerMotionShiftSet(work->playerNo, CAPMOVE_PLAYER_MOT_JUMP, 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);
    mbMasuPosGet(GwPlayer[work->playerNo].masuId, &landingPos);
    mbPlayerPosGet(work->playerNo, &playerPos);
    for (i = 0; i < 30; i++) {
        t = (float)i / 30.0f;
        PSVECSubtract(&landingPos, &playerPos, &offset);
        PSVECScale(&offset, &offset, t);
        PSVECAdd(&playerPos, &offset, &offset);
        offset.y += sin((t * 180.0f * M_PI) / 180.0f) * 100.0f;
        mbPlayerPosSetV(work->playerNo, &offset);
        HuPrcVSleep();
    }
    mbPlayerPosSetV(work->playerNo, &landingPos);
    mbPlayerMotIdleSet(work->playerNo);
    mbPlayerColSnapPlayerSet(work->playerNo, TRUE);
    mbCameraMoveWait();
    if (mbev_CapPlayerMoveObjCheck()) {
        mbev_CapPlayerIdleWait(work->playerNo);
    }
    mbev_PlayerColMasuSet(work->playerNo,
        GwPlayer[work->playerNo].masuId, TRUE);
    HuPrcEnd();
}

void mbev_CapKillerMoveKill(void)
{
}

static void ev_CapEffKillerDustCreate(CAPWORK *work, HuVecF *pos,
    HuVecF *rot)
{
    Mtx mtx;
    HuVecF effectPos;
    HuVecF velocity;
    HuVecF effectPosArg;
    HuVecF velocityArg;
    GXColor colorTemp;
    GXColor color;
    GXColor *colorP;
    HuVecF *velocityP;
    HuVecF *effectPosP;
    float colorValue;
    float radius;

    mtxRot(mtx, rot->x, rot->y, rot->z);
    effectPos.x = 0.0f;
    effectPos.y = 0.0f;
    effectPos.z = -180.0f;
    PSMTXMultVec(mtx, &effectPos, &effectPos);
    PSVECAdd(pos, &effectPos, &effectPos);
    radius = (0.5f + (MBCapsuleEffRandF() * 0.5f)) * 10.0f;
    velocity.x = radius * sin(((180.0f + rot->y) * M_PI) / 180.0f)
        * cos((rot->x * M_PI) / 180.0f);
    velocity.y = radius * sin((rot->x * M_PI) / 180.0f);
    velocity.z = radius * cos(((180.0f + rot->y) * M_PI) / 180.0f)
        * cos((rot->x * M_PI) / 180.0f);
    colorValue = MBCapsuleEffRandF();
    colorTemp.r = (u8)(32.0f + (32.0f * colorValue));
    colorTemp.g = (u8)(32.0f + (32.0f * colorValue));
    colorTemp.b = (u8)(32.0f + (32.0f * colorValue));
    colorTemp.a = (u8)(192.0f + (63.0f * MBCapsuleEffRandF()));
    color = colorTemp;
    colorP = &color;
    velocityArg = velocity;
    velocityP = &velocityArg;
    effectPosArg = effectPos;
    effectPosP = &effectPosArg;
    mbev_CapEffExplodeKillerAdd(work->explodeObj, effectPosP,
        velocityP,
        100.0f * (1.0f + (0.5f * MBCapsuleEffRandF())),
        -0.5f + MBCapsuleEffRandF(),
        100.0f * (0.25f + (0.25f * MBCapsuleEffRandF())),
        0.5f + (0.25f * MBCapsuleEffRandF()), colorP);
}

static void ev_CapEffKillerExplodeCreate(CAPWORK *work, HuVecF *pos,
    HuVecF *rot, int count)
{
    Mtx mtx;
    HuVecF effectPos;
    HuVecF velocity;
    HuVecF effectPosArg;
    HuVecF velocityArg;
    GXColor colorTemp;
    GXColor color;
    GXColor *colorP;
    HuVecF *velocityP;
    HuVecF *effectPosP;
    float scale;
    int angle;
    int colorValue;
    int i;

    mtxRot(mtx, rot->x, rot->y, rot->z);
    for (i = 0; i < count; i++) {
        effectPos.x = pos->x;
        effectPos.y = pos->y;
        effectPos.z = pos->z;
        angle = (360.0f / count) * i;
        scale = 0.9f + (0.1f * MBCapsuleEffRandF());
        velocity.x = 0.075f * (100.0f *
            sin((angle * M_PI) / 180.0f)) * scale;
        scale = 0.9f + (0.1f * MBCapsuleEffRandF());
        velocity.y = 0.075f * (100.0f *
            cos((angle * M_PI) / 180.0f)) * scale;
        velocity.z = 0.0f;
        PSMTXMultVec(mtx, &velocity, &velocity);
        colorValue = 63.0f * MBCapsuleEffRandF();
        colorTemp.r = colorValue + 32;
        colorTemp.g = colorValue + 32;
        colorTemp.b = colorValue + 32;
        colorTemp.a = (u8)(192.0f + (63.0f * MBCapsuleEffRandF()));
        color = colorTemp;
        colorP = &color;
        velocityArg = velocity;
        velocityP = &velocityArg;
        effectPosArg = effectPos;
        effectPosP = &effectPosArg;
        mbev_CapEffExplodeAdd(work->explodeObj, effectPosP,
            velocityP, 200.0f, 0.1f, 0.33f, colorP);
    }
}

static void ev_CapEffKillerBoostCreate(CAPWORK *work, HuVecF *pos,
    HuVecF *rot)
{
    HuVecF effectPos;
    HuVecF velocity;
    HuVecF effectRot;
    HuVecF effectPosArg;
    HuVecF velocityArg;
    GXColor colorTemp;
    GXColor color;
    GXColor *colorP;
    HuVecF *velocityP;
    HuVecF *effectPosP;
    float speed;
    int time;

    effectPos.x = pos->x + (50.0f * (-0.5f + MBCapsuleEffRandF()));
    effectPos.y = pos->y + (50.0f * (-0.5f + MBCapsuleEffRandF()));
    effectPos.z = pos->z + (50.0f * (-0.5f + MBCapsuleEffRandF()));
    speed = 30.000002f * (1.0f + (0.5f * MBCapsuleEffRandF()));
    effectRot.x = rot->x + (50.0f * (-0.5f + MBCapsuleEffRandF()));
    effectRot.y = rot->y + (50.0f * (-0.5f + MBCapsuleEffRandF()));
    effectRot.z = rot->z;
    velocity.x = speed * sin(((180.0f + effectRot.y) * M_PI) / 180.0f)
        * cos((effectRot.x * M_PI) / 180.0f);
    velocity.y = speed * sin((effectRot.x * M_PI) / 180.0f);
    velocity.z = speed * cos(((180.0f + effectRot.y) * M_PI) / 180.0f)
        * cos((effectRot.x * M_PI) / 180.0f);
    colorTemp.r = (u8)(64.0f + (192.0f * MBCapsuleEffRandF()));
    colorTemp.g = (u8)(64.0f + (192.0f * MBCapsuleEffRandF()));
    colorTemp.b = (u8)(64.0f + (192.0f * MBCapsuleEffRandF()));
    colorTemp.a = (u8)(128.0f + (64.0f * MBCapsuleEffRandF()));
    color = colorTemp;
    colorP = &color;
    velocityArg = velocity;
    velocityP = &velocityArg;
    effectPosArg = effectPos;
    effectPosP = &effectPosArg;
    time = (int)(60.0f * (1.0f + (0.5f * MBCapsuleEffRandF())));
    mbev_CapEffBoostAdd(work->boostObj, effectPosP, velocityP,
        50.0f * (1.0f + (0.5f * MBCapsuleEffRandF())),
        2.0f * (-0.5f + MBCapsuleEffRandF()),
        time, colorP);
}

static void ev_CapEffKinokoCreate(CAPWORK *work)
{
    OMOBJ *obj;

    obj = omAddObjEx(mbObjMan, -32768, 0, 0, -1, ev_CapEffKinokoOMExec);
    obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPWORK), HU_MEMNUM_OVL);
    memcpy(obj->data, work, sizeof(CAPWORK));
}

static void ev_CapEffKinokoOMExec(OMOBJ *obj)
{
    CAPWORK *work = obj->data;
    HuVecF pos;
    HuVecF scale;
    GXColor color;
    int particleNo;

    if (mbExitCheck() || obj->stat) {
        omDelObjEx(mbObjMan, obj);
        return;
    }
    mbObjPosGet(work->_unkB6C, &pos);
    mbObjScaleGet(work->_unkB6C, &scale);
    if (scale.x <= 0.0f) {
        return;
    }
    switch (work->capsuleNo) {
        case 0:
            color.r = 192;
            color.g = 192;
            color.b = 192;
            color.a = 255;
            break;
        case 1:
            color.r = 255;
            color.g = 255;
            color.b = 127;
            color.a = 255;
            break;
        case 2:
            color.r = 127;
            color.g = 255;
            color.b = 127;
            color.a = 255;
            break;
        case 3:
            color.r = 192;
            color.g = 192;
            color.b = 255;
            color.a = 255;
            break;
        default:
            mbev_CapEffColorSet(&color,
                (int)(MBCapsuleEffRandF() * 32768.0f));
            break;
    }
    particleNo = mbev_CapEffGlowKinokoAdd(work->glowObj, &pos, 30,
        scale.x, 40.0f, 40.0f, 40.0f, work->capsuleNo, &color);
    if (particleNo >= 0) {
        mbev_CapEffGlowKinokoTimeSet(work->glowObj, particleNo, 30, 0);
    }
}
