#define _MATH_H
#include "dolphin/math.h"
#include "game/board/opening.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/coin.h"
#include "game/board/comchoice.h"
#include "game/board/effect.h"
#include "game/board/main.h"
#include "game/board/object.h"
#include "game/board/player.h"
#include "game/board/status.h"
#include "game/board/window.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/wipe.h"

#include "dolphin/mtx.h"
#include "dolphin/os.h"
#include "dolphin/pad.h"

extern void mbMasuPosGet(s16 id, HuVecF *pos);
extern s16 mbMasuFind_AttrIdGet(s16 id, u16 attr);
extern s16 mbMasuFind_TypeListGet2(s16 id, s16 type, BOOL hookF, BOOL dispF, s16 *list);
extern float mbHermiteCalcSlope(float a, float b, float c, float d, float t);
extern void mbHermiteCalcV(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d,
    HuVecF *dst, float t);
extern void mbDiceExec(int playerNo, int type, void *pos, int value, BOOL flag1,
    BOOL flag2, void *hook, int color);
extern void mbDicePadBtnHookSet(int playerNo, u16 (*hook)(int playerNo));
extern BOOL mbDiceKillCheckAll(void);
extern void mbDiceNumShrinkSet(int playerNo);
extern void mbDirClose(void);
extern OMOBJ *mbGuideCreateFlag(HuVecF *pos, s8 *motTbl, BOOL screenF,
    BOOL altMtxF, BOOL layerF);
extern void mbGuideKill(OMOBJ *obj);
extern MBMODELID mbGuideModelGet(OMOBJ *obj);
extern void mbGuideMotionNextSet(OMOBJ *obj, s16 motNo);
extern void mbGuideMotionSet(OMOBJ *obj, s16 motNo, BOOL shiftF);
extern void mbPlayerColSnapSet(BOOL enable);
extern void mbPlayerSwap(int playerNo1, int playerNo2);
extern BOOL mbPlayerRotateCheckAll(void);
extern u32 mbPlayerNameMesGet(int playerNo);
extern void mbTelopCreate(int playerNo, int telopNo, BOOL statF);
extern BOOL mbTelopCheck(void);
extern void mbStarMapViewProcExec(void);
extern void mbWipeFadeIn(void);
extern void mbWipeFadeOut(void);
extern void mbWipeWait(void);

typedef struct OpeningCoinWork_s {
    s16 coinId;
    u16 pad02;
    HuVecF pos;
    HuVecF rot;
    HuVecF vel;
    BOOL dispF;
    int playerNo;
    u32 unk30[3];
} OPENINGCOINWORK;

typedef struct SingleGuideWork_s {
    BOOL dispF;
    BOOL endF;
} SINGLEGUIDEWORK;

static int guideMotFileTbl[] = {
    0x00110001, 0x00110002, 0x00110003, 0x00110004,
    0x00110005, 0x00110006, 0x00110007, 0x00110008,
    0x00110009, 0x0011000A, 0x0011000B, 0x0011000C,
    0x0011000D, 0x0011000E, 0x0011000F, 0x00110010,
    0x00110011, 0x00110012, 0x00110013, 0x00110014,
    0x00110015, 0x00110016, 0x00110017, -1
};

static u32 welcomeMesTbl[] = {
    0x002C0000, 0x002C0001, 0x002C0002,
    0x002C0003, 0x002C0004, 0x002C0005
};

static HuVecF lbl_80248620[] = {
    { -28.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 500.0f }
};

static float playerCenterDist[] = { -3.0f, -1.0f, 1.0f, 3.0f };

static char lbl_80248648[] =
    "/-------------------------SINGLE_DATA_KILL-------------------------/\n";

static int lbl_80248690[] = { 45, 46, 47 };

static HuVecF singleGuidePosTbl[] = {
    { 0.0f, 0.0f, 0.0f },
    { -28.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 500.0f },
    { 0.0f, 250.0f, 0.0f }
};

static HuVecF singleGuideCameraRot = { -20.0f, 0.0f, 0.0f };

static HuVecF singleGuidePathOfsTbl[] = {
    { 800.0f, -100.0f, 2000.0f },
    { -700.0f, -100.0f, 0.0f },
    { -700.0f, 600.0f, 0.0f },
    { -400.0f, 600.0f, 0.0f },
    { -200.0f, 0.0f, 0.0f }
};

static int singleGuideMotFileTbl[] = {
    0x00110001, 0x00110004, 0x00110005, 0x00110006,
    0x00110007, 0x0011000A, 0x0011000C, 0x0011000F,
    0x00110011, 0x00110013, 0x00110014, -1
};

static u32 singleWelcomeMesTbl[] = {
    0x002C0006, 0x002C0007, 0x002C0008
};

static HuVecF openingRot;
static HuVecF openingPos;
static int openingPadDelay[GW_PLAYER_MAX];
static OPENINGCOINWORK openingCoinWork[GW_PLAYER_MAX * 10];
static HuVecF guideCurPos;
static HuVecF openingCameraRestoreRot = { -20.0f, 0.0f, 0.0f };
static HuVecF openingCameraRestorePos = { 0.0f, 250.0f, 0.0f };
static MBMODELID openingGuideObjId = -1;
static float openingZoom;
static BOOL singleGuideEffOnF;
static SINGLEGUIDEWORK singleGuideWork;
static OMOBJ *singleGuideObj;
static HuVecF *singleOpeningPosTbl2;
static HuVecF *singleOpeningPosTbl;
static float *singleOpeningZoomTbl;
static s16 singleWinId;
static HU3D_MODELID singleEff1MdlId;
static int singleFXNo;
static void (*openingInstHook)(void);
static void (*openingStarInstHook)(void);
static HUPROCESS *openingSingleProc;
static HUPROCESS *openingProc;

static void ev_OpeningParty(void);
static u16 OpeningPadBtn(int playerNo);
static void ev_OpeningPartyKill(void);
static void OpeningCoinExec(void);
static void PlayerDropExec(HuVecF *center);
static void PlayerOrderSet(int *order);
static void ev_OpeningSingle(void);
static void ev_OpeningSingleKill(void);
static void OpeningSingleEffHook(HU3D_MODEL *modelP, MBPARTICLE *particleP,
    Mtx mtx);

static float OpeningCurveEval(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, float t)
{
    HuVecF vec;

    vec.x = mbHermiteCalcSlope(a->x, b->x, c->x, d->x, t);
    vec.y = mbHermiteCalcSlope(a->y, b->y, c->y, d->y, t);
    vec.z = mbHermiteCalcSlope(a->z, b->z, c->z, d->z, t);
    return VECMag(&vec);
}

typedef float (*OPENINGCURVEEVALFUNC)();

static float OpeningCurveIntegrate(OPENINGCURVEEVALFUNC eval,
    HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d, float t)
{
    int i;
    int div;
    float sampleLength;
    float baseT;
    float sampleT;
    float deltaT;

    div = 10;
    baseT = 0.0f;
    deltaT = (t - baseT) / div;
    sampleT = baseT;
    sampleLength = 0.0f;
    for (i = 0; i < div - 1; i++) {
        sampleT += deltaT;
        sampleLength += eval(a, b, c, d, sampleT);
    }
    sampleLength = deltaT * 0.5
        * (eval(a, b, c, d, baseT) + eval(a, b, c, d, t)
            + (2.0 * sampleLength));
    return sampleLength;
}

static float OpeningCurveNewton(OPENINGCURVEEVALFUNC eval, HuVecF *a,
    HuVecF *b, HuVecF *c, HuVecF *d, float t, float distance, int maxStep)
{
    int step;
    float sampleLength;
    float pathLength;
    float oldT;
    float minLength;

    minLength = 0.1f;
    step = 0;
    do {
        pathLength = OpeningCurveIntegrate(eval, a, b, c, d, t) - distance;
        if (fabs(sampleLength = eval(a, b, c, d, t)) < minLength) {
            sampleLength = 1.0f;
        }
        oldT = t;
        t -= pathLength / sampleLength;
        step++;
    } while (t != oldT && step < maxStep);
    return t;
}

static float OpeningCurveLength(OPENINGCURVEEVALFUNC eval, HuVecF *a, HuVecF *b, HuVecF *c,
    HuVecF *d, float t)
{
    int div = 10;
    float baseT = 0.0f;
    float pathLength = 0.0f;
    float deltaT = t - baseT;
    float edgeLength = deltaT
        * (eval(a, b, c, d, baseT) + eval(a, b, c, d, t))
        * 0.5f;
    float sampleLength;
    int j;
    int i;

    for (i = 1; i <= div; i *= 2) {
        sampleLength = 0.0f;
        for (j = 1; j <= i; j++) {
            sampleLength += eval(a, b, c, d,
                baseT + deltaT * (j - 0.5f));
        }
        sampleLength *= deltaT;
        pathLength = (1.0f / 3.0f)
            * (edgeLength + (2.0f * sampleLength));
        deltaT *= 0.5f;
        edgeLength = (edgeLength + sampleLength) * 0.5f;
    }
    return pathLength;
}

static inline void OpeningCurveControlGet(HuVecF *path, int point, int count,
    HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *d)
{
    HuVecF tangentIn;
    HuVecF tangentOut;
    HuVecF *segment;

    if (point > count - 1) {
        point = count - 1;
    }
    segment = &path[point];
    if (point == 0) {
        VECSubtract(&segment[1], &segment[0], &tangentIn);
    } else {
        VECSubtract(&segment[1], &segment[-1], &tangentIn);
    }
    if (point == count - 2) {
        VECSubtract(&segment[1], &segment[0], &tangentOut);
    } else {
        VECSubtract(&segment[2], &segment[0], &tangentOut);
    }
    VECScale(&tangentIn, &tangentIn, 0.5f);
    VECScale(&tangentOut, &tangentOut, 0.5f);
    *a = segment[0];
    *b = segment[1];
    *c = tangentIn;
    *d = tangentOut;
}

void mbev_Opening(void)
{
    BOOL partyF = GwSystem.partyF;

    if (!partyF) {
        mbev_OpeningSingle();
    } else {
        mbev_OpeningParty();
    }
}

void mbev_OpeningParty(void)
{
    if (!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        openingProc = HuPrcChildCreate(ev_OpeningParty, 0x200C, 0x4000, 0, mbMainProc);
    }
    HuPrcDestructorSet2(openingProc, ev_OpeningPartyKill);
    while (openingProc != NULL) {
        HuPrcVSleep();
    }
}

static void ev_OpeningParty(void)
{
    int orderTbl[10];
    HuVecF playerPos;
    HuVecF cameraOfs = { 0.0f, 250.0f, 0.0f };
    HuVecF masuPos;
    HuVecF guidePos;
    HuVecF cameraPos;
    HuVecF playerRotDir;
    HuVecF cameraRot = { -20.0f, 0.0f, 0.0f };
    int playerOrder[GW_PLAYER_MAX];
    int temp;
    s16 playerRotAngle;
    s16 masuId;
    int manPlayerNo;
    float cameraZoom = 20000.0f;
    int comNum;
    int order1;
    int order2;
    BOOL allComF;
    int i;
    GW_PLAYER *playerP;
    MBPLAYERWORK *playerWorkP;

    mbCameraNearFarSet(10.0f, 30000.0f);
    HuDataDirRead(0x00110000);
    openingGuideObjId = mbObjCreate(0x00110000, guideMotFileTbl, FALSE);
    HuDataDirClose(0x00110000);

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbObjDispSet(mbPlayerObjIDGet(i), FALSE);
    }

    for (i = GW_PLAYER_MAX - 1, comNum = 0; i > -1; i--) {
        if (GwPlayerConf[i].type != 0) {
            comNum++;
        } else {
            manPlayerNo = i;
        }
    }
    if (comNum >= GW_PLAYER_MAX) {
        allComF = TRUE;
    } else {
        allComF = FALSE;
    }

    mbCameraZoomSet(openingZoom);
    mbCameraRotSetV(&openingRot);
    mbCameraCenterSetV(&openingPos);

    masuId = mbMasuFind_AttrIdGet(-1, 0x8000);
    mbMasuPosGet(masuId, &masuPos);
    guidePos = masuPos;
    guidePos.z -= 200.0f;
    mbObjPosSetV(openingGuideObjId, &guidePos);
    mbObjMotionShiftSet(openingGuideObjId, 1, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);

    mbWipeFadeIn();
    mbMusPlay(0, 11, 127, 0);
    {
        int boardNo = GwSystem.boardNo;

        mbTelopCreate(-1, boardNo + 16, FALSE);
    }
    HuPrcSleep(72);

    cameraPos = masuPos;
    cameraPos.y -= 50.0f;
    mbCameraMovePos(&cameraPos, &cameraRot, &cameraOfs,
        1800.0f, -1.0f,
        180);
    mbCameraMoveWait();

    playerPos.x = masuPos.x;
    playerPos.y = masuPos.y;
    playerPos.z = masuPos.z;
    PlayerDropExec(&playerPos);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerPosGet(i, &playerPos);
        VECSubtract(&guidePos, &playerPos, &playerRotDir);
        playerRotAngle = 90.0
            - (180.0
                * (atan2(playerRotDir.z, playerRotDir.x) / M_PI));
        mbPlayerRotateStart(i, playerRotAngle, 30);
    }
    while (!mbPlayerRotateCheckAll()) {
        HuPrcVSleep();
    }

    mbObjMotionShiftSet(openingGuideObjId, 12, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    mbAudGuidePlay(950);
    {
        int boardNo = GwSystem.boardNo;

        mbWinCreate(2, welcomeMesTbl[boardNo], 6);
    }
    mbWinTopWait();

    mbAudGuidePlay(952);
    mbWinCreateChoice(2, 0x002C0009, 6, 0);
    if (allComF) {
        GwSystem.turnPlayerNo = 0;
        mbComChoiceDownSet();
    }
    mbWinTopWait();
    if (mbWinTopChoiceGet() == 0 && openingInstHook != NULL) {
        openingInstHook();
    }

    mbObjMotionShiftSet(openingGuideObjId, 12, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    mbAudGuidePlay(952);
    mbWinCreate(2, 0x002C000A, 6);
    mbWinTopWait();
    mbObjMotionShiftSet(openingGuideObjId, 1, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerRotateStart(i, 0, 30);
    }
    while (!mbPlayerRotateCheckAll()) {
        HuPrcVSleep();
    }

    for (i = 0; i < 10; i++) {
        orderTbl[i] = i;
    }
    for (i = 0; i < 100; i++) {
        order1 = mbRandMod(10);
        order2 = mbRandMod(10);
        temp = orderTbl[order1];
        orderTbl[order1] = orderTbl[order2];
        orderTbl[order2] = temp;
    }

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbDiceExec(i, 0, NULL, orderTbl[i], FALSE, FALSE, NULL, 0);
        if (GwPlayer[i].comF) {
            openingPadDelay[i] = mbRandMod(30) + 30;
            mbDicePadBtnHookSet(i, OpeningPadBtn);
        }
    }
    mbWinCreateHelp(0x00260002);
    mbWinTopPosSet(228, 392);
    while (!mbDiceKillCheckAll()) {
        HuPrcVSleep();
    }
    mbWinTopKill();

    memcpy(playerOrder, orderTbl, sizeof(playerOrder));
    PlayerOrderSet(playerOrder);
    mbStatusReset();
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbAudGuidePlay(952);
        mbWinCreateTime(2, 0x002C000B + i, -1);
        mbWinTopInsertMesSet(mbPlayerNameMesGet(i), 0);
        if (GwPlayer[i].comF || HuPadStatGet(GwPlayer[i].padNo) != PAD_ERR_NONE) {
            mbWinTopPlayerDisable(-1);
        } else {
            mbWinTopPlayerDisable(i);
        }
        mbWinTopWait();

        playerP = GWPlayerGet(i);
        playerWorkP = mbPlayerWorkGet(i);
        mbDiceNumShrinkSet(playerP->playerNo);
        playerP->playerNo = playerWorkP->playerNo = i;
        if (!GWTeamFGet()) {
            mbStatusDispSet(i, TRUE);
        }
        mbPlayerWinLoseVoicePlay(i, 12, 0x243);
        mbPlayerMotionShiftSet(i, 12, 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
        omVibrate(i, 20, 20, 0);
        HuPrcSleep(8);
        while (!mbPlayerMotionEndCheck(i)) {
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(i, 1, 0.0f, 8.0f,
            HU3D_MOTATTR_LOOP);
    }

    HuPrcSleep(30);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerMotIdleSet(i);
    }
    if (GWTeamFGet()) {
        mbStatusDispSetAll(TRUE);
    }
    HuPrcSleep(30);

    mbObjMotionShiftSet(openingGuideObjId, 12, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    mbAudGuidePlay(952);
    mbWinCreate(2, 0x002C000F, 6);
    mbWinTopWait();
    mbObjMotionShiftSet(openingGuideObjId, 6, 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);
    HuPrcSleep(40);
    OpeningCoinExec();
    HuPrcSleep(30);
    mbObjMotionShiftSet(openingGuideObjId, 1, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerWinLoseVoicePlay(i, 12, 0x243);
        mbPlayerMotionShiftSet(i, 7, 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
    }
    while (!mbPlayerMotionEndCheckAll()) {
        HuPrcVSleep();
    }

    openingZoom = mbCameraZoomGet();
    mbCameraRotGet(&openingRot);
    mbCameraCenterGet(&openingPos);
    if (openingStarInstHook != NULL) {
        openingStarInstHook();
    } else {
        mbStarMapViewProcExec();
    }

    mbObjMotionShiftSet(openingGuideObjId, 12, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    mbAudGuidePlay(950);
    mbWinCreate(2, 0x002C0010, 6);
    mbWinTopWait();
    mbObjMotionShiftSet(openingGuideObjId, 1, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerWinLoseVoicePlay(i, 12, 0x243);
        mbPlayerMotionShiftSet(i, 12, 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
    }
    while (!mbPlayerMotionEndCheckAll()) {
        HuPrcVSleep();
    }

    HuPrcSleep(30);
    mbMusFadeOutSpeed(0, 1000);
    WipeColorSet(0, 0, 0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 21);
    for (i = 0; i < 60 || WipeCheck(); i++) {
        HuPrcVSleep();
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerMotionSet(i, 1, HU3D_MOTATTR_LOOP);
    }
    mbCameraNearFarSet(100.0f, 20000.0f);
    mbObjKill(openingGuideObjId);
    openingGuideObjId = -1;
    mbMusBoardPlay();
    HuPrcEnd();
}

static u16 OpeningPadBtn(int playerNo)
{
    if (openingPadDelay[playerNo] != 0 && --openingPadDelay[playerNo] == 0) {
        return PAD_BUTTON_A;
    }
    return 0;
}

static void ev_OpeningPartyKill(void)
{
    openingProc = NULL;
}

void mbOpeningViewSet(HuVecF *rot, HuVecF *pos, float zoom)
{

    if (rot) {
        openingRot = *rot;
    }
    if (pos) {
        openingPos = *pos;
    }
    if (zoom >= 0.0f) {
        openingZoom = zoom;
    }
}

static void OpeningCoinExec(void)
{
    OPENINGCOINWORK *work;
    HuVecF playerPos;
    int coinNum;
    int i;
    int j;

    work = openingCoinWork;
    coinNum = 0;
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerPosGet(i, &playerPos);
        for (j = 0; j < 10; j++, work++) {
            work->coinId = mbCoinCreate();
            work->pos.x = playerPos.x + (0.2f * frandf() * 100.0f) - 10.0f;
            work->pos.y = playerPos.y + 800.0f;
            work->pos.z = playerPos.z;
            playerPos.y += 120.00001f;
            work->vel.y = -10.0f;
            work->vel.x = work->vel.z = 0.0f;
            work->rot.y = mbRandMod(360);
            work->dispF = TRUE;
            mbCoinObjPosSetV(work->coinId, &work->pos);
            mbCoinObjRotSetV(work->coinId, &work->rot);
            mbCoinObjDispSet(work->coinId, work->dispF);
            work->playerNo = i;
            coinNum++;
        }
    }

    while (TRUE) {
        work = openingCoinWork;
        for (i = 0; i < GW_PLAYER_MAX * 10; i++, work++) {
            if (!work->dispF) {
                continue;
            }
            mbPlayerPosGet(work->playerNo, &playerPos);
            VECAdd(&work->pos, &work->vel, &work->pos);
            work->vel.y -= 0.4f;
            work->rot.y += 5.0f;
            mbCoinObjPosSetV(work->coinId, &work->pos);
            mbCoinObjRotSetV(work->coinId, &work->rot);
            if (work->pos.y <= playerPos.y + 150.0f) {
                mbCoinObjKill(work->coinId);
                HuAudFXPlay(7);
                mbPlayerCoinAdd(work->playerNo, 1);
                work->dispF = FALSE;
                coinNum--;
            }
        }
        if (coinNum <= 0) {
            break;
        }
        HuPrcVSleep();
    }
    mbAudFXPlay(15);
}

static void PlayerDropExec(HuVecF *center)
{
    int delay[GW_PLAYER_MAX] = { 0, 30, 60, 90 };
    BOOL landF[GW_PLAYER_MAX];
    HuVecF pos[GW_PLAYER_MAX];
    HuVecF vel[GW_PLAYER_MAX];
    int endNum;
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbObjDispSet(mbPlayerObjIDGet(i), TRUE);
        mbPlayerRotYSet(i, 0.0f);
        pos[i].x = center->x + 100.0f * playerCenterDist[i];
        pos[i].y = 800.0f + center->y;
        pos[i].z = center->z;
        vel[i].y = -20.0f;
        vel[i].x = vel[i].z = 0.0f;
        landF[i] = FALSE;
        mbPlayerPosSetV(i, &pos[i]);
        mbPlayerMotionShiftSet(i, 4, 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
        mbPlayerMotionVoiceOnSet(i, 4, FALSE);
    }

    endNum = 0;
    while (TRUE) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (landF[i] && mbPlayerMotionEndCheck(i)) {
                mbPlayerMotionShiftSet(i, 1, 0.0f, 8.0f,
                    HU3D_MOTATTR_LOOP);
                endNum++;
            }
            if (delay[i]-- <= 0 && !landF[i]) {
                VECAdd(&pos[i], &vel[i], &pos[i]);
                vel[i].y -= 0.2f;
                if (pos[i].y <= center->y) {
                    pos[i].y = center->y;
                    landF[i] = TRUE;
                    mbPlayerMotionShiftSet(i, 5, 0.0f, 8.0f,
                        HU3D_MOTATTR_NONE);
                }
                mbPlayerPosSetV(i, &pos[i]);
            }
        }
        if (endNum >= GW_PLAYER_MAX) {
            break;
        }
        HuPrcVSleep();
    }

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerMotionVoiceOnSet(i, 4, TRUE);
    }
}

static void PlayerOrderSet(int *order)
{
    int teamCoin[GW_PLAYER_MAX / 2];
    int teamStar[GW_PLAYER_MAX / 2];
    int teamNo;
    int temp;
    int i;
    int j;

    mbPlayerColSnapSet(FALSE);
    for (i = 0; i < GW_PLAYER_MAX - 1; i++) {
        for (j = i + 1; j < GW_PLAYER_MAX; j++) {
            if (order[i] < order[j]) {
                temp = order[i];
                order[i] = order[j];
                order[j] = temp;
                mbPlayerSwap(i, j);
            }
        }
    }

    if (GWTeamFGet()) {
        for (i = 0; i < GW_PLAYER_MAX / 2; i++) {
            teamCoin[i] = 0;
            teamStar[i] = 0;
        }
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            teamNo = mbPlayerGrpGet(i);
            teamCoin[teamNo] += GwPlayer[i].coin;
            teamStar[teamNo] += GwPlayer[i].star;
            GwPlayer[i].coin = 0;
            GwPlayer[i].star = 0;
        }
        for (i = 0; i < GW_PLAYER_MAX / 2; i++) {
            mbPlayerTeamCoinSet(i, teamCoin[i]);
            mbPlayerGrpStarSet(i, teamStar[i]);
        }
    }
    HuPrcVSleep();
}

void mbev_OpeningSingle(void)
{
    SINGLEGUIDEWORK *work = &singleGuideWork;
    HuVecF *cameraPathTbl;
    HuVecF *masuPosTbl;
    float *pathLengthTbl;
    s16 winId;
    int playerNo;

    mbDirClose();
    work->endF = FALSE;
    work->dispF = FALSE;
    singleGuideObj = NULL;
    singleOpeningPosTbl2 = NULL;
    singleOpeningPosTbl = NULL;
    singleOpeningZoomTbl = NULL;
    singleWinId = -1;
    singleEff1MdlId = -1;
    singleFXNo = -1;
    winId = -1;
    if (!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        openingSingleProc = HuPrcChildCreate(ev_OpeningSingle, 0x200C,
            0x4000, 0, mbMainProc);
    }
    HuPrcDestructorSet2(openingSingleProc, ev_OpeningSingleKill);
    do {
        if (work->dispF && mbTelopCheck()) {
            if (winId < 0) {
                winId = mbWinCreateHelp(0x0026000C);
                mbWinPosSet(winId, 228, 408);
            }
            for (playerNo = 0; playerNo < GW_PLAYER_MAX; playerNo++) {
                if (!GwPlayer[playerNo].comF
                    && (HuPadBtnDown[GwPlayer[playerNo].padNo]
                        & PAD_BUTTON_START)) {
                    if (singleWinId != -1) {
                        mbWinKill(singleWinId);
                        singleWinId = -1;
                    }
                    HuPrcKill(openingSingleProc);
                    work->endF = TRUE;
                    break;
                }
            }
        }
        HuPrcVSleep();
    } while (!work->endF);
    mbWipeWait();
    if (!WipeCheckIn()) {
        mbWipeFadeOut();
    }
    mbWinKill(winId);
    OSReport(lbl_80248648);
    if (singleGuideObj != NULL) {
        mbGuideKill(singleGuideObj);
    }
    if (singleOpeningPosTbl2 != NULL) {
        cameraPathTbl = singleOpeningPosTbl2;
        HuMemDirectFree(cameraPathTbl);
    }
    if (singleOpeningPosTbl != NULL) {
        masuPosTbl = singleOpeningPosTbl;
        HuMemDirectFree(masuPosTbl);
    }
    if (singleOpeningZoomTbl != NULL) {
        pathLengthTbl = singleOpeningZoomTbl;
        HuMemDirectFree(pathLengthTbl);
    }
    if (singleEff1MdlId != -1) {
        mbParticleKill(singleEff1MdlId);
        HuDataDirClose(0x00210000);
    }
    if (singleFXNo != -1) {
        mbAudFXStop(singleFXNo);
    }
}

static inline void *OpeningAlloc(s32 size)
{
    return HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
}

static inline void OpeningFree(void *ptr)
{
    HuMemDirectFree(ptr);
}

static inline void OpeningBezierCalc(HuVecF *points, int count, HuVecF *dst, float t)
{
    HuVecF *work = OpeningAlloc(count * sizeof(HuVecF));
    int j;
    int k;

    memcpy(work, points, count * sizeof(HuVecF));
    for (j = 1; j < count; j++) {
        for (k = 0; k < count - j; k++) {
            work[k].x = work[k].x + t * (work[k + 1].x - work[k].x);
            work[k].y = work[k].y + t * (work[k + 1].y - work[k].y);
            work[k].z = work[k].z + t * (work[k + 1].z - work[k].z);
        }
    }
    *dst = work[0];
    OpeningFree(work);
}

static void ev_OpeningSingle(void)
{
    s16 masuList[256];
    HuVecF guidePath[5];
    HuVecF fallVelocity;
    HuVecF cameraPos;
    HuVecF cameraCurvePos;
    HuVecF cameraRot;
    HuVecF cameraCenter;
    HuVecF guidePos;
    HuVecF masuPos;
    HuVecF curveTangentIn;
    HuVecF curveTangentOut;
    HuVecF curveStart;
    HuVecF curveEnd;
    HuVecF segmentDir;
    HuVecF prevSegmentDir;
    HuVec2f winPos;
    MBMODELID guideMdlId;
    s16 startMasuId;
    int masuNum;
    int cameraPathNum;
    int cameraPathNo;
    SINGLEGUIDEWORK *work = &singleGuideWork;
    int j;
    int playerNo;
    int cameraType;
    int size;
    float baseY;
    ANIMDATA *animP;
    int i;
    float distance;
    float t;
    float angle;

    playerNo = 0;
    singleGuideObj = mbGuideCreateFlag(&singleGuidePosTbl[0],
        (s8 *)singleGuideMotFileTbl, FALSE, FALSE, FALSE);
    mbGuideMotionNextSet(singleGuideObj, 1);
    guideMdlId = mbGuideModelGet(singleGuideObj);
    mbObjDispSet(guideMdlId, FALSE);
    mbObjMotionSet(guideMdlId, 5, HU3D_MOTATTR_NONE);

    startMasuId = mbMasuFind_AttrIdGet(-1, 0x8000);
    masuNum = mbMasuFind_TypeListGet2(startMasuId, 7, FALSE, FALSE,
        masuList);
    size = masuNum * sizeof(HuVecF);
    singleOpeningPosTbl2 = OpeningAlloc(size);
    singleOpeningPosTbl = OpeningAlloc(size);
    for (i = masuNum - 1, j = 0; i > -1; i--, j++) {
        mbMasuPosGet(masuList[i], &singleOpeningPosTbl[j]);
    }

    cameraPathNum = 0;
    singleOpeningPosTbl2[cameraPathNum++] = singleOpeningPosTbl[0];
    for (i = 0; i < masuNum - 1; i++) {
        VECSubtract(&singleOpeningPosTbl[i + 1], &singleOpeningPosTbl[i],
            &segmentDir);
        if (i != 0) {
            VECNormalize(&prevSegmentDir, &prevSegmentDir);
            VECNormalize(&segmentDir, &segmentDir);
            if (VECDotProduct(&prevSegmentDir, &segmentDir) < 0.7f) {
                singleOpeningPosTbl2[cameraPathNum++] =
                    singleOpeningPosTbl[i];
            }
        }
        prevSegmentDir = segmentDir;
    }
    singleOpeningPosTbl2[cameraPathNum++] = singleOpeningPosTbl[masuNum - 1];

    mbCameraNearFarSet(10.0f, 30000.0f);
    mbCameraZoomSet(mbOpeningZoomGet());
    mbOpeningRotGet(&cameraRot);
    mbCameraRotSetV(&cameraRot);
    mbOpeningPosGet(&cameraCenter);
    mbCameraCenterSetV(&cameraCenter);
    mbWipeFadeIn();
    mbMusBoardPlay();
    cameraType = MBBoardNoGet() - 5;
    mbTelopCreate(-1, MBBoardNoGet() + 16, FALSE);
    HuPrcSleep(90);
    work->dispF = TRUE;
    cameraPos = singleOpeningPosTbl2[0];

    if (cameraType < 3) {
        mbCameraMovePos(&cameraPos, &singleGuideCameraRot, NULL, 2400.0f,
            -1.0f, 180);
        mbCameraMoveWait();
        HuPrcSleep(12);
        baseY = cameraPos.y;
        singleOpeningZoomTbl = OpeningAlloc(cameraPathNum * sizeof(float));
        for (i = 0; i < cameraPathNum - 1; i++) {
            OpeningCurveControlGet(singleOpeningPosTbl2, i, cameraPathNum,
                &curveStart, &curveEnd, &curveTangentIn, &curveTangentOut);

            distance = OpeningCurveLength((OPENINGCURVEEVALFUNC)(u32)OpeningCurveEval,
                &curveStart, &curveEnd, &curveTangentIn, &curveTangentOut, 1.0f);
            singleOpeningZoomTbl[i] = distance;
        }

        t = 0.0f;
        distance = 0.0f;
        cameraPathNo = 0;
        do {
            OpeningCurveControlGet(singleOpeningPosTbl2, cameraPathNo, cameraPathNum,
                &curveStart, &curveEnd, &curveTangentIn, &curveTangentOut);

            t = OpeningCurveNewton((OPENINGCURVEEVALFUNC)(u32)OpeningCurveEval,
                &curveStart, &curveEnd, &curveTangentIn, &curveTangentOut,
                t, distance, 10);
            mbHermiteCalcV(&curveStart, &curveEnd, &curveTangentIn,
                &curveTangentOut, &cameraCurvePos, t);
            mbCameraCenterSetV(&cameraCurvePos);
            distance += 20.0f;
            if (distance >= singleOpeningZoomTbl[cameraPathNo]) {
                distance -= singleOpeningZoomTbl[cameraPathNo];
                cameraPathNo++;
                t -= 1.0f;
            }
            HuPrcVSleep();
        } while (cameraPathNo < cameraPathNum - 1);
    } else {
        mbCameraMovePos(&cameraPos, &singleGuideCameraRot, NULL, 3700.0f,
            -1.0f, 180);
        mbCameraMoveWait();
        HuPrcSleep(12);
        singleOpeningZoomTbl = NULL;
        mbCameraMoveMasu(startMasuId, &singleGuideCameraRot, NULL, 2400.0f,
            -1.0f, 300);
        mbCameraMoveWait();
    }

    HuPrcSleep(12);
    mbMasuPosGet(startMasuId, &masuPos);
    for (i = 0; i < 5; i++) {
        guidePath[i] = masuPos;
        guidePath[i].x += singleGuidePathOfsTbl[i].x;
        guidePath[i].y += singleGuidePathOfsTbl[i].y;
        guidePath[i].z += singleGuidePathOfsTbl[i].z;
    }
    prevSegmentDir = guidePath[0];
    animP = HuSprAnimRead(HuDataReadNum(0x00210001, HU_MEMNUM_OVL));
    singleEff1MdlId = mbParticleCreate(animP, 128);
    mbParticleHookSet(singleEff1MdlId, OpeningSingleEffHook);
    Hu3DModelLayerSet(singleEff1MdlId, 5);
    singleGuideEffOnF = TRUE;
    mbCameraMovePlayer(playerNo, NULL, NULL, 1800.0f, -1.0f, 60);
    mbObjDispSet(guideMdlId, TRUE);
    singleFXNo = mbAudFXPlay(1093);
    for (i = 0; i <= 180u; i++) {
        t = i / 180.0f;
        OpeningBezierCalc(guidePath, 5, &guidePos, t);
        mbObjPosSetV(guideMdlId, &guidePos);
        guideCurPos = guidePos;
        VECSubtract(&guidePos, &prevSegmentDir, &segmentDir);
        angle = HuAtan(segmentDir.x, segmentDir.z);
        if (i == 150u) {
            mbObjMotionShiftSet(guideMdlId, 4, 0.0f, 16.0f,
                HU3D_MOTATTR_LOOP);
        } else if (i > 150u) {
            t = (i - 150u) / 30.0f;
            angle = (1.0f - t) * angle;
        }
        mbObjRotYSet(guideMdlId, angle);
        prevSegmentDir = guidePos;
        HuPrcVSleep();
    }
    singleGuideEffOnF = FALSE;
    mbCameraMoveWait();
    if (singleFXNo != -1) {
        mbAudFXStop(singleFXNo);
    }
    HuPrcSleep(60);

    mbGuideMotionSet(singleGuideObj, 12, TRUE);
    mbAudGuidePlay(950);
    singleWinId = mbWinCreate(2, singleWelcomeMesTbl[MBBoardNoGet() - 6], 6);
    mbWinTopPosGet(&winPos);
    winPos.y -= 35.0f;
    mbWinTopPosSet(winPos.x, winPos.y);
    mbWinTopWait();
    singleWinId = -1;

    mbAudGuidePlay(952);
    singleWinId = mbWinCreate(2, 0x002C0011, 6);
    mbWinTopPosGet(&winPos);
    winPos.y -= 35.0f;
    mbWinTopPosSet(winPos.x, winPos.y);
    mbWinTopWait();
    singleWinId = -1;

    mbAudGuidePlay(950);
    singleWinId = mbWinCreate(2, 0x002C0012, 6);
    mbWinTopPosGet(&winPos);
    winPos.y -= 35.0f;
    mbWinTopPosSet(winPos.x, winPos.y);
    mbWinTopWait();
    singleWinId = -1;

    mbObjMotionShiftSet(guideMdlId, 5, 0.0f, 8.0f, HU3D_MOTATTR_NONE);
    fallVelocity.x = fallVelocity.y = fallVelocity.z = 0.0f;
    mbObjPosGet(guideMdlId, &guidePos);
    singleFXNo = mbAudFXPlay(1093);
    for (i = 0; i < 120u; i++) {
        fallVelocity.y += -16.333334f;
        guidePos.y -= 0.016666668f * fallVelocity.y;
        mbObjPosSetV(guideMdlId, &guidePos);
        guideCurPos = guidePos;
        if (i == 12u) {
            singleGuideEffOnF = TRUE;
        }
        HuPrcVSleep();
    }
    if (singleFXNo != -1) {
        mbAudFXStop(singleFXNo);
    }
    HuPrcSleep(30);
    mbWipeFadeOut();
    work->dispF = FALSE;
    work->endF = TRUE;
    HuPrcEnd();
}

static void ev_OpeningSingleKill(void)
{
    openingSingleProc = NULL;
}

float mbOpeningZoomGet(void)
{
    return openingZoom;
}

void mbOpeningRotGet(HuVecF *rot)
{
    *rot = openingRot;
}

void mbOpeningPosGet(HuVecF *pos)
{
    *pos = openingPos;
}

void mbOpeningInstHookSet(void (*hook)(void))
{
    openingInstHook = hook;
}

void mbOpeningStarInstHookSet(void (*hook)(void))
{
    openingStarInstHook = hook;
}

MBMODELID mbOpeningGuideObjIdGet(void)
{
    return openingGuideObjId;
}

void mbOpeningGuidePosRestore(void)
{
    s16 masuId;
    HuVecF masuPos;
    HuVecF pos;

    masuId = mbMasuFind_AttrIdGet(-1, 0x8000);
    mbMasuPosGet(masuId, &masuPos);
    pos = masuPos;
    pos.z -= 200.0f;
    mbObjPosSetV(openingGuideObjId, &pos);
    mbObjMotionShiftSet(openingGuideObjId, 1, 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);
}

void mbOpeningCameraPosRestore(void)
{
    s16 masuId;
    HuVecF masuPos;
    HuVecF pos;

    masuId = mbMasuFind_AttrIdGet(-1, 0x8000);
    mbMasuPosGet(masuId, &masuPos);
    pos = masuPos;
    pos.y -= 50.0f;
    mbCameraMovePos(&pos, &openingCameraRestoreRot, &openingCameraRestorePos,
        1800.0f, -1.0f, 0);
}

static void OpeningSingleEffHook(HU3D_MODEL *modelP, MBPARTICLE *particleP,
    Mtx mtx)
{
    HuVecF dir;
    GXColor colorTbl[4] = {
        { 255, 255, 255, 128 },
        { 128, 255, 0, 128 },
        { 128, 255, 255, 128 },
        { 255, 128, 255, 128 }
    };
    MBPARTICLEDATA *dataP;
    float rand;
    int i;

    if (particleP->mode == 0) {
        dataP = particleP->data;
        particleP->blendMode = MB_PARTICLE_BLEND_ADDCOL;
        for (i = 0; i < particleP->num; i++, dataP++) {
            dataP->vel.y = 0.0f;
            dataP->scale = 0.0f;
            dataP->time = 0;
            dataP->activeF = -1;
        }
        particleP->mode = 1;
    }

    dataP = particleP->data;
    for (i = 0; i < particleP->num; i++, dataP++) {
        if (dataP->activeF < 0 && mbRandMod(100) < 5) {
            dataP->activeF = 60.0f * (0.4f + 0.4f * mbParticleRandF());

            rand = frandf();
            dir.x = frandf() - rand;
            rand = frandf();
            dir.y = frandf() - rand;
            rand = frandf();
            dir.z = frandf() - rand;
            VECNormalize(&dir, &dir);
            VECScale(&dir, &dir, 40.0f);

            dataP->pos.x = dir.x + guideCurPos.x;
            dataP->pos.y = dir.y + guideCurPos.y;
            dataP->pos.z = dir.z + guideCurPos.z;
            dataP->vel.x = 0.0f;
            dataP->vel.y = 0.0f;
            dataP->vel.z = 0.0f;
            dataP->guideScaleBase = 20.0f + 20.0f * frandf();
            dataP->color = colorTbl[mbRandMod(4)];
            if (!singleGuideEffOnF) {
                dataP->color.a = 0;
                dataP->scale = 0.0f;
            }
            dataP->time = 0;
        }

        if (dataP->activeF >= 0) {
            dataP->pos.y += 0.016666668f * dataP->vel.y;
            dataP->vel.y += -16.333334f;
            dataP->scale = dataP->guideScaleBase
                * (1.0f - (float)dataP->time / (float)dataP->activeF);
            if (dataP->time++ >= dataP->activeF) {
                dataP->activeF = -1;
            }
        }
    }
}
