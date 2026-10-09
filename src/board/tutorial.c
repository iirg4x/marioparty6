/* Coordinates the board tutorial, its guidance windows, and minigame selection. */
#define _MATH_H
#include "dolphin/math.h"

#include "game/armem.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/guide.h"
#include "game/board/main.h"
#include "game/board/player.h"
#include "game/board/tutorial.h"
#include "game/board/window.h"
#include "game/esprite.h"
#include "game/mgdata.h"
#include "game/pad.h"
#include "game/wipe.h"
#include "messdir_enum.h"

extern BOOL mbSaveNewF;
extern void mbExitReq(void);
extern void mbPauseEnableSet(void);
extern BOOL mbWipeSpecialCheck(void);
extern BOOL mbWipeSpecialStatGet(void);
extern void mbWipeWait(void);

typedef struct TutorialCallWork_s {
    int scene; /* Board event scene currently waiting for a tutorial response. */
    int callNum; /* Repeated board-event calls left before the tutorial advances. */
    int result; /* Value returned by the board event after its tutorial call. */
    int stat; /* 0 while a call is pending, positive while ready, negative when ended. */
    int mode; /* Tutorial mode selected by the board setup. */
} TUTORIALCALLWORK;

typedef struct TutorialWinData_s {
    s8 stat; /* 0 while tracking an open window; negative when no window is tracked. */
    s8 delay; /* One-frame transition delay after the window enters its closing state. */
    s16 time; /* Frame count accumulated during the window closing transition. */
} TUTORIALWINDATA;

typedef struct TutorialGuideData_s {
    int message; /* Guide message ID whose voice cue is listed here. */
    int seId; /* Sound effect played when that guide message opens. */
} TUTORIALGUIDEDATA;

typedef struct TutorialMgCallWork_s {
    u8 killF : 1; /* Requests removal of this minigame choice object. */
    u8 slideInF : 1; /* Set until the choice finishes entering from the screen edge. */
    u8 state : 3; /* Animation state: slide-in, idle, selected, 8-frame transition, grow, or
                   * drop. */
    u8 cursorNo : 3; /* This choice's slot in the four-item roulette. */
    u8 dispF : 1; /* Whether its help window is displayed. */
    s16 winNo; /* Help-window handle for this choice. */
    s16 unused; /* Reserved short; this file does not read it. */
    s16 time; /* Current frame within the active animation. */
    s16 maxTime; /* Last frame of the active animation. */
    int message; /* Minigame name message, or -1 when the slot is empty. */
} TUTORIALMGCALLWORK;

typedef struct TutorialMgCallData_s {
    int active; /* Nonzero while the minigame roulette owns its effects and objects. */
    int unused; /* Reserved word; this file does not read it. */
    s16 espId[3]; /* Sprite handles for the roulette frame, highlight, and selector. */
    s16 unusedShorts[5]; /* Reserved handles; this file does not use them. */
    OMOBJ *obj[4]; /* The four displayed minigame choices. */
} TUTORIALMGCALLDATA;

static s16 tutorialSprId[16];
static s16 tutorialSprGrpId[16];
static s16 tutorialMdlId[32];
static TUTORIALCALLWORK tutorialCallWork;
static TUTORIALWINDATA tutorialWinData[HUWIN_MAX];
static TUTORIALMGCALLDATA tutorialMgCallData;
static HUPROCESS *tutorialWatchProc;
static HUPROCESS *tutorialMainProc;
static BOOL tutorialExitReqF;
static BOOL tutorialExitOnF;
static s16 tutorialMgCallCursorPos;
static TUTORIALMAINFUNC tutorialMain;
static OMOBJ *tutorialGuideObj;

/* Message IDs for board tutorial guide windows. */
#define MES_BOARD_GUIDE_0001 MESSNUM(MESS_BOARD_TUTORIAL, 1)
#define MES_BOARD_GUIDE_0007 MESSNUM(MESS_BOARD_TUTORIAL, 7)
#define MES_BOARD_GUIDE_0008 MESSNUM(MESS_BOARD_TUTORIAL, 8)
#define MES_BOARD_GUIDE_000A MESSNUM(MESS_BOARD_TUTORIAL, 10)
#define MES_BOARD_GUIDE_000B MESSNUM(MESS_BOARD_TUTORIAL, 11)
#define MES_BOARD_GUIDE_000C MESSNUM(MESS_BOARD_TUTORIAL, 12)
#define MES_BOARD_GUIDE_000D MESSNUM(MESS_BOARD_TUTORIAL, 13)
#define MES_BOARD_GUIDE_000E MESSNUM(MESS_BOARD_TUTORIAL, 14)
#define MES_BOARD_GUIDE_000F MESSNUM(MESS_BOARD_TUTORIAL, 15)
#define MES_BOARD_GUIDE_0010 MESSNUM(MESS_BOARD_TUTORIAL, 16)
#define MES_BOARD_GUIDE_0011 MESSNUM(MESS_BOARD_TUTORIAL, 17)
#define MES_BOARD_GUIDE_0014 MESSNUM(MESS_BOARD_TUTORIAL, 20)
#define MES_BOARD_GUIDE_0015 MESSNUM(MESS_BOARD_TUTORIAL, 21)
#define MES_BOARD_GUIDE_0016 MESSNUM(MESS_BOARD_TUTORIAL, 22)
#define MES_BOARD_GUIDE_0017 MESSNUM(MESS_BOARD_TUTORIAL, 23)
#define MES_BOARD_GUIDE_0018 MESSNUM(MESS_BOARD_TUTORIAL, 24)
#define MES_BOARD_GUIDE_0019 MESSNUM(MESS_BOARD_TUTORIAL, 25)
#define MES_BOARD_GUIDE_001B MESSNUM(MESS_BOARD_TUTORIAL, 27)
#define MES_BOARD_GUIDE_001C MESSNUM(MESS_BOARD_TUTORIAL, 28)
#define MES_BOARD_GUIDE_001D MESSNUM(MESS_BOARD_TUTORIAL, 29)
#define MES_BOARD_GUIDE_001E MESSNUM(MESS_BOARD_TUTORIAL, 30)
#define MES_BOARD_GUIDE_001F MESSNUM(MESS_BOARD_TUTORIAL, 31)
#define MES_BOARD_GUIDE_0020 MESSNUM(MESS_BOARD_TUTORIAL, 32)
#define MES_BOARD_GUIDE_0021 MESSNUM(MESS_BOARD_TUTORIAL, 33)
#define MES_BOARD_GUIDE_0023 MESSNUM(MESS_BOARD_TUTORIAL, 35)
#define MES_BOARD_GUIDE_0024 MESSNUM(MESS_BOARD_TUTORIAL, 36)
#define MES_BOARD_GUIDE_0025 MESSNUM(MESS_BOARD_TUTORIAL, 37)
#define MES_BOARD_GUIDE_0026 MESSNUM(MESS_BOARD_TUTORIAL, 38)
#define MES_BOARD_GUIDE_0027 MESSNUM(MESS_BOARD_TUTORIAL, 39)
#define MES_BOARD_GUIDE_0028 MESSNUM(MESS_BOARD_TUTORIAL, 40)
#define MES_BOARD_GUIDE_0029 MESSNUM(MESS_BOARD_TUTORIAL, 41)
#define MES_BOARD_GUIDE_002A MESSNUM(MESS_BOARD_TUTORIAL, 42)
#define MES_BOARD_GUIDE_002D MESSNUM(MESS_BOARD_TUTORIAL, 45)
#define MES_BOARD_GUIDE_002E MESSNUM(MESS_BOARD_TUTORIAL, 46)
#define MES_BOARD_GUIDE_002F MESSNUM(MESS_BOARD_TUTORIAL, 47)
#define MES_BOARD_GUIDE_0031 MESSNUM(MESS_BOARD_TUTORIAL, 49)
#define MES_BOARD_GUIDE_0032 MESSNUM(MESS_BOARD_TUTORIAL, 50)
#define MES_BOARD_GUIDE_0033 MESSNUM(MESS_BOARD_TUTORIAL, 51)
#define MES_BOARD_GUIDE_0034 MESSNUM(MESS_BOARD_TUTORIAL, 52)
#define MES_BOARD_GUIDE_0000 MESSNUM(MESS_BOARD_TUTORIAL, 0)
#define MES_BOARD_GUIDE_003D MESSNUM(MESS_BOARD_TUTORIAL, 61)
#define MES_BOARD_GUIDE_0048 MESSNUM(MESS_BOARD_TUTORIAL, 72)
#define MES_BOARD_GUIDE_0035 MESSNUM(MESS_BOARD_TUTORIAL, 53)
#define MES_BOARD_GUIDE_0036 MESSNUM(MESS_BOARD_TUTORIAL, 54)
#define MES_BOARD_GUIDE_0038 MESSNUM(MESS_BOARD_TUTORIAL, 56)
#define MES_BOARD_GUIDE_0039 MESSNUM(MESS_BOARD_TUTORIAL, 57)
#define MES_BOARD_GUIDE_003A MESSNUM(MESS_BOARD_TUTORIAL, 58)
#define MES_BOARD_GUIDE_003C MESSNUM(MESS_BOARD_TUTORIAL, 60)
#define MES_BOARD_GUIDE_003E MESSNUM(MESS_BOARD_TUTORIAL, 62)
#define MES_BOARD_GUIDE_003F MESSNUM(MESS_BOARD_TUTORIAL, 63)
#define MES_BOARD_GUIDE_0040 MESSNUM(MESS_BOARD_TUTORIAL, 64)
#define MES_BOARD_GUIDE_0041 MESSNUM(MESS_BOARD_TUTORIAL, 65)
#define MES_BOARD_GUIDE_0042 MESSNUM(MESS_BOARD_TUTORIAL, 66)
#define MES_BOARD_GUIDE_0043 MESSNUM(MESS_BOARD_TUTORIAL, 67)
#define MES_BOARD_GUIDE_0044 MESSNUM(MESS_BOARD_TUTORIAL, 68)
#define MES_BOARD_GUIDE_0045 MESSNUM(MESS_BOARD_TUTORIAL, 69)
#define MES_BOARD_GUIDE_0046 MESSNUM(MESS_BOARD_TUTORIAL, 70)
#define MES_BOARD_GUIDE_0047 MESSNUM(MESS_BOARD_TUTORIAL, 71)
#define MES_BOARD_GUIDE_0049 MESSNUM(MESS_BOARD_TUTORIAL, 73)
#define MES_BOARD_GUIDE_004A MESSNUM(MESS_BOARD_TUTORIAL, 74)
#define MES_BOARD_GUIDE_004C MESSNUM(MESS_BOARD_TUTORIAL, 76)
#define MES_BOARD_GUIDE_004D MESSNUM(MESS_BOARD_TUTORIAL, 77)
#define MES_BOARD_GUIDE_004E MESSNUM(MESS_BOARD_TUTORIAL, 78)
#define MES_BOARD_GUIDE_004F MESSNUM(MESS_BOARD_TUTORIAL, 79)
#define MES_BOARD_GUIDE_0050 MESSNUM(MESS_BOARD_TUTORIAL, 80)
#define MES_BOARD_GUIDE_0051 MESSNUM(MESS_BOARD_TUTORIAL, 81)
#define MES_BOARD_GUIDE_0052 MESSNUM(MESS_BOARD_TUTORIAL, 82)
#define MES_BOARD_GUIDE_0054 MESSNUM(MESS_BOARD_TUTORIAL, 84)
#define MES_BOARD_GUIDE_0055 MESSNUM(MESS_BOARD_TUTORIAL, 85)
#define MES_BOARD_GUIDE_0057 MESSNUM(MESS_BOARD_TUTORIAL, 87)
#define MES_BOARD_GUIDE_0058 MESSNUM(MESS_BOARD_TUTORIAL, 88)
#define MES_BOARD_GUIDE_0059 MESSNUM(MESS_BOARD_TUTORIAL, 89)
#define MES_BOARD_GUIDE_005B MESSNUM(MESS_BOARD_TUTORIAL, 91)
#define MES_BOARD_GUIDE_005C MESSNUM(MESS_BOARD_TUTORIAL, 92)

static TUTORIALGUIDEDATA tutorialGuideTbl[] = {
    { MES_BOARD_GUIDE_0000, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0001, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0007, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0008, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_000A, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_000B, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_000C, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_000D, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_000E, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_000F, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0010, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0011, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0014, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0015, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0016, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0017, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0018, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0019, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_001B, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_001C, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_001D, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_001E, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_001F, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0020, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0021, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0023, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0024, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0025, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0026, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0027, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0028, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0029, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_002A, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_002D, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_002E, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_002F, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0031, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0032, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0033, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0034, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0035, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0036, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0038, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0039, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_003A, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_003C, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_003E, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_003F, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0040, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0041, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0042, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0043, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0044, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0045, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0046, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0047, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0049, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_004A, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_004C, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_004D, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_004E, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_004F, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0050, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0051, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0052, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0054, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0055, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0057, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_0058, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_0059, MSM_SE_GUIDE_26 },
    { MES_BOARD_GUIDE_005B, MSM_SE_GUIDE_28 },
    { MES_BOARD_GUIDE_005C, MSM_SE_GUIDE_26 },
    { -1, -1 },
};

static int mgCallWinYOfsTbl[5][4] = {
    { 0, 0, 0, 0 },
    { 238, 0, 0, 0 },
    { 214, 262, 0, 0 },
    { 198, 238, 278, 0 },
    { 193, 223, 253, 283 },
};

static void TutorialWatch(void);
static void TutorialSprClose(void);
static void TutorialSprGrpClose(void);
static void TutorialModelKillAll(void);
static void TutorialWinInit(void);
static void TutorialWinUpdate(void);
static void TutorialMgCallOMExec(OMOBJ *obj);
static void TutorialMgCallListGet(int type, int num, s16 *list);
static void TutorialMgCallSlideInSet(OMOBJ *obj);
static BOOL TutorialMgCallSlideInCheck(OMOBJ *obj);
static void TutorialMgCallGrowSet(OMOBJ *obj);
static void TutorialMgCallKill(OMOBJ *obj);

/* Resets tutorial resources and call state before a board tutorial is created. */
void mbTutorialInit(void)
{
    int i;

    tutorialGuideObj = NULL;
    for (i = 0; i < 16; i++) {
        tutorialSprId[i] = -1;
    }
    for (i = 0; i < 16; i++) {
        tutorialSprGrpId[i] = -1;
    }
    for (i = 0; i < 32; i++) {
        tutorialMdlId[i] = -1;
    }
    mbTutorialMainFuncSet(NULL);
    mbTutorialMgCallInit();
    tutorialExitReqF = FALSE;
    tutorialExitOnF = FALSE;
}

void mbTutorialMainFuncSet(TUTORIALMAINFUNC mainFunc)
{
    tutorialMain = mainFunc;
}

/* Board initialization calls this when FLAG_BOARD_TUTORIAL is set to start both tutorial
 * processes. */
void mbTutorialCreate(void)
{
    tutorialCallWork.scene = -1;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.result = -1;
    tutorialCallWork.stat = 0;
    if (mbSaveNewF) {
        tutorialCallWork.mode = 0;
    }
    _ClearFlag(FLAG_BOARD_DEBUG);
    _ClearFlag(FLAGNUM(FLAG_GROUP_BOARD, 3));
    _ClearFlag(FLAGNUM(FLAG_GROUP_BOARD, 7));
    _ClearFlag(FLAGNUM(FLAG_GROUP_BOARD, 8));
    _ClearFlag(FLAG_BOARD_NOMG);
    /* TutorialWatch is the child-process callback for this board tutorial session. */
    tutorialWatchProc = HuPrcChildCreate(TutorialWatch, 0x2011, 0x2000, 0, mbMainProc);
    tutorialMainProc = HuPrcChildCreate(tutorialMain, 0x2011, 0x2000, 0, mbMainProc);
    HuPrcVSleep();
}

/* Watches input as the child created by mbTutorialCreate, then fades out and releases tutorial
 * resources. */
static void TutorialWatch(void)
{
    s16 winNo;
    HuVec2f size;

    HuPrcSleep(60);
    TutorialWinInit();
    winNo = mbWinCreateHelp(MES_BOARD_GUIDE_0048);
    mbWinMesMaxSizeGet(winNo, &size);
    mbWinPosSet(winNo, 288.0f - ((s16)size.x / 2), 400);
    tutorialExitOnF = TRUE;
    while (!tutorialExitReqF) {
        u16 btn = HuPadBtnDown[0];

        TutorialWinUpdate();
        mbWinDispSet(winNo, tutorialExitOnF);
        if (!mbWipeSpecialCheck() && !mbWipeSpecialStatGet()
            && (btn & PAD_BUTTON_START) && tutorialExitOnF) {
            tutorialExitReqF = TRUE;
            break;
        }
        HuPrcVSleep();
    }
    mbPauseEnableSet();
    while (HuARDMACheck()) {
        HuPrcVSleep();
    }
    mbMusFadeOutSpeed(MB_MUS_CHAN_BG, 1000);
    if (tutorialExitOnF) {
        BOOL partyF = GwSystem.partyF;

        if (partyF) {
            WipeColorSet(0, 0, 0);
        } else {
            /* Both game modes currently use the same black wipe color. */
            WipeColorSet(0, 0, 0);
        }
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
        mbWipeWait();
    }
    while (mbMusCheck(MB_MUS_CHAN_BG)) {
        HuPrcVSleep();
    }
    mbWinKill(winNo);
    HuPrcKill(tutorialMainProc);
    tutorialMainProc = NULL;
    TutorialSprClose();
    TutorialSprGrpClose();
    TutorialModelKillAll();
    mbTutorialMgCallClose();
    if (tutorialGuideObj) {
        mbTutorialGuideClose(tutorialGuideObj);
    }
    mbExitReq();
    HuPrcSleep(-1);
}

/* Serves a board event's request for tutorial guidance and returns its result. */
int mbTutorialCall(int scene)
{
    if (!_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return -1;
    }
    if (tutorialMainProc == NULL) {
        return -1;
    }
    while (tutorialCallWork.stat == 0) {
        HuPrcVSleep();
    }
    if (tutorialCallWork.stat < 0) {
        return -1;
    }
    if (tutorialCallWork.scene >= 0) {
        if (tutorialCallWork.scene != scene) {
            return -1;
        }
        tutorialCallWork.callNum--;
        if (tutorialCallWork.callNum > 0) {
            return -1;
        }
    }
    tutorialCallWork.callNum = scene;
    tutorialCallWork.result = -1;
    tutorialCallWork.stat = 0;
    while (tutorialCallWork.stat == 0) {
        HuPrcVSleep();
    }
    return tutorialCallWork.result;
}

/* Marks the current tutorial call sequence as ended. */
void mbTutorialCallEnd(void)
{
    tutorialCallWork.stat = -1;
}

/* Sleeps one frame, terminating the caller process after a tutorial exit request. */
void mbTutorialVSleep(void)
{
    if (mbTutorialExitReqGet()) {
        HuPrcSleep(-1);
    }
    HuPrcVSleep();
}

/* Signals one tutorial event and waits until the tutorial process acknowledges it. */
void mbTutorialCallWait(int scene)
{
    tutorialCallWork.scene = scene;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
}

/* Allows a board event to request the same tutorial scene a fixed number of times. */
void mbTutorialMultiCall(int scene, int callNum)
{
    tutorialCallWork.scene = scene;
    tutorialCallWork.callNum = callNum;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
}

/* Waits for a scene and then publishes the supplied result to its board caller. */
void mbTutorialCallResult(int scene, int result)
{
    tutorialCallWork.scene = scene;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
    mbTutorialResultSet(result);
}

/* Waits for a scene, then displays its guide message until its timed window ends. */
void mbTutorialMesCall(int scene, int message)
{
    tutorialCallWork.scene = scene;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
    mbTutorialWinMesExec(message);
}

/* Shows a guide message with a board-space marker after the requested scene. */
void mbTutorialMesMasuCall(int scene, int message, int masuId)
{
    tutorialCallWork.scene = scene;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
    mbTutorialWinMesMasuExec(message, masuId);
}

/* Waits for the turn tutorial scene and reports the displayed turn as zero-based. */
void mbTutorialTurnCall(int turn)
{
    tutorialCallWork.scene = 5;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
    mbTutorialResultSet(turn - 1);
}

/* Waits for the guide scene, then stores the selected guide number as the pending board-call
 * result. */
void mbTutorialGuideCall(int guideNo)
{
    tutorialCallWork.scene = 13;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
    mbTutorialResultSet(guideNo);
}

/* Waits for capsule-use guidance, then reports the player's capsule slot. */
void mbTutorialCapsuleUseCall(int capsuleNo)
{
    int capsuleIndex;

    tutorialCallWork.scene = 15;
    tutorialCallWork.callNum = 0;
    tutorialCallWork.stat = 1;
    while (tutorialCallWork.stat != 0) {
        mbTutorialVSleep();
    }
    capsuleIndex = mbPlayerCapsuleFind(GwSystem.turnPlayerNo, capsuleNo);
    mbTutorialResultSet(capsuleIndex);
}

/* Stores the result that the pending board tutorial call will return. */
void mbTutorialResultSet(int result)
{
    tutorialCallWork.result = result;
}

/* Returns the board event scene currently registered with the tutorial. */
int mbTutorialSceneGet(void)
{
    return tutorialCallWork.scene;
}

/* Stores the board-selected tutorial mode. */
void mbTutorialModeSet(int mode)
{
    tutorialCallWork.mode = mode;
}

/* Returns the currently stored tutorial mode. */
int mbTutorialModeGet(void)
{
    return tutorialCallWork.mode;
}

/* Moves the camera to a tutorial board space using the player's normal view zoom. */
s16 mbTutorialViewSet(void)
{
    HuVecF rot = { -35.0f, 0.0f, 0.0f };
    HuVecF offset = { 0.0f, 200.0f, 0.0f };
    int masuId;

    masuId = (s16)mbMasuFind_AttrIdGet(-1, 0x8000);
    mbCameraMoveMasu(masuId, &rot, &offset, mbCameraPlayerViewZoomGet(0), -1.0f, -1);
    mbCameraMoveOnSet(FALSE);
    return masuId;
}

/* Moves the camera to a selected board space for a tutorial demonstration. */
void mbTutorialViewMasuSet(s16 masuId)
{
    HuVecF rot = { -35.0f, 0.0f, 0.0f };
    HuVecF offset = { 0.0f, 50.0f, 0.0f };

    mbCameraMoveMasu(masuId, &rot, &offset, mbCameraPlayerViewZoomGet(0), -1.0f, -1);
    mbCameraMoveOnSet(FALSE);
}

/* Creates and tracks a tutorial sprite using the requested board data entry. */
s16 mbTutorialSprCreate(unsigned int dataNum)
{
    s16 sprId;
    int i;

    for (i = 0; i < 16; i++) {
        if (tutorialSprId[i] < 0) {
            break;
        }
    }
    sprId = tutorialSprId[i] = espEntry(dataNum, 1500, 0);
    espAttrSet(sprId, HUSPR_ATTR_LINEAR);
    return sprId;
}

/* Creates a centered tutorial sprite and scales it from zero to full size. */
s16 mbTutorialSprDispOn(unsigned int dataNum)
{
    s16 sprId;
    int i;

    sprId = mbTutorialSprCreate(dataNum);
    espPosSet(sprId, 288.0f, 144.0f);
    for (i = 0; i <= 12u; i++) {
        float scale = i / 12.0f;
        espScaleSet(sprId, scale, scale);
        mbTutorialVSleep();
    }
    return sprId;
}

/* Kills a tracked tutorial sprite and frees its slot in the local handle list. */
void mbTutorialSprKill(s16 sprId)
{
    int i;
    int index;

    for (i = 0; i < 16; i++) {
        if (sprId == tutorialSprId[i]) {
            break;
        }
    }
    index = i;
    espKill(sprId);
    tutorialSprId[index] = -1;
}

/* Shrinks a tutorial sprite to zero before removing it from the sprite system. */
void mbTutorialSprDispOff(s16 sprId)
{
    int i;

    for (i = 0; i <= 12u; i++) {
        float scale = 1.0f - (i / 12.0f);
        espScaleSet(sprId, scale, scale);
        mbTutorialVSleep();
    }
    mbTutorialSprKill(sprId);
}

/* Removes every sprite still tracked when the tutorial watcher exits. */
static void TutorialSprClose(void)
{
    int i;

    for (i = 0; i < 16; i++) {
        if (tutorialSprId[i] >= 0) {
            espKill(tutorialSprId[i]);
        }
        tutorialSprId[i] = -1;
    }
}

/* Records a sprite-group handle for cleanup when the tutorial exits. */
void mbTutorialSprGrpSet(s16 grpId)
{
    s16 id;
    int i;

    for (i = 0; i < 16; i++) {
        if (tutorialSprGrpId[i] < 0) {
            break;
        }
    }
    id = tutorialSprGrpId[i] = grpId;
}

/* Clears the sprite-handle slot at this index; the group handle remains tracked for exit
 * cleanup. */
void mbTutorialSprGrpKill(s16 grpId)
{
    int i;
    int index;

    for (i = 0; i < 16; i++) {
        if (grpId == tutorialSprGrpId[i]) {
            break;
        }
    }
    index = i;
    HuSprGrpKill(grpId);
    tutorialSprId[index] = -1;
}

/* Kills all sprite groups still owned by the tutorial. */
static void TutorialSprGrpClose(void)
{
    int i;

    for (i = 0; i < 16; i++) {
        if (tutorialSprGrpId[i] >= 0) {
            HuSprGrpKill(tutorialSprGrpId[i]);
        }
        tutorialSprGrpId[i] = -1;
    }
}

/* Creates a board model and tracks its handle for tutorial cleanup. */
MBMODELID mbTutorialModelCreate(int dataNum, BOOL linkF)
{
    MBMODELID modelId;
    int i;

    for (i = 0; i < 32; i++) {
        if (tutorialMdlId[i] < 0) {
            break;
        }
    }
    modelId = tutorialMdlId[i] = mbObjCreate(dataNum, NULL, linkF);
    return modelId;
}

/* Kills a tracked tutorial model and releases its handle slot. */
void mbTutorialModelKill(int modelId)
{
    int i;
    int modelNo;

    for (i = 0; i < 32; i++) {
        if ((MBMODELID)modelId == tutorialMdlId[i]) {
            break;
        }
    }
    modelNo = i;
    mbObjKill(modelId);
    tutorialMdlId[modelNo] = -1;
}

/* Removes every model still tracked when the tutorial watcher exits. */
static void TutorialModelKillAll(void)
{
    int i;

    for (i = 0; i < 32; i++) {
        if (tutorialMdlId[i] >= 0) {
            mbObjKill(tutorialMdlId[i]);
        }
        tutorialMdlId[i] = -1;
    }
}

/* Creates the guide character, hides its model, and prepares its opening motion. */
OMOBJ *mbTutorialGuideCreate(s8 *motTbl, BOOL screenF)
{
    tutorialGuideObj = mbGuideCreateFlag(NULL, motTbl, screenF, FALSE, TRUE);
    mbObjDispSet(mbGuideModelGet(tutorialGuideObj), FALSE);
    mbGuideMotionNextSet(tutorialGuideObj, 1);
    mbGuideMotionShiftSet(tutorialGuideObj, 1, FALSE);
    return tutorialGuideObj;
}

/* Removes the guide character and clears the stored object handle. */
void mbTutorialGuideClose(OMOBJ *obj)
{
    mbGuideKill(obj);
    tutorialGuideObj = NULL;
}

/* Returns the guide character object used by tutorial message windows. */
OMOBJ *mbTutorialGuideGet(void)
{
    return tutorialGuideObj;
}

/* Reports whether the tutorial watcher has received an exit request. */
BOOL mbTutorialExitReqGet(void)
{
    return tutorialExitReqF;
}

/* Requests that tutorial processes exit on their next tutorial sleep. */
void mbTutorialExitSet(void)
{
    tutorialExitReqF = TRUE;
}

/* Controls whether the watcher accepts START and performs the exit wipe. */
void mbTutorialExitOnSet(BOOL exitOnF)
{
    tutorialExitOnF = exitOnF;
}

/* Marks every tutorial-window timer slot as unused before tracking begins. */
static void TutorialWinInit(void)
{
    int i;

    for (i = 0; i < HUWIN_MAX; i++) {
        tutorialWinData[i].delay = tutorialWinData[i].stat = -1;
        tutorialWinData[i].time = -1;
    }
}

/* Advances tutorial-window timers from the shared window state each frame. */
static void TutorialWinUpdate(void)
{
    int i;

    for (i = 0; i < HUWIN_MAX; i++) {
        if (winData[i].grpId != HUSPR_GROUP_NONE) {
            if (tutorialWinData[i].stat < 0) {
                tutorialWinData[i].stat = tutorialWinData[i].time = 0;
                tutorialWinData[i].delay = 0;
            }
            if (winData[i].stat == 2) {
                tutorialWinData[i].stat = 1;
                tutorialWinData[i].delay = 1;
            } else if (winData[i].stat != 0 && tutorialWinData[i].stat != 0) {
                if (tutorialWinData[i].delay != 0) {
                    tutorialWinData[i].delay--;
                } else {
                    tutorialWinData[i].stat = 0;
                    tutorialWinData[i].time++;
                }
            }
        } else {
            tutorialWinData[i].stat = tutorialWinData[i].time = -1;
        }
    }
}

/* Consumes a tracked close-transition tick and reports whether it was pending. */
BOOL mbTutorialWinWait(int winNo)
{
    MBWIN *win;
    BOOL waitF = FALSE;

    while (TRUE) {
        mbTutorialVSleep();
        if (mbWinDoneCheck(winNo)) {
            break;
        }
        win = mbWinGet(winNo);
        if (win->winId < 0) {
            break;
        }
        if (tutorialWinData[win->winId].stat < 0) {
            break;
        }
        if (tutorialWinData[win->winId].time > 0) {
            tutorialWinData[win->winId].time--;
            waitF = TRUE;
            break;
        }
    }
    return waitF;
}

/* Clears the tutorial timer for an open window so its wait can finish. */
void mbTutorialWinClose(int winNo)
{
    MBWIN *win;

    if (!mbWinDoneCheck(winNo)) {
        win = mbWinGet(winNo);
        if (win->winId >= 0 && tutorialWinData[win->winId].stat >= 0) {
            tutorialWinData[win->winId].stat = tutorialWinData[win->winId].time = 0;
        }
    }
}

/* Creates a guide window, plays its mapped cue, and waits for its close transition. */
void mbTutorialWinMesExec(int message)
{
    int winNo;
    int i;

    winNo = mbWinCreateTime(MBWIN_TYPE_GUIDE, message, -1);
    mbWinPlayerDisable(winNo, -1);
    for (i = 0; tutorialGuideTbl[i].message >= 0; i++) {
        if (message == tutorialGuideTbl[i].message) {
            mbAudGuidePlay(tutorialGuideTbl[i].seId);
            break;
        }
    }
    do {
        OMOBJ *guideObj = tutorialGuideObj;

        mbGuideMotionShiftSet(guideObj, 12, TRUE);
    } while (mbTutorialWinWait(winNo));
    mbWinWait(winNo);
}

/* Shows a guide message and marker until its close transition and window process finish. */
void mbTutorialWinMesMasuExec(int message, int masuId)
{
    int winNo;
    int sprId;
    int i;

    winNo = mbWinCreateTime(MBWIN_TYPE_GUIDE, message, -1);
    mbWinPlayerDisable(winNo, -1);
    for (i = 0; tutorialGuideTbl[i].message >= 0; i++) {
        if (message == tutorialGuideTbl[i].message) {
            mbAudGuidePlay(tutorialGuideTbl[i].seId);
            break;
        }
    }
    mbGuideMotionShiftSet(mbTutorialGuideGet(), 12, TRUE);
    sprId = mbTutorialSprDispOn(masuId);
    while (mbTutorialWinWait(winNo)) {
        mbGuideMotionShiftSet(mbTutorialGuideGet(), 12, TRUE);
    }
    mbWinWait(winNo);
    mbTutorialSprDispOff(sprId);
}

/* Opens a guide window, plays its mapped cue, and returns the window handle. */
int mbTutorialWinCreate(int message)
{
    int winNo;
    int i;

    winNo = mbWinCreateTime(MBWIN_TYPE_GUIDE, message, -1);
    mbWinPlayerDisable(winNo, -1);
    for (i = 0; tutorialGuideTbl[i].message >= 0; i++) {
        if (message == tutorialGuideTbl[i].message) {
            mbAudGuidePlay(tutorialGuideTbl[i].seId);
            break;
        }
    }
    {
        OMOBJ *guideObj = tutorialGuideObj;

        mbGuideMotionShiftSet(guideObj, 12, TRUE);
    }
    return winNo;
}

/* Keeps the guide animation active through the close transition, then waits for the window
 * process. */
void mbTutorialWinKeyWait(int winNo)
{
    while (mbTutorialWinWait(winNo)) {
        OMOBJ *guideObj = tutorialGuideObj;

        mbGuideMotionShiftSet(guideObj, 12, TRUE);
    }
    mbWinWait(winNo);
}

/* Clears the roulette's object and sprite ownership state. */
void mbTutorialMgCallInit(void)
{
    memset(&tutorialMgCallData, 0, sizeof(tutorialMgCallData));
}

/* Kills roulette sprites and marks choice objects for deletion by their callbacks when active. */
void mbTutorialMgCallClose(void)
{
    int i;

    if (tutorialMgCallData.active != 0) {
        for (i = 0; i < 3; i++) {
            if (tutorialMgCallData.espId[i] >= 0) {
                espKill(tutorialMgCallData.espId[i]);
            }
            tutorialMgCallData.espId[i] = -1;
        }
        for (i = 0; i < 4; i++) {
            if (tutorialMgCallData.obj[i]) {
                TutorialMgCallKill(tutorialMgCallData.obj[i]);
            }
            tutorialMgCallData.obj[i] = NULL;
        }
        tutorialMgCallData.active = 0;
    }
}

/* Displays a randomly selected eligible minigame in four roulette slots when one exists, animates
 * the selection, and highlights the result. */
void mbTutorialMgCallExec(int type)
{
    TUTORIALMGCALLDATA *data = &tutorialMgCallData;
    s16 list[4];
    TUTORIALMGCALLWORK *work;
    int nextMaxTime;
    int delay;
    int nextDelay;
    int listNum;
    int delayOfs;
    int nextTime;
    int result;
    int speed;
    MGDATA *mgData;
    BOOL unlocked;
    int i;
    float weight;

    for (i = 0; i < 3; i++) {
        data->espId[i] = -1;
    }
    for (i = 0; i < 4; i++) {
        data->obj[i] = NULL;
    }
    data->active = TRUE;
    listNum = 4;
    TutorialMgCallListGet(type, listNum, list);

    data->espId[0] = espEntry(mbBoardDataNumGet(DATANUM(DATA_board, 0x87)), 100, 0);
    espPosSet(data->espId[0], 288.0f, 230.4f);
    espDrawNoSet(data->espId[0], 32);
    data->espId[1] = espEntry(mbBoardDataNumGet(DATANUM(DATA_board, 0x88)), 102, 0);
    espPosSet(data->espId[1], 288.0f, 238.4f);
    espDrawNoSet(data->espId[1], 32);
    espTPLvlSet(data->espId[1], 0.5f);
    data->espId[2] = espEntry(mbBoardDataNumGet(DATANUM(DATA_board, 0x89)), 101, 0);
    espPosSet(data->espId[2], 288.0f, 246.4f);
    espDrawNoSet(data->espId[2], 32);
    espTPLvlSet(data->espId[2], 0.5f);
    espDispOff(data->espId[2]);
    for (i = 1; i <= 30; i++) {
        weight = 1.0f - (i / 30.0f);
        espPosSet(data->espId[0], 288.0f, 230.4f + (480.0f * weight));
        espPosSet(data->espId[1], 288.0f, 238.4f + (480.0f * weight));
        mbTutorialVSleep();
    }

    for (i = 0; i < listNum; i++) {
        mgData = NULL;
        if (list[i] >= 0) {
            mgData = &MgDataTbl[list[i]];
        }
        data->obj[i] = omAddObjEx(mbObjMan, 257, 0, 0, -1, TutorialMgCallOMExec);
        work = omObjGetWork(data->obj[i], TUTORIALMGCALLWORK);
        work->dispF = TRUE;
        work->cursorNo = i;
        data->obj[i]->trans.y = mgCallWinYOfsTbl[listNum][i];
        unlocked = FALSE;
    work->winNo = mbWinCreateHelp(MES_BOARD_GUIDE_003D);
        work->message = -1;
        if (mgData) {
            work->message = mgData->nameMes;
        }
        mbWinPriSet(work->winNo, 90);
        TutorialMgCallSlideInSet(data->obj[i]);
    }
    tutorialMgCallCursorPos = -1;
    while (!TutorialMgCallSlideInCheck(data->obj[0])) {
        mbTutorialVSleep();
    }

    tutorialMgCallCursorPos = 0;
    result = mbRandMod(listNum);
    delay = mbRandMod(30) + 90;
    delayOfs = (int)((-7.0f + sqrtf((delay * 8.0f) + 49.0f)) / 2.0f);
    speed = (result - delayOfs) % listNum;
    if (speed < 0) {
        speed += listNum;
    }
    nextDelay = (listNum * 12) + (speed * 4);
    nextTime = nextMaxTime = 4;
    for (i = 0; i < nextDelay + delay; i++) {
        if (--nextTime == 0) {
            tutorialMgCallCursorPos = (tutorialMgCallCursorPos + 1) % listNum;
            if (i > nextDelay) {
                nextMaxTime++;
                nextTime = nextMaxTime;
            } else {
                nextTime = nextMaxTime;
            }
            espPosSet(data->espId[2], 288.0f, data->obj[tutorialMgCallCursorPos]->trans.y);
            espDispOn(data->espId[2]);
            mbAudFXPlay(MSM_SE_BRD00_05);
        }
        mbTutorialVSleep();
    }
    tutorialMgCallCursorPos = result;
    espPosSet(data->espId[2], 288.0f, data->obj[tutorialMgCallCursorPos]->trans.y);
    TutorialMgCallGrowSet(data->obj[tutorialMgCallCursorPos]);
    mbAudFXPlay(MSM_SE_BRD00_130);
    HuPrcSleep(120);
}

/* Selects a random eligible minigame of the requested type, or -1 if none, and fills every slot
 * with it. */
static void TutorialMgCallListGet(int type, int num, s16 *list)
{
    s8 candidates[64];
    int candidateNum = 0;
    int mgNo = -1;
    int i;

    for (i = 0; MgDataTbl[i].ovl != 0xFFFF; i++) {
        if ((MgDataTbl[i].flag & 0x2C0) == 0 && type == MgDataTbl[i].type) {
            candidates[candidateNum++] = i;
        }
    }
    if (candidateNum > 0) {
        mgNo = candidates[mbRandMod(candidateNum)];
    }
    /* Each slot gets the same random minigame; the roulette animates selection position. */
    for (i = 0; i < num; i++) {
        list[i] = mgNo;
    }
}

/* Object callback passed to omAddObjEx; updates one choice's animation and help window each
 * frame. */
static void TutorialMgCallOMExec(OMOBJ *obj)
{
    TUTORIALMGCALLWORK *work = omObjGetWork(obj, TUTORIALMGCALLWORK);
    HuVec2f size;
    float weight;
    float sinValue;

    if (work->killF || mbExitCheck()) {
        mbWinKill(work->winNo);
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    if (work->dispF) {
        mbWinDispSet(work->winNo, TRUE);
    } else {
        mbWinDispSet(work->winNo, FALSE);
    }
    switch (work->state) {
        case 0:
            if (work->time > work->maxTime) {
                work->slideInF = FALSE;
                work->state = 1;
            } else {
                weight = (float)work->time++ / work->maxTime;
                sinValue = sin((M_PI * (90.0f * weight)) / 180.0);
                if ((work->cursorNo & 1) == 0) {
                    obj->trans.x = -160.0f + (448.0f * sinValue);
                } else {
                    obj->trans.x = 736.0f + (-448.0f * sinValue);
                }
            }
            break;
        case 1:
            if (work->cursorNo == tutorialMgCallCursorPos) {
                work->state = 2;
            }
            break;
        case 2:
            if (work->cursorNo != tutorialMgCallCursorPos) {
                work->state = 3;
                work->time = 0;
                work->maxTime = 8;
            }
            break;
        case 3:
            if (work->time > work->maxTime) {
                work->state = 1;
            } else {
                weight = (float)work->time++ / work->maxTime;
            }
            break;
        case 4:
            if (work->time <= work->maxTime) {
                weight = (float)work->time++ / work->maxTime;
                obj->scale.x = obj->scale.y =
                    1.0f + (0.2f * sin((M_PI * (720.0f * (1.0f - weight))) / 180.0));
            }
            break;
        case 5:
            if (work->time <= work->maxTime) {
                weight = (float)work->time++ / work->maxTime;
                obj->trans.y = obj->rot.y + (weight * (96.0f - obj->rot.y));
                obj->scale.x = obj->scale.y =
                    0.5f + (0.5f * cos((M_PI * (90.0f * weight)) / 180.0));
            }
            break;
    }
    mbWinScaleSet(work->winNo, obj->scale.x, obj->scale.y);
    mbWinMesMaxSizeGet(work->winNo, &size);
    mbWinPosSet(work->winNo,
        obj->trans.x - ((size.x / 2) * obj->scale.x),
        obj->trans.y - ((size.y / 2) * obj->scale.y));
}

/* Starts a choice's 30-frame slide-in animation. */
static void TutorialMgCallSlideInSet(OMOBJ *obj)
{
    TUTORIALMGCALLWORK *work = omObjGetWork(obj, TUTORIALMGCALLWORK);

    work->slideInF = TRUE;
    work->state = 0;
    work->time = 0;
    work->maxTime = 30;
}

/* Reports whether a roulette choice has completed its slide-in. */
static BOOL TutorialMgCallSlideInCheck(OMOBJ *obj)
{
    TUTORIALMGCALLWORK *work = omObjGetWork(obj, TUTORIALMGCALLWORK);

    if (work->slideInF) {
        return FALSE;
    }
    return TRUE;
}

/* Starts the selected choice's grow animation and replaces its help label when a name message is
 * available. */
static void TutorialMgCallGrowSet(OMOBJ *obj)
{
    TUTORIALMGCALLWORK *work = omObjGetWork(obj, TUTORIALMGCALLWORK);

    work->state = 4;
    work->time = 0;
    work->maxTime = 90;
    if (work->message >= 0) {
        mbWinKill(work->winNo);
        work->winNo = mbWinCreateHelp(work->message);
    }
}

/* Marks a roulette choice for removal by its object callback. */
static void TutorialMgCallKill(OMOBJ *obj)
{
    TUTORIALMGCALLWORK *work = omObjGetWork(obj, TUTORIALMGCALLWORK);

    work->killF = TRUE;
}
