// Runs the board's party or single-player introduction and the player setup it contains.
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
#include "messdir_enum.h"
#include "msm_se.h"

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

#define OPENING_PROCESS_PRIORITY 8204
#define OPENING_PROCESS_STACK_SIZE 16384
#define OPENING_MASU_FLAG_START (1 << 15)

typedef struct OpeningCoinWork_s {
    s16 coinId; // Coin object created for this falling reward.
    HuVecF pos; // Current world position of the coin.
    HuVecF rot; // Current rotation; the y angle spins while falling.
    HuVecF vel; // Per-frame movement velocity in world units.
    BOOL dispF; // Whether this coin is still falling and being updated.
    int playerNo; // Player who receives the coin when it lands.
    u32 unknown[3]; // Three words with no known use in the opening sequence.
} OPENINGCOINWORK;

typedef struct SingleGuideWork_s {
    BOOL dispF; // The guide introduction is visible and may show its help prompt.
    BOOL endF; // The single-player introduction has finished or was skipped.
} SINGLEGUIDEWORK;

static int guideMotFileTbl[] = {
    DATANUM(DATA_capsulechar4, 1),  DATANUM(DATA_capsulechar4, 2),  DATANUM(DATA_capsulechar4, 3),
    DATANUM(DATA_capsulechar4, 4),  DATANUM(DATA_capsulechar4, 5),  DATANUM(DATA_capsulechar4, 6),
    DATANUM(DATA_capsulechar4, 7),  DATANUM(DATA_capsulechar4, 8),  DATANUM(DATA_capsulechar4, 9),
    DATANUM(DATA_capsulechar4, 10), DATANUM(DATA_capsulechar4, 11), DATANUM(DATA_capsulechar4, 12),
    DATANUM(DATA_capsulechar4, 13), DATANUM(DATA_capsulechar4, 14), DATANUM(DATA_capsulechar4, 15),
    DATANUM(DATA_capsulechar4, 16), DATANUM(DATA_capsulechar4, 17), DATANUM(DATA_capsulechar4, 18),
    DATANUM(DATA_capsulechar4, 19), DATANUM(DATA_capsulechar4, 20), DATANUM(DATA_capsulechar4, 21),
    DATANUM(DATA_capsulechar4, 22), DATANUM(DATA_capsulechar4, 23), -1
};

static u32 welcomeMesTbl[] = {
    MESSNUM(MESS_BOARD_OPENING, 0), MESSNUM(MESS_BOARD_OPENING, 1), MESSNUM(MESS_BOARD_OPENING, 2),
    MESSNUM(MESS_BOARD_OPENING, 3), MESSNUM(MESS_BOARD_OPENING, 4), MESSNUM(MESS_BOARD_OPENING, 5)
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
    DATANUM(DATA_capsulechar4, 1),  DATANUM(DATA_capsulechar4, 4),  DATANUM(DATA_capsulechar4, 5),
    DATANUM(DATA_capsulechar4, 6),  DATANUM(DATA_capsulechar4, 7),  DATANUM(DATA_capsulechar4, 10),
    DATANUM(DATA_capsulechar4, 12), DATANUM(DATA_capsulechar4, 15), DATANUM(DATA_capsulechar4, 17),
    DATANUM(DATA_capsulechar4, 19), DATANUM(DATA_capsulechar4, 20), -1
};

static u32 singleWelcomeMesTbl[] = {
    MESSNUM(MESS_BOARD_OPENING, 6), MESSNUM(MESS_BOARD_OPENING, 7), MESSNUM(MESS_BOARD_OPENING, 8)
};

static HuVecF openingRot;
static HuVecF openingPos;
static int openingPadDelay[GW_PLAYER_MAX];
static OPENINGCOINWORK openingCoinWork[GW_PLAYER_MAX * 10];
static HuVecF guideCurPos;
static HuVecF openingCameraRestoreRot = { -20.0f, 0.0f, 0.0f };
/* Target-relative camera offset used when restoring the party opening view. */
static HuVecF openingCameraRestoreOffset = { 0.0f, 250.0f, 0.0f };
static MBMODELID openingGuideObjId = -1;
static float openingZoom;
static BOOL singleGuideEffOnF;
static SINGLEGUIDEWORK singleGuideWork;
static OMOBJ *singleGuideObj;
static HuVecF *singleOpeningPosTbl2;
static HuVecF *singleOpeningPosTbl;
/* Hermite camera-path segment lengths, indexed by cameraPathNo. */
static float *singleOpeningPathLengthTbl;
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

// Returns the Hermite path speed used by the distance helpers during the single-player opening.
static float OpeningCurveEval(HuVecF *start, HuVecF *end, HuVecF *tangentIn, HuVecF *tangentOut,
                              float t)
{
    HuVecF slope;

    slope.x = mbHermiteCalcSlope(start->x, end->x, tangentIn->x, tangentOut->x, t);
    slope.y = mbHermiteCalcSlope(start->y, end->y, tangentIn->y, tangentOut->y, t);
    slope.z = mbHermiteCalcSlope(start->z, end->z, tangentIn->z, tangentOut->z, t);
    return VECMag(&slope);
}

typedef float (*OPENINGCURVEEVALFUNC)();

// Called by OpeningCurveNewton to estimate distance along a Hermite camera segment with ten
// trapezoid intervals.
static float OpeningCurveIntegrate(OPENINGCURVEEVALFUNC speedEval,
    HuVecF *start, HuVecF *end, HuVecF *tangentIn, HuVecF *tangentOut, float t)
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
        sampleLength += speedEval(start, end, tangentIn, tangentOut, sampleT);
    }
    sampleLength = deltaT * 0.5 *
                   (speedEval(start, end, tangentIn, tangentOut, baseT) +
                    speedEval(start, end, tangentIn, tangentOut, t) + (2.0 * sampleLength));
    return sampleLength;
}

// Called by ev_OpeningSingle to adjust the Hermite parameter to the camera's traveled distance.
static float OpeningCurveNewton(OPENINGCURVEEVALFUNC speedEval, HuVecF *start,
    HuVecF *end, HuVecF *tangentIn, HuVecF *tangentOut, float t, float distance, int maxStep)
{
    int step;
    float sampleLength;
    float pathLength;
    float oldT;
    float minLength;

    minLength = 0.1f;
    step = 0;
    do {
        pathLength =
            OpeningCurveIntegrate(speedEval, start, end, tangentIn, tangentOut, t) - distance;
        if (fabs(sampleLength = speedEval(start, end, tangentIn, tangentOut, t)) < minLength) {
            sampleLength = 1.0f;
        }
        oldT = t;
        t -= pathLength / sampleLength;
        step++;
    } while (t != oldT && step < maxStep);
    return t;
}

// Called by ev_OpeningSingle to estimate each segment's length for the single-player camera route.
static float OpeningCurveLength(OPENINGCURVEEVALFUNC speedEval, HuVecF *start, HuVecF *end,
                                HuVecF *tangentIn, HuVecF *tangentOut, float t)
{
    int div = 10;
    float baseT = 0.0f;
    float pathLength = 0.0f;
    float deltaT = t - baseT;
    float edgeLength = deltaT *
                       (speedEval(start, end, tangentIn, tangentOut, baseT) +
                        speedEval(start, end, tangentIn, tangentOut, t)) *
                       0.5f;
    float sampleLength;
    int j;
    int i;

    for (i = 1; i <= div; i *= 2) {
        sampleLength = 0.0f;
        for (j = 1; j <= i; j++) {
            sampleLength += speedEval(start, end, tangentIn, tangentOut,
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

// Called by ev_OpeningSingle to select a camera-path segment and derive its Hermite tangents.
static inline void OpeningCurveControlGet(HuVecF *path, int point, int count,
    HuVecF *segmentStart, HuVecF *segmentEnd, HuVecF *tangentIn, HuVecF *tangentOut)
{
    HuVecF tangentInVec;
    HuVecF tangentOutVec;
    HuVecF *segment;

    if (point > count - 1) {
        point = count - 1;
    }
    segment = &path[point];
    if (point == 0) {
        VECSubtract(&segment[1], &segment[0], &tangentInVec);
    } else {
        VECSubtract(&segment[1], &segment[-1], &tangentInVec);
    }
    if (point == count - 2) {
        VECSubtract(&segment[1], &segment[0], &tangentOutVec);
    } else {
        VECSubtract(&segment[2], &segment[0], &tangentOutVec);
    }
    VECScale(&tangentInVec, &tangentInVec, 0.5f);
    VECScale(&tangentOutVec, &tangentOutVec, 0.5f);
    *segmentStart = segment[0];
    *segmentEnd = segment[1];
    *tangentIn = tangentInVec;
    *tangentOut = tangentOutVec;
}

// Called by the board process at board setup to start the introduction for the current mode.
void mbev_Opening(void)
{
    BOOL partyF = GwSystem.partyF;

    if (!partyF) {
        mbev_OpeningSingle();
    } else {
        mbev_OpeningParty();
    }
}

// Called by mbev_Opening for party mode; waits for its child introduction process to finish.
void mbev_OpeningParty(void)
{
    if (!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        openingProc = HuPrcChildCreate(ev_OpeningParty, OPENING_PROCESS_PRIORITY,
                                       OPENING_PROCESS_STACK_SIZE, 0, mbMainProc);
    }
    HuPrcDestructorSet2(openingProc, ev_OpeningPartyKill);
    while (openingProc != NULL) {
        HuPrcVSleep();
    }
}

// Runs as the child created by mbev_OpeningParty, presenting the guide, player order, and intro.
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
    int swapValue;
    s16 playerRotAngle;
    s16 masuId;
    int manPlayerNo;
    float cameraZoom = 20000.0f;
    int comNum;
    int randomOrderA;
    int randomOrderB;
    BOOL allComF;
    int i;
    GW_PLAYER *playerP;
    MBPLAYERWORK *playerWorkP;

    mbCameraNearFarSet(10.0f, 30000.0f);
    HuDataDirRead(DATA_capsulechar4);
    openingGuideObjId = mbObjCreate(DATA_capsulechar4, guideMotFileTbl, FALSE);
    HuDataDirClose(DATA_capsulechar4);

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

    masuId = mbMasuFind_AttrIdGet(-1, OPENING_MASU_FLAG_START);
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
    mbWinCreateChoice(2, MESSNUM(MESS_BOARD_OPENING, 9), 6, 0);
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
    mbWinCreate(2, MESSNUM(MESS_BOARD_OPENING, 10), 6);
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
        randomOrderA = mbRandMod(10);
        randomOrderB = mbRandMod(10);
        swapValue = orderTbl[randomOrderA];
        orderTbl[randomOrderA] = orderTbl[randomOrderB];
        orderTbl[randomOrderB] = swapValue;
    }

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbDiceExec(i, 0, NULL, orderTbl[i], FALSE, FALSE, NULL, 0);
        if (GwPlayer[i].comF) {
            openingPadDelay[i] = mbRandMod(30) + 30;
            mbDicePadBtnHookSet(i, OpeningPadBtn);
        }
    }
    mbWinCreateHelp(MESSNUM(MESS_BOARD_OPE, 2));
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
        mbWinCreateTime(2, MESSNUM(MESS_BOARD_OPENING, 11) + i, -1);
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
        // Shrink the existing assignment before restoring both player records to this slot.
        playerP->playerNo = playerWorkP->playerNo = i;
        if (!GWTeamFGet()) {
            mbStatusDispSet(i, TRUE);
        }
        mbPlayerWinLoseVoicePlay(i, 12, (MSM_SE_CHARVOICE_MARIO + 6));
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
    mbWinCreate(2, MESSNUM(MESS_BOARD_OPENING, 15), 6);
    mbWinTopWait();
    mbObjMotionShiftSet(openingGuideObjId, 6, 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);
    HuPrcSleep(40);
    OpeningCoinExec();
    HuPrcSleep(30);
    mbObjMotionShiftSet(openingGuideObjId, 1, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerWinLoseVoicePlay(i, 12, (MSM_SE_CHARVOICE_MARIO + 6));
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
    mbWinCreate(2, MESSNUM(MESS_BOARD_OPENING, 16), 6);
    mbWinTopWait();
    mbObjMotionShiftSet(openingGuideObjId, 1, 0.0f, 8.0f,
        HU3D_MOTATTR_LOOP);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerWinLoseVoicePlay(i, 12, (MSM_SE_CHARVOICE_MARIO + 6));
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

// Registered by ev_OpeningParty as the dice input hook to supply a delayed A press for COMs.
static u16 OpeningPadBtn(int playerNo)
{
    if (openingPadDelay[playerNo] != 0 && --openingPadDelay[playerNo] == 0) {
        return PAD_BUTTON_A;
    }
    return 0;
}

// Registered by mbev_OpeningParty as the destructor that clears its child process handle.
static void ev_OpeningPartyKill(void)
{
    openingProc = NULL;
}

// Saves supplied camera values used by the party opening; negative zoom leaves zoom unchanged.
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

// Called by ev_OpeningParty to drop ten coins above each player and award them when they reach
// 150 units above the player.
static void OpeningCoinExec(void)
{
    OPENINGCOINWORK *coinWork;
    HuVecF playerPos;
    int coinNum;
    int i;
    int j;

    coinWork = openingCoinWork;
    coinNum = 0;
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerPosGet(i, &playerPos);
        for (j = 0; j < 10; j++, coinWork++) {
            coinWork->coinId = mbCoinCreate();
            coinWork->pos.x = playerPos.x + (0.2f * frandf() * 100.0f) - 10.0f;
            coinWork->pos.y = playerPos.y + 800.0f;
            coinWork->pos.z = playerPos.z;
            playerPos.y += 120.00001f;
            coinWork->vel.y = -10.0f;
            coinWork->vel.x = coinWork->vel.z = 0.0f;
            coinWork->rot.y = mbRandMod(360);
            coinWork->dispF = TRUE;
            mbCoinObjPosSetV(coinWork->coinId, &coinWork->pos);
            mbCoinObjRotSetV(coinWork->coinId, &coinWork->rot);
            mbCoinObjDispSet(coinWork->coinId, coinWork->dispF);
            coinWork->playerNo = i;
            coinNum++;
        }
    }

    while (TRUE) {
        coinWork = openingCoinWork;
        for (i = 0; i < GW_PLAYER_MAX * 10; i++, coinWork++) {
            if (!coinWork->dispF) {
                continue;
            }
            mbPlayerPosGet(coinWork->playerNo, &playerPos);
            VECAdd(&coinWork->pos, &coinWork->vel, &coinWork->pos);
            coinWork->vel.y -= 0.4f;
            coinWork->rot.y += 5.0f;
            mbCoinObjPosSetV(coinWork->coinId, &coinWork->pos);
            mbCoinObjRotSetV(coinWork->coinId, &coinWork->rot);
            if (coinWork->pos.y <= playerPos.y + 150.0f) {
                mbCoinObjKill(coinWork->coinId);
                HuAudFXPlay(MSM_SE_CMN_08);
                mbPlayerCoinAdd(coinWork->playerNo, 1);
                coinWork->dispF = FALSE;
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

// Called by ev_OpeningParty to drop players onto the starting space in sequence and await landing.
static void PlayerDropExec(HuVecF *center)
{
    int delay[GW_PLAYER_MAX] = { 0, 30, 60, 90 };
    BOOL playerLanded[GW_PLAYER_MAX];
    HuVecF playerPos[GW_PLAYER_MAX];
    HuVecF fallVelocity[GW_PLAYER_MAX];
    int finishedPlayerCount;
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbObjDispSet(mbPlayerObjIDGet(i), TRUE);
        mbPlayerRotYSet(i, 0.0f);
        playerPos[i].x = center->x + 100.0f * playerCenterDist[i];
        playerPos[i].y = 800.0f + center->y;
        playerPos[i].z = center->z;
        fallVelocity[i].y = -20.0f;
        fallVelocity[i].x = fallVelocity[i].z = 0.0f;
        playerLanded[i] = FALSE;
        mbPlayerPosSetV(i, &playerPos[i]);
        mbPlayerMotionShiftSet(i, 4, 0.0f, 8.0f,
            HU3D_MOTATTR_NONE);
        mbPlayerMotionVoiceOnSet(i, 4, FALSE);
    }

    finishedPlayerCount = 0;
    while (TRUE) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (playerLanded[i] && mbPlayerMotionEndCheck(i)) {
                mbPlayerMotionShiftSet(i, 1, 0.0f, 8.0f,
                    HU3D_MOTATTR_LOOP);
                finishedPlayerCount++;
            }
            if (delay[i]-- <= 0 && !playerLanded[i]) {
                VECAdd(&playerPos[i], &fallVelocity[i], &playerPos[i]);
                fallVelocity[i].y -= 0.2f;
                if (playerPos[i].y <= center->y) {
                    playerPos[i].y = center->y;
                    playerLanded[i] = TRUE;
                    mbPlayerMotionShiftSet(i, 5, 0.0f, 8.0f,
                        HU3D_MOTATTR_NONE);
                }
                mbPlayerPosSetV(i, &playerPos[i]);
            }
        }
        if (finishedPlayerCount >= GW_PLAYER_MAX) {
            break;
        }
        HuPrcVSleep();
    }

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPlayerMotionVoiceOnSet(i, 4, TRUE);
    }
}

// Called by ev_OpeningParty to sort rolled values, swap player slots, and pool team coins and
// stars.
static void PlayerOrderSet(int *order)
{
    int teamCoin[GW_PLAYER_MAX / 2];
    int teamStar[GW_PLAYER_MAX / 2];
    int teamNo;
    int swapValue;
    int i;
    int j;

    mbPlayerColSnapSet(FALSE);
    for (i = 0; i < GW_PLAYER_MAX - 1; i++) {
        for (j = i + 1; j < GW_PLAYER_MAX; j++) {
            if (order[i] < order[j]) {
                swapValue = order[i];
                order[i] = order[j];
                order[j] = swapValue;
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

// Called by mbev_Opening for single-player mode; runs the guide and releases its temporary
// resources.
void mbev_OpeningSingle(void)
{
    SINGLEGUIDEWORK *guideState = &singleGuideWork;
    HuVecF *cameraPathTbl;
    HuVecF *masuPosTbl;
    float *pathLengthTbl;
    s16 winId;
    int playerNo;

    mbDirClose();
    guideState->endF = FALSE;
    guideState->dispF = FALSE;
    singleGuideObj = NULL;
    singleOpeningPosTbl2 = NULL;
    singleOpeningPosTbl = NULL;
    singleOpeningPathLengthTbl = NULL;
    singleWinId = -1;
    singleEff1MdlId = -1;
    singleFXNo = -1;
    winId = -1;
    if (!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        openingSingleProc = HuPrcChildCreate(ev_OpeningSingle, OPENING_PROCESS_PRIORITY,
            OPENING_PROCESS_STACK_SIZE, 0, mbMainProc);
    }
    HuPrcDestructorSet2(openingSingleProc, ev_OpeningSingleKill);
    do {
        if (guideState->dispF && mbTelopCheck()) {
            if (winId < 0) {
                winId = mbWinCreateHelp(MESSNUM(MESS_BOARD_OPE, 12));
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
                    guideState->endF = TRUE;
                    break;
                }
            }
        }
        HuPrcVSleep();
    } while (!guideState->endF);
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
    if (singleOpeningPathLengthTbl != NULL) {
        pathLengthTbl = singleOpeningPathLengthTbl;
        HuMemDirectFree(pathLengthTbl);
    }
    if (singleEff1MdlId != -1) {
        mbParticleKill(singleEff1MdlId);
        HuDataDirClose(DATA_effect);
    }
    if (singleFXNo != -1) {
        mbAudFXStop(singleFXNo);
    }
}

// Used by ev_OpeningSingle and OpeningBezierCalc for temporary single-player opening data.
static inline void *OpeningAlloc(s32 size)
{
    return HuMemDirectMallocNum(HEAP_HEAP, size, HU_MEMNUM_OVL);
}

// Used by OpeningBezierCalc to release its temporary control-point copy.
static inline void OpeningFree(void *ptr)
{
    HuMemDirectFree(ptr);
}

// Called by ev_OpeningSingle to evaluate the guide's Bezier path at t by repeated interpolation.
static inline void OpeningBezierCalc(HuVecF *points, int count, HuVecF *dst, float t)
{
    HuVecF *controlPointWork = OpeningAlloc(count * sizeof(HuVecF));
    int j;
    int k;

    memcpy(controlPointWork, points, count * sizeof(HuVecF));
    for (j = 1; j < count; j++) {
        for (k = 0; k < count - j; k++) {
            controlPointWork[k].x =
                controlPointWork[k].x + t * (controlPointWork[k + 1].x - controlPointWork[k].x);
            controlPointWork[k].y =
                controlPointWork[k].y + t * (controlPointWork[k + 1].y - controlPointWork[k].y);
            controlPointWork[k].z =
                controlPointWork[k].z + t * (controlPointWork[k + 1].z - controlPointWork[k].z);
        }
    }
    *dst = controlPointWork[0];
    OpeningFree(controlPointWork);
}

// Runs as the child created by mbev_OpeningSingle, moving the guide along the board's opening
// route.
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
    SINGLEGUIDEWORK *guideState = &singleGuideWork;
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

    startMasuId = mbMasuFind_AttrIdGet(-1, OPENING_MASU_FLAG_START);
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
    guideState->dispF = TRUE;
    cameraPos = singleOpeningPosTbl2[0];

    if (cameraType < 3) {
        mbCameraMovePos(&cameraPos, &singleGuideCameraRot, NULL, 2400.0f,
            -1.0f, 180);
        mbCameraMoveWait();
        HuPrcSleep(12);
        baseY = cameraPos.y;
        singleOpeningPathLengthTbl = OpeningAlloc(cameraPathNum * sizeof(float));
        for (i = 0; i < cameraPathNum - 1; i++) {
            OpeningCurveControlGet(singleOpeningPosTbl2, i, cameraPathNum,
                &curveStart, &curveEnd, &curveTangentIn, &curveTangentOut);

            distance = OpeningCurveLength((OPENINGCURVEEVALFUNC)(u32)OpeningCurveEval,
                &curveStart, &curveEnd, &curveTangentIn, &curveTangentOut, 1.0f);
            singleOpeningPathLengthTbl[i] = distance;
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
            if (distance >= singleOpeningPathLengthTbl[cameraPathNo]) {
                distance -= singleOpeningPathLengthTbl[cameraPathNo];
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
        singleOpeningPathLengthTbl = NULL;
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
    animP = HuSprAnimRead(HuDataReadNum(DATANUM(DATA_effect, 1), HU_MEMNUM_OVL));
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
    singleWinId = mbWinCreate(2, MESSNUM(MESS_BOARD_OPENING, 17), 6);
    mbWinTopPosGet(&winPos);
    winPos.y -= 35.0f;
    mbWinTopPosSet(winPos.x, winPos.y);
    mbWinTopWait();
    singleWinId = -1;

    mbAudGuidePlay(950);
    singleWinId = mbWinCreate(2, MESSNUM(MESS_BOARD_OPENING, 18), 6);
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
    guideState->dispF = FALSE;
    guideState->endF = TRUE;
    HuPrcEnd();
}

// Registered by mbev_OpeningSingle as the destructor that clears its child process handle.
static void ev_OpeningSingleKill(void)
{
    openingSingleProc = NULL;
}

// Returns camera zoom saved for the board opening.
float mbOpeningZoomGet(void)
{
    return openingZoom;
}

// Copies the camera rotation saved for the board opening to the caller.
void mbOpeningRotGet(HuVecF *rot)
{
    *rot = openingRot;
}

// Copies the camera center saved for the board opening to the caller.
void mbOpeningPosGet(HuVecF *pos)
{
    *pos = openingPos;
}

// Registers the callback shown when the party opening's first choice is accepted.
void mbOpeningInstHookSet(void (*hook)(void))
{
    openingInstHook = hook;
}

// Registers the callback used to show the star map during the party opening.
void mbOpeningStarInstHookSet(void (*hook)(void))
{
    openingStarInstHook = hook;
}

// Returns the party guide object created for the board opening.
MBMODELID mbOpeningGuideObjIdGet(void)
{
    return openingGuideObjId;
}

// Places the party guide beside the board's starting space and restores its idle motion.
void mbOpeningGuidePosRestore(void)
{
    s16 masuId;
    HuVecF masuPos;
    HuVecF pos;

    masuId = mbMasuFind_AttrIdGet(-1, OPENING_MASU_FLAG_START);
    mbMasuPosGet(masuId, &masuPos);
    pos = masuPos;
    pos.z -= 200.0f;
    mbObjPosSetV(openingGuideObjId, &pos);
    mbObjMotionShiftSet(openingGuideObjId, 1, 0.0f,
        8.0f, HU3D_MOTATTR_LOOP);
}

// Moves the camera back to the party opening's view of the starting space.
void mbOpeningCameraPosRestore(void)
{
    s16 masuId;
    HuVecF masuPos;
    HuVecF pos;

    masuId = mbMasuFind_AttrIdGet(-1, OPENING_MASU_FLAG_START);
    mbMasuPosGet(masuId, &masuPos);
    pos = masuPos;
    pos.y -= 50.0f;
    mbCameraMovePos(&pos, &openingCameraRestoreRot, &openingCameraRestoreOffset,
        1800.0f, -1.0f, 0);
}

// Called by ParticleDraw for the effect created in ev_OpeningSingle to update the guide's trail.
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
    MBPARTICLEDATA *particleData;
    float randomValue;
    int i;

    if (particleP->mode == 0) {
        particleData = particleP->data;
        particleP->blendMode = MB_PARTICLE_BLEND_ADDCOL;
        for (i = 0; i < particleP->num; i++, particleData++) {
            particleData->vel.y = 0.0f;
            particleData->scale = 0.0f;
            particleData->time = 0;
            particleData->activeF = -1;
        }
        particleP->mode = 1;
    }

    particleData = particleP->data;
    for (i = 0; i < particleP->num; i++, particleData++) {
        if (particleData->activeF < 0 && mbRandMod(100) < 5) {
            particleData->activeF = 60.0f * (0.4f + 0.4f * mbParticleRandF());

            randomValue = frandf();
            dir.x = frandf() - randomValue;
            randomValue = frandf();
            dir.y = frandf() - randomValue;
            randomValue = frandf();
            dir.z = frandf() - randomValue;
            VECNormalize(&dir, &dir);
            VECScale(&dir, &dir, 40.0f);

            particleData->pos.x = dir.x + guideCurPos.x;
            particleData->pos.y = dir.y + guideCurPos.y;
            particleData->pos.z = dir.z + guideCurPos.z;
            particleData->vel.x = 0.0f;
            particleData->vel.y = 0.0f;
            particleData->vel.z = 0.0f;
            particleData->guideScaleBase = 20.0f + 20.0f * frandf();
            particleData->color = colorTbl[mbRandMod(4)];
            if (!singleGuideEffOnF) {
                particleData->color.a = 0;
                particleData->scale = 0.0f;
            }
            particleData->time = 0;
        }

        if (particleData->activeF >= 0) {
            particleData->pos.y += 0.016666668f * particleData->vel.y;
            particleData->vel.y += -16.333334f;
            particleData->scale = particleData->guideScaleBase
                * (1.0f - (float)particleData->time / (float)particleData->activeF);
            if (particleData->time++ >= particleData->activeF) {
                particleData->activeF = -1;
            }
        }
    }
}
