/* Runs the shared board mode lifecycle, turn loop, and day/night transitions. */
#define _DOLPHIN_MATH
#define _MATH_H

#include "game/board/main.h"
#include "game/board/pause.h"
#include "game/board/player.h"
#include "game/hu3d.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/flag.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/saveload.h"
#include "string.h"

typedef void (*MBHOOK)(void);

static inline s16 GWMgNightFGet(void)
{
    return lbl_802C0278;
}

static inline void GWMgPackSet(s32 mgPack)
{
    lbl_80265B70.mgPack = mgPack;
}

static void mbOMDestroy(void);
static void mbMain(void);
static void mbMainKill(void);
void mbClose(void);
void mbInit(void);
void mbNextTime(void);
BOOL mbNextTimeSet(void);
void mbev_NextTimeSet(MBHOOK nextTimeHook);
void mbLightSet(void);

/* Position, direction, and RGBA color passed to the board's Hu3D light. */
static Vec lightPos = {-500.0f, 500.0f, 2000.0f};
static Vec lightDir = {0.5f, -0.5f, -2.0f};
/* Overlay selected for mbOMDestroy to enter after closing board resources. */
static OMOVL nextOvl = -1;
static s32 lbl_802BFBAC = 20;
static s32 lbl_802BFBB0 = 20;
/* White RGBA color paired with the board light position and direction. */
static GXColor lightColor = {255, 255, 255, 255};
static const float lbl_802C3088 = 10000.0f;
static const float lbl_802C308C = -45.0f;
static const float lbl_802C3090 = 0.0f;

static s32 gap_10_802C0D14;
/* Called by mbMain when curTime and nextTime differ. */
static MBHOOK ev_NextTime;
/* Called by mbMain when returning through the common load-time flag. */
static MBHOOK ev_LoadTime;
/* Called by mbMain after either mode's turn execution. */
static MBHOOK ev_TurnEnd;
/* Called by mbMain before a turn when no return event interrupts it. */
static MBHOOK ev_TurnStart;
/* Optional board-specific light setup/reset callbacks. */
static MBHOOK lightResetFunc;
static MBHOOK lightSetFunc;
/* Board mode callbacks retained until the board process is closed. */
static MBHOOK closeHook;
static MBHOOK initHook;
BOOL mbSaveNewF; /* Set while mbObjectSetup initializes a new board save. */
HUPROCESS *mbMainProc; /* Child process that runs mbMain. */
OMOBJMAN *mbObjMan; /* Object manager whose teardown invokes mbOMDestroy. */

extern void mbBoardDataDirRead(void);
extern void mbMathInit(void);
extern void mbMathClose(void);
extern void mbSNpcInit(void);
extern void mbTutorialInit(void);
extern void mbTutorialCreate(void);
extern void mbTutorialCall(s32 mode);
extern BOOL mbTutorialExitReqGet(void);
extern void mbConfigPadDisableSet(BOOL disableF);
extern void mbExitWatchCreate(void);
extern void mbExitReq(void);
extern void mbWipeWait(void);
extern void mbWipeFadeIn(void);
extern void mbWipeDissolveFadeOut(void);
extern void mbWipeSpecialInit(void);
extern void mbWipeSpecialClose(void);
extern void mbCameraInit(void);
extern void mbCameraZoomSet(float zoom);
extern void mbCameraRotSet(float x, float y, float z);
extern void mbCameraCenterSet(float x, float y, float z);
extern void mbCameraMoveOnSet(BOOL onF);
extern void mbCameraMoveStop(void);
extern void mbAudInit(void);
extern void mbAudClose(void);
extern void mbObjInit(void);
extern void mbObjClose(void);
extern void mbPauseInit(void);
extern void mbWinInit(void);
extern void mbWinClose(void);
extern void mbEffInit(void);
extern void mbEffClose(void);
extern void mbGateInit(void);
extern void mbGateClose(void);
extern void mbCapInit(void);
extern void mbCoinInit(void);
extern void mbCoinClose(void);
extern void mbBranchInit(void);
extern void mbStarInit(void);
extern void mbStarClose(void);
extern void mbStarFlagInit(void);
extern void mbStarNextNoSet(s32 no);
extern s32 mbStarNoRandGet(void);
extern void mbStarNoSet(s32 no);
extern void mbStarReset(void);
extern void mbMasuCapsuleReset(void);
extern void mbev_CapBankCoinInit(void);
extern void mbev_CapInit(void);
extern void mbMasuClose(void);
extern void mbMasuPlayerPrizeReset(s32 playerNo);
extern void mbStatusInit(void);
extern void mbStatusClose(void);
extern void mbStatusDispForceSetAll(BOOL dispF);
extern void mbStatusColorAllSet(s32 color);
extern void mbStatusMasuDispSet(BOOL dispF);
extern void mbDiceInit(void);
extern void mbDiceClose(void);
extern void mbTauntInit(void);
extern void mbTauntClose(void);
extern void mbScrollClose(void);
extern void fn_8019A620(void);
extern void mbSingleInit(void);
extern void mbSingleClose(void);
extern void mbSingleSaveInit(s32 teamChar, s32 mgPack, s32 storyComDif);
extern void mbSingleGameEnd(void);
extern s32 mbSingleCall(s32 mode, s32 modeArg);
extern s32 mbev_SingleMgEnd(s32 playerNo);
extern s32 mbev_MgCall(void);
extern void mbMgCallDataClose(void);
extern void mbMgCallInit(void);
extern void mbev_Opening(void);
extern void mbev_Last5(void);
extern void mbev_CapKettouEndCall(s32 playerNo);
extern void mbev_CapDonkeyEndCall(s32 playerNo);
extern void mbev_CapKoopaEndCall(s32 playerNo);
extern void mbMusBoardPlay(void);
extern void mbTelopLastTurnCreate(void);
extern void mbTelopTimeChangeCreate(void);
extern void mbTelopTimeChangeKill(void);
extern BOOL mbTelopTimeChangeCheck(void);
extern void HuAudAllStop(void);
extern void mbLightFuncSet(MBHOOK lightSetHook, MBHOOK lightResetHook);
extern void mbDirClose(void);
extern void mbBGReadWait(s32 dataReadStat);
extern s32 mbBGRead(s32 boardDataNum);
void evdef_ChangeTime(void);
BOOL mbReturnMgCheck(void);

/* Sets board state and starts mbMain as the board object's child process. */
void mbObjectSetup(s32 boardNo, MBHOOK boardInit, MBHOOK boardClose)
{
    omSysPauseEnable(FALSE);
    mbPauseDisableSet(TRUE);
    mbSaveNewF = !fn_8006A124(65536);
    mbBoardDataDirRead();
    mbMathInit();
    if (mbSaveNewF) {
        if (!fn_8006A124(65550)) {
            if (GWPartyGet() != FALSE) {
                GWMgInstDispSet(lbl_80265E30.partyMgInstDispF);
                GWMgComDispSet(lbl_80265E30.partyMgComDispF);
                GWMgPackSet(lbl_80265E30.partyMgPack);
                GWMessSpeedSet(lbl_80265E30.partyMessSpeed);
                GWSaveModeSet(lbl_80265E30.partySaveMode);
            } else {
                GWMgInstDispSet(lbl_80265E30.storyMgInstDispF);
                GWMgComDispSet(lbl_80265E30.storyMgComDispF);
                GWMgPackSet(lbl_80265E30.storyMgPack);
                GWMessSpeedSet(lbl_80265E30.storyMessSpeed);
                GWSaveModeSet(lbl_80265E30.storySaveMode);
            }
        } else {
            lbl_80265B70.curTime = 0;
            lbl_802C0278 = 0;
        }
        fn_8006A1C4(65536);
        fn_8006A268(65537);
        fn_8006A268(65538);
        fn_8006A268(65541);
        fn_8006A268(65542);
        fn_8006A268(65539);
        fn_8006A268(65540);
        lbl_80265B70.turnNo = 1;
        mbStarFlagInit();
        mbStarNextNoSet(0);
        lbl_80265B70.starTotal = 0;
        lbl_80265B70.last5Effect = 0;
        lbl_80265B70.curTime = lbl_80265B70.nextTime = 0;
        if (GWMgNightFGet() == 1) {
            lbl_80265B70.curTime = lbl_80265B70.nextTime = 1;
        }
        lbl_80265B70.timeTurn = 0;
        lbl_80265B70.timeTurnMax = 3;
        fn_8006A268(131073);
        fn_8006A268(131075);
        fn_8006A268(131079);
        fn_8006A268(131080);
        fn_8006A268(131072);
        fn_8006A268(131074);
        fn_8006A268(131076);
        fn_8006A268(131078);
        fn_8006A268(3);
        mbMgCallInit();
        mbStarReset();
        mbMasuCapsuleReset();
        mbev_CapBankCoinInit();
        saveExecF = FALSE;
    }
    if (fn_8006A124(65571)) {
        mbMgCallInit();
        mbStarReset();
        fn_8006A268(65571);
        saveExecF = TRUE;
    }
    nextOvl = -1;
    fn_8006A1C4(65570);
    fn_8006A1C4(65552);
    fn_8006A268(65557);
    fn_8006A268(65558);
    fn_8006A268(65559);
    fn_8006A268(65562);
    fn_8006A268(65564);
    fn_8006A268(65565);
    fn_8006A268(65569);
    mbConfigPadDisableSet(TRUE);
    initHook = boardInit;
    closeHook = boardClose;
    lightSetFunc = lightResetFunc = NULL;
    mbSNpcInit();
    mbTutorialInit();
    mbev_CapInit();
    ev_TurnStart = ev_TurnEnd = NULL;
    ev_LoadTime = ev_NextTime = NULL;
    mbev_NextTimeSet((MBHOOK)evdef_ChangeTime);
    mbExitWatchCreate();
    mbObjMan = fn_8003A8BC(64, 8194);
    HuPrcDestructorSet2(mbObjMan, mbOMDestroy);
    omSystemKeyCheckSetup(mbObjMan);
    mbMainProc = fn_8000DAC0(mbMain, 8208, 0x8000, 0, mbObjMan);
    HuPrcDestructorSet2(mbMainProc, mbMainKill);
    lbl_80265B70.boardNo = boardNo;
}

/* Closes board resources, then follows the requested overlay or returns. */
static void mbOMDestroy(void)
{
    s32 currentOvl;
    mbClose();
    if (nextOvl != -1) {
        omOvlCallEx(nextOvl, TRUE, 0, 0);
        return;
    }
    if (fn_8006A124(65554)) {
        currentOvl = fn_8003B774();
        fn_8006A268(65554);
        omOvlGotoEx(currentOvl, TRUE, 0, 0);
        return;
    }
    fn_8006A268(65536);
    if (fn_8006A124(65569)) {
        fn_8003A5E4(1, TRUE);
    } else {
        if (GWPartyGet() != FALSE && !fn_8006A124(65550)) {
            omOvlGotoEx(92, TRUE, 0, 0);
        } else {
            fn_8003A5E4(1, TRUE);
        }
    }
}

/* Runs after mbObjectSetup starts the board process; handles entry, turns, and return events. */
static void mbMain(void)
{
    s32 playerNo;
    s32 turnInterruptF = FALSE;
    s32 minigameCalledF;
    s32 currentTime;
    s32 nightF;

    mbWipeWait();
    if (lbl_80265B70.turnNo > lbl_80265B70.turnMax && !fn_8006A124(65550)) {
        fn_8006A268(65554);
        mbExitReq();
        HuPrcSleep(-1);
    }
    mbInit();
    if (!fn_8006A124(65537)) {
        if (fn_8006A124(131073) && !fn_8006A124(65550)) {
            BOOL partyModeF = lbl_80265B70.partyF;

            if (partyModeF != FALSE) {
                s32 starNo = mbStarNoRandGet();
                if (starNo >= 0) {
                    mbStarNoSet(starNo);
                }
            }
        } else {
            lbl_80265B70.turnPlayerNo = -1;
            if (!fn_8006A124(65550)) {
                mbev_Opening();
            } else {
                mbMusBoardPlay();
                fn_8006A268(65570);
                mbTutorialCall(0);
            }
        }
        {
            BOOL partyModeF = lbl_80265B70.partyF;

            if (partyModeF != FALSE) {
                if (!GWTeamFGet()) {
                    for (playerNo = 0; playerNo < 4; playerNo++) {
                        mbPlayerCoinSet(playerNo, 10);
                    }
                } else {
                    for (playerNo = 0; playerNo < 2; playerNo++) {
                        mbPlayerTeamCoinSet(playerNo, 20);
                    }
                }
            } else {
                for (playerNo = 0; playerNo < 4; playerNo++) {
                    mbPlayerCoinSet(playerNo, 0);
                }
            }
        }
        lbl_80265B70.turnPlayerNo = 0;
        fn_8006A1C4(65537);
    }
    if (lbl_80265B70.curTime != lbl_80265B70.nextTime) {
        lbl_80265B70.timeTurn = 0;
        currentTime = lbl_80265B70.curTime;
        lbl_802C0278 = currentTime;
        if (ev_NextTime != NULL) {
            mbMusBoardPlay();
            fn_8006A268(65570);
            ev_NextTime();
            mbTelopTimeChangeKill();
            mbCameraMoveStop();
        }
        lbl_80265B70.nextTime = lbl_80265B70.curTime;
    }
    {
        BOOL isDayF;

        isDayF = (lbl_80265B70.curTime == 0);
        nightF = isDayF ? FALSE : TRUE;
        lbl_802C0278 = nightF;
    }
    if (mbReturnMgCheck()) {
        if (GWPartyGet() == FALSE) {
            turnInterruptF = mbev_SingleMgEnd(0);
        } else {
            if (fn_8006A124(65538)) {
                fn_8006A268(65538);
            } else if (fn_8006A124(65541)) {
                mbev_CapKettouEndCall(lbl_80265B70.turnPlayerNo);
                fn_8006A268(65541);
                turnInterruptF = TRUE;
            } else if (fn_8006A124(65542)) {
                mbev_CapDonkeyEndCall(lbl_80265B70.turnPlayerNo);
                fn_8006A268(65542);
                turnInterruptF = TRUE;
            } else if (fn_8006A124(65539)) {
                mbev_CapKoopaEndCall(lbl_80265B70.turnPlayerNo);
                fn_8006A268(65539);
                turnInterruptF = TRUE;
            } else if (fn_8006A124(65555)) {
                if (ev_LoadTime != NULL) {
                    ev_LoadTime();
                }
                fn_8006A268(65555);
                turnInterruptF = TRUE;
            }
            fn_8006A268(65554);
        }
    }
    while (1) {
    if (ev_TurnStart && !turnInterruptF) {
        ev_TurnStart();
    }
    if (lbl_80265B70.turnPlayerNo == 0 && !turnInterruptF
        && lbl_80265B70.turnMax - lbl_80265B70.turnNo < 5) {
        if (!fn_8006A124(3) && GWPartyGet() != FALSE) {
            mbev_Last5();
            fn_8006A1C4(3);
        } else {
            mbTelopLastTurnCreate();
            if (!fn_8006A124(3)) {
                if (GWPartyGet() == FALSE) {
                    mbSingleCall(12, 0);
                    fn_8006A1C4(3);
                }
            }
        }
    }
    mbMusBoardPlay();
    fn_8006A268(65570);
    if (fn_8006A124(65550)) {
        mbTutorialCall(1);
        if (mbTutorialExitReqGet()) {
            HuPrcSleep(-1);
        }
    }
    if (GWPartyGet() != FALSE) {
        mbStatusDispForceSetAll(TRUE);
        mbTurnExec(turnInterruptF);
        if (ev_TurnEnd) {
            ev_TurnEnd();
        }
        lbl_80265B70.turnNo++;
        lbl_80265B70.timeTurn++;
        lbl_80265B70.turnPlayerNo = 0;
        if (fn_8006A124(65550)) {
            mbTutorialCall(2);
            mbStatusColorAllSet(0);
            mbNextTime();
            continue;
        } else if (fn_8006A124(131072)) {
            turnInterruptF = FALSE;
            mbStatusColorAllSet(0);
            mbNextTime();
            continue;
        } else {
            lbl_80265B70.turnPlayerNo = -1;
            minigameCalledF = mbev_MgCall();
            mbStatusColorAllSet(0);
            if (!minigameCalledF) {
                mbNextTimeSet();
                HuPrcSleep(-1);
            } else {
                lbl_80265B70.turnPlayerNo = 0;
                mbNextTime();
            }
        }
    } else {
        mbStatusDispForceSetAll(TRUE);
        mbStatusMasuDispSet(TRUE);
        mbSingleTurnExec(turnInterruptF);
        if (ev_TurnEnd) {
            ev_TurnEnd();
        }
        lbl_80265B70.turnNo++;
        if (lbl_80265B70.turnNo > lbl_80265B70.turnMax) {
            mbSingleGameEnd();
            HuPrcSleep(-1);
        }
        lbl_80265B70.turnPlayerNo = 0;
        if (fn_8006A124(131072)) {
            turnInterruptF = FALSE;
            mbStatusColorAllSet(0);
            continue;
        }
        if (fn_8006A124(65550)) {
            mbStatusColorAllSet(0);
            continue;
        }
        lbl_80265B70.turnPlayerNo = -1;
        mbStatusColorAllSet(0);
        lbl_80265B70.turnPlayerNo = 0;
    }
    turnInterruptF = FALSE;
    }
}

static void mbMainKill(void)
{
    mbMainProc = NULL;
}

/* Changes the day/night value and leaves nextTime at the previous value to signal the
 * transition. */
void mbChangeTimeSet(void)
{
    /* mbMain compares these fields before running the time-change event. */
    lbl_80265B70.nextTime = lbl_80265B70.curTime;
    lbl_80265B70.curTime ^= 1;
}

/* Advances day/night only after the configured number of turns; reports whether it changed. */
BOOL mbNextTimeSet(void)
{
    s32 turnsPerTime = lbl_80265B70.timeTurnMax;
    s32 turnsInTime = lbl_80265B70.timeTurn;

    if (turnsInTime >= turnsPerTime) {
        mbChangeTimeSet();
        return TRUE;
    }
    return FALSE;
}

/* Handles a board-requested time change by marking a return event and exiting the process. */
void mbChangeTime(void)
{
    nextOvl = -1;
    fn_8006A1C4(65554);
    fn_8006A1C4(65555);
    mbChangeTimeSet();
    mbExitReq();
    HuPrcSleep(-1);
}

/* Called from mbMain after a turn; exits to the board overlay when this time segment ends. */
void mbNextTime(void)
{
    BOOL advancesTimeF;
    s32 turnsPerTime = lbl_80265B70.timeTurnMax;
    s32 turnsInTime = lbl_80265B70.timeTurn;

    if (turnsInTime >= turnsPerTime) {
        mbChangeTimeSet();
        advancesTimeF = TRUE;
    } else {
        advancesTimeF = FALSE;
    }
    if (advancesTimeF) {
        nextOvl = -1;
        fn_8006A1C4(65554);
        fn_8006A1C4(65538);
        mbExitReq();
        HuPrcSleep(-1);
    }
}

/* Default ev_NextTime callback installed by mbObjectSetup; frames the day/night change. */
void evdef_ChangeTime(void)
{
    mbCameraZoomSet(lbl_802C3088);
    mbCameraRotSet(lbl_802C308C, lbl_802C3090, lbl_802C3090);
    mbCameraCenterSet(lbl_802C3090, lbl_802C3090, lbl_802C3090);
    mbCameraMoveOnSet(FALSE);
    mbWipeFadeIn();
    mbTelopTimeChangeCreate();
    while (mbTelopTimeChangeCheck()) {
        HuPrcVSleep();
    }
    mbWipeDissolveFadeOut();
}

/* Requests an overlay transition; mbOMDestroy performs it after board cleanup. */
void mbOvlCall(OMOVL nextOverlay)
{
    nextOvl = nextOverlay;
    mbExitReq();
}

/* Reports whether mbMain must process a minigame return or board-move event. */
BOOL mbReturnMgCheck(void)
{
    if (fn_8006A124(65538)
        || fn_8006A124(65541)
        || fn_8006A124(65539)
        || fn_8006A124(65542)
        || fn_8006A124(65555)
        || fn_8006A124(65554)) {
        return TRUE;
    }
    return FALSE;
}

/* Initializes board subsystems from mbMain before its opening flow and turn loop. */
void mbInit(void)
{
    if (!SLSaveFlagGet()) {
        GWSaveModeSet(1);
    }
    if (GWMgInstDispGet()) {
        fn_8006A1C4(5);
    } else {
        fn_8006A268(5);
    }
    if (!mbReturnMgCheck()) {
        fn_8006A268(65538);
        fn_8006A268(65541);
        fn_8006A268(65539);
        fn_8006A268(65542);
        fn_8006A268(65540);
        fn_8006A268(65555);
        fn_8006A268(65554);
    }
    if (mbReturnMgCheck()) {
        mbMgCallDataClose();
    }
    HuSprExecLayerCameraSet(32, 2, 7);
    HuSprExecLayerCameraSet(64, 4, 3);
    mbCameraInit();
    mbAudInit();
    mbObjInit();
    mbPauseInit();
    mbWinInit();
    mbEffInit();
    mbWipeSpecialInit();
    mbGateInit();
    mbCapInit();
    mbCoinInit();
    mbBranchInit();
    mbStarInit();
    mbPlayerTurnInitHookSet(NULL);
    mbPlayerTurnCloseHookSet(NULL);
    initHook();
    mbPlayerInit(!mbReturnMgCheck());
    mbStatusInit();
    mbDiceInit();
    mbLightSet();
    fn_8005B7BC(-1);
    mbTauntInit();
    fn_8002A2A4(0);
    mbCapMasuObjInit();
    if (GWPartyGet() == FALSE) {
        mbSingleInit();
    }
    if (fn_8006A124(65550)) {
        mbTutorialCreate();
    }
    fn_8006A1C4(65557);
}

/* Releases board subsystems when mbOMDestroy tears down the board object manager; a clear init flag
 * takes the short data/math cleanup path. */
void mbClose(void)
{
    fn_8000BFB4(fn_8000B7A8(3), -1);
    if (!fn_8006A124(65557)) {
        mbDirClose();
        mbMathClose();
        return;
    }
    if (GWPartyGet() == FALSE) {
        mbSingleClose();
    }
    mbTauntClose();
    mbAudClose();
    HuAudAllStop();
    mbDiceClose();
    mbStatusClose();
    mbMasuClose();
    mbStarClose();
    mbGateClose();
    fn_8019A620();
    mbScrollClose();
    mbPlayerClose();
    if (closeHook) {
        closeHook();
    }
    mbCoinClose();
    mbEffClose();
    mbObjClose();
    mbWinClose();
    mbDirClose();
    initHook = closeHook = NULL;
    ev_TurnStart = NULL;
    ev_TurnEnd = NULL;
    ev_LoadTime = NULL;
    ev_NextTime = NULL;
    fn_8006A1C4(65570);
    fn_8006A1C4(65552);
    fn_8006A1C4(65558);
    mbMathClose();
}

void mbDirClose(void)
{
    fn_80009A30();
}

/* Installs optional board-specific callbacks run by mbLightSet and mbLightReset. */
void mbLightFuncSet(MBHOOK lightSetHook, MBHOOK lightResetHook)
{
    lightSetFunc = lightSetHook;
    lightResetFunc = lightResetHook;
}

/* Applies the board light hook, creates a global light from the configured vectors/color, and marks
 * it non-static and infinite. */
void mbLightSet(void)
{
    HU3D_LIGHTID lightId;
    Hu3DBGColorSet(0, 0, 0);
    if (lightSetFunc) {
        lightSetFunc();
    }
    lightId = (s16)fn_800286D0(&lightPos, &lightDir, &lightColor);
    /* Hu3DGLightCreateV can return HU3D_LIGHTID_NONE; this code passes the ID to both setters
     * without checking it. */
    Hu3DGLightStaticSet(lightId, FALSE);
    Hu3DGLightInfinitytSet((s16)lightId);
}

/* Runs the reset hook, sets the background to black, disables fog, and selects reflection map
 * zero. */
void mbLightReset(void)
{
    if (lightResetFunc) {
        lightResetFunc();
    }
    Hu3DBGColorSet(0, 0, 0);
    Hu3DFogClear();
    fn_8002A2A4(0);
}

void mbPauseEnableSet(void)
{
    fn_8006A1C4(65559);
}

void mbPauseEnableReset(void)
{
    fn_8006A268(65559);
}

BOOL mbPauseEnableCheck(void)
{
    return fn_8006A124(65559) ? TRUE : FALSE;
}

s32 mbBGRead(s32 boardDataNum)
{
    return fn_80007F20(boardDataNum);
}

/* Waits until the data-read status is complete, or returns when there is no status to wait on. */
void mbBGReadWait(s32 dataReadStat)
{
    if (dataReadStat == -1) {
        return;
    }
    while (fn_80008270(dataReadStat) == FALSE) {
        HuPrcVSleep();
    }
}

void mbev_NextTimeSet(MBHOOK nextTimeHook)
{
    ev_NextTime = nextTimeHook;
}

void mbev_LoadTimeSet(MBHOOK loadTimeHook)
{
    ev_LoadTime = loadTimeHook;
}

void mbev_TurnStartSet(MBHOOK turnStartHook)
{
    ev_TurnStart = turnStartHook;
}

void mbev_TurnEndSet(MBHOOK turnEndHook)
{
    ev_TurnEnd = turnEndHook;
}

void fn_8014C3B4(s32 storedValue)
{
    lbl_802BFBAC = storedValue;
}

void fn_8014C3BC(s32 storedValue)
{
    lbl_802BFBB0 = storedValue;
}

s32 fn_8014C3C4(void)
{
    return lbl_802BFBAC;
}

s32 fn_8014C3CC(void)
{
    return lbl_802BFBB0;
}

/* Resets board save fields and player totals before a new game; single-player/tutorial setup also
 * clears the team flag in the player loop. */
void mbSaveInit(s32 boardNo)
{
    s32 playerNo;
    s32 capsuleNo;
    lbl_80265B70.boardNo = boardNo;
    fn_8006A268(65536);
    fn_8006A268(65551);
    lbl_80265B70.mgNo = 0;
    lbl_80265B70.subGameNo = 0;
    lbl_80265B70.hiddenBlockMasuId = 0;
    memset(&lbl_80265B70.boardWork[0], 0, sizeof(lbl_80265B70.boardWork));
    lbl_80265B70.turnNo = 1;
    for (playerNo = 0; playerNo < 4; playerNo++) {
        lbl_80265750[playerNo].coin = 0;
        if (GWPartyGet() == FALSE || fn_8006A124(65550)) {
            lbl_80265750[playerNo].star = 0;
            lbl_80265B70.tagF = FALSE;
        } else {
            lbl_80265750[playerNo].star = mbPlayerHandicapGet(playerNo);
        }
        mbMasuPlayerPrizeReset(playerNo);
        lbl_80265750[playerNo].coinTotalMg = 0;
        lbl_80265750[playerNo].coinTotal = 0;
        lbl_80265750[playerNo].coinMax = 0;
        lbl_80265750[playerNo].starMax = 0;
        lbl_80265750[playerNo].coinBattle = 0;
        lbl_80265750[playerNo].mgCoin = 0;
        lbl_80265750[playerNo].mgCoinBonus = 0;
        lbl_80265750[playerNo].capsuleUseNum = 0;
        for (capsuleNo = 0; capsuleNo < 3; capsuleNo++) {
            lbl_80265750[playerNo].capsule[capsuleNo] = -1;
        }
    }
}

/* Applies single-player save setup and sets the night flag to daytime. */
void mbSaveStoryInit(s32 teamChar, s32 mgPack, s32 storyComDif)
{
    mbSingleSaveInit(teamChar, mgPack, storyComDif);
    lbl_802C0278 = 0;
}

/* Forces party mode on, stores party settings and handicaps, then clears tutorial/init state. */
void mbSavePartyInit(BOOL teamsF, BOOL bonusStarsF, s32 mgPack, s32 turnMax,
                     s32 player1Handicap, s32 player2Handicap,
                     s32 player3Handicap, s32 player4Handicap)
{
    lbl_80265B70.partyF = TRUE;
    lbl_80265B70.tagF = teamsF;
    lbl_80265B70.storyComDif = 0;
    lbl_80265B70.bonusStarF = bonusStarsF;
    lbl_80265B70.mgPack = mgPack;
    lbl_80265B70.turnMax = turnMax;
    lbl_80265750[0].handicap = player1Handicap;
    lbl_80265750[1].handicap = player2Handicap;
    lbl_80265750[2].handicap = player3Handicap;
    lbl_80265750[3].handicap = player4Handicap;
    fn_8006A268(65550);
    fn_8006A1C4(5);
    fn_8006A1C4(65549);
    fn_8006A268(65556);
    fn_8006A268(196610);
    lbl_80265B70.curTime = 0;
    lbl_802C0278 = 0;
}

void fn_8014C818(void)
{
}

void fn_8014C81C(void)
{
}
