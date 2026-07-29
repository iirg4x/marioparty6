#include "math.h"
#include "dolphin/pad.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/comchoice.h"
#include "game/board/coin.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/charman.h"
#include "game/esprite.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"

typedef int (*TERESA_STEAL_HOOK)(int);
typedef void (*TERESA_STEAL_BEGIN_HOOK)(int, int);

#define CAP_WORK_MAX 64

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

typedef struct TeresaFadeWork_s {
    void *textureData;
    u32 textureSize;
    BOOL activeF;
    float alpha;
    BOOL copyF;
    OMOBJ *object;
    u32 screenWidth;
    u32 screenHeight;
    u32 textureWidth;
    u32 textureHeight;
} TERESA_FADE_WORK;

typedef struct MiracleSprWork_s {
    BOOL activeF;
    int sprId;
    int backSprId;
    int sprIdTbl[6];
    int focusTime;
    int focusNo;
    BOOL hideF;
    float unk30;
    float unk34;
    HuVecF pos;
} MIRACLE_SPR_WORK;

static HuVecF capsuleCameraOfs = { 0.0f, 100.0f, 0.0f };
static HuVecF teresaCameraRot = { -30.0f, 0.0f, 0.0f };
static HuVecF teresaCameraOfs = { 0.0f, 100.0f, 0.0f };
static HuVecF teresaLightPos = { 0.0f, 0.0f, 0.0f };
static HuVecF teresaLightDir = { 0.0f, 1.0f, -1.0f };
static u32 MiracleGuideMotTbl[2][16] = {
    { 0x00110001, 0x00110004, 0x00110005, 0x00110015, 0x00110007, 0x0011000B,
        0x0011000C, 0x0011000D, 0x00110013, 0xFFFFFFFF },
    { 0x0011001C, 0x0011001F, 0x00110020, 0x0011002F, 0x00110022, 0x00110025,
        0x00110026, 0x00110027, 0x0011002D, 0xFFFFFFFF },
};
static int miracleTradeFileTbl[6] = {
    0x0011003B, 0x0011003C, 0x0011003D, 0x0011003E, 0x0011003F, 0x00110040,
};

static int koopaMdlId = -1;
static int teresaStealMesId = -1;
static GXColor teresaLightColor = { 0xFF, 0xBE, 0xFF, 0xFF };
static int miracleBackFile = 0x0011003A;
static int mgResultData[4];
static int kettouMotId[12];
static int diceHitTimer;
static OMOBJ *miracleSprObj;
static TERESA_STEAL_HOOK teresaStealHook;
static TERESA_STEAL_BEGIN_HOOK teresaStealBeginHook;
static int teresaStealCoinNum;
static TERESA_FADE_WORK *teresaFadeWork;

extern void mbDiceObjHit(int playerNo);
extern OMOBJ *mbev_CapEffGlowCreate(void);
extern OMOBJ *mbev_CapEffCoinCreate(void);
extern void mbev_CapEffCoinGlowSet(OMOBJ *obj, OMOBJ *glowObj);
extern int mbev_CapObjCreate(EVCAPWORK *work, int dataNum, int *motFile,
    BOOL linkF, int delay, BOOL closeDir);
extern void mbev_CapWait(CAPWORK *work);
extern BOOL mbev_CapPlayerCheck(int playerNo1, int playerNo2);
extern int mbev_CapPlayerComSelKettouGet(int playerNo, int type,
    int *playerList, int playerNum);
extern s16 mbev_CapPlayerMotionCreate(EVCAPWORK *work, int playerNo,
    int dataNum);
extern void mbev_CapPlayerMotShiftWait(int playerNo, int motionNo, int attr,
    BOOL waitF);
extern int mbev_CapEffCoinAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel,
    float scale, float gravity, int time, int arg);
extern BOOL mbev_CapEffCoinMaxYSet(OMOBJ *obj, int coinNo, float maxY);
extern int mbev_CapEffCoinNumGet(OMOBJ *obj);
extern void mbev_CapCoinAdd(OMOBJ *obj, int playerNo, int coinNum,
    BOOL highF);
extern void mbWipeDissolveFadeIn(void);
extern void mbWipeDissolveFadeOutTime(int time);
extern int mbStarObjCreate(void);
extern void mbStarObjPosSetV(int objNo, const HuVecF *pos);
extern void mbStarObjRotSet(int objNo, float x, float y, float z);
extern void mbStarObjScaleSet(int objNo, float x, float y, float z);
extern void mbStarObjDispSet(int objNo, BOOL dispF);
extern void mbStarObjDispSetAll(BOOL dispF);
extern void mbStarObjKill(int objNo);
extern void mbStarGetExec(int playerNo);

static void ev_CapTeresaFadeMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
static void ev_CapTeresaFadeOMExec(OMOBJ *obj);
void mbev_CapTeresaFadeCreate(int objectId);
void mbev_CapTeresaFadeKill(int objectId);
void mbev_CapTeresaFadeSet(float alpha);

void mbev_CapTeresa(void)
{
    CAPWORK *work = HuPrcCurrentGet()->property;
    HuVecF playerPos;
    HuVecF objectPos;
    HuVecF direction;
    HuVecF cameraRot;
    HuVecF targetPos;
    HuVecF targetStartPos;
    HuVecF lightAim;
    HuVecF coinPos;
    HuVecF coinVel;
    Mtx hookMtx;
    GXColor lightColors[HU3D_GLIGHT_MAX];
    int motionFiles[] = { 0x00130009, 0x0013000B, 0x0013000A, -1 };
    int targetPlayers[GW_PLAYER_MAX - 1];
    int enabledPlayers[GW_PLAYER_MAX - 1];
    char customMes[16];
    int playerNo = work->playerNo;
    int linkMasu;
    int objectId;
    int targetPlayer = -1;
    int targetNum;
    int coinTargetNum;
    int starTargetNum;
    int stealType = -1;
    int choice;
    int enabledNum;
    int capsuleIndex;
    int itemObjectId;
    int idleMotion;
    int stealMotion;
    int starMotion;
    int starObjectId;
    int helpWin;
    int pressNum;
    int alpha;
    int coinNum;
    int coinEffect;
    int lightId;
    int turnCoinMax;
    int i;
    int j;
    float angle;
    float time;
    float weight;
    float stealRate;
    float randomValue;
    float launchAngle;
    float launchElevation;
    float launchScale;
    char *hookName;
    BOOL musicChanged = FALSE;

    mbev_CapWait(work);
    if (!GwSystem.curTime) {
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, 0x40000001);
        mbWinCreate(2, 0x003B0000, 10);
        mbWinTopWait();
        HuPrcEnd();
        return;
    }

    work->glowObj = mbev_CapEffGlowCreate();
    work->coinObj = mbev_CapEffCoinCreate();
    mbev_CapEffCoinGlowSet(work->coinObj, work->glowObj);
    mbPlayerPosGet(playerNo, &playerPos);
    linkMasu = mbMasuAttrFindLink(GwPlayer[playerNo].masuId, 0x2000);
    if (linkMasu != -1) {
        mbMasuPosGet(linkMasu, &objectPos);
        PSVECSubtract(&objectPos, &playerPos, &direction);
        direction.y = 0.0f;
        if (PSVECMag(&direction) > 0.0f) {
            PSVECNormalize(&direction, &direction);
        }
        PSVECScale(&direction, &direction, 300.0f);
        PSVECAdd(&playerPos, &direction, &objectPos);
        objectPos.y += 125.0f;
    } else {
        objectPos = playerPos;
        objectPos.y -= 100.0f;
        objectPos.z -= 200.0f;
    }
    PSVECSubtract(&objectPos, &playerPos, &direction);
    objectId = mbev_CapObjCreate(&work->objWork, 0x00130008, motionFiles,
        FALSE, 5, FALSE);
    mbObjMotionSet(objectId, 1, 0x40000001);
    mbObjLayerSet(objectId, 4);
    mbObjScaleSet(objectId, 2.0f, 2.0f, 2.0f);
    mbObjPosSetV(objectId, &objectPos);
    angle = (float)(180.0 * (atan2(direction.x, direction.z) / M_PI));
    mbObjRotSet(objectId, 0.0f,
        (float)(180.0 + (180.0 * (atan2(direction.x, direction.z) / M_PI))),
        0.0f);
    mbev_CapTeresaFadeCreate(objectId);
    mbPlayerRotSet(playerNo, 0.0f, angle, 0.0f);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (i != playerNo) {
            mbPlayerDispSet(i, FALSE);
        }
    }
    cameraRot = teresaCameraRot;
    cameraRot.y = (float)(180.0
        + (180.0 * (atan2(direction.x, direction.z) / M_PI)));
    mbCameraMovePlayer(playerNo, &cameraRot, &teresaCameraOfs, 1500.0f,
        -1.0f, -1);
    mbCameraMoveWait();
    mbMusBoardFadeOut(0, 0, 1000, 1000, 32, FALSE);
    mbWipeDissolveFadeIn();

    if (!GwSystem.curTime) {
        mbAudFXPlay(0x39D);
        mbWinCreate(2, 0x003B0000, 10);
        mbWinTopWait();
    } else if (mbPlayerCoinGet(playerNo) < 5) {
        mbAudFXPlay(0x39D);
        mbWinCreate(2, 0x003B0001, 10);
        mbWinTopWait();
    } else {
        coinTargetNum = 0;
        starTargetNum = 0;
        if (mbPlayerCoinGet(playerNo) >= 5) {
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (!mbev_CapPlayerCheck(i, playerNo)
                    && mbPlayerCoinGet(i) > 0) {
                    coinTargetNum++;
                }
            }
        }
        if (mbPlayerCoinGet(playerNo) >= 40) {
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (!mbev_CapPlayerCheck(i, playerNo)
                    && mbPlayerStarGet(i) > 0) {
                    starTargetNum++;
                }
            }
        }
        targetNum = 0;
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (i != playerNo) {
                targetPlayers[targetNum++] = i;
            }
        }
        enabledNum = 0;
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (!mbev_CapPlayerCheck(i, playerNo)
                && mbPlayerStarGet(i) > 0) {
                enabledNum++;
            }
        }

        if (coinTargetNum <= 0 && starTargetNum <= 0
            && mbPlayerCoinGet(playerNo) < 40 && enabledNum >= 1) {
            mbAudFXPlay(0x39D);
            mbWinCreate(2, 0x003B0001, 10);
            mbWinTopWait();
        } else if (coinTargetNum <= 0 && starTargetNum <= 0) {
            mbAudFXPlay(0x39D);
            mbWinCreate(2, 0x003B0002, 10);
            mbWinTopWait();
        } else {
            mbAudFXPlay(0x39D);
            mbWinCreate(2, teresaStealMesId == -1
                    ? 0x003B0003 : 0x003B0004,
                10);
            mbWinTopWait();

            for (;;) {
                if (teresaStealMesId == -1) {
                    mbWinCreateChoice(1, 0x003B0005, 10, 0);
                    if (coinTargetNum == 0) {
                        mbWinTopChoiceDisable(0);
                    }
                    if (starTargetNum == 0) {
                        mbWinTopChoiceDisable(1);
                    }
                    if (GwPlayer[playerNo].comF) {
                        mbComChoiceListDownSet(
                            coinTargetNum != 0 && starTargetNum != 0);
                    }
                    mbWinTopWait();
                    stealType = mbWinTopChoiceGet();
                    if (stealType == 2 || stealType == -1) {
                        mbWinCreate(2, 0x003B000F, 10);
                        mbWinTopWait();
                        stealType = -1;
                        break;
                    }
                } else {
                    mbWinCreateChoice(1, 0x003B0006, 10, 0);
                    sprintf(customMes, "%d", teresaStealCoinNum);
                    mbWinTopInsertMesSet(teresaStealMesId, 0);
                    mbWinTopInsertMesSet((u32)customMes, 1);
                    if (coinTargetNum == 0) {
                        mbWinTopChoiceDisable(0);
                    }
                    if (starTargetNum == 0) {
                        mbWinTopChoiceDisable(1);
                    }
                    if (mbPlayerCoinGet(playerNo) < teresaStealCoinNum) {
                        mbWinTopChoiceDisable(2);
                    }
                    if (GwPlayer[playerNo].comF) {
                        mbComChoiceListDownSet(
                            coinTargetNum != 0 && starTargetNum != 0);
                    }
                    mbWinTopWait();
                    stealType = mbWinTopChoiceGet();
                    if (stealType == 3 || stealType == -1) {
                        mbWinCreate(2, 0x003B000F, 10);
                        mbWinTopWait();
                        stealType = -1;
                        break;
                    }
                    if (stealType == 2) {
                        break;
                    }
                }

                mbWinCreateChoice(1, 0x003B0007, 10, 0);
                for (i = 0; i < targetNum; i++) {
                    mbWinTopInsertMesSet(
                        mbPlayerNameMesGet(targetPlayers[i]), i);
                    enabledPlayers[i] = targetPlayers[i];
                    if ((stealType == 0
                            && mbPlayerCoinGet(targetPlayers[i]) <= 0)
                        || (stealType == 1
                            && mbPlayerStarGet(targetPlayers[i]) <= 0)
                        || (GwSystem.tagF
                            && mbev_CapPlayerCheck(
                                playerNo, targetPlayers[i]))) {
                        mbWinTopChoiceDisable(i);
                        enabledPlayers[i] = -1;
                    }
                }
                if (GwPlayer[playerNo].comF) {
                    mbComChoiceListDownSet(mbev_CapPlayerComSelKettouGet(
                        playerNo, stealType, enabledPlayers, targetNum));
                }
                mbWinTopWait();
                choice = mbWinTopChoiceGet();
                if (choice == -1) {
                    continue;
                }
                if (choice < targetNum) {
                    targetPlayer = targetPlayers[choice];
                } else {
                    j = mbRandMod(targetNum);
                    targetPlayer = -1;
                    for (i = 0; i < targetNum; i++) {
                        if (enabledPlayers[j] >= 0) {
                            targetPlayer = enabledPlayers[j];
                            break;
                        }
                        if (++j >= targetNum) {
                            j = 0;
                        }
                    }
                }
                if (targetPlayer >= 0) {
                    break;
                }
            }

            if (stealType >= 0) {
                if (teresaStealMesId != -1 && stealType == 2) {
                    mbCoinAddExec(playerNo, -teresaStealCoinNum);
                } else if (stealType == 0) {
                    mbCoinAddExec(playerNo, -5);
                } else {
                    mbCoinAddExec(playerNo, -40);
                }
                mbAudFXPlay(0x39D);
                mbWinCreate(2, 0x003B0008, 10);
                mbWinTopWait();

                i = 1;
                for (;;) {
                    if ((float)i > 60.0f) {
                        break;
                    }
                    mbev_CapTeresaFadeSet(
                        255.0f * (1.0f - ((float)i / 60.0f)));
                    HuPrcVSleep();
                    i++;
                }
                mbev_CapTeresaFadeSet(0.0f);

                if (teresaStealMesId != -1 && stealType == 2) {
                    if (teresaStealHook != NULL) {
                        teresaStealHook(TRUE);
                    } else {
                        mbWipeDissolveFadeOutTime(1);
                    }
                    if (teresaStealBeginHook != NULL) {
                        teresaStealBeginHook(playerNo, objectId);
                    }
                } else {
                    mbWipeDissolveFadeOutTime(1);
                    capsuleIndex = -1;
                    for (i = 0; i < mbPlayerCapsuleMaxGet(); i++) {
                        if (mbPlayerCapsuleGet(targetPlayer, i) == 0x1F) {
                            capsuleIndex = i;
                        }
                    }
                    targetPos.x = targetPos.y = targetPos.z = 0.0f;
                    mbPlayerPosGet(targetPlayer, &targetStartPos);
                    mbev_PlayerColMasu(targetPlayer,
                        GwPlayer[targetPlayer].masuId, TRUE);
                    for (i = 0; i < GW_PLAYER_MAX; i++) {
                        mbPlayerDispSet(i, i == targetPlayer);
                    }
                    mbCameraPlayerViewSetFast(targetPlayer, 0);
                    mbCameraMoveWait();
                    mbMasuPosGet(GwPlayer[targetPlayer].masuId, &cameraRot);
                    cameraRot.y += 300.0f;
                    if (capsuleIndex != -1) {
                        cameraRot.y -= 150.0f;
                        cameraRot.z -= 200.0f;
                    }
                    mbObjPosSetV(objectId, &cameraRot);
                    mbObjRotSet(objectId, 0.0f, 0.0f, 0.0f);
                    mbObjDispSet(objectId, FALSE);

                    idleMotion = mbev_CapPlayerMotionCreate(&work->objWork,
                        targetPlayer, 0x00930017);
                    stealMotion = mbev_CapPlayerMotionCreate(&work->objWork,
                        targetPlayer, 0x00930022);
                    starMotion = mbev_CapPlayerMotionCreate(&work->objWork,
                        targetPlayer, 0x0093006E);
                    mbPlayerMotionShiftSet(targetPlayer, idleMotion, 0.0f,
                        8.0f, 0x40000001);
                    itemObjectId = mbev_CapObjCreate(&work->objWork,
                        0x000C0049, NULL, FALSE, 0, FALSE);
                    hookName = CharModelItemHookGet(
                        GwPlayer[targetPlayer].charNo, 4, 0);
                    mbObjHookSet(mbPlayerObjIDGet(targetPlayer), hookName,
                        itemObjectId);
                    mbObjDispSet(itemObjectId, FALSE);

                    for (i = 0; i < HU3D_GLIGHT_MAX; i++) {
                        lightColors[i] = Hu3DGlobalLight[i].color;
                        if (Hu3DGlobalLight[i].type != -1) {
                            Hu3DGlobalLight[i].color.r *= 0.5f;
                            Hu3DGlobalLight[i].color.g *= 0.5f;
                            Hu3DGlobalLight[i].color.b *= 0.5f;
                        }
                    }
                    lightId = -1;
                    mbStarObjDispSetAll(FALSE);
                    mbWipeDissolveFadeIn();

                    if (capsuleIndex != -1) {
                        mbWinCreate(2, 0x003B000A, -1);
                        mbWinTopInsertMesSet(
                            mbPlayerNameMesGet(targetPlayer), 0);
                        mbWinTopPlayerDisable(targetPlayer);
                        mbWinTopWait();
                        mbObjLayerSet(itemObjectId, 5);
                        mbPlayerLayerSet(targetPlayer, 5);
                        mbObjDispSet(objectId, TRUE);
                        i = 1;
                        for (;;) {
                            if ((float)i > 60.0f) {
                                break;
                            }
                            mbev_CapTeresaFadeSet(
                                255.0f * ((float)i / 60.0f));
                            HuPrcVSleep();
                            i++;
                        }
                        mbev_CapTeresaFadeSet(255.0f);
                        mbPlayerRotateStart(targetPlayer, 180, 15);
                        while (!mbPlayerRotateCheck(targetPlayer)) {
                            HuPrcVSleep();
                        }
                        omVibrate(targetPlayer, 20, 7, 3);
                        mbPlayerMotionShiftSet(targetPlayer, starMotion,
                            0.0f, 8.0f, 0);
                        mbObjDispSet(itemObjectId, TRUE);
                        for (i = 1; (float)i < 18.0f; i++) {
                            time = (float)i / 18.0f;
                            mbObjScaleSet(itemObjectId, time, time, time);
                            HuPrcVSleep();
                        }
                        mbObjScaleSet(itemObjectId, 1.0f, 1.0f, 1.0f);
                        while (!mbPlayerMotionEndCheck(targetPlayer)) {
                            HuPrcVSleep();
                        }
                        lightId = Hu3DLLightCreateV(
                            mbObjModelIDGet(objectId), &teresaLightPos,
                            &teresaLightDir, &teresaLightColor);
                        Hu3DLLightSpotSet(mbObjModelIDGet(objectId), lightId,
                            GX_SP_SHARP, 0.001f);
                        Hu3DLLightStaticSet(
                            mbObjModelIDGet(objectId), lightId, TRUE);
                        Hu3DLLightInfinitytSet(
                            mbObjModelIDGet(objectId), lightId);
                        Hu3DMotionCalc(mbObjModelIDGet(
                            mbPlayerObjIDGet(playerNo)));
                        hookName = CharModelItemHookGet(
                            GwPlayer[playerNo].charNo, 4, 0);
                        Hu3DModelObjMtxGet(mbObjModelIDGet(
                                mbPlayerObjIDGet(playerNo)),
                            hookName, hookMtx);
                        objectPos.x = hookMtx[0][3];
                        objectPos.y = hookMtx[1][3];
                        objectPos.z = hookMtx[2][3];
                        lightAim = objectPos;
                        lightAim.y -= 100.0f;
                        lightAim.z += 200.0f;
                        Hu3DLLightPosAimSetV(mbObjModelIDGet(objectId),
                            lightId, &objectPos, &lightAim);
                        Hu3DLLightPosAngleSet(mbObjModelIDGet(objectId),
                            lightId, objectPos.x, objectPos.y, objectPos.z,
                            -45.0f, 0.0f);
                        mbObjMotionShiftSet(objectId, 3, 0.0f, 8.0f,
                            0x40000001);
                        HuPrcSleep(180);
                        mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
                        musicChanged = TRUE;
                        stealType = -1;
                    } else if (stealType == 0) {
                        mbWinCreate(2, 0x003B0009, -1);
                        mbWinTopInsertMesSet(
                            mbPlayerNameMesGet(targetPlayer), 0);
                        mbWinTopPlayerDisable(targetPlayer);
                        mbWinTopWait();
                        mbObjDispSet(objectId, TRUE);
                        i = 1;
                        for (;;) {
                            if ((float)i > 60.0f) {
                                break;
                            }
                            mbev_CapTeresaFadeSet(
                                255.0f * ((float)i / 60.0f));
                            HuPrcVSleep();
                            i++;
                        }
                        mbev_CapTeresaFadeSet(255.0f);
                        mbPlayerMotionShiftSet(targetPlayer, stealMotion,
                            0.0f, 8.0f, 0x40000001);
                        mbObjMotionShiftSet(objectId, 2, 0.0f, 8.0f,
                            0x40000001);
                        helpWin = mbWinCreateHelp(0x003B0011);
                        mbPlayerColSnapPlayerSet(targetPlayer, FALSE);
                        for (i = 0; (float)i < 30.0f; i++) {
                            weight = (float)i / 30.0f;
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &objectPos);
                            objectPos.y += 100.0f
                                * (3.0f - (2.0f * weight));
                            alpha = 255.0f - (127.0f * weight);
                            mbObjPosSetV(objectId, &objectPos);
                            mbev_CapTeresaFadeSet(alpha);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 50.0f * weight;
                            mbPlayerPosSetV(targetPlayer, &targetPos);
                            HuPrcVSleep();
                        }
                        pressNum = 0;
                        for (i = 0; (float)i < 120.0f; i++) {
                            weight = (float)i / 120.0f;
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &objectPos);
                            objectPos.y += 100.0f
                                + (20.0f * (float)sin((M_PI
                                    * (1440.0f * weight)) / 180.0));
                            mbObjPosSetV(objectId, &objectPos);
                            mbev_CapTeresaFadeSet(alpha);
                            if (HuPadBtnDown[GwPlayer[targetPlayer].padNo]
                                & PAD_BUTTON_A) {
                                pressNum++;
                                if (alpha > 0) {
                                    alpha--;
                                }
                            }
                            alpha += (int)(2.5f * (float)sin((M_PI
                                * (360.0f * weight)) / 180.0));
                            if (alpha < 64 && (i & 7) == 0) {
                                alpha++;
                            }
                            if (alpha < 32) {
                                alpha = 32;
                            } else if (alpha > 255) {
                                alpha = 255;
                            }
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 50.0f;
                            mbPlayerPosSetV(targetPlayer, &targetPos);
                            HuPrcVSleep();
                        }
                        mbWinKill(helpWin);
                        for (i = 0; (float)i < 30.0f; i++) {
                            weight = 1.0f - ((float)i / 30.0f);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &objectPos);
                            objectPos.y += 100.0f
                                + (100.0f * (2.0f - (2.0f * weight)));
                            mbObjPosSetV(objectId, &objectPos);
                            if (alpha < 255) {
                                alpha += 25;
                            }
                            if (alpha > 255) {
                                alpha = 255;
                            }
                            mbev_CapTeresaFadeSet(alpha);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 50.0f * weight;
                            mbPlayerPosSetV(targetPlayer, &targetPos);
                            HuPrcVSleep();
                        }
                        mbPlayerMotionShiftSet(targetPlayer, 6, 0.0f,
                            8.0f, 0x40000001);
                        mbPlayerColSnapPlayerSet(targetPlayer, TRUE);
                        mbObjMotionShiftSet(objectId, 1, 0.0f, 8.0f,
                            0x40000001);
                        if (GwSystem.turnNo <= 10) {
                            turnCoinMax = 20;
                        } else if (GwSystem.turnNo <= 20) {
                            turnCoinMax = 25;
                        } else if (GwSystem.turnNo <= 30) {
                            turnCoinMax = 30;
                        } else if (GwSystem.turnNo <= 40) {
                            turnCoinMax = 35;
                        } else {
                            turnCoinMax = 40;
                        }
                        if (!GwPlayer[targetPlayer].comF) {
                            stealRate = 1.0f - (pressNum * 0.03125f);
                        } else {
                            randomValue = (float)mbRandMod(0x10000000)
                                * 3.7252903e-9f;
                            stealRate = 1.0f
                                - (0.1f + (GwPlayer[targetPlayer].comDif
                                    * (0.2f + (0.1f * randomValue))));
                        }
                        if (stealRate < 0.1f) {
                            stealRate = 0.1f;
                        } else if (stealRate > 1.0f) {
                            stealRate = 1.0f;
                        }
                        coinNum = stealRate * turnCoinMax;
                        if (coinNum > mbPlayerCoinGet(targetPlayer)) {
                            coinNum = mbPlayerCoinGet(targetPlayer);
                        }
                        omVibrate(targetPlayer, 20, 7, 3);
                        mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                            &objectPos);
                        for (i = 0; i < coinNum; i++) {
                            launchAngle = 360.0f
                                * ((float)mbRandMod(0x10000000)
                                    * 3.7252903e-9f);
                            coinPos = objectPos;
                            coinPos.y += 100.0f;
                            launchElevation = 70.0f
                                + (15.0f * ((float)mbRandMod(0x10000000)
                                    * 3.7252903e-9f));
                            launchScale = 0.8f
                                + (0.3f * ((float)mbRandMod(0x10000000)
                                    * 3.7252903e-9f));
                            coinVel.x = (float)(65.0f * launchScale
                                * sin((M_PI * launchAngle) / 180.0)
                                * cos((M_PI * launchElevation) / 180.0));
                            coinVel.y = (float)(65.0f * launchScale
                                * sin((M_PI * launchElevation) / 180.0));
                            coinVel.z = (float)(65.0f * launchScale
                                * cos((M_PI * launchAngle) / 180.0)
                                * cos((M_PI * launchElevation) / 180.0));
                            coinEffect = mbev_CapEffCoinAdd(work->coinObj,
                                &coinPos, &coinVel, 0.75f, 4.9f, 30, 4);
                            if (coinEffect >= 0) {
                                mbev_CapEffCoinMaxYSet(work->coinObj,
                                    coinEffect, objectPos.y + 300.0f);
                            }
                            mbPlayerCoinAdd(targetPlayer, -1);
                            mbAudFXPlay(14);
                            HuPrcVSleep();
                        }
                        mbAudFXPlay(15);
                        while (mbev_CapEffCoinNumGet(work->coinObj) > 0) {
                            HuPrcVSleep();
                        }
                    } else {
                        mbWinCreate(2, 0x003B000A, -1);
                        mbWinTopInsertMesSet(
                            mbPlayerNameMesGet(targetPlayer), 0);
                        mbWinTopPlayerDisable(targetPlayer);
                        mbWinTopWait();
                        mbObjDispSet(objectId, TRUE);
                        i = 1;
                        for (;;) {
                            if ((float)i > 60.0f) {
                                break;
                            }
                            mbev_CapTeresaFadeSet(
                                255.0f * ((float)i / 60.0f));
                            HuPrcVSleep();
                            i++;
                        }
                        mbev_CapTeresaFadeSet(255.0f);
                        starObjectId = mbStarObjCreate();
                        mbStarObjDispSet(starObjectId, FALSE);
                        mbPlayerMotionShiftSet(targetPlayer, stealMotion,
                            0.0f, 8.0f, 0x40000001);
                        mbObjMotionShiftSet(objectId, 2, 0.0f, 8.0f,
                            0x40000001);
                        mbPlayerColSnapPlayerSet(targetPlayer, FALSE);
                        for (i = 0; (float)i < 30.0f; i++) {
                            weight = (float)i / 30.0f;
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &objectPos);
                            objectPos.y += 100.0f
                                * (3.0f - (2.0f * weight));
                            alpha = 255.0f - (127.0f * weight);
                            mbObjPosSetV(objectId, &objectPos);
                            mbev_CapTeresaFadeSet(alpha);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 50.0f * weight;
                            mbPlayerPosSetV(targetPlayer, &targetPos);
                            HuPrcVSleep();
                        }
                        for (i = 0; (float)i < 30.0f; i++) {
                            weight = 1.0f - ((float)i / 30.0f);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &objectPos);
                            objectPos.y += 100.0f
                                + (100.0f * (2.0f - (2.0f * weight)));
                            mbObjPosSetV(objectId, &objectPos);
                            if (alpha < 255) {
                                alpha += 25;
                            }
                            if (alpha > 255) {
                                alpha = 255;
                            }
                            mbev_CapTeresaFadeSet(alpha);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 50.0f * weight;
                            mbPlayerPosSetV(targetPlayer, &targetPos);
                            HuPrcVSleep();
                        }
                        mbPlayerMotionShiftSet(targetPlayer, 6, 0.0f,
                            8.0f, 0x40000001);
                        mbPlayerColSnapPlayerSet(targetPlayer, TRUE);
                        mbPlayerStarAdd(targetPlayer, -1);
                        omVibrate(targetPlayer, 20, 20, 0);
                        for (i = 0; (float)i <= 60.0f; i++) {
                            weight = (float)sin((M_PI
                                * (90.0f * ((float)i / 60.0f))) / 180.0);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 300.0f * weight;
                            mbStarObjPosSetV(starObjectId, &targetPos);
                            mbStarObjRotSet(starObjectId, 0.0f,
                                360.0f * weight, 0.0f);
                            mbStarObjScaleSet(
                                starObjectId, weight, weight, weight);
                            mbStarObjDispSet(starObjectId, TRUE);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &objectPos);
                            objectPos.y += 300.0f;
                            objectPos.z -= 200.0f * weight;
                            mbObjPosSetV(objectId, &objectPos);
                            HuPrcVSleep();
                        }
                        for (i = 0; (float)i <= 60.0f; i++) {
                            weight = (float)sin((M_PI
                                * (90.0f * ((float)i / 60.0f))) / 180.0);
                            mbMasuPosGet(GwPlayer[targetPlayer].masuId,
                                &targetPos);
                            targetPos.y += 300.0f + (500.0f * weight);
                            mbStarObjPosSetV(starObjectId, &targetPos);
                            mbStarObjRotSet(starObjectId, 0.0f,
                                720.0f * weight, 0.0f);
                            HuPrcVSleep();
                        }
                        mbStarObjKill(starObjectId);
                    }

                    mbWipeDissolveFadeOutTime(1);
                    mbStarObjDispSetAll(TRUE);
                    for (i = 0; i < HU3D_GLIGHT_MAX; i++) {
                        if (Hu3DGlobalLight[i].type != -1) {
                            Hu3DGlobalLight[i].color = lightColors[i];
                        }
                    }
                    if (lightId != -1) {
                        Hu3DLLightKill(mbObjModelIDGet(objectId), lightId);
                    }
                    if (capsuleIndex != -1) {
                        mbPlayerCapsuleRemove(targetPlayer, capsuleIndex);
                        GwPlayer[targetPlayer].capsuleUseNum++;
                    }
                    mbPlayerLayerSet(targetPlayer, 3);
                    hookName = CharModelItemHookGet(
                        GwPlayer[targetPlayer].charNo, 4, 0);
                    mbObjHookObjReset(
                        mbPlayerObjIDGet(targetPlayer), hookName);
                    mbObjDispSet(itemObjectId, FALSE);
                    mbPlayerPosSetV(targetPlayer, &targetStartPos);
                    mbPlayerRotSet(targetPlayer, 0.0f, 0.0f, 0.0f);
                    mbPlayerMotionSet(targetPlayer, 1, 0x40000001);
                }

                mbObjMotionShiftSet(objectId, 1, 0.0f, 8.0f,
                    0x40000001);
                PSVECSubtract(&objectPos, &playerPos, &direction);
                mbObjPosSetV(objectId, &objectPos);
                mbObjRotSet(objectId, 0.0f,
                    (float)(180.0 + (180.0
                        * (atan2(direction.x, direction.z) / M_PI))),
                    0.0f);
                mbev_CapTeresaFadeSet(0.0f);
                mbev_PlayerColMasu(
                    playerNo, GwPlayer[playerNo].masuId, TRUE);
                mbPlayerRotSet(playerNo, 0.0f,
                    (float)(180.0
                        * (atan2(direction.x, direction.z) / M_PI)),
                    0.0f);
                for (i = 0; i < GW_PLAYER_MAX; i++) {
                    mbPlayerDispSet(i, i == playerNo);
                }
                cameraRot = teresaCameraRot;
                cameraRot.y = (float)(180.0 + (180.0
                    * (atan2(direction.x, direction.z) / M_PI)));
                mbCameraMovePlayer(playerNo, &cameraRot, &teresaCameraOfs,
                    1500.0f, -1.0f, -1);
                mbCameraMoveWait();
                if (teresaStealMesId != -1 && stealType == 2) {
                    if (teresaStealHook != NULL) {
                        teresaStealHook(FALSE);
                    } else {
                        mbWipeDissolveFadeIn();
                    }
                } else {
                    mbWipeDissolveFadeIn();
                }
                i = 1;
                for (;;) {
                    if ((float)i > 60.0f) {
                        break;
                    }
                    mbev_CapTeresaFadeSet(
                        255.0f * ((float)i / 60.0f));
                    HuPrcVSleep();
                    i++;
                }
                mbev_CapTeresaFadeSet(255.0f);

                switch (stealType) {
                    case 0:
                        mbAudFXPlay(0x39D);
                        mbWinCreate(2, 0x003B000B, 10);
                        mbWinTopWait();
                        mbPlayerRotateStart(playerNo,
                            (s16)(180.0 + (180.0
                                * (atan2(direction.x, direction.z) / M_PI))),
                            15);
                        while (!mbPlayerRotateCheck(playerNo)) {
                            HuPrcVSleep();
                        }
                        mbev_CapCoinAdd(
                            work->coinObj, playerNo, coinNum, TRUE);
                        mbAudFXPlay(0x39D);
                        mbWinCreate(2, 0x003B000D, 10);
                        mbWinTopWait();
                        break;
                    case 1:
                        mbAudFXPlay(0x39D);
                        mbWinCreate(2, 0x003B000C, 10);
                        mbWinTopWait();
                        mbMusFadeOutSpeed(1, 1000);
                        while (mbMusCheck(1)) {
                            HuPrcVSleep();
                        }
                        mbPlayerRotateStart(playerNo,
                            (s16)(180.0 + (180.0
                                * (atan2(direction.x, direction.z) / M_PI))),
                            15);
                        while (!mbPlayerRotateCheck(playerNo)) {
                            HuPrcVSleep();
                        }
                        mbStarGetExec(playerNo);
                        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f,
                            0x40000001);
                        mbMusBoardPlay();
                        musicChanged = TRUE;
                        mbAudFXPlay(0x39D);
                        mbWinCreate(2, 0x003B000D, 10);
                        mbWinTopWait();
                        break;
                    case 2:
                        mbAudFXPlay(0x39D);
                        mbWinCreate(2, 0x003B000E, 10);
                        mbWinTopWait();
                        break;
                    default:
                        mbAudFXPlay(0x39E);
                        mbWinCreate(2, 0x003B0010, 10);
                        mbWinTopWait();
                        mbPlayerRotateStart(playerNo,
                            (s16)(180.0 + (180.0
                                * (atan2(direction.x, direction.z) / M_PI))),
                            15);
                        while (!mbPlayerRotateCheck(playerNo)) {
                            HuPrcVSleep();
                        }
                        mbev_CapPlayerMotShiftWait(playerNo, 13, 0, TRUE);
                        break;
                }
            }
        }
    }

    if (!musicChanged) {
        mbMusBoardFadeOut(0, 0, 1000, 1000, -1, FALSE);
    }
    mbWipeDissolveFadeOutTime(1);
    mbObjDispSet(objectId, FALSE);
    mbev_CapTeresaFadeKill(objectId);
    mbPlayerRotSet(playerNo, 0.0f, 0.0f, 0.0f);
    mbPlayerMotionSet(playerNo, 1, 0x40000001);
    if (!mbMasuDispCheck(GwPlayer[playerNo].masuId)
        || GwPlayer[playerNo].moveNum > 1) {
        mbCameraPlayerViewSetFast(playerNo, 2);
    } else {
        mbCameraPlayerViewSetFast(playerNo, 0);
    }
    mbCameraMoveWait();
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerDispSet(i, TRUE);
    }
    HuDataDirClose(0x00130000);
    HuPrcEnd();
}

void mbev_CapTeresaKill(void)
{
}

void mbev_CapTeresaStealSet(int mesId, int coinNum, TERESA_STEAL_BEGIN_HOOK beginHook,
    TERESA_STEAL_HOOK hook)
{
    teresaStealMesId = mesId;
    teresaStealCoinNum = coinNum;
    teresaStealBeginHook = beginHook;
    teresaStealHook = hook;
}

void mbev_CapTeresaFadeCreate(int objectId)
{
    int modelId;
    HU3D_MODEL *model;
    HSF_DATA *hsf;
    HSF_MATERIAL *material;
    int i;
    int mallocNo;
    u32 textureSize;
    int textureMallocNo;
    TERESA_FADE_WORK *workData;
    TERESA_FADE_WORK *work;
    void *textureData;
    void *texture;

    modelId = mbObjModelIDGet(objectId);
    model = &Hu3DData[modelId];
    hsf = model->hsf;
    material = hsf->material;
    Hu3DModelMatHookSet(modelId, ev_CapTeresaFadeMatHook);
    for (i = 0; i < hsf->materialNum; i++, material++) {
        material->flags |= HSF_MATERIAL_MATHOOK;
    }

    mallocNo = model->mallocNo;
    workData = HuMemDirectMallocNum(
        HEAP_MODEL, sizeof(TERESA_FADE_WORK), mallocNo);
    work = workData;
    teresaFadeWork = work;
    memset(teresaFadeWork, 0, sizeof(TERESA_FADE_WORK));
    teresaFadeWork->activeF = TRUE;
    teresaFadeWork->alpha = 255.0f;
    teresaFadeWork->copyF = FALSE;
    teresaFadeWork->screenWidth = 640;
    teresaFadeWork->screenHeight = 480;
    teresaFadeWork->textureWidth = 320;
    teresaFadeWork->textureHeight = 240;
    teresaFadeWork->object = omAddObjEx(
        mbObjMan, -32768, 0, 0, OM_GRP_NONE, ev_CapTeresaFadeOMExec);
    teresaFadeWork->textureSize = GXGetTexBufferSize(
        teresaFadeWork->textureWidth, teresaFadeWork->textureHeight,
        GX_TF_RGB565, GX_FALSE, 0);
    textureMallocNo = model->mallocNo;
    textureSize = teresaFadeWork->textureSize;
    textureData = HuMemDirectMallocNum(
        HEAP_MODEL, textureSize, textureMallocNo);
    texture = textureData;
    teresaFadeWork->textureData = texture;
    memset(teresaFadeWork->textureData, 0, teresaFadeWork->textureSize);
    DCFlushRange(teresaFadeWork->textureData, teresaFadeWork->textureSize);
}

void mbev_CapTeresaFadeKill(int objectId)
{
    Hu3DModelMatHookSet(mbObjModelIDGet(objectId), NULL);
    if (teresaFadeWork) {
        HuMemDirectFree(teresaFadeWork->textureData);
        HuMemDirectFree(teresaFadeWork);
        teresaFadeWork = NULL;
    }
}

static void ev_CapTeresaFadeOMExec(OMOBJ *obj)
{
    if (mbExitCheck() || !teresaFadeWork) {
        omDelObjEx(mbObjMan, obj);
    } else {
        teresaFadeWork->copyF = FALSE;
    }
}

void mbev_CapTeresaFadeSet(float alpha)
{
    if (teresaFadeWork) {
        if (alpha < 0.0f) {
            alpha = 0.0f;
        }
        if (alpha > 255.0f) {
            alpha = 255.0f;
        }
        teresaFadeWork->alpha = alpha;
    }
}

void mbev_CapMiracleKill(void)
{
}

static int ev_CapMiracleMesGet(int messNo)
{
    if (GwSystem.curTime == FALSE) {
        return messNo;
    }
    return messNo + 16;
}

static void ev_CapMiracleDiceHitHook(void)
{
    diceHitTimer = 0;
}

static void ev_CapMiracleSprDestroy(void)
{
    miracleSprObj = NULL;
}

static void ev_CapMiracleTradeFocusSet(void)
{
    int i;
    OMOBJ *obj = miracleSprObj;
    MIRACLE_SPR_WORK *work;

    if (miracleSprObj != NULL) {
        work = obj->data;
        for (i = 0; i < 6; i++, work++) {
            if (work->activeF) {
                work->focusTime = 32;
                work->focusNo = 0;
            }
        }
    }
}

static void ev_CapMiracleTradeHideSet(void)
{
    int i;
    OMOBJ *obj = miracleSprObj;
    MIRACLE_SPR_WORK *work;

    if (miracleSprObj != NULL) {
        work = obj->data;
        for (i = 0; i < 6; i++, work++) {
            if (work->activeF) {
                work->hideF = TRUE;
            }
        }
    }
}

void mbev_CapKettouKill(void)
{
}

static int ev_CapKettouMesGet(int messNo)
{
    if (GwSystem.curTime == FALSE) {
        return messNo;
    }
    return messNo + 23;
}

void mbev_CapDonkeyKill(void)
{
}

void mbev_CapKoopaKill(void)
{
}

static int ev_CapKoopaDicePadBtnHook(void)
{
    if (--diceHitTimer <= 0) {
        return PAD_BUTTON_A;
    }
    return 0;
}

static void ev_CapKoopaDiceMotHook(void)
{
    int i;

    if (koopaMdlId != -1) {
        mbObjMotionSet(koopaMdlId, 4, 0);
        i = 0;
        do {
            if (i++ == 27) {
                mbDiceObjHit(-1);
            }
            HuPrcVSleep();
        } while (!mbObjMotionEndCheck(koopaMdlId));
        mbObjMotionSet(koopaMdlId, 1, HU3D_MOTATTR_LOOP);
    }
}
