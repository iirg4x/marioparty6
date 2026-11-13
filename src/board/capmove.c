// Board capsule handlers for player movement, character encounters, and mushroom effects.
#include "dolphin/math.h"
#include "dolphin/mtx.h"
#include "datanum/charmot.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/board/camera.h"
#include "game/board/capevent.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/process.h"
#include "game/board/audio.h"
#include "game/board/capsule.h"
#include "game/board/coin.h"
#include "game/board/main.h"
#include "game/board/player.h"
#include "game/board/roulette.h"
#include "game/board/window.h"
#include "datadir_enum.h"
#include "msm_se.h"
#include "messdir_enum.h"
#include "game/board/dice.h"
#include "humath.h"

typedef struct EvCapWork {
    int motId[64][4];
    int objId[64];
    int sprId[64];
    void *mem[64];
    int masuId[64];
    HuVecF objPos[64];
    int playerMasuId[4];
    HuVecF playerPos[4];
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
    int effectModelId;
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

#define CAPMOVE_DATA_KINOKO DATANUM(0xc0000, 68)
#define CAPMOVE_DATA_S_KINOKO DATANUM(0xc0000, 69)
#define CAPMOVE_DATA_N_KINOKO DATANUM(0xc0000, 70)
#define CAPMOVE_DATA_P_KINOKO DATANUM(0xc0000, 71)
#define CAPMOVE_DATA_M_KINOKO DATANUM(0xc0000, 72)
#define CAPMOVE_DATA_KILLER DATANUM(0xd0000, 0)
#define CAPMOVE_DATA_KILLER_RIDE_START DATANUM(0x930000, 67)
#define CAPMOVE_DATA_KILLER_RIDE DATANUM(0x930000, 66)

enum {
    CAPMOVE_DATA_HANACHAN = DATANUM(0xd0000, 1),
    CAPMOVE_DATA_HANACHAN_MOTION_A = DATANUM(0xd0000, 2),
    CAPMOVE_DATA_HANACHAN_MOTION_B = DATANUM(0xd0000, 3),
    CAPMOVE_DATA_DOKAN = DATANUM(0xd0000, 4),
    CAPMOVE_DATA_CAMERA_TARGET = DATANUM(0xc0000, 68),
    CAPMOVE_MASU_TYPE_NONE = 0,
    CAPMOVE_MASU_TYPE_STAR = 7,
    CAPMOVE_PLAYER_MOT_JUMP = 4,
    CAPMOVE_EFFECT_COLOR_RANGE = 1 << 15,
};

#define CAPMOVE_MESS_DOKAN_SAME_SPACE MESSNUM(MESS_CAPSULE_EX01, 0)
#define CAPMOVE_MESS_DOKAN_TARGET MESSNUM(MESS_CAPSULE_EX01, 1)
#define CAPMOVE_MESS_HANACHAN_ARRIVE MESSNUM(MESS_CAPSULE_EX01, 3)
#define CAPMOVE_MESS_HANACHAN_DEPART MESSNUM(MESS_CAPSULE_EX01, 4)
#define CAPMOVE_MESS_HANACHAN_STAR MESSNUM(MESS_CAPSULE_EX01, 5)
#define CAPMOVE_MESS_STAR_MAX_DAY MESSNUM(MESS_BOARD_STAR, 10)
#define CAPMOVE_MESS_STAR_MAX_NIGHT MESSNUM(MESS_BOARD_STAR, 11)

static HuVecF capsuleCameraOfs = { 0.0f, 100.0f, 0.0f };
static HuVecF hanachanPlayerOfs[14] = {
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

extern OMOBJMAN *mbObjMan;
void mbev_StarMasu(int playerNo, int masuId);
#define CAPMOVE_EFFECT_MIN_SCALE 0.2f

static void ev_CapEffKinokoCreate(CAPWORK *work);
static void ev_CapEffKinokoOMExec(OMOBJ *obj);
static void ev_CapHanachanOMExec(OMOBJ *obj);
static void ev_CapEffKillerDustCreate(CAPWORK *work, HuVecF *pos,
    HuVecF *rot);
static void ev_CapEffKillerExplodeCreate(CAPWORK *work, HuVecF *pos,
    HuVecF *rot, int count);
static void ev_CapEffKillerBoostCreate(CAPWORK *work, HuVecF *pos,
    HuVecF *rot);

// Runs from the capsule event table when the standard Mushroom capsule is used; shows its effect
// and sets the resulting dice mode.
void mbev_CapKinoko(void)
{
    CAPWORK *capWork = fn_8000DD9C()->property;
    HuVecF currentPlayerPos;
    HuVecF mushroomPos;
    int mushroomModelId;
    int frameIndex;
    float animProgress;
    float modelScale;
    float orbitFactor;
    float heightOffset;

    mbev_CapWait(capWork);
    capWork->explodeObj = mbev_CapEffExplodeCreate();
    capWork->glowObj = mbev_CapEffGlowCreate();
    mbev_CapEffGlowBlendModeSet(capWork->glowObj, 1);
    mbPlayerPosGet(capWork->playerNo, &mushroomPos);
    mushroomPos.y += 250.0f;
    mushroomModelId =
        mbev_CapObjCreate(&capWork->objWork, CAPMOVE_DATA_KINOKO, NULL, FALSE, 0, FALSE);
    mbObjPosSet(mushroomModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
    mbObjScaleSet(mushroomModelId, 1.0f, 1.0f, 1.0f);
    mbObjDispSet(mushroomModelId, FALSE);
    mbCapEffUseCreate(capWork->playerNo, capWork->capsuleNo);
    while (mbCapEffUseModeGet(capWork->playerNo) < 2) {
        HuPrcVSleep();
    }
    mbObjDispSet(mushroomModelId, TRUE);
    capWork->effectModelId = mushroomModelId;
    ev_CapEffKinokoCreate(capWork);
    for (frameIndex = 0; frameIndex < 30.0f; frameIndex++) {
        animProgress = frameIndex / 30.0f;
        modelScale = sin((M_PI * (180.0f * animProgress)) / 180.0f) + 1.0f;
        mbObjScaleSet(mushroomModelId, modelScale, modelScale, modelScale);
        HuPrcVSleep();
    }
    // Keep the model hovering for at least 60 frames and until the bonus-coin effect finishes.
    mbev_CapRandomBonusCoin(capWork->playerNo, capWork->capsuleNo, FALSE);
    for (frameIndex = 0; frameIndex < 60.0f || !mbev_CapBonusCoinCheck(capWork->playerNo);
         frameIndex++) {
        animProgress = frameIndex / 60.0f;
        mbPlayerPosGet(capWork->playerNo, &mushroomPos);
        mushroomPos.y += (100.0f * sin((M_PI * (360.0f * animProgress)) / 180.0f))
            * 0.1f + 250.0f;
        mbObjPosSet(mushroomModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
        HuPrcVSleep();
    }
    // X and Z remain at the earlier position while the model height eases to the current player
    // height.
    for (frameIndex = 1; frameIndex <= 10; frameIndex++) {
        animProgress = frameIndex / 10.0f;
        mbPlayerPosGet(capWork->playerNo, &currentPlayerPos);
        mushroomPos.y += animProgress * ((currentPlayerPos.y + 250.0f) - mushroomPos.y);
        mbObjPosSet(mushroomModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
        HuPrcVSleep();
    }
    // Shrink and orbit the model around the player before applying this capsule's dice mode.
    mbAudFXPlay(MSM_SE_BRD00_MUSHROOM_ORBIT_SHRINK);
    for (frameIndex = 0; frameIndex < 90.0f; frameIndex++) {
        animProgress = 1.0f - (frameIndex / 90.0f);
        heightOffset = 50.0f + (animProgress * animProgress * 200.0f);
        orbitFactor = sin((M_PI * (180.0f * (animProgress * animProgress))) / 180.0f);
        mbPlayerPosGet(capWork->playerNo, &mushroomPos);
        mushroomPos.x +=
            (orbitFactor * cos((M_PI * (2.0f * (animProgress * 360.0f))) / 180.0f))
                * 100.0f * 1.5f;
        mushroomPos.z +=
            (orbitFactor * sin((M_PI * (2.0f * (animProgress * 360.0f))) / 180.0f))
                * 100.0f * 1.5f;
        mushroomPos.y += heightOffset;
        mbObjPosSet(mushroomModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
        mbObjScaleSet(mushroomModelId, animProgress, animProgress, animProgress);
        HuPrcVSleep();
    }
    lbl_80265750[capWork->playerNo].diceMode = 1;
    fn_8006A030(capWork->playerNo, 20, 4, 4);
    for (frameIndex = 0; frameIndex < 6.0f; frameIndex++) {
        HuPrcVSleep();
    }
    while (mbev_CapEffGlowDispGet(capWork->glowObj) > 0) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

void mbev_CapKinokoKill(void)
{
}

// Runs from the capsule event table when the S Mushroom capsule is used; shows its effect and sets
// the resulting dice mode.
void mbev_CapSKinoko(void)
{
    CAPWORK *capWork = fn_8000DD9C()->property;
    HuVecF currentPlayerPos;
    HuVecF mushroomPos;
    int mushroomModelId;
    int frameIndex;
    float animProgress;
    float modelScale;
    float orbitFactor;
    float heightOffset;

    mbev_CapWait(capWork);
    capWork->explodeObj = mbev_CapEffExplodeCreate();
    capWork->glowObj = mbev_CapEffGlowCreate();
    mbev_CapEffGlowBlendModeSet(capWork->glowObj, 1);
    mbPlayerPosGet(capWork->playerNo, &mushroomPos);
    mushroomPos.y += 250.0f;
    mushroomModelId =
        mbev_CapObjCreate(&capWork->objWork, CAPMOVE_DATA_S_KINOKO, NULL, FALSE, 0, FALSE);
    mbObjPosSet(mushroomModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
    mbObjScaleSet(mushroomModelId, 1.0f, 1.0f, 1.0f);
    mbObjDispSet(mushroomModelId, FALSE);
    mbCapEffUseCreate(capWork->playerNo, capWork->capsuleNo);
    while (mbCapEffUseModeGet(capWork->playerNo) < 2) {
        HuPrcVSleep();
    }
    mbObjDispSet(mushroomModelId, TRUE);
    capWork->effectModelId = mushroomModelId;
    ev_CapEffKinokoCreate(capWork);
    for (frameIndex = 0; frameIndex < 30.0f; frameIndex++) {
        animProgress = frameIndex / 30.0f;
        modelScale = sin((M_PI * (180.0f * animProgress)) / 180.0f) + 1.0f;
        mbObjScaleSet(mushroomModelId, modelScale, modelScale, modelScale);
        HuPrcVSleep();
    }
    // Keep the model hovering for at least 60 frames and until the bonus-coin effect finishes.
    mbev_CapRandomBonusCoin(capWork->playerNo, capWork->capsuleNo, FALSE);
    for (frameIndex = 0; frameIndex < 60.0f || !mbev_CapBonusCoinCheck(capWork->playerNo);
         frameIndex++) {
        animProgress = frameIndex / 60.0f;
        mbPlayerPosGet(capWork->playerNo, &mushroomPos);
        mushroomPos.y += (100.0f * sin((M_PI * (360.0f * animProgress)) / 180.0f))
            * 0.1f + 250.0f;
        mbObjPosSet(mushroomModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
        HuPrcVSleep();
    }
    // X and Z remain at the earlier position while the model height eases to the current player
    // height.
    for (frameIndex = 1; frameIndex <= 10; frameIndex++) {
        animProgress = frameIndex / 10.0f;
        mbPlayerPosGet(capWork->playerNo, &currentPlayerPos);
        mushroomPos.y += animProgress * ((currentPlayerPos.y + 250.0f) - mushroomPos.y);
        mbObjPosSet(mushroomModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
        HuPrcVSleep();
    }
    // Shrink and orbit the model around the player before applying this capsule's dice mode.
    mbAudFXPlay(MSM_SE_BRD00_MUSHROOM_ORBIT_SHRINK);
    for (frameIndex = 0; frameIndex < 90.0f; frameIndex++) {
        animProgress = 1.0f - (frameIndex / 90.0f);
        heightOffset = 50.0f + (animProgress * animProgress * 200.0f);
        orbitFactor = sin((M_PI * (180.0f * (animProgress * animProgress))) / 180.0f);
        mbPlayerPosGet(capWork->playerNo, &mushroomPos);
        mushroomPos.x +=
            (orbitFactor * cos((M_PI * (2.0f * (animProgress * 360.0f))) / 180.0f))
                * 100.0f * 1.5f;
        mushroomPos.z +=
            (orbitFactor * sin((M_PI * (2.0f * (animProgress * 360.0f))) / 180.0f))
                * 100.0f * 1.5f;
        mushroomPos.y += heightOffset;
        mbObjPosSet(mushroomModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
        mbObjScaleSet(mushroomModelId, animProgress, animProgress, animProgress);
        HuPrcVSleep();
    }
    lbl_80265750[capWork->playerNo].diceMode = 2;
    fn_8006A030(capWork->playerNo, 20, 4, 4);
    for (frameIndex = 0; frameIndex < 6.0f; frameIndex++) {
        HuPrcVSleep();
    }
    while (mbev_CapEffGlowDispGet(capWork->glowObj) > 0) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

void mbev_CapSKinokoKill(void)
{
}

// Runs from the capsule event table when the P Mushroom capsule is used; shows its effect and sets
// the resulting dice mode.
void mbev_CapPKinoko(void)
{
    CAPWORK *capWork = fn_8000DD9C()->property;
    HuVecF currentPlayerPos;
    HuVecF mushroomPos;
    int mushroomModelId;
    int frameIndex;
    float animProgress;
    float modelScale;
    float orbitFactor;
    float heightOffset;

    mbev_CapWait(capWork);
    capWork->explodeObj = mbev_CapEffExplodeCreate();
    capWork->glowObj = mbev_CapEffGlowCreate();
    mbev_CapEffGlowBlendModeSet(capWork->glowObj, 1);
    mbPlayerPosGet(capWork->playerNo, &mushroomPos);
    mushroomPos.y += 250.0f;
    mushroomModelId =
        mbev_CapObjCreate(&capWork->objWork, CAPMOVE_DATA_P_KINOKO, NULL, FALSE, 0, FALSE);
    mbObjPosSet(mushroomModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
    mbObjScaleSet(mushroomModelId, 1.0f, 1.0f, 1.0f);
    mbObjDispSet(mushroomModelId, FALSE);
    mbCapEffUseCreate(capWork->playerNo, capWork->capsuleNo);
    while (mbCapEffUseModeGet(capWork->playerNo) < 2) {
        HuPrcVSleep();
    }
    mbObjDispSet(mushroomModelId, TRUE);
    capWork->effectModelId = mushroomModelId;
    ev_CapEffKinokoCreate(capWork);
    for (frameIndex = 0; frameIndex < 30.0f; frameIndex++) {
        animProgress = frameIndex / 30.0f;
        modelScale = sin((M_PI * (180.0f * animProgress)) / 180.0f) + 1.0f;
        mbObjScaleSet(mushroomModelId, modelScale, modelScale, modelScale);
        HuPrcVSleep();
    }
    // Keep the model hovering for at least 60 frames and until the bonus-coin effect finishes.
    mbev_CapRandomBonusCoin(capWork->playerNo, capWork->capsuleNo, FALSE);
    for (frameIndex = 0; frameIndex < 60.0f || !mbev_CapBonusCoinCheck(capWork->playerNo);
         frameIndex++) {
        animProgress = frameIndex / 60.0f;
        mbPlayerPosGet(capWork->playerNo, &mushroomPos);
        mushroomPos.y += (100.0f * sin((M_PI * (360.0f * animProgress)) / 180.0f))
            * 0.1f + 250.0f;
        mbObjPosSet(mushroomModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
        HuPrcVSleep();
    }
    // X and Z remain at the earlier position while the model height eases to the current player
    // height.
    for (frameIndex = 1; frameIndex <= 10; frameIndex++) {
        animProgress = frameIndex / 10.0f;
        mbPlayerPosGet(capWork->playerNo, &currentPlayerPos);
        mushroomPos.y += animProgress * ((currentPlayerPos.y + 250.0f) - mushroomPos.y);
        mbObjPosSet(mushroomModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
        HuPrcVSleep();
    }
    // Shrink and orbit the model around the player before applying this capsule's dice mode.
    mbAudFXPlay(MSM_SE_BRD00_MUSHROOM_ORBIT_SHRINK);
    for (frameIndex = 0; frameIndex < 90.0f; frameIndex++) {
        animProgress = 1.0f - (frameIndex / 90.0f);
        heightOffset = 50.0f + (animProgress * animProgress * 200.0f);
        orbitFactor = sin((M_PI * (180.0f * (animProgress * animProgress))) / 180.0f);
        mbPlayerPosGet(capWork->playerNo, &mushroomPos);
        mushroomPos.x +=
            (orbitFactor * cos((M_PI * (2.0f * (animProgress * 360.0f))) / 180.0f))
                * 100.0f * 1.5f;
        mushroomPos.z +=
            (orbitFactor * sin((M_PI * (2.0f * (animProgress * 360.0f))) / 180.0f))
                * 100.0f * 1.5f;
        mushroomPos.y += heightOffset;
        mbObjPosSet(mushroomModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
        mbObjScaleSet(mushroomModelId, animProgress, animProgress, animProgress);
        HuPrcVSleep();
    }
    lbl_80265750[capWork->playerNo].diceMode = 3;
    fn_8006A030(capWork->playerNo, 20, 4, 4);
    for (frameIndex = 0; frameIndex < 6.0f; frameIndex++) {
        HuPrcVSleep();
    }
    while (mbev_CapEffGlowDispGet(capWork->glowObj) > 0) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

void mbev_CapPKinokoKill(void)
{
}

// Runs from the capsule event table for the M Mushroom; awards bonus coins, turns the player metal,
// and sets dice mode 4.
void mbev_CapMKinoko(void)
{
    CAPWORK *capWork = fn_8000DD9C()->property;
    HuVecF currentPlayerPos;
    HuVecF mushroomPos;
    int effectModelId;
    int frameIndex;
    float animProgress;
    float modelScale;
    float orbitFactor;
    float heightOffset;

    mbev_CapWait(capWork);
    capWork->explodeObj = mbev_CapEffExplodeCreate();
    capWork->glowObj = mbev_CapEffGlowCreate();
    mbev_CapEffGlowBlendModeSet(capWork->glowObj, 1);
    mbPlayerPosGet(capWork->playerNo, &mushroomPos);
    mushroomPos.y += 250.0f;
    effectModelId =
        mbev_CapObjCreate(&capWork->objWork, CAPMOVE_DATA_M_KINOKO, NULL, FALSE, 0, FALSE);
    mbObjPosSet(effectModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
    mbObjScaleSet(effectModelId, 1.0f, 1.0f, 1.0f);
    mbObjDispSet(effectModelId, FALSE);
    mbCapEffUseCreate(capWork->playerNo, capWork->capsuleNo);
    while (mbCapEffUseModeGet(capWork->playerNo) < 2) {
        HuPrcVSleep();
    }
    mbObjDispSet(effectModelId, TRUE);
    capWork->effectModelId = effectModelId;
    ev_CapEffKinokoCreate(capWork);
    for (frameIndex = 0; frameIndex < 30.0f; frameIndex++) {
        animProgress = frameIndex / 30.0f;
        modelScale = sin((M_PI * (180.0f * animProgress)) / 180.0f) + 1.0f;
        mbObjScaleSet(effectModelId, modelScale, modelScale, modelScale);
        HuPrcVSleep();
    }
    // Keep the model hovering for at least 60 frames and until the bonus-coin effect finishes.
    mbev_CapRandomBonusCoin(capWork->playerNo, capWork->capsuleNo, FALSE);
    for (frameIndex = 0; frameIndex < 60.0f || !mbev_CapBonusCoinCheck(capWork->playerNo);
         frameIndex++) {
        animProgress = frameIndex / 60.0f;
        mbPlayerPosGet(capWork->playerNo, &mushroomPos);
        mushroomPos.y += (100.0f * sin((M_PI * (360.0f * animProgress)) / 180.0f))
            * 0.1f + 250.0f;
        mbObjPosSet(effectModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
        HuPrcVSleep();
    }
    // X and Z remain at the earlier position while the model height eases to the current player
    // height.
    for (frameIndex = 1; frameIndex <= 10; frameIndex++) {
        animProgress = frameIndex / 10.0f;
        mbPlayerPosGet(capWork->playerNo, &currentPlayerPos);
        mushroomPos.y += animProgress * ((currentPlayerPos.y + 250.0f) - mushroomPos.y);
        mbObjPosSet(effectModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
        HuPrcVSleep();
    }
    // Shrink and orbit the model around the player before applying this capsule's dice mode.
    mbAudFXPlay(MSM_SE_BRD00_MUSHROOM_ORBIT_SHRINK);
    for (frameIndex = 0; frameIndex < 90.0f; frameIndex++) {
        animProgress = 1.0f - (frameIndex / 90.0f);
        heightOffset = 50.0f + (animProgress * animProgress * 200.0f);
        orbitFactor = sin((M_PI * (180.0f * (animProgress * animProgress))) / 180.0f);
        mbPlayerPosGet(capWork->playerNo, &mushroomPos);
        mushroomPos.x +=
            (orbitFactor * cos((M_PI * (2.0f * (animProgress * 360.0f))) / 180.0f))
                * 100.0f * 1.5f;
        mushroomPos.z +=
            (orbitFactor * sin((M_PI * (2.0f * (animProgress * 360.0f))) / 180.0f))
                * 100.0f * 1.5f;
        mushroomPos.y += heightOffset;
        mbObjPosSet(effectModelId, mushroomPos.x, mushroomPos.y, mushroomPos.z);
        mbObjScaleSet(effectModelId, animProgress, animProgress, animProgress);
        HuPrcVSleep();
    }
    lbl_80265750[capWork->playerNo].diceMode = 4;
    mbPlayerMetalSet(capWork->playerNo, TRUE);
    fn_8006A030(capWork->playerNo, 20, 4, 4);
    mbAudFXPlay(MSM_SE_BRD00_MUSHROOM_METAL_TRANSFORM);
    while (mbev_CapEffGlowDispGet(capWork->glowObj) > 0) {
        HuPrcVSleep();
    }
    HuPrcEnd();
}

void mbev_CapMKinokoKill(void)
{
}

// Runs from the Killer capsule event entry, plays its effect, awards bonus coins, and sets the
// player's dice mode to five dice.
void mbev_CapKiller(void)
{
    CAPWORK *work = fn_8000DD9C()->property;

    mbev_CapWait(work);
    mbCapEffUseCreate(work->playerNo, work->capsuleNo);
    fn_8006A030(work->playerNo, 20, 4, 4);
    HuPrcSleep(20);
    mbev_CapRandomBonusCoin(work->playerNo, work->capsuleNo, TRUE);
    while (mbCapEffUseModeGet(work->playerNo) >= 0) {
        HuPrcVSleep();
    }
    lbl_80265750[work->playerNo].diceMode = 5;
    HuPrcEnd();
}

void mbev_CapKillerKill(void)
{
}

// Runs from the Dokan capsule event entry, awards bonus coins, then swaps spaces with the
// roulette-selected player unless both occupy the same space.
void mbev_CapDokan(void)
{
    CAPWORK *work = fn_8000DD9C()->property;
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
    fn_8006A030(work->playerNo, 20, 4, 4);
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
    masuId[0] = lbl_80265750[playerNo[0]].masuId;
    masuId[1] = lbl_80265750[playerNo[1]].masuId;
    if (masuId[0] == masuId[1]) {
        mbWinCreate(2, CAPMOVE_MESS_DOKAN_SAME_SPACE, -1);
        mbWinTopWait();
        HuPrcEnd();
    }
    if (mbev_CapPlayerMasuNumGet(masuId[0]) > 1) {
        mbev_PlayerColCircleAdd(playerNo[0], masuId[0], FALSE, 125.0f);
        while (lbl_80265750[playerNo[0]].moveF) {
            HuPrcVSleep();
        }
    }
    if (mbev_CapPlayerMasuNumGet(masuId[1]) > 1) {
        mbev_PlayerColCircleAdd(playerNo[1], masuId[1], FALSE, 125.0f);
        while (lbl_80265750[playerNo[1]].moveF) {
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
    mbAudFXPlay(MSM_SE_BRD00_PIPE_WARP);
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
                0.0f, 8.0f, 0);
        }
        fn_8006A030(playerNo[i], 20, 7, 3);
    }
    mbAudFXPlay(MSM_SE_BRD00_PIPE_LAND_THUMP);
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
    lbl_80265750[playerNo[0]].masuId = masuId[1];
    mbPlayerPosSetV(playerNo[1], &playerPos);
    lbl_80265750[playerNo[1]].masuId = masuId[0];
    mbCameraMoveMasu(masuId[1], NULL, &capsuleCameraOfs, -1.0f, -1.0f,
        -1);
    mbCameraMoveWait();
    mbStarMasuDispSet(masuId[1], FALSE);
    mbWipeDissolveFadeIn();
    mbAudFXPlay(MSM_SE_BRD00_PIPE_WARP);
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
                0);
            mbPlayerMotionSpeedSet(playerNo[i], 0.75f);
            mbPlayerScaleSet(playerNo[i], 1.0f, 1.0f, 1.0f);
        }
        fn_8006A030(playerNo[i], 20, 7, 3);
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
            ((1 << 30) | (1 << 0)));
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
        lbl_80265750[playerNo[j]].masuIdNext = masuId[j ^ 1];
    }
    mbObjDispSet(modelId[0], FALSE);
    mbObjDispSet(modelId[1], FALSE);
    HuPrcEnd();
}

void mbev_CapDokanKill(void)
{
}

// Runs from the Hanachan capsule event entry; awards bonus coins, flies the player to the Star
// Space
// (or current space if none exists), and awards a star only below 999 stars.
void mbev_CapHanachan(void)
{
    CAPWORK *work = fn_8000DD9C()->property;
    Mtx hookMtx;
    int motFile[8];
    HuVecF setupPos[5];
    HuVecF playerPos;
    HuVecF starPos;
    HuVecF startPos;
    HuVecF endPos;
    HuVecF controlPos;
    HuVecF modelPos;
    HuVecF basePos;
    HuVecF hookPos;
    HuVecF tempVec;
    OMOBJ *obj;
    int motionId[2];
    int playerMasuId;
    BOOL cancelCheckF;
    int modelId;
    int focusModelId;
    int starMasuId;
    int soundId;
    int cancelF;
    HuVecF *modelPosP;
    int i;
    float time;
    float angle;
    float height;
    float baseAngle;
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
    motionId[0] = mbev_CapPlayerMotionCreate(&work->objWork,
        work->playerNo, CHARMOT_HSF_c000m1_344);
    HuPrcVSleep();
    motionId[1] = mbev_CapPlayerMotionCreate(&work->objWork,
        work->playerNo, CHARMOT_HSF_c000m1_376);
    HuPrcVSleep();
    mbev_CapWait(work);
    motFile[0] = CAPMOVE_DATA_HANACHAN_MOTION_A;
    motFile[1] = CAPMOVE_DATA_HANACHAN_MOTION_B;
    motFile[2] = -1;
    modelId = mbev_CapObjCreate(&work->objWork, CAPMOVE_DATA_HANACHAN,
        motFile, FALSE, 5, FALSE);
    mbObjLayerSet(modelId, 3);
    mbObjMotionSet(modelId, 1, ((1 << 30) | (1 << 0)));
    mbObjDispSet(modelId, FALSE);
    focusModelId = mbev_CapObjCreate(&work->objWork,
        CAPMOVE_DATA_CAMERA_TARGET, NULL, FALSE, 5, FALSE);
    mbObjDispSet(focusModelId, FALSE);
    HuPrcVSleep();
    mbPlayerPosGet(work->playerNo, &playerPos);
    basePos = playerPos;
    playerMasuId = lbl_80265750[work->playerNo].masuId;
    for (i = 1; i <= mbMasuRawNumGet(); i++) {
        if (mbMasuTypeGet(i) == CAPMOVE_MASU_TYPE_STAR) {
            starMasuId = i;
            break;
        }
    }
    if (i > mbMasuRawNumGet()) {
        starMasuId = lbl_80265750[work->playerNo].masuId;
    }
    mbMasuPosGet(starMasuId, &starPos);
    mbObjPosSetV(focusModelId, &playerPos);
    mbCameraMoveObj(focusModelId, NULL, &capsuleCameraOfs, -1.0f, -1.0f,
        21);
    mbCameraMoveWait();
    obj = fn_8003AB38(
        mbObjMan, -32768, 0, 0, -1, ev_CapHanachanOMExec);
    obj->work[0] = 0;
    obj->work[1] = 0;
    work->effectModelId = modelId;
    obj->data = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(CAPWORK), 0x10000000);
    memcpy(obj->data, work, sizeof(CAPWORK));
    soundId = mbAudFXPlay(MSM_SE_BRD00_HANACHAN_MOVE_LOOP);
    startPos.x = playerPos.x;
    startPos.y = playerPos.y + 250.0f;
    startPos.z = playerPos.z - 100.0f;
    setupPos[0] = basePos;
    setupPos[0].y += 250.0f;
    setupPos[1] = setupPos[0];
    setupPos[1].y -= 200.0f;
    setupPos[1].z += 500.0f;
    setupPos[2] = setupPos[0];
    setupPos[2].y += 300.0f;
    setupPos[3] = setupPos[0];
    setupPos[3].y += 200.0f;
    setupPos[3].z -= 300.0f;
    setupPos[4] = startPos;
    mbCapEffUseCreate(work->playerNo, work->capsuleNo);
    fn_8006A030(work->playerNo, 20, 4, 4);
    while (mbCapEffUseModeGet(work->playerNo) < 2) {
        HuPrcVSleep();
    }
    mbObjDispSet(modelId, TRUE);
    for (i = 0; i <= 90.0f; i++) {
        time = i / 90.0f;
        mbObjRotSet(modelId, 45.0f - (405.0f * time), 0.0f, 0.0f);
        if (time < 0.5f) {
            time =
                sin((M_PI * (90.0f * (2.0f * time))) / 180.0f);
            mbev_CapBezierGetV(time, &setupPos[0].x,
                &setupPos[1].x, &setupPos[2].x,
                &tempVec.x);
        } else {
            time = cos((M_PI *
                (90.0f - (90.0f * (2.0f * (time - 0.5f))))) / 180.0f);
            mbev_CapBezierGetV(time, &setupPos[2].x,
                &setupPos[3].x, &setupPos[4].x,
                &tempVec.x);
        }
        mbObjPosSetV(modelId, &tempVec);
        HuPrcVSleep();
    }
    mbev_CapRandomBonusCoin(work->playerNo, work->capsuleNo, TRUE);
    mbPlayerRotateStart(work->playerNo, 180, 15);
    while (!mbPlayerRotateCheck(work->playerNo)) {
        HuPrcVSleep();
    }
    HuPrcSleep(15);
    mbAudFXPlay(MSM_SE_GUIDE_HANACHAN_ARRIVE);
    mbWinCreate(2, CAPMOVE_MESS_HANACHAN_ARRIVE,
        1);
    mbWinTopWait();
    // cancelF is forced false here, so the cancellation branch below cannot run.
    cancelF = FALSE;
    if (!cancelF) {
        cancelCheckF = FALSE;
    } else {
        cancelCheckF = TRUE;
    }
    if (cancelCheckF) {
        HuVecF explodePos;

        mbAudFXPlay(MSM_SE_BRD00_EXPLOSION_POP);
        mbAudFXPlay(MSM_SE_BRD00_HANACHAN_BURST_TAIL);
        if (soundId != -1) {
            mbAudFXStop(soundId);
        }
        if (!mbExitCheck()) {
            obj->work[0]++;
        }
        explodePos = startPos;
        modelPosP = &explodePos;
        mbev_CapEffDustExplodeAdd(work->explodeObj, modelPosP);
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
            1);
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
        radius = hanachanPlayerOfs[lbl_80265750[work->playerNo].charNo].z;
        hookYOfs = hanachanPlayerOfs[lbl_80265750[work->playerNo].charNo].y;
        rotX = hanachanPlayerOfs[lbl_80265750[work->playerNo].charNo].x;
        angle = 0.0f;
        mbPlayerMotionShiftSet(work->playerNo, 4, 0.0f, 8.0f,
            0);
        mbPlayerColSnapPlayerSet(work->playerNo, FALSE);
        for (i = 0; i <= 30.0f; i++) {
            time = i / 30.0f;
            mbev_CapBezierGetV(time, &modelPos.x,
                &controlPos.x, &endPos.x,
                &startPos.x);
            mbObjPosSetV(modelId, &startPos);
            tempVec = basePos;
            tempVec.y += 200.0f *
                sin((M_PI * (90.0f * time)) / 180.0f);
            mbPlayerPosSetV(work->playerNo, &tempVec);
            mbPlayerRotSet(work->playerNo, 0.0f,
                180.0f - (180.0f * time), 0.0f);
            HuPrcVSleep();
        }
        startPos = endPos;
        mbObjPosSetV(modelId, &startPos);
        mbObjMotionShiftSet(modelId, 2, 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
        if (motionId[1] != -1) {
            mbPlayerMotionShiftSet(work->playerNo, motionId[1], 0.0f,
                15.0f, ((1 << 30) | (1 << 0)));
        }
        fn_8006A030(work->playerNo, 20, 7, 3);
        mbPlayerPosGet(work->playerNo, &playerPos);
        for (i = 0; i <= 24.0f; i++) {
            time = i / 24.0f;
            fn_80023398(
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
        fn_8006A030(work->playerNo, 180, 4, 4);
        endPos = startPos;
        baseAngle = angle;
        for (i = 0; i < 90.0f; i++) {
            time = i / 90.0f;
            angle = baseAngle + (180.0f * time);
            startPos.x = endPos.x + (300.0f *
                (sin((M_PI * angle) / 180.0f) *
                sin((M_PI * (90.0f * time)) / 180.0f)));
            startPos.z = endPos.z + (300.0f *
                (cos((M_PI * angle) / 180.0f) *
                sin((M_PI * (90.0f * time)) / 180.0f)));
            startPos.y = endPos.y + (550.0f * time);
            mbObjPosSet(modelId, startPos.x, startPos.y, startPos.z);
            mbObjRotSet(modelId, 0.0f, angle, 0.0f);
            fn_80023398(
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
        lbl_80265750[work->playerNo].masuId = starMasuId;
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
        fn_8006A030(work->playerNo, 180, 4, 4);
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
            startPos.x = endPos.x + (2.0 * (distance *
                sin((M_PI * angle) / 180.0f)));
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
            fn_80023398(
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
            ((1 << 30) | (1 << 0)));
        mbMasuPosGet(starMasuId, &playerPos);
        for (i = 0; i < 8; i++) {
            time = 1.0f - (i / 7.0f);
            fn_80023398(
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
            ((1 << 30) | (1 << 0)));
        for (i = 0; i < 30.0f; i++) {
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(work->playerNo, 1, 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
        mbev_CapBubbleHookCall(3, modelId, TRUE, FALSE, FALSE);
        mbWinCreate(2, CAPMOVE_MESS_HANACHAN_STAR,
            1);
        mbWinTopWait();
        mbPlayerPosGet(work->playerNo, &tempVec);
        endPos.x = tempVec.x;
        endPos.y = startPos.y;
        endPos.z = tempVec.z;
        distance = fabs(tempVec.z - startPos.z);
        for (i = 0; i < 120.0f; i++) {
            time = i / 120.0f;
            height = 650.0f;
            angle = 180.0f + (360.0f * time);
            if (time < 0.2f) {
                ease = 5.0f * time;
            }
            if (ease > 1.0f) {
                time = 1.0f;
            }
            startPos.x = endPos.x + ((distance +
                (2.0f * (3.0f * (100.0f * time)))) *
                sin((M_PI * angle) / 180.0f));
            startPos.z = endPos.z + ((distance +
                (3.0f * (100.0f * time))) *
                cos((M_PI * angle) / 180.0f));
            startPos.y = endPos.y + (time * height);
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
            work->playerNo, 1, ((1 << 30) | (1 << 0)), TRUE);
        mbCameraPlayerViewSet(work->playerNo, 0);
        mbCameraMoveWait();
        if (mbPlayerStarGet(work->playerNo) >= 999) {
            if (!lbl_80265B70.curTime) {
                mbWinCreate(0, CAPMOVE_MESS_STAR_MAX_DAY,
                    mbGuideSpeakerNoGet());
            } else {
                mbWinCreate(0, CAPMOVE_MESS_STAR_MAX_NIGHT,
                    mbGuideSpeakerNoGet());
            }
            mbWinTopWait();
        } else {
            mbev_StarMasu(work->playerNo, lbl_80265750[work->playerNo].masuId);
        }
    }
    HuPrcEnd();
}

void mbev_CapHanachanKill(void)
{
}

// Per-frame callback that emits colored particles around Hanachan on alternating frames during the
// capsule encounter.
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
    switch ((int)obj->work[0]) {
    case 0:
        if (++obj->work[1] & 1) {
        mbObjPosGet(work->effectModelId, &pos);
        mbObjRotGet(work->effectModelId, &rot);
        pos.y += 50.0f;
        offset.x = 100.0f * (1.5f * (-0.5f + MBCapsuleEffRandF()));
        offset.y = 100.0f * (-0.5f + MBCapsuleEffRandF());
        offset.z = -100.0f * (1.0f +
            (0.5f * MBCapsuleEffRandF()));
        fn_80022E38(matrix, rot.x, rot.y, rot.z);
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
            0.05f + (0.02f * MBCapsuleEffRandF()), 4.9 / 60.0,
            colorP);
        mbev_CapEffGlowKinokoTimeSet(
            work->glowObj, particleNo, 1, 90);
        }
        break;
    default:
        omDelObjEx(mbObjMan, obj);
        return;
    }
}

// Runs from the Poison Mushroom capsule event entry; animates the mushroom, awards its coins, and
// sets the player's dice mode.
void mbev_CapNKinoko(void)
{
    CAPWORK *work = fn_8000DD9C()->property;
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
    work->effectModelId = modelId;
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
        playerPos.y += (100.0f * sin((M_PI * (360.0f * time)) / 180.0f)) * 0.1f
            + 250.0f;
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
    mbAudFXPlay(MSM_SE_BRD00_BONUS_MUSHROOM_ORBIT);
    for (i = 0; i < 90.0f; i++) {
        time = 1.0f - (i / 90.0f);
        yOfs = 50.0f + (time * time * 200.0f);
        radius = sin((M_PI * (180.0f * (time * time))) / 180.0f);
        mbPlayerPosGet(work->playerNo, &playerPos);
        playerPos.x += (radius * cos((M_PI * (2.0f * (time * 360.0f))) / 180.0f))
            * 100.0f * 1.5f;
        playerPos.z += (radius * sin((M_PI * (2.0f * (time * 360.0f))) / 180.0f))
            * 100.0f * 1.5f;
        playerPos.y += yOfs;
        mbObjPosSet(modelId, playerPos.x, playerPos.y, playerPos.z);
        mbObjScaleSet(modelId, time, time, time);
        HuPrcVSleep();
    }
    lbl_80265750[work->playerNo].diceMode = 6;
    fn_8006A030(work->playerNo, 20, 4, 4);
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

static HuVecF killerOfsTbl[9] = {
    { 0.0f, 0.0f, 0.0f },
    { 80.0f, 0.0f, 80.0f },
    { -80.0f, 0.0f, 80.0f },
    { 80.0f, 0.0f, -80.0f },
    { -80.0f, 0.0f, -80.0f },
    { 100.0f, 0.0f, 0.0f },
    { -100.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 100.0f },
    { 0.0f, 0.0f, -100.0f },
};

// Runs from the Killer movement event entry; rides the Bullet Bill across spaces, hits players, and
// steals up to 20 coins from each hit player for the rider.
void mbev_CapKillerMove(void)
{
    CAPWORK *work;
    HuVecF *dustPosP;
    Mtx matrix;
    HuVecF path[4];
    HuVecF finalPlayerPos;
    HuVecF finalPlayerRot;
    HuVecF playerPos;
    HuVecF candidatePos;
    HuVecF killerPos;
    HuVecF killerRotation;
    HuVecF killerScale;
    HuVecF targetPos;
    HuVecF offset;
    HuVecF playerStartPos;
    HuVecF velocity;
    HuVecF landingPos;
    HuVecF playerPosStart;
    HuVecF ridePos;
    HuVecF playerRotStart;
    HuVecF targetRotation;
    GXColor color;
    HuVecF effectPos;
    HuVecF nextRot;
    HuVecF boostPos;
    int motionRide;
    int motionStart;
    int nextMasuId;
    int playerNo;
    int modelId;
    int readStat;
    int coinNum;
    int coinTotal;
    int moveNum;
    int stepNum;
    int masuId;
    int hitF;
    int waypointMasuId[4];
    int coinDisp[4];
    int playerHitF[4];
    int i;
    int j;
    int boostBudget;
    int masuType;
    int masuAttr;
    float distance;
    float t;
    float modelScale;
    float scalePos;
    float segmentLength;

    work = fn_8000DD9C()->property;
    readStat = mbBGRead(CAPMOVE_DATA_KILLER);
    if (readStat != -1) {
        mbBGReadWait(readStat);
    }
    work->explodeObj = mbev_CapEffExhaustCreate();
    work->boostObj = mbev_CapEffBoostCreate();
    mbev_CapEffBoostBlendModeSet(work->boostObj, 1);
    mbev_CapPlayerMoveObjInit();

    playerNo = work->playerNo;
    mbPlayerPosGet(playerNo, &finalPlayerPos);
    mbPlayerRotGet(playerNo, &finalPlayerRot);
    masuId = lbl_80265750[playerNo].masuId;
    coinTotal = 0;
    for (i = 0; i < 4; i++) {
        playerHitF[i] = FALSE;
    }
    modelId = mbev_CapObjCreate(&work->objWork, CAPMOVE_DATA_KILLER, NULL,
        FALSE, 5, FALSE);
    mbObjDispSet(modelId, FALSE);
    motionStart = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
        CAPMOVE_DATA_KILLER_RIDE_START);
    motionRide = mbev_CapPlayerMotionCreate(&work->objWork, playerNo,
        CAPMOVE_DATA_KILLER_RIDE);
    for (i = 0; i < 4; i++) {
        waypointMasuId[i] = lbl_80265750[playerNo].masuId;
        mbMasuPosGet(waypointMasuId[i], &path[i]);
        path[i].y += 120.00001f;
    }
    waypointMasuId[2] = mbev_CapMasuLinkNextRandomGet(waypointMasuId[1],
        &path[2]);
    path[2].y += 120.00001f;
    waypointMasuId[3] = mbev_CapMasuLinkNextRandomGet(waypointMasuId[2],
        &path[3]);
    path[3].y += 120.00001f;
    moveNum = mbDiceResultGet(playerNo);
    mbev_CapStatusDispSetAll(TRUE, FALSE);

    mbPlayerPosGet(playerNo, &landingPos);
    mbPlayerRotGet(playerNo, &playerRotStart);
    mbev_CapHermiteGetV(0.0f, &path[0], &path[1], &path[2], &path[3],
        &playerPosStart);
    killerPos = playerPosStart;
    mbev_CapHermiteGetV(1.0f, &path[0], &path[1], &path[2], &path[3],
        &ridePos);
    PSVECSubtract(&ridePos, &playerPosStart, &playerPos);
    if (PSVECMag(&playerPos) > 0.0f) {
        PSVECNormalize(&playerPos, &playerPos);
    }
    killerRotation.x = atan2(-playerPos.y,
        sqrtf(playerPos.x * playerPos.x + playerPos.z * playerPos.z))
        / M_PI * 180.0;
    killerRotation.y = atan2(playerPos.x, playerPos.z) / M_PI * 180.0;
    killerRotation.z = 0.0f;
    targetRotation = killerRotation;
    fn_80022E38(matrix, -killerRotation.x, killerRotation.y, killerRotation.z);
    effectPos.x = 0.0f;
    effectPos.y = 60.000004f;
    effectPos.z = -75.0f;
    PSMTXMultVec(matrix, &effectPos, &effectPos);
    PSVECAdd(&killerPos, &effectPos, &playerPosStart);
    mbPlayerColSnapPlayerSet(playerNo, FALSE);
    mbPlayerMotionShiftSet(playerNo, CAPMOVE_PLAYER_MOT_JUMP, 0.0f,
        8.0f, 0);

    for (i = 0; i < 30.0f; i++) {
        t = (float)i / 30.0f;
        PSVECSubtract(&playerPosStart, &landingPos, &effectPos);
        if ((distance = PSVECMag(&effectPos)) > 0.0f) {
            PSVECNormalize(&effectPos, &effectPos);
        }
        PSVECScale(&effectPos, &effectPos, t * distance);
        PSVECAdd(&landingPos, &effectPos, &finalPlayerPos);
        finalPlayerPos.y += 200.0
            * sin((M_PI * (180.0f * t)) / 180.0);
        finalPlayerRot.x = mbev_CapAngleSumLerp(t, playerRotStart.x,
            targetRotation.x);
        finalPlayerRot.y = mbev_CapAngleSumLerp(t, playerRotStart.y,
            targetRotation.y);
        finalPlayerRot.z = mbev_CapAngleSumLerp(t, playerRotStart.z,
            targetRotation.z);
        mbPlayerPosSetV(playerNo, &finalPlayerPos);
        mbPlayerRotSetV(playerNo, &finalPlayerRot);
        if (i == 15) {
            HuVecF dustPos;

            fn_80022E38(matrix, killerRotation.x, killerRotation.y, killerRotation.z);
            effectPos.x = 0.0f;
            effectPos.y = 0.0f;
            effectPos.z = -75.0f;
            PSMTXMultVec(matrix, &effectPos, &effectPos);
            PSVECAdd(&killerPos, &effectPos, &effectPos);
            effectPos.z += 50.0f;
            dustPos = effectPos;
            dustPosP = &dustPos;
            mbev_CapEffDustCloudAdd(work->explodeObj, dustPosP);
            mbAudFXPlay(MSM_SE_BRD00_PRIZE_SPARKLE);
            mbObjPosSetV(modelId, &killerPos);
            mbObjRotSetV(modelId, &killerRotation);
            mbObjScaleSet(modelId, 0.0f, 0.0f, 0.0f);
            mbObjDispSet(modelId, TRUE);
            for (j = 0; j < 4; j++) {
                if (j == playerNo ||
                    lbl_80265750[j].masuId != masuId) {
                    continue;
                }
                mbev_CapPlayerMoveIdleCreate(j, 12);
                mbAudFXPlay(MSM_SE_BRD00_KILLER_HIT_PLAYER);
                CharFXPlay(lbl_80265750[j].charNo, MSM_SE_CHARVOICE_ALARMED_GASP);
                fn_8006A030(j, 20, 7, 3);
                fn_8006A030(playerNo, 20, 7, 3);
                hitF = TRUE;
                playerHitF[j] = TRUE;
                coinNum = 20;
                if (mbPlayerCoinGet(j) < coinNum) {
                    coinNum = mbPlayerCoinGet(j);
                }
                if (coinNum > 0) {
                    mbPlayerPosGet(j, &effectPos);
                    effectPos.y += 250.0f;
                    coinDisp[j] = mbCoinDispCapsuleCreate(&effectPos, -coinNum);
                    mbCoinAddDispExec(j, -coinNum, FALSE, TRUE);
                    mbCoinAddDispExec(playerNo, coinNum, FALSE,
                        TRUE);
                    coinTotal += coinNum;
                }
            }
            mbPlayerMotionShiftSet(playerNo, motionStart, 0.0f,
                8.0f, 0);
        } else if (i > 15) {
            scalePos = ((float)i - 15.0f) / 10.0f;
            if (scalePos > 1.0f) {
                scalePos = 1.0f;
            }
            modelScale = scalePos + (0.25f
                * sin((M_PI * (180.0f * scalePos)) / 180.0f));
            mbObjScaleSet(modelId, modelScale, modelScale, modelScale);
        }
        HuPrcVSleep();
    }

    for (i = 0; i < 45.0f; i++) {
        t = (float)i / 45.0f;
        mbObjPosSetV(modelId, &killerPos);
        mbObjRotSet(modelId, killerRotation.x,
        killerRotation.y + (30.0f * sin((M_PI * (540.0f * t)) / 180.0f)),
            killerRotation.z);
        mbObjRotGet(modelId, &effectPos);
        fn_80022E38(matrix, effectPos.x, effectPos.y, effectPos.z);
        effectPos.x = 0.0f;
        effectPos.y = 60.000004f;
        effectPos.z = -75.0f;
        PSMTXMultVec(matrix, &effectPos, &effectPos);
        PSVECAdd(&killerPos, &effectPos, &finalPlayerPos);
        mbObjRotGet(modelId, &finalPlayerRot);
        mbPlayerPosSetV(playerNo, &finalPlayerPos);
        mbPlayerRotSetV(playerNo, &finalPlayerRot);
        for (j = 0; j < 3; j++) {
            effectPos = killerPos;
            nextRot = killerRotation;
            nextRot.y += 30.0f
                * sin((M_PI * (360.0f * t)) / 180.0f);
            ev_CapEffKillerDustCreate(work, &effectPos, &nextRot);
        }
        HuPrcVSleep();
    }

    mbObjRotSet(modelId, -killerRotation.x, killerRotation.y, killerRotation.z);
    mbObjScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    for (i = 0; i < 30.0f; i++) {
        t = (float)i / 60.0f;
        mbObjPosSetV(modelId, &killerPos);
        mbObjRotSet(modelId,
            killerRotation.x
                + (30.0f * sin((M_PI * (180.0f * t)) / 180.0f)),
            killerRotation.y, killerRotation.z);
        modelScale = -(0.2f
            * sin((M_PI * (180.0f * t)) / 180.0f));
        scalePos = 0.2f
            * sin((M_PI * (180.0f * t)) / 180.0f);
        mbObjScaleSet(modelId, 1.0f + scalePos, 1.0f + scalePos,
            1.0f + modelScale);
        mbObjRotGet(modelId, &effectPos);
        fn_80022E38(matrix, effectPos.x, effectPos.y, effectPos.z);
        effectPos.x = 0.0f;
        effectPos.y = 60.000004f;
        effectPos.z = -75.0f;
        PSMTXMultVec(matrix, &effectPos, &effectPos);
        PSVECAdd(&killerPos, &effectPos, &finalPlayerPos);
        mbObjRotGet(modelId, &finalPlayerRot);
        mbPlayerPosSetV(playerNo, &finalPlayerPos);
        mbPlayerRotSetV(playerNo, &finalPlayerRot);
        HuPrcVSleep();
    }

    nextRot = killerRotation;
    fn_80022E38(matrix, nextRot.x, nextRot.y, nextRot.z);
    effectPos.x = 0.0f;
    effectPos.y = 0.0f;
    effectPos.z = -120.00001f;
    PSMTXMultVec(matrix, &effectPos, &effectPos);
    PSVECAdd(&killerPos, &effectPos, &effectPos);
    ev_CapEffKillerExplodeCreate(work, &effectPos, &nextRot, 32);
    boostPos = effectPos;
    for (i = 0; i < 32; i++) {
        ev_CapEffKillerBoostCreate(work, &boostPos, &killerRotation);
    }
    boostBudget = 12;
    mbObjRotGet(modelId, &effectPos);
    killerRotation.x = mbev_CapAngleSumLerp(0.5f, killerRotation.x, effectPos.x);
    killerRotation.y = mbev_CapAngleSumLerp(0.5f, killerRotation.y, effectPos.y);
    killerRotation.z = mbev_CapAngleSumLerp(0.5f, killerRotation.z, effectPos.z);
    mbObjScaleGet(modelId, &killerScale);
    mbAudFXPlay(MSM_SE_BRD00_KILLER_FLY);
    hitF = 0;
    do {
        masuId = waypointMasuId[1];
        nextMasuId = waypointMasuId[2];
        hitF++;
        if (hitF % 20 == 1) {
            fn_8006A030(playerNo, 20, 4, 4);
        }
        if (nextMasuId <= 0 ||
            mbev_CapMasuLinkNextGet(nextMasuId, NULL) <= 0) {
            break;
        }
        landingPos = path[1];
        playerPosStart = path[2];
        targetPos = path[0];
        offset = killerRotation;
        PSVECSubtract(&playerPosStart, &landingPos, &playerPos);
        if ((segmentLength = PSVECMag(&playerPos))) {
            PSVECNormalize(&playerPos, &playerPos);
        }
        playerRotStart = killerRotation;
        targetRotation.x = atan2(-playerPos.y,
            sqrtf(playerPos.x * playerPos.x + playerPos.z * playerPos.z))
            / M_PI * 180.0;
        targetRotation.y = atan2(playerPos.x, playerPos.z) / M_PI * 180.0;
        targetRotation.z = 0.0f;
        stepNum = (int)(segmentLength / 20.0f);
        for (i = 1; i <= stepNum; i++) {
            t = (float)i / (float)stepNum;
            mbev_CapHermiteGetV(t, &path[0], &path[1], &path[2], &path[3],
                &killerPos);
            j = stepNum;
            if (j > 20) {
                j = 20;
            }
            scalePos = (float)i / (float)j;
            if (scalePos > 1.0f) {
                scalePos = 1.0f;
            }
            effectPos.x = mbev_CapAngleSumLerp(scalePos, playerRotStart.x,
                targetRotation.x);
            effectPos.y = mbev_CapAngleSumLerp(scalePos, playerRotStart.y,
                targetRotation.y);
            effectPos.z = 0.0f;
            killerRotation = effectPos;
            if (killerScale.x > 1.0f) {
                killerScale.x -= 0.01f;
            } else {
                killerScale.x = 1.0f;
            }
            if (killerScale.y > 1.0f) {
                killerScale.y -= 0.01f;
            } else {
                killerScale.y = 1.0f;
            }
            if (killerScale.z < 1.0f) {
                killerScale.z += 0.01f;
            } else {
                killerScale.z = 1.0f;
            }
            mbObjPosSetV(modelId, &killerPos);
            mbObjRotSetV(modelId, &killerRotation);
            mbObjScaleSetV(modelId, &killerScale);
            mbObjRotGet(modelId, &effectPos);
            fn_80022E38(matrix, effectPos.x, effectPos.y, effectPos.z);
            effectPos.x = 0.0f;
            effectPos.y = 60.000004f;
            effectPos.z = -75.0f;
            PSMTXMultVec(matrix, &effectPos, &effectPos);
            PSVECAdd(&killerPos, &effectPos, &finalPlayerPos);
            mbObjRotGet(modelId, &finalPlayerRot);
            mbPlayerPosSetV(playerNo, &finalPlayerPos);
            mbPlayerRotSetV(playerNo, &finalPlayerRot);
            if (i & 1) {
                ev_CapEffKillerDustCreate(work, &killerPos, &killerRotation);
            }
            if (--boostBudget > 0) {
                nextRot = killerRotation;
                fn_80022E38(matrix, nextRot.x, nextRot.y, nextRot.z);
                effectPos.x = 0.0f;
                effectPos.y = 0.0f;
                effectPos.z = -120.00001f;
                PSMTXMultVec(matrix, &effectPos, &effectPos);
                PSVECAdd(&killerPos, &effectPos, &effectPos);
                for (j = 0; j < 16; j++) {
                    ev_CapEffKillerBoostCreate(work, &effectPos, &killerRotation);
                }
            }
            HuPrcVSleep();
        }
        if (mbMasuTypeGet(nextMasuId) != CAPMOVE_MASU_TYPE_NONE) {
            HuAudFXPlay(MSM_SE_BRD00_MOVE_COUNT);
        }
        for (j = 0; j < 4; j++) {
            if (j == playerNo || playerHitF[j] == TRUE ||
                lbl_80265750[j].masuId != nextMasuId) {
                continue;
            }
            mbAudFXPlay(MSM_SE_BRD00_KILLER_HIT_PLAYER);
            mbev_CapPlayerMoveHitCreate(j, TRUE, TRUE);
            CharFXPlay(lbl_80265750[j].charNo, MSM_SE_CHARVOICE_ALARMED_GASP);
            fn_8006A030(j, 20, 7, 3);
            fn_8006A030(playerNo, 20, 7, 3);
            hitF = TRUE;
            playerHitF[j] = TRUE;
            coinNum = 20;
            if (mbPlayerCoinGet(j) < coinNum) {
                coinNum = mbPlayerCoinGet(j);
            }
            if (coinNum > 0) {
                mbPlayerPosGet(j, &effectPos);
                effectPos.y += 250.0f;
                coinDisp[j] = mbCoinDispCapsuleCreate(&effectPos, -coinNum);
                mbCoinAddDispExec(j, -coinNum, FALSE, TRUE);
                mbCoinAddDispExec(playerNo, coinNum, FALSE, TRUE);
                coinTotal += coinNum;
            }
        }
        lbl_80265750[playerNo].masuIdPrev = masuId;
        lbl_80265750[playerNo].masuId = nextMasuId;
        lbl_80265750[playerNo].masuIdPrev = lbl_80265750[playerNo].masuId;
        lbl_80265750[playerNo].masuId = nextMasuId;
        masuType = mbMasuTypeGet(nextMasuId);
        masuAttr = mbMasuAttrGet(nextMasuId);
        if (mbMasuDispCheck(nextMasuId)) {
            lbl_80265750[playerNo].moveNum--;
            if (lbl_80265750[playerNo].moveNum < 0) {
                lbl_80265750[playerNo].moveNum = 0;
            }
            moveNum--;
        }
        for (i = 0; i < 3; i++) {
            waypointMasuId[i] = waypointMasuId[i + 1];
            path[i] = path[i + 1];
        }
        waypointMasuId[3] = mbev_CapMasuLinkNextRandomGet(
            waypointMasuId[2], &path[3]);
        path[3].y += 120.00001f;
    } while (moveNum > 0);

    mbMoveNumKill(playerNo);
    mbObjDispSet(modelId, FALSE);
    {
        HuVecF dustPos;

        mbev_CapEffDustCloudAdd(work->explodeObj,
            (dustPos = killerPos, &dustPos));
    }
    mbAudFXPlay(MSM_SE_BRD00_EXPLOSION_POP);
    mbPlayerMotionShiftSet(playerNo, CAPMOVE_PLAYER_MOT_JUMP, 0.0f, 8.0f,
        0);
    mbPlayerPosGet(playerNo, &finalPlayerPos);
    playerStartPos = finalPlayerPos;
    for (i = 0; i < 9; i++) {
        mbMasuPosGet(lbl_80265750[playerNo].masuId, &candidatePos);
        PSVECAdd(&candidatePos, &killerOfsTbl[i], &candidatePos);
        for (j = 0; j < 4; j++) {
            if (j == playerNo) {
                continue;
            }
            mbPlayerPosGet(j, &effectPos);
            PSVECSubtract(&candidatePos, &effectPos, &effectPos);
            if (PSVECMag(&effectPos) < 95.0f) {
                break;
            }
        }
        if (j >= 4) {
            break;
        }
    }
    for (i = 0; i < 30.0f; i++) {
        t = (float)i / 30.0f;
        PSVECSubtract(&candidatePos, &playerStartPos, &effectPos);
        PSVECScale(&effectPos, &effectPos, t);
        PSVECAdd(&playerStartPos, &effectPos, &finalPlayerPos);
        finalPlayerPos.y += 2.0
            * (100.0 * sin((M_PI * (180.0f * t)) / 180.0));
        finalPlayerRot.x = mbev_CapAngleSumLerp(t, finalPlayerRot.x, 0.0f);
        finalPlayerRot.y = mbev_CapAngleSumLerp(t, finalPlayerRot.y, 0.0f);
        finalPlayerRot.z = mbev_CapAngleSumLerp(t, finalPlayerRot.z, 0.0f);
        mbPlayerPosSetV(playerNo, &finalPlayerPos);
        mbPlayerRotSetV(playerNo, &finalPlayerRot);
        HuPrcVSleep();
    }
    mbPlayerPosSetV(playerNo, &candidatePos);
    mbPlayerMotIdleSet(playerNo);
    mbPlayerColSnapPlayerSet(playerNo, TRUE);
    mbCameraMoveWait();
    for (i = 0; i < 60.0f; i++) {
        HuPrcVSleep();
    }
    if (coinTotal > 0) {
        mbev_CapCoinDisp(playerNo, coinTotal, TRUE, TRUE);
    }
    do {
        for (i = 0; i < 4; i++) {
            if (!mbev_CapPlayerMoveObjCheck(i)) {
                break;
            }
        }
        HuPrcVSleep();
    } while (i < 4);
    mbev_CapPlayerIdleWait();
    mbev_PlayerColMasuSet(playerNo, lbl_80265750[playerNo].masuId, FALSE);
    mbMasuPosGet(lbl_80265750[playerNo].masuId, &finalPlayerPos);
    mbPlayerPosGet(playerNo, &effectPos);
    PSVECSubtract(&finalPlayerPos, &effectPos, &effectPos);
    if (PSVECMag(&effectPos) > 25.0f) {
        mbMasuPosGet(lbl_80265750[playerNo].masuId, &finalPlayerPos);
        mbPlayerMoveExec(playerNo, 0, &finalPlayerPos, 30, 0, TRUE);
        mbPlayerRotateStart(playerNo, 0, 15);
        while (!mbPlayerRotateCheck(playerNo)) {
            HuPrcVSleep();
        }
    }
    mbev_CapPlayerMoveObjKill();
    HuPrcEnd();
}

void mbev_CapKillerMoveKill(void)
{
}

// Adds a randomized dust particle behind the Killer, oriented by rot, to the capsule's exhaust
// effect.
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

    fn_80022E38(mtx, rot->x, rot->y, rot->z);
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

// Adds count particles in a radial burst at pos, rotated by rot, to the Killer's explosion effect.
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
    int angle;
    int colorValue;
    int i;

    fn_80022E38(mtx, rot->x, rot->y, rot->z);
    for (i = 0; i < count; i++) {
        effectPos.x = pos->x;
        effectPos.y = pos->y;
        effectPos.z = pos->z;
        angle = (360.0f / count) * i;
        velocity.x = 0.075f * (100.0f *
            sin((angle * M_PI) / 180.0f)) *
            (0.9f + (0.1f * MBCapsuleEffRandF()));
        velocity.y = 0.075f * (100.0f *
            cos((angle * M_PI) / 180.0f)) *
            (0.9f + (0.1f * MBCapsuleEffRandF()));
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

// Adds a randomized boost particle near pos, directed by a perturbed rot, to the Killer's boost
// effect.
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
    mbev_CapEffBoostAdd(work->boostObj, effectPosP, velocityP,
        50.0f * (1.0f + (0.5f * MBCapsuleEffRandF())),
        2.0f * (-0.5f + MBCapsuleEffRandF()),
        (int)(60.0f * (1.0f + (0.5f * MBCapsuleEffRandF()))), colorP);
}
// Creates a per-frame mushroom particle object with a copy of the capsule's effect state.
static void ev_CapEffKinokoCreate(CAPWORK *work)
{
    OMOBJ *obj;

    obj = fn_8003AB38(mbObjMan, -32768, 0, 0, -1, ev_CapEffKinokoOMExec);
    obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAPWORK), 0x10000000);
    memcpy(obj->data, work, sizeof(CAPWORK));
}

// Per-frame callback that emits colored particles around the active mushroom model until it shrinks
// away.
static void ev_CapEffKinokoOMExec(OMOBJ *obj)
{
    CAPWORK *work;
    HuVecF pos;
    HuVecF scale;
    HuVecF effectPos;
    GXColor color;
    float colorValue;
    float particleScale;
    float scaleValue;
    int modelId;
    int capsuleNo;
    int particleNo;
    HuVecF *effectPosP;

    if (mbExitCheck() || obj->work[0]) {
        omDelObjEx(mbObjMan, obj);
        return;
    }
    work = obj->data;
    modelId = work->effectModelId;
    capsuleNo = work->capsuleNo;
    mbObjPosGet(modelId, &pos);
    mbObjScaleGet(modelId, &scale);
    scaleValue = scale.x;
    if (scaleValue > 1.0f) {
        scaleValue = 1.0f;
    }
    if (scaleValue <= CAPMOVE_EFFECT_MIN_SCALE) {
        obj->work[0] = 1;
    }
    switch (capsuleNo) {
        case 0:
            colorValue = MBCapsuleEffRandF();
            color.r = (u8)(192.0f + (63.0f * colorValue));
            color.g = (u8)(192.0f + (63.0f * colorValue));
            color.b = (u8)(192.0f + (63.0f * colorValue));
            color.a = (u8)(192.0f + (63.0f * MBCapsuleEffRandF()));
            break;
        case 1:
            colorValue = MBCapsuleEffRandF();
            color.r = (u8)(64.0f + (63.0f * colorValue));
            color.g = (u8)(192.0f + (63.0f * colorValue));
            color.b = (u8)(192.0f + (63.0f * colorValue));
            color.a = (u8)(192.0f + (63.0f * MBCapsuleEffRandF()));
            break;
        case 2:
            mbev_CapEffColorSet(&color,
                mbRandMod(CAPMOVE_EFFECT_COLOR_RANGE));
            color.a = (u8)(192.0f + (63.0f * MBCapsuleEffRandF()));
            break;
        case 3:
            colorValue = MBCapsuleEffRandF();
            color.r = (u8)(128.0f + (63.0f * colorValue));
            color.g = (u8)(128.0f + (63.0f * colorValue));
            color.b = (u8)(128.0f + (63.0f * colorValue));
            color.a = (u8)(192.0f + (63.0f * MBCapsuleEffRandF()));
            break;
        case 7:
            colorValue = MBCapsuleEffRandF();
            color.r = (u8)(128.0f + (32.0f * colorValue));
            color.g = (u8)(32.0f + (32.0f * colorValue));
            color.b = (u8)(128.0f + (32.0f * colorValue));
            color.a = (u8)(128.0f + (63.0f * MBCapsuleEffRandF()));
            break;
    }
    effectPos = pos;
    effectPosP = &effectPos;
    particleScale = 100.0f * scaleValue
        * (CAPMOVE_EFFECT_MIN_SCALE + (0.1f * MBCapsuleEffRandF()));
    particleNo = mbev_CapEffGlowKinokoAdd(work->glowObj, effectPosP,
        (int)(60.0f * (0.6f
            + (3.0f * (-0.5f + MBCapsuleEffRandF())))),
        particleScale, 150.0f * scaleValue, 150.0f * scaleValue,
        50.0f * scaleValue, 2, &color);
    if (particleNo >= 0) {
        mbev_CapEffGlowKinokoTimeSet(work->glowObj, particleNo, 1,
            (int)(80.0f
                + (20.0f * (-0.5f + MBCapsuleEffRandF()))));
    }
}
