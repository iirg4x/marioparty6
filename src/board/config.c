/* Implements the board pause configuration menu, its panels, guide, and controller setup. */
#include "dolphin/math.h"
#include "stdlib.h"
#include "messdir_enum.h"
#include "msm_se.h"

#include "game/board/main.h"
#include "game/board/object.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/data.h"
#include "game/disp.h"
#include "game/esprite.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/sprite.h"

#include "dolphin/gx.h"
#include "dolphin/mtx.h"

extern void *mbMalloc(s32 size);
extern void *mbMallocFlush(s32 size);
extern void *mbMallocFlushModel(s32 size);
extern void mbNormPosto2D(HuVecF *src, HuVecF *dst);
extern void mbNormPosto3D(HuVecF *src, s16 cameraMask, HuVecF *dst);
extern void mbPos3DtoNorm(HuVecF *src, s16 cameraMask, HuVecF *dst);
extern float mbSinDeg(float angle);
extern float mbCosDeg(float angle);
extern void HuPadRumbleAllStop(void);
extern s32 HuMCProbe(s32 chan);
extern s32 HuMCMicGet(void);
extern void HuMCMicSet(s32 value);
extern s16 HuPadStatGet(s16 padNo);
extern void mbWipeFadeOut(void);
extern BOOL SLSaveFlagGet(void);
extern BOOL saveExecF;
extern void mbSingleSaveFlush(int value);
extern int mbAudGuidePlay(s16 seId);

/* Copies a position vector: a panel's current, starting or target position, or the guide's. */
static inline void ConfigVecCopy(
    register HuVecF *srcP, register HuVecF *dstP)
{
    register float xy;
    register float z;

    asm {
        psq_l xy, 0(srcP), 0, 0
        lfs z, 8(srcP)
        psq_st xy, 0(dstP), 0, 0
        stfs z, 8(dstP)
    }
}

/* Returns the nonnegative cosine scale used while the label flips between banks. */
static inline float ConfigAbsFloat(register float value)
{
    asm {
        fabs value, value
    }
    return value;
}

typedef struct PausePanelWork_s {
int modelId;              /* Main pause-panel model. */
int batsuModelId;         /* Overlay model used for the disabled-choice mark. */
int sprId;                /* Label sprite, or -1 when the panel has no sprite. */
ANIMDATA *anim;           /* Texture animation resource attached to the panel model. */
HU3D_ANIMID animId[2];    /* Main and linked animation tracks for the panel. */
HuVecF pos;               /* Current model position in normalized board space. */
HuVecF posStart;          /* Starting position for the active slide. */
HuVecF posTarget;         /* Destination position for the active slide. */
BOOL batsuF;              /* Whether the disabled-choice mark should be shown. */
float scale;              /* Current panel animation scale. */
float scaleStart;         /* Starting scale for the active size animation. */
float scaleTarget;        /* Destination scale for the active size animation. */
float scaleBase;          /* Multiplier applied to the panel and label's displayed scale. */
s16 bank;                 /* Panel texture-animation frame and corresponding label sprite bank. */
s16 motion;               /* Active panel motion kind; zero means settled. */
s16 time;                 /* Elapsed frames for the active panel motion or its start delay. */
s16 maxTime;              /* Duration in frames for the active panel motion. */
s16 delay;                /* Start-delay length in frames; zero when no delay is pending. */
s16 animTime;             /* Elapsed frames for the bank-change flip. */
s16 animMaxTime;          /* Duration in frames for the bank-change flip. */
} PAUSE_PANEL_WORK;

typedef struct PausePadWork_s {
    s32 padNo;             /* Controller number currently assigned to this player. */
    s32 port;              /* Player slot assigned to this controller. */
    s32 padStatus;         /* Controller error status returned by HuPadStatGet. */
    BOOL statusChangeFrames; /* Consecutive frames the controller status has differed from
                             * padStatus. */
} PAUSE_PAD_WORK;

typedef struct ConfigMenuWork_s {
    s32 menuIndex;          /* Position of this setting in the pause menu. */
    s32 optionIndex;        /* Currently selected option for this setting. */
    s32 optionCount;        /* Number of choices available for this setting. */
    s16 panelId;            /* Panel displaying this setting. */
    s16 labelPanelId;       /* Label panel slot; this member is not used in this file. */
    BOOL enabled;           /* Whether this panel participates in menu transitions. */
    s32 available;          /* Nonzero when this setting is available in the current pause-menu
                             * configuration. */
} CONFIG_MENU_WORK;

typedef struct PauseCursorWork_s {
    HuVecF pos;             /* Current cursor position. */
    HuVecF posStart;        /* Position at the start of a cursor move. */
    HuVecF posDelta;        /* Cursor movement vector. */
    BOOL activeF;           /* Whether this cursor style is active. */
    s32 cursorNo;           /* Cursor style index used for sprite offsets; -1 when inactive. */
    s32 mask;               /* Current directional input mask. */
    s32 directionMask;      /* Mask of directions allowed for this cursor style. */
    s32 moveTime;           /* Frames remaining in the cursor move. */
    s32 maxMoveTime;        /* Cursor move duration in frames. */
    float alpha[4];         /* Opacity of each directional cursor sprite. */
    s16 sprId[4];           /* Directional cursor sprite IDs. */
    s16 hiliteSprId[4];     /* Highlight sprite IDs for the cursor directions. */
} PAUSE_CURSOR_WORK;

typedef struct ConfigPadWork_s {
    s32 padNo;              /* Controller number assigned to this player. */
    s32 playerNo;           /* Player slot shown in the assignment row. */
    s32 comDif;              /* CPU difficulty, or GW_PLAYER_COM_DIF_MAX for a human. */
    BOOL activeF;            /* Whether this row has a connected controller. */
    HuVecF pos;              /* Offset from the character panel to its controller icons. */
    s16 panelId;             /* Character panel for this player row. */
    s16 sprId[3];            /* Human/CPU, controller-number, and CPU-difficulty sprite IDs. */
} CONFIG_PAD_WORK;

typedef struct PauseWork_s {
    s32 playerNo; /* Player who opened the pause menu and supplies its controller input. */
    s32 cursorPos; /* Highlighted entry in the eight-choice pause menu; -1 before selection. */
    s32 configPadPlayerIndex; /* Highlighted player row in controller assignment. */
    s32 configPadChangeCooldown; /* Frames before another CPU setting change is accepted. */
    s32 activeF; /* Selection latch or remaining input-lock frames while handling a choice. */
    s32 menuEntered; /* Written at menu entry and exit, but not read in this file. */
    s32 menuExitRequested; /* Set when the pause-menu selection loop should exit. */
    s32 padWinNo; /* Menu-controls or quit-confirmation help window, or -1 when closed. */
    s32 helpWinNo; /* Menu setting help window, or -1 when closed. */
    s32 unusedWorkWord0; /* Unused by the pause menu. */
    s32 unusedWorkWord1; /* Unused by the pause menu. */
    s32 unusedWorkWord2; /* Unused by the pause menu. */
    s32 unusedWorkWord3; /* Unused by the pause menu. */
    s32 unusedWorkWord4; /* Unused by the pause menu. */
    s32 unusedWorkWord5; /* Unused by the pause menu. */
    s32 talkTime; /* Remaining frames in the guide's talking animation. */
    s32 prevTalkTime; /* Prior update's timer, used to detect talk start and end. */
    CONFIG_MENU_WORK menu[8]; /* The eight pause-menu settings and their panels. */
    CONFIG_PAD_WORK configPad[4]; /* Controller-assignment rows for the four players. */
    PAUSE_CURSOR_WORK cursor; /* Cursor sprites and movement state for either menu. */
    PAUSE_PAD_WORK padWork[GW_PLAYER_MAX]; /* Current controller-to-player assignments. */
} PAUSE_WORK;

static HuVecF playerPos;
static PAUSE_WORK pauseWork;
static BOOL playerDispF[GW_PLAYER_MAX];

static HUPROCESS *configProc = NULL;
static s32 configPadEnabled = 0;
static HUPROCESS *pauseGuideProc = NULL;
static PAUSE_PANEL_WORK *pausePanelWork = NULL;
static BOOL pauseGuideKillF = FALSE;
static s32 pauseDispCopyModelId = 0;
static s32 pauseDispCopyCounter = 0;
static void *pauseDispCopyFb = NULL;
static s16 pausePlayer = 0;
static s32 configResult = 0;
static BOOL configDoneF = FALSE;

static s32 mesSpeedAnmNo[4] = {
    GW_MESS_SPEED_SLOW,
    GW_MESS_SPEED_NORMAL,
    GW_MESS_SPEED_FAST,
    GW_MESS_SPEED_FAST
};
static HuVecF pauseGuidePos = { -0.7f, -0.75f, -750.0f };
static HuVecF pauseGuideQuitPos = { 0.0f, -0.75f, -500.0f };
static s32 pauseMicValueTbl[3] = { 1, 0, 2 };
static s16 pauseGridPosTbl[][2] = {
    0, 0, 1, 0, 2, 0, 3, 0,
    0, 1, 1, 1, 2, 1, 3, 1
};
static s32 pausePanelFileTbl[8] = {
    DATANUM(DATA_bpause6, 3), DATANUM(DATA_bpause6, 4),
    DATANUM(DATA_bpause6, 5), DATANUM(DATA_bpause6, 6),
    DATANUM(DATA_bpause6, 7), DATANUM(DATA_bpause6, 8),
    DATANUM(DATA_bpause6, 9), DATANUM(DATA_bpause6, 10)
};
static s32 pausePanelLabelFileTbl[8] = {
    DATANUM(DATA_bpause6, 11), DATANUM(DATA_bpause6, 12),
    DATANUM(DATA_bpause6, 13), DATANUM(DATA_bpause6, 14),
    DATANUM(DATA_bpause6, 15), DATANUM(DATA_bpause6, 16),
    DATANUM(DATA_bpause6, 17), DATANUM(DATA_bpause6, 18)
};
static s16 pauseValueNumTbl[8] = { 1, 2, 2, 5, 2, 3, 3, 1 };
static s16 pauseBatsuValueTbl[8] = { -1, 1, 1, -1, 1, -1, 1, -1 };
static u32 pauseWinMesTbl[8][5] = {
    MESSNUM(MESS_BOARD_PAUSE, 0), MESSNUM(MESS_BOARD_PAUSE, 1),
    MESSNUM(MESS_BOARD_PAUSE, 2), 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 3), MESSNUM(MESS_BOARD_PAUSE, 4), 0, 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 5), MESSNUM(MESS_BOARD_PAUSE, 6), 0, 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 7), MESSNUM(MESS_BOARD_PAUSE, 8),
    MESSNUM(MESS_BOARD_PAUSE, 9), MESSNUM(MESS_BOARD_PAUSE, 10),
    MESSNUM(MESS_BOARD_PAUSE, 23),
    MESSNUM(MESS_BOARD_PAUSE, 11), MESSNUM(MESS_BOARD_PAUSE, 12), 0, 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 15), MESSNUM(MESS_BOARD_PAUSE, 14),
    MESSNUM(MESS_BOARD_PAUSE, 13), 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 16), MESSNUM(MESS_BOARD_PAUSE, 17),
    MESSNUM(MESS_BOARD_PAUSE, 18), 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 19), MESSNUM(MESS_BOARD_PAUSE, 40), 0, 0, 0
};
static u32 pauseSingleWinMesTbl[8][5] = {
    MESSNUM(MESS_BOARD_PAUSE, 42), MESSNUM(MESS_BOARD_PAUSE, 1),
    MESSNUM(MESS_BOARD_PAUSE, 2), 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 3), MESSNUM(MESS_BOARD_PAUSE, 4), 0, 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 5), MESSNUM(MESS_BOARD_PAUSE, 43), 0, 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 7), MESSNUM(MESS_BOARD_PAUSE, 8),
    MESSNUM(MESS_BOARD_PAUSE, 9), MESSNUM(MESS_BOARD_PAUSE, 10),
    MESSNUM(MESS_BOARD_PAUSE, 23),
    MESSNUM(MESS_BOARD_PAUSE, 11), MESSNUM(MESS_BOARD_PAUSE, 12), 0, 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 15), MESSNUM(MESS_BOARD_PAUSE, 14),
    MESSNUM(MESS_BOARD_PAUSE, 13), 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 16), MESSNUM(MESS_BOARD_PAUSE, 17),
    MESSNUM(MESS_BOARD_PAUSE, 18), 0, 0,
    MESSNUM(MESS_BOARD_PAUSE, 40), MESSNUM(MESS_BOARD_PAUSE, 40), 0, 0, 0
};
static HuVecF pauseGuidePos2 = { -0.7f, -0.75f, -750.0f };
static char configExitMessage[] =
    "-------------------- Config Exit! ----------------------\n";
static u32 pausePadWinMesTbl[4] = {
    MESSNUM(MESS_BOARD_PAUSE, 21), MESSNUM(MESS_BOARD_PAUSE, 22),
    MESSNUM(MESS_BOARD_PAUSE, 22), MESSNUM(MESS_BOARD_PAUSE, 22)
};
static float pausePadWinPosTbl[4][2] = {
    { 64.0f, 288.0f },
    { 64.0f, 288.0f },
    { 0.0f, 196.0f },
    { 0.0f, 212.0f }
};
static s32 pauseCharPanelFileTbl[14] = {
    DATANUM(DATA_bpause6, 20), DATANUM(DATA_bpause6, 21),
    DATANUM(DATA_bpause6, 22), DATANUM(DATA_bpause6, 23),
    DATANUM(DATA_bpause6, 24), DATANUM(DATA_bpause6, 25),
    DATANUM(DATA_bpause6, 26), DATANUM(DATA_bpause6, 27),
    DATANUM(DATA_bpause6, 28), DATANUM(DATA_bpause6, 29),
    DATANUM(DATA_bpause6, 30), DATANUM(DATA_bpause6, 29),
    DATANUM(DATA_bpause6, 29), DATANUM(DATA_bpause6, 29)
};
static s32 pausePadSpriteFileTbl[3] = {
    DATANUM(DATA_bpause6, 19), DATANUM(DATA_bpause6, 31),
    DATANUM(DATA_bpause6, 31)
};
static s16 configPadSprOfsTbl[3][2] = {
    { 0, 0 },
    { 20, -30 },
    { 42, -70 }
};
static s16 pauseCursorSprOfsTbl[2][4][2] = {
    { { 0, -62 }, { 0, 68 }, { 62, 0 }, { -62, 0 } },
    { { 0, -20 }, { 0, 20 }, { 43, -60 }, { -45, -60 } }
};
static s32 pauseCursorMaskTbl[4] = { 8, 4, 2, 1 };

static void ConfigKill(void);
static void ConfigMain(void);
static void ConfigOpen(void);
static void ConfigGrowWait(void);
static inline void ConfigWinUpdate(s32 index);
static void ConfigPadWinSet(s32 index);
static void ConfigPadOpen(void);
static void ConfigPadSprSet(CONFIG_PAD_WORK *workP);
static void ConfigPadTimeSet(CONFIG_PAD_WORK *workP, float scale);
static void ConfigPadClose(void);
static BOOL ConfigPadMain(void);
static void ConfigSettingRead(void);
static void ConfigSettingWrite(void);
BOOL mbPausePanelFreezeGet(s16 panelId);
s16 mbPausePanelCreate(int dataNum, unsigned int espDataNum);
void mbPausePanelKill(s16 panelId);
void mbPausePanelPosSet(s16 panelId, float panelX, float panelY);
void mbPausePanelPosGet(s16 panelId, HuVecF *posOut);
float mbPausePanelScaleGet(s16 panelId);
void mbPausePanelBatsuSet(s16 panelId, BOOL batsuF);
void mbPausePanelSizeSet(s16 panelId, int durationFrames, float scale);
void mbPausePanelGrowSet(s16 panelId, int time, int delay, float scale);
void mbPausePanelShrinkSet(s16 panelId, int time, int delay);
void mbPausePanelSlideSet(s16 panelId, int durationFrames, HuVecF *targetPos);
void mbPausePanelBankSet(s16 panelId, int animBank);
void mbPausePanelAnmNoSet(s16 panelId, s32 animMaxTime, s32 bank);
static BOOL PausePlayerComRead(s32 initializeAssignments);
static void ConfigExec(void);
static BOOL ConfigQuitExec(void);
static void ConfigClose(s32 result);
static void PauseCursorCreate(void);
static void PauseCursorKill(void);
static void PauseCursorHiliteSet(s32 cursorNo, s32 directionMask, s32 mask);
static void PauseCursorPosNextSet(s32 time, float x, float y);
static int PauseCursorMove(void);
static BOOL PausePadCheck(s32 padNo);
void mbPauseGuideMoveSet(MBMODELID modelId, s32 time, HuVecF *posP,
    HuVecF *posNormP);
void mbPauseGuideTalkSet(MBMODELID modelId);
static void PauseDispCopyDraw(HU3D_MODEL *modelP, Mtx *mtx);
static void PauseGuideMain(void);
static void PauseGuideDestroy(void);
static BOOL GWStorySingleCheck(void);

#define lbl_802C4CA8 0.3f
#define lbl_802C4CB0 0.5f
#define lbl_802C4CB4 0.0f

/* mbConfigExec starts the pause-menu process and waits until its destructor marks it complete. */
BOOL mbConfigExec(int playerNo, MBMODELID modelId)
{
    pausePlayer = modelId;
    mbObjPosGet(pausePlayer, &playerPos);
    configDoneF = FALSE;
    configResult = FALSE;
    memset(&pauseWork, 0, sizeof(PAUSE_WORK));
    pauseWork.padWinNo = pauseWork.helpWinNo = -1;
    pauseWork.playerNo = playerNo;
    pauseWork.cursorPos = -1;
    pauseWork.talkTime = pauseWork.prevTalkTime = 0;
    ConfigSettingRead();
    configProc = HuPrcChildCreate(ConfigMain, 8210, 14336, 0, mbMainProc);
    HuPrcSetStat(configProc, HU_PRC_STAT_PAUSE_ON | HU_PRC_STAT_UPAUSE_ON);
    HuPrcDestructorSet2(configProc, ConfigKill);
    while (!configDoneF) {
        HuPrcVSleep();
    }
    ConfigSettingWrite();
    return configResult;
}

/* ConfigKill is the process destructor installed by mbConfigExec; it releases any remaining menu
 * panels. */
static void ConfigKill(void)
{
    CONFIG_MENU_WORK *menuEntry;
    int menuIndex;

    menuEntry = pauseWork.menu;
    for (menuIndex = 0; menuIndex < 8; menuIndex++, menuEntry++) {
        if (menuEntry->panelId != 0) {
            mbPausePanelKill((s16)menuEntry->panelId);
            menuEntry->panelId = 0;
        }
    }
    configProc = NULL;
    configDoneF = TRUE;
}

/* ConfigMain runs in the pause child process: it opens the menu, handles selection, then closes its
 * UI. */
static void ConfigMain(void)
{
    int menuIndex;

    PausePlayerComRead(1);
    ConfigOpen();
    ConfigExec();
    if (pauseWork.helpWinNo >= 0) {
        mbWinKill((s16)pauseWork.helpWinNo);
        pauseWork.helpWinNo = -1;
    }
    ConfigPadWinSet(-1);
    for (menuIndex = 0; menuIndex < 8; menuIndex++) {
        pauseWork.menu[menuIndex].enabled = TRUE;
    }
    ConfigClose(2);
    PauseCursorKill();
    pauseWork.menuEntered = 0;
    HuPrcEnd();
}

/* ConfigOpen creates and lays out the eight configuration panels before enabling menu input. */
static void ConfigOpen(void)
{
    int menuIndex;
    HuVecF panelPos;
    CONFIG_MENU_WORK *menuEntry;
    extern float lbl_802C4CCC;

    menuEntry = pauseWork.menu;
    for (menuIndex = 0; menuIndex < 8; menuIndex++, menuEntry++) {
        menuEntry->panelId = 0;
    }
    pauseWork.activeF = FALSE;
    menuEntry = pauseWork.menu;
    for (menuIndex = 0; menuIndex < 8; menuIndex++, menuEntry++) {
        menuEntry->menuIndex = menuIndex;
        menuEntry->panelId = mbPausePanelCreate(pausePanelFileTbl[menuIndex],
            pausePanelLabelFileTbl[menuIndex]);
        if (menuEntry->available == 0
            || menuEntry->optionIndex == pauseBatsuValueTbl[menuIndex]) {
            mbPausePanelBatsuSet((s16)menuEntry->panelId, TRUE);
        }
        panelPos.x = 0.4f * ((float)pauseGridPosTbl[menuIndex][0] - 1.5f);
        panelPos.y = 0.3f + (-0.5f * ((float)pauseGridPosTbl[menuIndex][1] - 0.5f));
        panelPos.z = 0.0f;
        mbPausePanelPosSet((s16)menuEntry->panelId, panelPos.x, panelPos.y);
        mbPausePanelGrowSet(
            (s16)menuEntry->panelId, 16, menuIndex * 2, lbl_802C4CCC);
        menuEntry->optionCount = pauseValueNumTbl[menuIndex];
        mbPausePanelBankSet((s16)menuEntry->panelId, menuEntry->optionIndex);
    }
    PauseCursorCreate();
    mbPauseGuideMoveSet(pausePlayer, 20, NULL, &pauseGuidePos);
    ConfigGrowWait();
}

/* ConfigGrowWait yields until all newly opened menu panels report that their motion has settled. */
static void ConfigGrowWait(void)
{
    BOOL allPanelsSettled = FALSE;
    CONFIG_MENU_WORK *menuEntry;
    int menuIndex;

    do {
        allPanelsSettled = TRUE;
        menuEntry = pauseWork.menu;
        for (menuIndex = 0; menuIndex < 8; menuIndex++, menuEntry++) {
            if (menuEntry->panelId != 0
                && !mbPausePanelFreezeGet((s16)menuEntry->panelId)) {
                allPanelsSettled = FALSE;
            }
        }
        if (!allPanelsSettled) {
            HuPrcVSleep();
        }
    } while (!allPanelsSettled);
}

/* ConfigClose shrinks enabled panels, moves the guide when requested, and waits for the animation
 * to finish. */
static void ConfigClose(s32 result)
{
    BOOL doneF = FALSE;
    BOOL waitDoneF;
    s32 waitPanelIndex;
    CONFIG_MENU_WORK *waitMenuP;
    s32 i;
    CONFIG_MENU_WORK *menuP;
    HuVecF pos;

    menuP = pauseWork.menu;
    for (i = 0; i < 8; i++, menuP++) {
        if (menuP->enabled && menuP->panelId != 0) {
            pos.x = 0.4f * ((float)pauseGridPosTbl[i][0] - 1.5f);
            pos.y = 0.3f
                + (-0.5f
                    * ((float)pauseGridPosTbl[i][1] - 0.5f));
            pos.z = lbl_802C4CB4;
            mbPausePanelPosSet((s16)menuP->panelId, pos.x, pos.y);
            mbPausePanelShrinkSet((s16)menuP->panelId, 16, 0);
        }
    }
    if (result != 0) {
        if (result == 1) {
            mbPauseGuideMoveSet(pausePlayer, 20, NULL, &pauseGuideQuitPos);
        } else {
            mbPauseGuideMoveSet(pausePlayer, 20, &playerPos, NULL);
        }
    }
    waitDoneF = FALSE;
    do {
        waitDoneF = TRUE;
        waitMenuP = pauseWork.menu;
        for (waitPanelIndex = 0; waitPanelIndex < 8;
            waitPanelIndex++, waitMenuP++) {
            if (waitMenuP->panelId != 0
                && !mbPausePanelFreezeGet((s16)waitMenuP->panelId)) {
                waitDoneF = FALSE;
            }
        }
        if (!waitDoneF) {
            HuPrcVSleep();
        }
    } while (!waitDoneF);
    menuP = pauseWork.menu;
    for (i = 0; i < 8; i++, menuP++) {
        if (menuP->enabled && menuP->panelId != 0) {
            mbPausePanelKill((s16)menuP->panelId);
            menuP->panelId = 0;
            menuP->enabled = FALSE;
        }
    }
}

/* ConfigExec runs after ConfigOpen in the pause child process and handles the eight menu
 * choices. */
static void ConfigExec(void)
{
    CONFIG_MENU_WORK *menuP;
    HuVecF pos;
    s32 cursorPos;
    s32 previousCursorPos;
    s32 playerController;
    s32 pressedDirections;
    BOOL doneF;
    s32 previousOptionIndex;
    s32 messageSpeed;
    s32 menuIndex;
    BOOL vibrationEnabled;
    int cursorMove;
    extern float lbl_802C4CC8;
    extern float lbl_802C4CCC;
    extern float lbl_802C4CD0;
    extern float lbl_802C4CD4;
    extern float lbl_802C4CD8;
    struct {
        BOOL controllerError;
        BOOL confirmPressed;
    } inputState;
    int stickDirectionMaskByCursor[8] = { 6, 7, 7, 5, 10, 11, 11, 9 };

    ConfigPadWinSet(0);
    pauseWork.menuEntered = 1;
    previousCursorPos = -1;
    while (pauseWork.menuExitRequested == 0) {
        HuPrcVSleep();
        mbPauseGuideTalkSet(pausePlayer);
        playerController = GwPlayer[pauseWork.playerNo].padNo;
        inputState.controllerError = HuPadStatGet((s16)playerController) != 0;
        cursorMove = PauseCursorMove();
        if (cursorMove != 0 && previousCursorPos >= 0) {
            if ((HuPadBtnDown[playerController] & PAD_BUTTON_A)
                && !inputState.controllerError && cursorMove < 10) {
                pauseWork.activeF = 1;
            }
            continue;
        }

        cursorPos = pauseWork.cursorPos;
        previousCursorPos = cursorPos;
        if (cursorPos < 0) {
            cursorPos = 0;
        }
        if (pauseWork.activeF == 0) {
            pressedDirections = HuPadDStk[playerController]
                & stickDirectionMaskByCursor[cursorPos];
            if (inputState.controllerError) {
                pressedDirections = 0;
            }
            if (previousCursorPos >= 0) {
                if (pressedDirections & PAD_BUTTON_UP) {
                    mbAudFXPlay(MSM_SE_CMN_01);
                    cursorPos = (cursorPos + 4) & 7;
                } else if (pressedDirections & PAD_BUTTON_DOWN) {
                    mbAudFXPlay(MSM_SE_CMN_01);
                    cursorPos = (cursorPos - 4) & 7;
                }
                if (pressedDirections & PAD_BUTTON_RIGHT) {
                    mbAudFXPlay(MSM_SE_CMN_01);
                    cursorPos = (cursorPos + 1) & 7;
                } else if (pressedDirections & PAD_BUTTON_LEFT) {
                    mbAudFXPlay(MSM_SE_CMN_01);
                    cursorPos = (cursorPos - 1) & 7;
                }
            }
            pauseWork.cursorPos = cursorPos;
            PauseCursorHiliteSet(
                0, stickDirectionMaskByCursor[cursorPos], HuPadDStk[playerController]);
            if (previousCursorPos != cursorPos) {
                pos.x = 0.4f * ((float)pauseGridPosTbl[cursorPos][0] - 1.5f);
                pos.y = 0.3f
                    + (-0.5f
                        * ((float)pauseGridPosTbl[cursorPos][1] - 0.5f));
                pos.z = 0.0f;
                PauseCursorPosNextSet(12, pos.x, pos.y);
                if (previousCursorPos >= 0) {
                    mbPausePanelSizeSet(
                        (s16)pauseWork.menu[previousCursorPos].panelId, 12, lbl_802C4CCC);
                }
                if (cursorPos >= 0) {
                    mbPausePanelSizeSet(
                        (s16)pauseWork.menu[cursorPos].panelId, 12, lbl_802C4CD0);
                }
                ConfigWinUpdate(cursorPos);
            }
        }

        inputState.confirmPressed = FALSE;
        if ((HuPadBtnDown[playerController] & PAD_BUTTON_A)
            && !inputState.controllerError) {
            pauseWork.activeF = TRUE;
        }
        if (pauseWork.activeF != 0) {
            if (mbPausePanelFreezeGet(
                    (s16)pauseWork.menu[cursorPos].panelId) == FALSE) {
                goto config_exec_bottom;
            }
            pauseWork.activeF = 0;
            if (pauseWork.menu[cursorPos].available == 0) {
                mbAudFXPlay(MSM_SE_CMN_05);
                goto config_exec_bottom;
            }
            if (cursorPos == 0) {
                HuVecF shrinkPos;
                HuVecF growPos;

                mbAudFXPlay(MSM_SE_CMN_03);
                pos.x = lbl_802C4CD4;
                pos.y = lbl_802C4CC8;
                pos.z = 0.0f;
                pos.x += lbl_802C4CD8;
                mbPausePanelSlideSet(
                    (s16)pauseWork.menu[0].panelId, 16, &pos);
                for (cursorMove = 1; cursorMove < 8; cursorMove++) {
                    pauseWork.menu[cursorMove].enabled = TRUE;
                }
                PauseCursorHiliteSet(-1, 0, 0);
                doneF = FALSE;
                menuP = pauseWork.menu;
                for (menuIndex = 0; menuIndex < 8; menuIndex++, menuP++) {
                    if (menuP->enabled && menuP->panelId != 0) {
                        s32 gridIndex = menuIndex;

                        shrinkPos.x = 0.4f
                            * ((float)pauseGridPosTbl[gridIndex][0] - 1.5f);
                        shrinkPos.y = 0.3f
                            + (-0.5f
                                * ((float)pauseGridPosTbl[gridIndex][1] - 0.5f));
                        shrinkPos.z = 0.0f;
                        mbPausePanelPosSet(
                            (s16)menuP->panelId, shrinkPos.x, shrinkPos.y);
                        mbPausePanelShrinkSet(
                            (s16)menuP->panelId, 16, 0);
                    }
                }
                {
                    BOOL waitDoneF;
                    s32 waitPanelIndex;
                    CONFIG_MENU_WORK *waitMenuP;

                    waitDoneF = FALSE;
                    do {
                        waitDoneF = TRUE;
                        waitMenuP = pauseWork.menu;
                        for (waitPanelIndex = 0; waitPanelIndex < 8;
                            waitPanelIndex++, waitMenuP++) {
                            if (waitMenuP->panelId != 0
                            && !mbPausePanelFreezeGet(
                                (s16)waitMenuP->panelId)) {
                                waitDoneF = FALSE;
                            }
                        }
                        if (!waitDoneF) {
                            HuPrcVSleep();
                        }
                    } while (!waitDoneF);
                }
                menuP = pauseWork.menu;
                for (menuIndex = 0; menuIndex < 8; menuIndex++, menuP++) {
                    if (menuP->enabled && menuP->panelId != 0) {
                        mbPausePanelKill((s16)menuP->panelId);
                        menuP->panelId = 0;
                        menuP->enabled = FALSE;
                    }
                }
                ConfigPadOpen();
                while (!ConfigPadMain()) {
                    HuPrcVSleep();
                    mbPauseGuideTalkSet(pausePlayer);
                }
                mbAudFXPlay(MSM_SE_CMN_04);
                PauseCursorHiliteSet(-1, 0, 0);
                ConfigPadClose();
                pauseWork.activeF = 0;
                {
                    CONFIG_MENU_WORK *growMenuP;
                    s32 growI;

                    growMenuP = pauseWork.menu;
                    for (growI = 0; growI < 8; growI++, growMenuP++) {
                        if (growMenuP->panelId <= 0) {
                            s32 gridIndex;

                            growMenuP->panelId = mbPausePanelCreate(
                                pausePanelFileTbl[growI],
                                pausePanelLabelFileTbl[growI]);
                            if (growMenuP->available == 0
                                || growMenuP->optionIndex == pauseBatsuValueTbl[growI]) {
                                mbPausePanelBatsuSet((s16)growMenuP->panelId, TRUE);
                            }
                            gridIndex = growI;
                            growPos.x = 0.4f
                                * ((float)pauseGridPosTbl[gridIndex][0] - 1.5f);
                            growPos.y = 0.3f
                                + (-0.5f
                                    * ((float)pauseGridPosTbl[gridIndex][1] - 0.5f));
                            growPos.z = 0.0f;
                            mbPausePanelPosSet(
                                (s16)growMenuP->panelId, growPos.x, growPos.y);
                            mbPausePanelGrowSet(
                                (s16)growMenuP->panelId, 16, growI * 2, lbl_802C4CCC);
                            mbPausePanelBankSet(
                                (s16)growMenuP->panelId, growMenuP->optionIndex);
                        }
                    }
                }
                mbPauseGuideMoveSet(pausePlayer, 20, NULL, &pauseGuidePos);
                {
                    BOOL waitDoneF;
                    s32 waitPanelIndex;
                    CONFIG_MENU_WORK *waitMenuP;

                    waitDoneF = FALSE;
                    do {
                        waitDoneF = TRUE;
                        waitMenuP = pauseWork.menu;
                        for (waitPanelIndex = 0; waitPanelIndex < 8;
                            waitPanelIndex++, waitMenuP++) {
                            if (waitMenuP->panelId != 0
                            && !mbPausePanelFreezeGet(
                                (s16)waitMenuP->panelId)) {
                                waitDoneF = FALSE;
                            }
                        }
                        if (!waitDoneF) {
                            HuPrcVSleep();
                        }
                    } while (!waitDoneF);
                }
                pauseWork.cursorPos = -1;
                HuPrcVSleep();
                goto config_exec_bottom;
            }
            if (cursorPos == 7) {
                HuVecF quitPos;

                mbAudFXPlay(MSM_SE_CMN_02);
                if (ConfigQuitExec()) {
                    mbWipeFadeOut();
                    _SetFlag(FLAG_BOARD_TURN_NOSTART);
                    pauseWork.menuExitRequested = 1;
                    goto config_exec_bottom;
                }
                pauseWork.activeF = 0;
                {
                    CONFIG_MENU_WORK *quitMenuP;
                    s32 quitI;

                    quitMenuP = pauseWork.menu;
                    for (quitI = 0; quitI < 8; quitI++, quitMenuP++) {
                        if (quitMenuP->panelId <= 0) {
                            s32 gridIndex;

                            quitMenuP->panelId = mbPausePanelCreate(
                                pausePanelFileTbl[quitI],
                                pausePanelLabelFileTbl[quitI]);
                            if (quitMenuP->available == 0
                                || quitMenuP->optionIndex == pauseBatsuValueTbl[quitI]) {
                                mbPausePanelBatsuSet((s16)quitMenuP->panelId, TRUE);
                            }
                            gridIndex = quitI;
                            quitPos.x = 0.4f
                                * ((float)pauseGridPosTbl[gridIndex][0] - 1.5f);
                            quitPos.y = 0.3f
                                + (-0.5f
                                    * ((float)pauseGridPosTbl[gridIndex][1] - 0.5f));
                            quitPos.z = 0.0f;
                            mbPausePanelPosSet(
                                (s16)quitMenuP->panelId, quitPos.x, quitPos.y);
                            mbPausePanelGrowSet(
                                (s16)quitMenuP->panelId, 16, quitI * 2, lbl_802C4CCC);
                            mbPausePanelBankSet(
                                (s16)quitMenuP->panelId, quitMenuP->optionIndex);
                        }
                    }
                }
                mbPauseGuideMoveSet(pausePlayer, 20, NULL, &pauseGuidePos);
                {
                    BOOL waitDoneF;
                    s32 waitPanelIndex;
                    CONFIG_MENU_WORK *waitMenuP;

                    waitDoneF = FALSE;
                    do {
                        waitDoneF = TRUE;
                        waitMenuP = pauseWork.menu;
                        for (waitPanelIndex = 0; waitPanelIndex < 8;
                            waitPanelIndex++, waitMenuP++) {
                            if (waitMenuP->panelId != 0
                            && !mbPausePanelFreezeGet(
                                (s16)waitMenuP->panelId)) {
                                waitDoneF = FALSE;
                            }
                        }
                        if (!waitDoneF) {
                            HuPrcVSleep();
                        }
                    } while (!waitDoneF);
                }
                ConfigPadWinSet(0);
                pauseWork.cursorPos = -1;
                goto config_exec_bottom;
            }

            mbAudFXPlay(MSM_SE_CMN_02);
            if (pauseWork.menu[cursorPos].optionCount > 1) {
                previousOptionIndex = pauseWork.menu[cursorPos].optionIndex;
                pauseWork.menu[cursorPos].optionIndex++;
                if (pauseWork.menu[cursorPos].optionIndex
                    >= pauseWork.menu[cursorPos].optionCount) {
                    pauseWork.menu[cursorPos].optionIndex = 0;
                }
                /* Skip microphone option 0 when probing channel 1 returns nonzero. */
                if (pauseWork.menu[cursorPos].menuIndex == 6
                    && pauseWork.menu[cursorPos].optionIndex == 0) {
                    if (HuMCProbe(1)) {
                        pauseWork.menu[cursorPos].optionIndex = 1;
                    }
                }
                (void)previousOptionIndex;
                mbPausePanelAnmNoSet(
                    (s16)pauseWork.menu[cursorPos].panelId, 12,
                    pauseWork.menu[cursorPos].optionIndex);
                if (pauseWork.menu[cursorPos].optionIndex
                    == pauseBatsuValueTbl[pauseWork.menu[cursorPos].menuIndex]) {
                    mbPausePanelBatsuSet(
                        (s16)pauseWork.menu[cursorPos].panelId, TRUE);
                } else {
                    mbPausePanelBatsuSet(
                        (s16)pauseWork.menu[cursorPos].panelId, FALSE);
                }
            }
            if (cursorPos == 5) {
                messageSpeed = mesSpeedAnmNo[pauseWork.menu[5].optionIndex];
                GwSystem.messSpeed = messageSpeed;
                switch (messageSpeed) {
                    case GW_MESS_SPEED_FAST:
                        GwSystem.comKeyDelay = 16;
                        break;

                    case GW_MESS_SPEED_SLOW:
                        GwSystem.comKeyDelay = 48;
                        break;

                    default:
                        GwSystem.comKeyDelay = 32;
                        break;
                }
            } else if (cursorPos == 4) {
                vibrationEnabled = pauseWork.menu[4].optionIndex == 0;

                GwCommon.vibrateF = vibrationEnabled;
                if (!vibrationEnabled) {
                    HuPadRumbleAllStop();
                }
                omVibrate((s16)pauseWork.playerNo, 20, 7, 3);
            } else if (cursorPos == 6) {
                HuMCMicSet(
                    pauseMicValueTbl[pauseWork.menu[6].optionIndex]);
            }
            ConfigWinUpdate(cursorPos);
        }

config_exec_bottom:
        if ((HuPadBtnDown[playerController] & PAD_BUTTON_B)
            && !inputState.controllerError
            && pauseWork.menuExitRequested == 0) {
            pauseWork.talkTime = 0;
            pauseWork.prevTalkTime = 1;
            mbPauseGuideTalkSet(pausePlayer);
            pauseWork.menuExitRequested = 1;
            mbAudFXPlay(MSM_SE_CMN_04);
            OSReport(configExitMessage);
        }
    }
    PauseCursorHiliteSet(-1, 0, 0);
}

/* ConfigExec calls ConfigQuitExec for the exit choice; it asks for confirmation and handles
 * saving. */
static BOOL ConfigQuitExec(void)
{
    CONFIG_MENU_WORK *menuP;
    CONFIG_MENU_WORK *waitMenuP;
    s32 choice;
    MBWIN *winP;
    HuVecF pos;
    HuVecF menuPos;
    BOOL doneF;
    BOOL waitDoneF;
    s32 padStat;
    BOOL partyF;
    BOOL partyF2;
    BOOL comF;
    s32 confirmWindowNo;
    s32 menuGridIndex;
    s32 saveValue;
    s32 waitPanelIndex;
    s32 menuPanelIndex;
    s32 i;
    float guideBobOffset;
    extern float lbl_802C4CDC;
    extern float lbl_802C4CE0;
    extern float lbl_802C4CE4;
    extern float lbl_802C4CE8;
    extern float lbl_802C4CEC;
    extern float lbl_802C4CF0;

    for (i = 0; i < 8; i++) {
        pauseWork.menu[i].enabled = TRUE;
    }
    PauseCursorHiliteSet(-1, 0, 0);
    ConfigWinUpdate(-1);
    ConfigPadWinSet(-1);

    doneF = FALSE;
    menuP = pauseWork.menu;
    for (menuPanelIndex = 0; menuPanelIndex < 8; menuPanelIndex++, menuP++) {
        if (menuP->enabled && menuP->panelId != 0) {
            menuGridIndex = menuPanelIndex;
            menuPos.x = 0.4f * ((float)pauseGridPosTbl[menuGridIndex][0] - 1.5f);
            menuPos.y = 0.3f
                + (-0.5f
                    * ((float)pauseGridPosTbl[menuGridIndex][1] - 0.5f));
            menuPos.z = lbl_802C4CB4;
            mbPausePanelPosSet((s16)menuP->panelId, menuPos.x, menuPos.y);
            mbPausePanelShrinkSet((s16)menuP->panelId, 16, 0);
        }
    }
    mbPauseGuideMoveSet(pausePlayer, 20, NULL, &pauseGuideQuitPos);

    waitDoneF = FALSE;
    do {
        waitDoneF = TRUE;
        waitMenuP = pauseWork.menu;
        for (waitPanelIndex = 0; waitPanelIndex < 8;
            waitPanelIndex++, waitMenuP++) {
            if (waitMenuP->panelId != 0
                && !mbPausePanelFreezeGet((s16)waitMenuP->panelId)) {
                waitDoneF = FALSE;
            }
        }
        if (!waitDoneF) {
            HuPrcVSleep();
        }
    } while (!waitDoneF);

    menuP = pauseWork.menu;
    for (menuPanelIndex = 0; menuPanelIndex < 8; menuPanelIndex++, menuP++) {
        if (menuP->enabled && menuP->panelId != 0) {
            mbPausePanelKill((s16)menuP->panelId);
            menuP->panelId = 0;
            menuP->enabled = FALSE;
        }
    }

    mbObjMotionShiftSet(pausePlayer, 11, lbl_802C4CB4, lbl_802C4CDC, HU3D_MOTATTR_LOOP);
    /* Temporarily force human input for the quit prompt; restore both type fields from comF. */
    comF = GwPlayer[pauseWork.playerNo].comF;
    GwPlayer[pauseWork.playerNo].comF = FALSE;
    GwPlayerConf[pauseWork.playerNo].type = FALSE;
    padStat = HuPadStatGet((s16)GwPlayer[pauseWork.playerNo].padNo);
    partyF = GwSystem.partyF;
    if (partyF) {
        if (SLSaveFlagGet() && saveExecF) {
            confirmWindowNo = mbWinCreate(3, MESSNUM(MESS_BOARD_PAUSE, 25), -1);
            mbWinPriSet((s16)confirmWindowNo, 100);
            mbWinPlayerDisable((s16)confirmWindowNo, pauseWork.playerNo);
            mbWinMesSpeedSet((s16)confirmWindowNo, 0);
            winP = mbWinGet(confirmWindowNo);
            winP->size.x = lbl_802C4CE0;
            mbWinCenterGet((s16)confirmWindowNo, (HuVec2f *)&pos);
            mbWinPosSet((s16)confirmWindowNo, (s16)pos.x, 100);
            mbWinAttrReset((s16)confirmWindowNo, HUWIN_ATTR_NOCANCEL);
            mbWinWait((s16)confirmWindowNo);
        }
        confirmWindowNo = mbWinCreateChoice(
            3, MESSNUM(MESS_BOARD_PAUSE, 20), -1, 0);
    } else {
        confirmWindowNo = mbWinCreateChoice(
            3, MESSNUM(MESS_BOARD_PAUSE, 41), -1, 0);
    }
    mbWinPriSet((s16)confirmWindowNo, 100);
    mbWinPlayerDisable((s16)confirmWindowNo, pauseWork.playerNo);
    mbWinMesSpeedSet((s16)confirmWindowNo, 0);
    winP = mbWinGet(confirmWindowNo);
    winP->size.x = lbl_802C4CE0;
    mbWinCenterGet((s16)confirmWindowNo, (HuVec2f *)&pos);
    mbWinPosSet((s16)confirmWindowNo, (s16)pos.x, 100);

    partyF2 = GwSystem.partyF;
    if (partyF2) {
        ConfigPadWinSet(2);
    } else {
        ConfigPadWinSet(3);
    }
    if (padStat == 0) {
        mbWinAttrReset((s16)confirmWindowNo, HUWIN_ATTR_NOCANCEL);
        mbWinWait((s16)confirmWindowNo);
        choice = mbWinChoiceGet((s16)confirmWindowNo);
        ConfigPadWinSet(-1);
        if (choice > 0) {
            BOOL exitPartyF;

            mbObjMotionShiftSet(
                pausePlayer, 7, lbl_802C4CB4, lbl_802C4CE4, HU3D_MOTATTR_LOOP);
            for (i = 0; i < 30; i++) {
                HuPrcVSleep();
            }
            exitPartyF = GwSystem.partyF;
            /* This negated party-mode value is discarded and has no effect. */
            !exitPartyF;
        } else if (choice == 0) {
            mbObjPosGet(pausePlayer, &pos);
            mbObjMotionShiftSet(
                pausePlayer, 23, lbl_802C4CB4, lbl_802C4CDC, HU3D_MOTATTR_NONE);
            mbAudGuidePlay(MSM_SE_GUIDE_26);
            for (i = 0; i < 10; i++) {
                HuPrcVSleep();
            }
            for (i = 0; i <= 30; i++) {
            guideBobOffset = lbl_802C4CE8 * (float)i;
            guideBobOffset = lbl_802C4CEC
                * (0.5f * mbSinDeg(lbl_802C4CF0 * guideBobOffset));
            mbObjPosSet(pausePlayer, pos.x, pos.y + guideBobOffset, pos.z);
                HuPrcVSleep();
            }
        }
    } else {
        /* A controller error forces choice 0, cancelling the quit request without waiting for
         * input. */
        choice = 0;
        mbWinKill((s16)confirmWindowNo);
        ConfigPadWinSet(-1);
    }

    GwPlayer[pauseWork.playerNo].comF = comF;
    GwPlayerConf[pauseWork.playerNo].type = comF;
    configResult = FALSE;
    if (choice > 0) {
        BOOL savePartyF;

        configResult = TRUE;
        savePartyF = GwSystem.partyF;
        if (!savePartyF) {
            if (choice > 1) {
                saveValue = -1;
            } else {
                saveValue = 0;
            }
            mbSingleSaveFlush(saveValue);
        }
    }
    return configResult;
}

/* ConfigWinUpdate refreshes the help text for the highlighted menu entry. */
static inline void ConfigWinUpdate(s32 menuIndex)
{
    HuVec2f pos;
    s32 mesNo;
    BOOL partyF;
    s32 i;
    u32 *winMesTbl;

    if (pauseWork.helpWinNo >= 0) {
        mbWinKill((s16)pauseWork.helpWinNo);
        pauseWork.helpWinNo = -1;
    }
    if (menuIndex >= 0) {
        partyF = GwSystem.partyF;
        if (partyF) {
            winMesTbl = pauseWinMesTbl[menuIndex];
        } else {
            winMesTbl = pauseSingleWinMesTbl[menuIndex];
        }
        if (menuIndex == 0) {
            pauseWork.helpWinNo = mbWinCreateTime(8, winMesTbl[0], -1);
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (GwPlayer[pauseWork.padWork[i].port].comF) {
                    mesNo = 2;
                } else {
                    mesNo = 1;
                }
                mbWinInsertMesSet((s16)pauseWork.helpWinNo,
                    pauseWinMesTbl[menuIndex][mesNo], i);
            }
        } else {
            pauseWork.helpWinNo = mbWinCreateTime(8,
                winMesTbl[pauseWork.menu[menuIndex].optionIndex], -1);
        }
        mbWinPosGet((s16)pauseWork.helpWinNo, &pos);
        mbWinPosSet((s16)pauseWork.helpWinNo, 144, (s16)pos.y);
        mbWinPause((s16)pauseWork.helpWinNo);
        pauseWork.talkTime = 30;
    }
}

/* Replaces or removes the pause menu's controls or quit-confirmation help window. */
static void ConfigPadWinSet(s32 padHelpIndex)
{
    HuVec2f pos;

    if (pauseWork.padWinNo >= 0) {
        mbWinKill((s16)pauseWork.padWinNo);
        pauseWork.padWinNo = -1;
    }
    if (padHelpIndex < 0) {
        return;
    }
    pauseWork.padWinNo = mbWinCreateHelp(pausePadWinMesTbl[padHelpIndex]);
    mbWinCenterGet((s16)pauseWork.padWinNo, &pos);
    pos.x += pausePadWinPosTbl[padHelpIndex][0];
    pos.y = pausePadWinPosTbl[padHelpIndex][1];
    mbWinPosSet((s16)pauseWork.padWinNo, (s16)pos.x, (s16)pos.y);
}

/* Before menu creation, maps current settings to choices, resets invalid packs and the MAX
 * message-speed sentinel in GwSystem, and enforces mode, controller-setup and microphone
 * restrictions. */
static void ConfigSettingRead(void)
{
    s32 microphoneMode;
    s32 packValue;
    s32 messSpeedIndex;
    BOOL instDispF;
    BOOL comDispF;
    BOOL vibrateF;
    BOOL partyF;
    s32 i;
    s32 instValue;
    s32 comValue;
    s32 vibrateValue;

    pauseWork.menu[0].optionIndex = 0;
    instDispF = GwSystem.mgInstDispF;
    if (instDispF) {
        instValue = 0;
    } else {
        instValue = 1;
    }
    pauseWork.menu[1].optionIndex = instValue;
    comDispF = GwSystem.mgComDispF;
    if (comDispF) {
        comValue = 0;
    } else {
        comValue = 1;
    }
    pauseWork.menu[2].optionIndex = comValue;
    if (GwSystem.mgPack >= GW_MINIGAME_PACK_MAX) {
        GwSystem.mgPack = GW_MINIGAME_PACK_ALL;
    }
    packValue = GwSystem.mgPack;
    pauseWork.menu[3].optionIndex = packValue;
    vibrateF = GwCommon.vibrateF;
    if (vibrateF) {
        vibrateValue = 0;
    } else {
        vibrateValue = 1;
    }
    pauseWork.menu[4].optionIndex = vibrateValue;
    if (GwSystem.messSpeed == GW_MESS_SPEED_MAX) {
        GwSystem.messSpeed = GW_MESS_SPEED_NORMAL;
    }
    messSpeedIndex = GwSystem.messSpeed;
    pauseWork.menu[5].optionIndex = mesSpeedAnmNo[messSpeedIndex];
    microphoneMode = HuMCMicGet();
    for (i = 0; i < 3; i++) {
        if (microphoneMode == pauseMicValueTbl[i]) {
            break;
        }
    }
    if (i >= 3) {
        i = 0;
    }
    pauseWork.menu[6].optionIndex = i;
    if (i == 0 && HuMCProbe(1)) {
        pauseWork.menu[6].optionIndex = 1;
    }
    for (i = 0; i < 8; i++) {
        pauseWork.menu[i].available = TRUE;
    }
    partyF = GwSystem.partyF;
    if (!partyF) {
        pauseWork.menu[0].available = FALSE;
        pauseWork.menu[2].optionIndex = 1;
        pauseWork.menu[2].available = FALSE;
    }
    if (configPadEnabled == 0) {
        pauseWork.menu[0].available = FALSE;
    }
}

/* Stores choices after ConfigMain ends; microphone option 0 is forced to option 1 when
 * HuMCProbe(1) returns nonzero. */
static void ConfigSettingWrite(void)
{
    BOOL vibrateF;
    s32 messageSpeed;
    BOOL instDispF;
    BOOL comDispF;
    s32 packValue;
    BOOL partyF;

    instDispF = !pauseWork.menu[1].optionIndex;
    GwSystem.mgInstDispF = instDispF;
    partyF = GwSystem.partyF;
    if (partyF) {
        comDispF = !pauseWork.menu[2].optionIndex;
        GwSystem.mgComDispF = comDispF;
    }
    packValue = pauseWork.menu[3].optionIndex;
    GwSystem.mgPack = packValue;
    vibrateF = !pauseWork.menu[4].optionIndex;
    GwCommon.vibrateF = vibrateF;
    if (!vibrateF) {
        HuPadRumbleAllStop();
    }
    messageSpeed = mesSpeedAnmNo[pauseWork.menu[5].optionIndex];
    GwSystem.messSpeed = messageSpeed;
    switch (messageSpeed) {
        case GW_MESS_SPEED_FAST:
            GwSystem.comKeyDelay = 16;
            break;
        case GW_MESS_SPEED_SLOW:
            GwSystem.comKeyDelay = 48;
            break;
        default:
            GwSystem.comKeyDelay = 32;
            break;
    }
    if (pauseWork.menu[6].optionIndex == 0 && HuMCProbe(1)) {
        pauseWork.menu[6].optionIndex = 1;
    }
    HuMCMicSet(pauseMicValueTbl[pauseWork.menu[6].optionIndex]);
}

/* ConfigExec calls this after the player selects controller setup; it builds and grows each row. */
static void ConfigPadOpen(void)
{
    CONFIG_PAD_WORK *padWork;
    HuVecF iconPos;
    int iconGrowFrames[GW_PLAYER_MAX];
    int settledPadCount;
    int playerIndex;
    int iconIndex;
    int spriteId;
    int animId;
    float posStep;
    float weight;
    extern float lbl_802C4CC0;
    extern float lbl_802C4CC4;
    extern float lbl_802C4CC8;
    extern float lbl_802C4CCC;
    extern float lbl_802C4CD4;
    extern float lbl_802C4CF4;
    extern float lbl_802C4CF8;
    extern float lbl_802C4CFC;
    extern float lbl_802C4D00;
    extern float lbl_802C4D04;
    extern float lbl_802C4D08;
    extern float lbl_802C4D0C;

    pauseWork.configPadPlayerIndex = -1;
    pauseWork.activeF = FALSE;
    PausePlayerComRead(FALSE);
    padWork = pauseWork.configPad;
    for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++, padWork++) {
        for (iconIndex = 0; iconIndex < 3; iconIndex++) {
            padWork->sprId[iconIndex] = -1;
        }
        padWork->playerNo = pauseWork.padWork[playerIndex].port;
        padWork->padNo = pauseWork.padWork[playerIndex].padNo;
    }

    iconPos.x = lbl_802C4CD4;
    iconPos.y = lbl_802C4CC8;
    iconPos.z = 0.0f;
    weight = iconPos.x;
    iconPos.x = lbl_802C4CF4;
    iconPos.y = lbl_802C4CC8;
    iconPos.z = 0.0f;
    posStep = lbl_802C4CF8 * (iconPos.x - weight);
    for (padWork = pauseWork.configPad, animId = HU3D_ANIMID_NONE, playerIndex = 0;
         playerIndex < GW_PLAYER_MAX; playerIndex++, padWork++) {
        padWork->comDif = GwPlayer[padWork->playerNo].comDif;
        padWork->activeF = PausePadCheck(padWork->padNo);
        if (!GwPlayer[padWork->playerNo].comF) {
            padWork->comDif = GW_PLAYER_COM_DIF_MAX;
        }
        padWork->panelId = mbPausePanelCreate(
            pauseCharPanelFileTbl[GwPlayer[padWork->playerNo].charNo], 0);
        iconPos.x = (lbl_802C4CC0 * (float)playerIndex) - lbl_802C4CC4;
        iconPos.y = lbl_802C4CC8;
        iconPos.z = 0.0f;
        mbPausePanelPosSet((s16)padWork->panelId, iconPos.x, iconPos.y);
        mbPausePanelGrowSet((s16)padWork->panelId, 16, playerIndex * 4, lbl_802C4CFC);
        mbNormPosto2D(&iconPos, &iconPos);
        padWork->pos.x = lbl_802C4D00;
        padWork->pos.y = lbl_802C4D04;
        padWork->pos.z = 0.0f;
        PSVECAdd(&iconPos, &padWork->pos, &iconPos);
        for (iconIndex = 0; iconIndex < 3; iconIndex++) {
            spriteId = espEntry(
                mbBoardDataNumGet(pausePadSpriteFileTbl[iconIndex]),
                (s16)(100 - iconIndex), 0);
            padWork->sprId[iconIndex] = spriteId;
            espAttrSet(spriteId, HUSPR_ATTR_LINEAR);
            espPosSet(spriteId, iconPos.x + configPadSprOfsTbl[iconIndex][0],
                iconPos.y + configPadSprOfsTbl[iconIndex][1]);
        }
        ConfigPadSprSet(padWork);
        ConfigPadTimeSet(padWork, 0.0f);
    }

    for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
        iconGrowFrames[playerIndex] = 0;
    }
    settledPadCount = 0;
    while (settledPadCount < GW_PLAYER_MAX) {
        settledPadCount = 0;
        padWork = pauseWork.configPad;
        for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++, padWork++) {
            if (mbPausePanelFreezeGet((s16)padWork->panelId)) {
                iconGrowFrames[playerIndex]++;
                weight = lbl_802C4D08 * (float)iconGrowFrames[playerIndex];
                if (weight > lbl_802C4CCC) {
                    weight = lbl_802C4CCC;
                    settledPadCount++;
                }
                weight = mbSinDeg(lbl_802C4D0C * weight);
                ConfigPadTimeSet(padWork, weight);
            }
        }
        HuPrcVSleep();
    }
}

/* ConfigPadSprSet selects the controller icon sprites for one player's assignment row. */
static void ConfigPadSprSet(CONFIG_PAD_WORK *workP)
{
    s32 sprId = workP->sprId[0];
    s32 bank = 0;

    if (workP->comDif < GW_PLAYER_COM_DIF_MAX) {
        bank = 1;
    }
    espBankSet(sprId, bank);
    sprId = workP->sprId[1];
    bank = workP->padNo;
    if (workP->comDif < GW_PLAYER_COM_DIF_MAX) {
        bank = 4;
    }
    espBankSet(sprId, bank);
    sprId = workP->sprId[2];
    if (workP->comDif < GW_PLAYER_COM_DIF_MAX) {
        espDispOn(sprId);
        espBankSet(sprId, workP->comDif + 5);
    } else {
        espDispOff(sprId);
    }
}

/* ConfigPadTimeSet animates the controller assignment row to the requested scale. */
static void ConfigPadTimeSet(CONFIG_PAD_WORK *workP, float scale)
{
    HuVecF pos;
    float panelScale;
    s32 sprId;

    mbPausePanelPosGet((s16)workP->panelId, &pos);
    mbNormPosto2D(&pos, &pos);
    panelScale = mbPausePanelScaleGet((s16)workP->panelId);
    pos.x += workP->pos.x * panelScale;
    pos.y += workP->pos.y * panelScale;

    sprId = workP->sprId[0];
    espScaleSet(sprId, scale, scale);
    espPosSet(sprId, pos.x + (configPadSprOfsTbl[0][0] * scale),
        pos.y + (configPadSprOfsTbl[0][1] * scale));

    sprId = workP->sprId[1];
    espScaleSet(sprId, scale, scale);
    espPosSet(sprId, pos.x + (configPadSprOfsTbl[1][0] * scale),
        pos.y + (configPadSprOfsTbl[1][1] * scale));

    sprId = workP->sprId[2];
    espScaleSet(sprId, scale, scale);
    espPosSet(sprId, pos.x + (configPadSprOfsTbl[2][0] * scale),
        pos.y + (configPadSprOfsTbl[2][1] * scale));
}

/* ConfigExec calls this after ConfigPadMain reports that the player is leaving controller
* assignment; it shrinks and removes the rows. */
static void ConfigPadClose(void)
{
    CONFIG_MENU_WORK *menuP;
    CONFIG_PAD_WORK *workP;
    HuVecF pos;
    float posStep;
    float weight;
    int value;
    int i;
    int j;
    extern float lbl_802C4CC8;
    extern float lbl_802C4CCC;
    extern float lbl_802C4CD0;
    extern float lbl_802C4CD4;
    extern float lbl_802C4CF4;
    extern float lbl_802C4CF8;
    extern float lbl_802C4D08;
    extern float lbl_802C4D0C;

    menuP = pauseWork.menu;
    if (menuP->panelId != 0) {
        value = menuP->menuIndex;
        pos.x = 0.4f * ((float)pauseGridPosTbl[value][0] - 1.5f);
        pos.y = 0.3f
            + (-0.5f * ((float)pauseGridPosTbl[value][1] - 0.5f));
        pos.z = lbl_802C4CB4;
        mbPausePanelSlideSet((s16)menuP->panelId, 20, &pos);
        mbPausePanelSizeSet((s16)menuP->panelId, 20, lbl_802C4CD0);
    }

    pos.x = lbl_802C4CD4;
    pos.y = lbl_802C4CC8;
    pos.z = lbl_802C4CB4;
    weight = pos.x;
    pos.x = lbl_802C4CF4;
    pos.y = lbl_802C4CC8;
    pos.z = lbl_802C4CB4;
    posStep = lbl_802C4CF8 * (pos.x - weight);

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        mbPausePanelShrinkSet(
            (s16)pauseWork.configPad[i].panelId, 20, 0);
    }

    for (i = 0;
         !mbPausePanelFreezeGet((s16)pauseWork.configPad[0].panelId); i++) {
        HuPrcVSleep();
        weight = lbl_802C4D08 * (float)i;
        if (weight > lbl_802C4CCC) {
            weight = lbl_802C4CCC;
        }
        weight = mbCosDeg(lbl_802C4D0C * weight);
        workP = pauseWork.configPad;
        for (j = 0; j < GW_PLAYER_MAX; j++, workP++) {
            ConfigPadTimeSet(workP, weight);
        }
    }

    workP = pauseWork.configPad;
    for (i = 0; i < GW_PLAYER_MAX; i++, workP++) {
        if (workP->panelId != 0) {
            mbPausePanelKill((s16)workP->panelId);
            workP->panelId = 0;
        }
        for (j = 0; j < 3; j++) {
            if (workP->sprId[j] >= 0) {
                espKill(workP->sprId[j]);
                workP->sprId[j] = -1;
            }
        }
    }
}

/* ConfigExec calls this each assignment-screen frame to update rows and detect when the player
 * leaves. */
static BOOL ConfigPadMain(void)
{
    HuVecF pos;
    CONFIG_PAD_WORK *workP;
    s32 cursorPos;
    BOOL controllerDisconnected;
    s32 humanOptionIndex;
    s32 i;
    s32 controllerConnected;
    s32 previousPlayerSlot;
    s32 controllerNo;
    s32 stickDirections;
    BOOL result = FALSE;
    BOOL playerSetupChanged;
    int stickDirectionMaskBySlot[4] = { 2, 3, 3, 1 };
    extern float lbl_802C4CC0;
    extern float lbl_802C4CC4;
    extern float lbl_802C4CC8;

    controllerNo = GwPlayer[pauseWork.playerNo].padNo;
    playerSetupChanged = PausePlayerComRead(FALSE);
    controllerDisconnected = !PausePadCheck(controllerNo);
    if (PauseCursorMove()) {
        if (HuPadBtnDown[controllerNo] & PAD_BUTTON_A) {
            pauseWork.activeF = 12;
        }
        if (controllerDisconnected) {
            pauseWork.activeF = 0;
        }
        if (pauseWork.activeF != 0) {
            pauseWork.activeF--;
        }
        pauseWork.configPadChangeCooldown = 0;
        return FALSE;
    }

    cursorPos = pauseWork.configPadPlayerIndex;
    previousPlayerSlot = cursorPos;
    if (cursorPos < 0) {
        cursorPos = pauseWork.configPadPlayerIndex = 0;
    }
    stickDirections = HuPadDStk[controllerNo] & stickDirectionMaskBySlot[cursorPos];
    if (controllerDisconnected) {
        stickDirections = 0;
    }
    if (stickDirections & PAD_BUTTON_RIGHT) {
        mbAudFXPlay(MSM_SE_CMN_01);
        cursorPos = (cursorPos + 1) & 3;
    } else if (stickDirections & PAD_BUTTON_LEFT) {
        mbAudFXPlay(MSM_SE_CMN_01);
        cursorPos = (cursorPos - 1) & 3;
    }
    pauseWork.configPadPlayerIndex = cursorPos;
    PauseCursorHiliteSet(1, stickDirectionMaskBySlot[cursorPos], HuPadDStk[controllerNo]);

    workP = &pauseWork.configPad[pauseWork.configPadPlayerIndex];
    if (cursorPos != previousPlayerSlot) {
        pos.x = (lbl_802C4CC0 * (float)cursorPos) - lbl_802C4CC4;
        pos.y = lbl_802C4CC8;
        pos.z = 0.0f;
        PauseCursorPosNextSet(12, pos.x, pos.y);
    }
    if (previousPlayerSlot >= 0) {
        /* On a cursor-change update, apply confirmation to the previously highlighted player
         * row. */
        workP = &pauseWork.configPad[previousPlayerSlot];
    }

    if (HuPadBtnDown[controllerNo] & PAD_BUTTON_A) {
        pauseWork.activeF = 8;
    }
    if (controllerDisconnected) {
        pauseWork.activeF = 0;
    }
    if (pauseWork.configPadChangeCooldown == 0 && pauseWork.activeF != 0) {
        humanOptionIndex = GW_PLAYER_COM_DIF_MAX;
        workP->comDif++;
        pauseWork.configPadChangeCooldown = 16;
        pauseWork.activeF = 0;
        mbAudFXPlay(MSM_SE_CMN_02);
        /* Skip Very Hard without bank flag 3; an unavailable controller makes the human choice fall
         * back to Easy. */
        if (!GWBankFlagGet(3) && workP->comDif == humanOptionIndex - 1) {
            workP->comDif++;
        }
        if (workP->comDif > humanOptionIndex) {
            workP->comDif = 0;
        }
        if (workP->comDif == humanOptionIndex) {
            if (PausePadCheck(workP->padNo)) {
                GwPlayer[workP->playerNo].comF = FALSE;
                GwPlayerConf[workP->playerNo].type = FALSE;
            } else {
                workP->comDif = 0;
            }
        }
        if (workP->comDif < humanOptionIndex) {
            GwPlayer[workP->playerNo].comF = TRUE;
            GwPlayerConf[workP->playerNo].type = TRUE;
            GwPlayer[workP->playerNo].comDif = workP->comDif;
                GwPlayerConf[workP->playerNo].comDif = workP->comDif;
        }
        playerSetupChanged = TRUE;
        ConfigPadSprSet(workP);
        mbPausePanelAnmNoSet((s16)workP->panelId, 12, 0);
    }

    /* A lost controller forces CPU control; a newly available controller forces human control. */
    workP = pauseWork.configPad;
    for (i = 0; i < GW_PLAYER_MAX; i++, workP++) {
        controllerConnected = PausePadCheck(workP->padNo);
        if (!controllerConnected && workP->activeF) {
            GwPlayer[workP->playerNo].comF = TRUE;
            GwPlayerConf[workP->playerNo].type = TRUE;
            workP->comDif = GwPlayer[workP->playerNo].comDif;
            ConfigPadSprSet(workP);
        } else if (controllerConnected && !workP->activeF) {
            workP->comDif = GW_PLAYER_COM_DIF_MAX;
            GwPlayer[workP->playerNo].comF = FALSE;
            GwPlayerConf[workP->playerNo].type = FALSE;
            ConfigPadSprSet(workP);
        }
        workP->activeF = controllerConnected;
    }

    if (playerSetupChanged) {
        ConfigWinUpdate(pauseWork.cursorPos);
    }
    if ((HuPadBtnDown[controllerNo] & PAD_BUTTON_B) && !controllerDisconnected) {
        result = TRUE;
    }
    if (pauseWork.configPadChangeCooldown != 0) {
        pauseWork.configPadChangeCooldown--;
    }
    if (pauseWork.activeF != 0) {
        pauseWork.activeF--;
    }
    return result;
}

/* ConfigOpen creates four directional cursor sprites and their highlights before menu input. */
static void PauseCursorCreate(void)
{
    PAUSE_CURSOR_WORK *cursorP;
    int i;
    int sprId;

    cursorP = &pauseWork.cursor;
    {
        s32 pauseCursorBankTbl[4] = { 1, 3, 2, 0 };

        cursorP->activeF = FALSE;
        cursorP->mask = 0;
        cursorP->directionMask = 0;
        cursorP->cursorNo = -1;
        cursorP->moveTime = -1;
        cursorP->maxMoveTime = -1;
        cursorP->pos.x = cursorP->pos.y = cursorP->pos.z = lbl_802C4CB4;
        for (i = 0; i < 4; i++) {
            cursorP->alpha[i] = lbl_802C4CB4;
            cursorP->sprId[i] = sprId = espEntry(
                mbBoardDataNumGet(DATANUM(DATA_bpause6, 32)), 90,
                (s16)(pauseCursorBankTbl[i] + 4));
            espAttrSet(sprId, HUSPR_ATTR_LINEAR);
            espPosSet(sprId, cursorP->pos.x, cursorP->pos.y);
            espDispOff(sprId);
            cursorP->hiliteSprId[i] = sprId = espEntry(
                mbBoardDataNumGet(DATANUM(DATA_bpause6, 32)), 89,
                (s16)pauseCursorBankTbl[i]);
            espAttrSet(sprId, HUSPR_ATTR_LINEAR);
            espPosSet(sprId, cursorP->pos.x, cursorP->pos.y);
            espDispOff(sprId);
        }
    }
}

/* ConfigMain removes the cursor sprites after the panels finish closing. */
static void PauseCursorKill(void)
{
    PAUSE_CURSOR_WORK *cursorP;
    int i;

    cursorP = &pauseWork.cursor;
    for (i = 0; i < 4; i++) {
        if (cursorP->sprId[i] >= 0) {
            espKill(cursorP->sprId[i]);
        }
        if (cursorP->hiliteSprId[i] >= 0) {
            espKill(cursorP->hiliteSprId[i]);
        }
        cursorP->sprId[i] = cursorP->hiliteSprId[i] = -1;
    }
}

/* PauseCursorHiliteSet updates cursor highlight directions from the current stick input. */
static void PauseCursorHiliteSet(s32 cursorNo, s32 directionMask, s32 mask)
{
    PAUSE_CURSOR_WORK *cursorP;
    int i;

    cursorP = &pauseWork.cursor;
    cursorP->activeF = FALSE;
    if (cursorNo >= 0) {
        cursorP->activeF = TRUE;
    }
    if (cursorP->cursorNo < 0 && cursorNo >= 0) {
        cursorP->moveTime = -1;
        cursorP->maxMoveTime = -1;
    }
    cursorP->mask = mask;
    cursorP->directionMask = directionMask;
    cursorP->cursorNo = cursorNo;
    for (i = 0; i < 4; i++) {
        espDispOff(cursorP->hiliteSprId[i]);
        if (cursorP->activeF) {
            espDispOn(cursorP->sprId[i]);
            if (mask & pauseCursorMaskTbl[i]) {
                espDispOn(cursorP->hiliteSprId[i]);
            }
        } else {
            espDispOff(cursorP->sprId[i]);
        }
    }
}

/* Snaps to the destination before cursor movement is initialized or when time is
* nonpositive; otherwise starts a timed move. */
static void PauseCursorPosNextSet(s32 time, float x, float y)
{
    PAUSE_CURSOR_WORK *cursorP;
    HuVecF pos2D;
    int i;
    extern float lbl_802C4CCC;

    cursorP = &pauseWork.cursor;
    if (cursorP->moveTime < 0 || time <= 0) {
        cursorP->moveTime = 0;
        cursorP->maxMoveTime = 0;
        cursorP->pos.x = x;
        cursorP->pos.y = y;
        for (i = 0; i < 4; i++) {
            cursorP->alpha[i] = lbl_802C4CB4;
            if (cursorP->directionMask & pauseCursorMaskTbl[i]) {
                cursorP->alpha[i] = lbl_802C4CCC;
            }
        }
    } else {
        cursorP->maxMoveTime = time;
        cursorP->moveTime = time;
        ConfigVecCopy(&cursorP->pos, &cursorP->posStart);
        cursorP->posDelta.x = x - cursorP->posStart.x;
        cursorP->posDelta.y = y - cursorP->posStart.y;
        cursorP->posDelta.z = lbl_802C4CB4;
    }
    mbNormPosto2D(&cursorP->pos, &pos2D);
    for (i = 0; i < 4; i++) {
        espPosSet(cursorP->sprId[i],
            pos2D.x + pauseCursorSprOfsTbl[cursorP->cursorNo][i][0],
            pos2D.y + pauseCursorSprOfsTbl[cursorP->cursorNo][i][1]);
        espTPLvlSet(cursorP->sprId[i], cursorP->alpha[i]);
        espPosSet(cursorP->hiliteSprId[i],
            pos2D.x + pauseCursorSprOfsTbl[cursorP->cursorNo][i][0],
            pos2D.y + pauseCursorSprOfsTbl[cursorP->cursorNo][i][1]);
        espTPLvlSet(cursorP->hiliteSprId[i], cursorP->alpha[i]);
    }
}

/* ConfigExec and ConfigPadMain call this each input frame to advance cursor movement. */
static int PauseCursorMove(void)
{
    extern float lbl_802C4CCC;
    extern float lbl_802C4D0C;
    extern float lbl_802C4D10;
    PAUSE_CURSOR_WORK *cursorP;
    HuVecF pos;
    float moveWeight;
    int directionIndex;

    cursorP = &pauseWork.cursor;
    if (cursorP->moveTime <= 0) {
        return 0;
    }
    cursorP->moveTime--;
    moveWeight = (float)(cursorP->maxMoveTime - cursorP->moveTime)
        / (float)cursorP->maxMoveTime;
    moveWeight = mbSinDeg(lbl_802C4D0C * moveWeight);
    PSVECScale(&cursorP->posDelta, &pos, moveWeight);
    PSVECAdd(&cursorP->posStart, &pos, &cursorP->pos);
    for (directionIndex = 0; directionIndex < 4; directionIndex++) {
        if (cursorP->directionMask & pauseCursorMaskTbl[directionIndex]) {
            if (cursorP->alpha[directionIndex] < lbl_802C4CCC) {
                cursorP->alpha[directionIndex] += lbl_802C4D10;
            }
            if (cursorP->alpha[directionIndex] > lbl_802C4CCC) {
                cursorP->alpha[directionIndex] = lbl_802C4CCC;
            }
        } else {
            if (cursorP->alpha[directionIndex] > lbl_802C4CB4) {
                cursorP->alpha[directionIndex] -= lbl_802C4D10;
            }
            if (cursorP->alpha[directionIndex] < lbl_802C4CB4) {
                cursorP->alpha[directionIndex] = lbl_802C4CB4;
            }
        }
    }
    mbNormPosto2D(&cursorP->pos, &pos);
    for (directionIndex = 0; directionIndex < 4; directionIndex++) {
        espPosSet(cursorP->sprId[directionIndex],
            pos.x + pauseCursorSprOfsTbl[cursorP->cursorNo][directionIndex][0],
            pos.y + pauseCursorSprOfsTbl[cursorP->cursorNo][directionIndex][1]);
        espTPLvlSet(cursorP->sprId[directionIndex], cursorP->alpha[directionIndex]);
        espPosSet(cursorP->hiliteSprId[directionIndex],
            pos.x + pauseCursorSprOfsTbl[cursorP->cursorNo][directionIndex][0],
            pos.y + pauseCursorSprOfsTbl[cursorP->cursorNo][directionIndex][1]);
        espTPLvlSet(cursorP->hiliteSprId[directionIndex], cursorP->alpha[directionIndex]);
    }
    return cursorP->moveTime;
}

/* ConfigMain initializes assignments; ConfigPadOpen and ConfigPadMain refresh connected players. */
static BOOL PausePlayerComRead(s32 initializeAssignments)
{
    extern s16 HuPadStatGet(s16 padNo);
    PAUSE_PAD_WORK *work;
    BOOL changeF;
    s32 i;
    s32 j;
    s32 padValue;

    work = pauseWork.padWork;
    changeF = FALSE;
    if (initializeAssignments) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            work[i].port = i;
            work[i].padNo = GwPlayer[i].padNo;
            work[i].padStatus = 0;
            work[i].statusChangeFrames = FALSE;
        }
        for (i = 0; i < GW_PLAYER_MAX - 1; i++) {
            for (j = i + 1; j < GW_PLAYER_MAX; j++) {
                if (work[i].padNo > work[j].padNo
                    || (work[i].padNo == work[j].padNo
                        && work[i].port > work[j].port)) {
                    padValue = work[i].padNo;
                    work[i].padNo = work[j].padNo;
                    work[j].padNo = padValue;
                    padValue = work[i].port;
                    work[i].port = work[j].port;
                    work[j].port = padValue;
                }
            }
        }
        {
            BOOL partyF;

            partyF = GwSystem.partyF;
            if (partyF) {
                for (i = 0; i < GW_PLAYER_MAX; i++) {
                    /* On entry, any controller error forces its human player to CPU control. */
                    work[i].padStatus = HuPadStatGet((s16)work[i].padNo);
                    if (work[i].padStatus != 0
                        && !GwPlayer[work[i].port].comF) {
                        GwPlayer[work[i].port].comF = TRUE;
                        GwPlayerConf[work[i].port].type = TRUE;
                    }
                }
            }
        }
    }
    {
        BOOL partyF;

        partyF = GwSystem.partyF;
        if (!partyF) {
            return FALSE;
        }
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        padValue = HuPadStatGet((s16)work[i].padNo);
        if (padValue != work[i].padStatus) {
            work[i].statusChangeFrames++;
        } else {
            work[i].statusChangeFrames = 0;
        }
        if (work[i].statusChangeFrames < 4) {
            continue;
        }
        /* After four consecutive reads differing from the cached status, force human control for 0
        * or CPU control for -1; leave other error statuses uncached. */
        if (padValue == 0) {
            GwPlayer[work[i].port].comF = FALSE;
            GwPlayerConf[work[i].port].type = FALSE;
            changeF = TRUE;
            work[i].padStatus = padValue;
        } else if (padValue == -1) {
            GwPlayer[work[i].port].comF = TRUE;
            GwPlayerConf[work[i].port].type = TRUE;
            changeF = TRUE;
            work[i].padStatus = padValue;
        }
    }
    return changeF;
}

/* PausePadCheck rejects only cached status -1; other controller errors still count as available. */
static BOOL PausePadCheck(s32 padNo)
{
    PAUSE_PAD_WORK *work = pauseWork.padWork;

    return work[padNo].padStatus != -1;
}

/* Sets up the guide's background copy, then hides player models that were visible before
 * capture. */
void mbPauseDispCopyCreate(void)
{
    int i;
    int fbSize;

    pauseDispCopyFb = NULL;
    pauseDispCopyCounter = 0;
    fbSize = GXGetTexBufferSize(HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2, GX_TF_RGB565, GX_FALSE, 0);
    /* GWStorySingleCheck is referenced without a call, so this nonnull function pointer makes the
     * branch unreachable. */
    if (!GWStorySingleCheck) {
        pauseDispCopyFb = mbMallocFlush(fbSize);
    } else {
        pauseDispCopyFb = mbMallocFlushModel(fbSize);
    }
    pauseDispCopyModelId = Hu3DHookFuncCreate(PauseDispCopyDraw);
    Hu3DModelCameraSet(pauseDispCopyModelId, 4);
    Hu3DModelLayerSet(pauseDispCopyModelId, 2);
    HuPrcVSleep();
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        playerDispF[i] = mbObjDispGet(mbPlayerObjIDGet(i));
        mbObjDispSet(mbPlayerObjIDGet(i), FALSE);
    }
}

/* Frees the saved background image and restores players that were visible before its capture. */
void mbPauseDispCopyKill(void)
{
    int i;

    pauseDispCopyCounter = 0;
    HuMemDirectFree(pauseDispCopyFb);
    pauseDispCopyFb = NULL;
    Hu3DModelKill(pauseDispCopyModelId);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (playerDispF[i]) {
            mbObjDispSet(mbPlayerObjIDGet(i), TRUE);
        }
    }
}

/* The first draw copies the framebuffer; later draws render that saved image behind the guide. */
static void PauseDispCopyDraw(HU3D_MODEL *modelP, Mtx *mtx)
{
    extern float lbl_802C4CCC;
    extern float lbl_802C4CEC;
    extern float lbl_802C4D14;
    extern float lbl_802C4D18;

    if (!pauseDispCopyCounter) {
        GXSetTexCopySrc(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT);
        GXSetTexCopyDst(HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2, GX_TF_RGB565, GX_TRUE);
        GXCopyTex(pauseDispCopyFb, FALSE);
        GXPixModeSync();
        pauseDispCopyCounter++;
    } else {
        Mtx modelview;
        Mtx44 proj;
        GXTexObj texObj;

        MTXOrtho(proj, lbl_802C4CB4, lbl_802C4D14, lbl_802C4CB4,
            lbl_802C4D18, lbl_802C4CB4, lbl_802C4CEC);
        GXSetProjection(proj, GX_ORTHOGRAPHIC);
        GXSetViewport(lbl_802C4CB4, lbl_802C4CB4, lbl_802C4D18,
            lbl_802C4D14, lbl_802C4CB4, lbl_802C4CCC);
        GXSetScissor(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT);
        MTXIdentity(modelview);
        GXLoadPosMtxImm(modelview, GX_PNMTX0);
        GXSetCullMode(GX_CULL_NONE);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_CLAMP, GX_AF_NONE);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetNumTevStages(1);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
        GXInitTexObj(&texObj, pauseDispCopyFb, HU_FB_WIDTH / 2, HU_FB_HEIGHT / 2, GX_TF_RGB565,
                     GX_CLAMP, GX_CLAMP, GX_TRUE);
        GXInitTexObjLOD(&texObj, GX_LINEAR, GX_LINEAR, lbl_802C4CB4,
            lbl_802C4CB4, lbl_802C4CB4, GX_FALSE, GX_FALSE, GX_ANISO_1);
        GXLoadTexObj(&texObj, GX_TEXMAP0);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3s16(0, 0, -50);
        GXTexCoord2s16(0, 0);
        GXPosition3s16(HU_FB_WIDTH, 0, -50);
        GXTexCoord2s16(256, 0);
        GXPosition3s16(HU_FB_WIDTH, HU_FB_HEIGHT, -50);
        GXTexCoord2s16(256, 256);
        GXPosition3s16(0, HU_FB_HEIGHT, -50);
        GXTexCoord2s16(0, 256);
        GXEnd();
    }
}

/* The board pause process allocates panel state and starts its per-frame animator on entry. */
void mbPauseGuideCreate(void)
{
    pausePanelWork = mbMalloc(sizeof(PAUSE_PANEL_WORK) * 20);
    pauseGuideKillF = FALSE;
    pauseGuideProc = HuPrcChildCreate(PauseGuideMain, 8209, 8192, 0,
        mbMainProc);
    HuPrcDestructorSet2(pauseGuideProc, PauseGuideDestroy);
    HuPrcSetStat(pauseGuideProc,
        HU_PRC_STAT_PAUSE_ON | HU_PRC_STAT_UPAUSE_ON);
}

/* The board pause process requests that the panel animator stop on its next update when closing. */
void mbPauseGuideKill(void)
{
    pauseGuideKillF = TRUE;
}

/* The process started by mbPauseGuideCreate updates panel movement, flips, and labels each
 * frame until mbPauseGuideKill requests its exit. */
static void PauseGuideMain(void)
{
    int slotIndex;
    BOOL isForegroundPanel;
    HU3D_CAMERA *pauseCamera;
    PAUSE_PANEL_WORK *panelWork;
    BOOL motionComplete;
    float animationProgress;
    float scaleFactor;
    float panelScale;
    float flipProgress;
    HuVecF transformVector;
    HuVecF panelRotation;
    Mtx facingMatrix;
    Mtx rotationMatrix;
    Mtx inverseRotationMatrix;
    extern float lbl_802C4CCC;
    extern float lbl_802C4CD0;
    extern float lbl_802C4CEC;
    extern float lbl_802C4CF0;
    extern float lbl_802C4D0C;
    extern float lbl_802C4D1C;
    extern float lbl_802C4D20;
    extern float lbl_802C4D24;
    extern float lbl_802C4D28;
    extern float lbl_802C4D2C;
    extern float lbl_802C4D30;

    for (slotIndex = 0; slotIndex < HU3D_CAM_MAX; slotIndex++) {
        if ((1 << slotIndex) & HU3D_CAM2) {
            break;
        }
    }
    pauseCamera = &Hu3DCamera[slotIndex];
    while (!pauseGuideKillF) {
        panelWork = pausePanelWork;
        for (slotIndex = 0; slotIndex < 20; slotIndex++, panelWork++) {
            if (panelWork->modelId == 0) {
                continue;
            }
            motionComplete = FALSE;
            panelWork->time++;
            if (panelWork->delay != 0) {
                animationProgress = lbl_802C4CB4;
                if (panelWork->time >= panelWork->delay) {
                    panelWork->delay = panelWork->time = 0;
                    if (panelWork->motion == 4) {
                        mbAudFXPlay(MSM_SE_CMN_53);
                    } else if (panelWork->motion == 5) {
                        mbAudFXPlay(MSM_SE_CMN_54);
                    }
                }
            } else {
                animationProgress = lbl_802C4CCC;
                if (panelWork->time < panelWork->maxTime && panelWork->maxTime > 0) {
                    animationProgress = panelWork->time / (float)panelWork->maxTime;
                } else {
                    panelWork->time = panelWork->maxTime = 0;
                    motionComplete = TRUE;
                }
                /* Entrance and exit spins also control the panel and label's visibility. */
                switch (panelWork->motion) {
                    case 0:
                    case 1:
                    case 2:
                        break;

                    case 4:
                        mbObjRotGet(panelWork->modelId, &transformVector);
                        transformVector.y = lbl_802C4D1C * (lbl_802C4CCC - animationProgress);
                        if (motionComplete) {
                            transformVector.y = lbl_802C4CB4;
                            if (panelWork->batsuModelId != 0 && panelWork->batsuF) {
                                mbObjDispSet(panelWork->batsuModelId, TRUE);
                            }
                        }
                        mbObjRotSetV(panelWork->modelId, &transformVector);
                        mbObjDispSet(panelWork->modelId, TRUE);
                        if (panelWork->sprId >= 0) {
                            espDispOn(panelWork->sprId);
                        }
                        break;

                    case 5:
                        mbObjRotGet(panelWork->modelId, &transformVector);
                        transformVector.y = lbl_802C4D20 * animationProgress;
                        if (panelWork->batsuModelId != 0) {
                            mbObjDispSet(panelWork->batsuModelId, FALSE);
                        }
                        if (motionComplete) {
                            transformVector.y = lbl_802C4CB4;
                            mbObjDispSet(panelWork->modelId, FALSE);
                            if (panelWork->sprId >= 0) {
                                espDispOff(panelWork->sprId);
                            }
                        }
                        panelWork->scale = mbCosDeg(lbl_802C4D0C * animationProgress);
                        panelWork->scaleTarget = panelWork->scaleStart = panelWork->scale;
                        mbObjRotSetV(panelWork->modelId, &transformVector);
                        break;
                }
            }
            /* Keep the disabled-choice mark hidden until the bank-change flip finishes. */
            if (panelWork->animMaxTime != 0) {
                if (panelWork->batsuModelId != 0) {
                    mbObjDispSet(panelWork->batsuModelId, FALSE);
                }
                mbObjRotGet(panelWork->modelId, &transformVector);
                panelWork->animTime++;
                if (panelWork->animTime < panelWork->animMaxTime && panelWork->animMaxTime > 0) {
                    flipProgress = panelWork->animTime / (float)panelWork->animMaxTime;
                    transformVector.y = lbl_802C4CF0 * flipProgress;
                } else {
                    panelWork->animTime = panelWork->animMaxTime = 0;
                    Hu3DAnmNoSet(panelWork->animId[0], panelWork->bank);
                    transformVector.y = lbl_802C4CB4;
                    if (panelWork->batsuModelId != 0 && panelWork->batsuF) {
                        mbObjDispSet(panelWork->batsuModelId, TRUE);
                    }
                }
                mbObjRotSetV(panelWork->modelId, &transformVector);
            }
            if (motionComplete) {
                panelWork->motion = 0;
                ConfigVecCopy(&panelWork->posTarget, &panelWork->pos);
                ConfigVecCopy(&panelWork->posTarget, &panelWork->posStart);
                panelWork->scaleStart = panelWork->scale = panelWork->scaleTarget;
            } else {
                scaleFactor = mbSinDeg(lbl_802C4D0C * animationProgress);
                PSVECSubtract(&panelWork->posTarget, &panelWork->posStart, &transformVector);
                PSVECScale(&transformVector, &transformVector, scaleFactor);
                PSVECAdd(&panelWork->posStart, &transformVector, &panelWork->pos);
                panelWork->scale = panelWork->scaleStart
                    + (scaleFactor * (panelWork->scaleTarget - panelWork->scaleStart));
            }
            isForegroundPanel = FALSE;
            if (panelWork->scaleTarget >= lbl_802C4CD0) {
                isForegroundPanel = TRUE;
            }
            mbNormPosto3D(&panelWork->pos, HU3D_CAM2, &transformVector);
            mbObjPosSetV(panelWork->modelId, &transformVector);
            if (panelWork->batsuModelId != 0) {
                mbObjPosSetV(panelWork->batsuModelId, &transformVector);
            }
            if (panelWork->sprId >= 0) {
                transformVector.y -= lbl_802C4CEC
                    * (lbl_802C4CA8 * (panelWork->scale * panelWork->scaleBase));
                mbPos3DtoNorm(&transformVector, HU3D_CAM2, &transformVector);
                mbNormPosto2D(&transformVector, &transformVector);
                espPosSet(panelWork->sprId, transformVector.x, transformVector.y);
                if (isForegroundPanel) {
                    espPriSet(panelWork->sprId, 99);
                } else {
                    espPriSet(panelWork->sprId, 100);
                }
            }
            panelScale = panelWork->scale * panelWork->scaleBase;
            mbObjScaleSet(panelWork->modelId, lbl_802C4D24 * panelScale,
                lbl_802C4D24 * panelScale, lbl_802C4D24 * panelScale);
            if (panelWork->batsuModelId != 0) {
                mbObjScaleSet(panelWork->batsuModelId,
                    lbl_802C4D28 * (lbl_802C4D24 * panelScale),
                    lbl_802C4D28 * (lbl_802C4D24 * panelScale), lbl_802C4D24 * panelScale);
            }
            if (panelWork->sprId >= 0) {
                float labelBaseScale;

                labelBaseScale = lbl_802C4D2C * panelScale;
                if (panelWork->animMaxTime != 0) {
                    /* Collapse and darken the label at mid-flip, then reveal its new bank. */
                    animationProgress = panelWork->animTime / (float)panelWork->animMaxTime;
                    scaleFactor = ConfigAbsFloat(mbCosDeg(lbl_802C4CF0 * animationProgress));
                    espScaleSet(panelWork->sprId, labelBaseScale, labelBaseScale * scaleFactor);
                    if (animationProgress >= lbl_802C4CB0) {
                        espBankSet(panelWork->sprId, panelWork->bank);
                    }
                    scaleFactor = lbl_802C4D30 * scaleFactor;
                    espColorSet(panelWork->sprId, scaleFactor, scaleFactor, scaleFactor);
                } else {
                    espScaleSet(panelWork->sprId, labelBaseScale, labelBaseScale);
                    espColorSet(panelWork->sprId, 255, 255, 255);
                }
            }
            /* Face the panel toward the pause camera while retaining its animated rotation. */
            mbObjRotGet(panelWork->modelId, &panelRotation);
            mtxRot(rotationMatrix, panelRotation.x, panelRotation.y, panelRotation.z);
            PSMTXInverse(rotationMatrix, inverseRotationMatrix);
            mbObjPosGet(panelWork->modelId, &transformVector);
            C_MTXLookAt(facingMatrix, &pauseCamera->pos, &pauseCamera->up, &transformVector);
            facingMatrix[0][3] = facingMatrix[1][3] = facingMatrix[2][3] = lbl_802C4CB4;
            PSMTXInverse(facingMatrix, facingMatrix);
            mbObjMtxSet(panelWork->batsuModelId, &facingMatrix);
            PSMTXConcat(inverseRotationMatrix, facingMatrix, facingMatrix);
            PSMTXConcat(facingMatrix, rotationMatrix, facingMatrix);
            mbObjMtxSet(panelWork->modelId, &facingMatrix);
        }
        HuPrcVSleep();
    }
    HuPrcEnd();
}

/* The panel animator's destructor frees its work array when the process ends; callers release
 * panel models and sprites separately. */
static void PauseGuideDestroy(void)
{
    int i;
    PAUSE_PANEL_WORK *work;

    if (pausePanelWork) {
        /* This loop performs no per-panel cleanup. */
        for (i = 0; i < 20; i++) {
        }
        work = pausePanelWork;
        HuMemDirectFree(work);
        pausePanelWork = NULL;
    }
    pauseGuideProc = NULL;
}

/* ConfigOpen and the controller menu build panels with a shared model and supplied texture data. */
s16 mbPausePanelCreate(int dataNum, unsigned int espDataNum)
{
    extern float lbl_802C4CCC;
    extern float lbl_802C4D1C;
    int i;
    PAUSE_PANEL_WORK *work;
    int panelId;

    for (panelId = 1; panelId < 20; panelId++) {
        if (pausePanelWork[panelId].modelId <= 0) {
            break;
        }
    }
    work = &pausePanelWork[panelId];
    memset(work, 0, sizeof(PAUSE_PANEL_WORK));
    work->scale = work->scaleStart = work->scaleTarget = work->scaleBase = lbl_802C4CCC;
    work->pos.z = work->posStart.z = work->posTarget.z = lbl_802C4D1C;
    work->modelId = mbObjCreate(mbBoardDataNumGet(DATANUM(DATA_bpause6, 37)),
        NULL, FALSE);
    mbObjCameraSet(work->modelId, 4);
    mbObjLayerSet(work->modelId, 4);
    {
        MBMODELID modelId = work->modelId;

        mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    }
    {
        MBMODELID modelId = work->modelId;

        mbObjAttrSet(modelId, HU3D_ATTR_NOPAUSE);
    }
    mbObjDispSet(work->modelId, FALSE);
    work->anim = HuSprAnimRead(HuDataSelHeapReadNum(
        mbBoardDataNumGet(dataNum), HU_MEMNUM_OVL, HEAP_MODEL));
    for (i = 0; i < 2; i++) {
        if (i == 0) {
            work->animId[0] = Hu3DAnimCreate(work->anim,
                mbObjModelIDGet(work->modelId), "S3TCys77120");
        } else {
            work->animId[i] = Hu3DAnimLink(work->animId[0],
                mbObjModelIDGet(work->modelId), "S3TCys77121");
        }
        Hu3DAnmNoSet(work->animId[i], 0);
    }
    work->batsuModelId = mbObjCreate(
        mbBoardDataNumGet(DATANUM(DATA_bpause6, 36)), NULL, TRUE);
    mbObjCameraSet(work->batsuModelId, 4);
    mbObjLayerSet(work->batsuModelId, 4);
    {
        MBMODELID modelId = work->batsuModelId;

        mbObjAttrSet(modelId, HU3D_ATTR_NOPAUSE);
    }
    mbObjDispSet(work->batsuModelId, FALSE);
    work->sprId = -1;
    if (espDataNum != 0) {
        work->sprId = espEntry(mbBoardDataNumGet(espDataNum), 100, 0);
        espAttrSet(work->sprId, HUSPR_ATTR_LINEAR);
        espDispOff(work->sprId);
    }
    return panelId;
}

/* mbPausePanelKill releases the models, animation tracks, and optional label for a panel. */
void mbPausePanelKill(s16 panelId)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];
    int i;

    for (i = 0; i < 2; i++) {
        if (work->animId[i] >= 0) {
            Hu3DAnimKill(work->animId[i]);
        }
        work->animId[i] = -1;
    }
    if (work->anim) {
        HuSprAnimKill(work->anim);
    }
    work->anim = NULL;
    if (work->modelId != 0) {
        mbObjKill(work->modelId);
        work->modelId = 0;
    }
    if (work->batsuModelId != 0) {
        mbObjKill(work->batsuModelId);
        work->batsuModelId = 0;
    }
    if (work->sprId >= 0) {
        espKill(work->sprId);
        work->sprId = -1;
    }
}

/* ConfigOpen, ConfigExec, and ConfigClose use this to place a panel and reset its slide
 * endpoints. */
void mbPausePanelPosSet(s16 panelId, float panelX, float panelY)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->pos.x = work->posStart.x = work->posTarget.x = panelX;
    work->pos.y = work->posStart.y = work->posTarget.y = panelY;
}

/* ConfigPadTimeSet reads the panel's current X/Y for its icons; returned Z is always zero. */
void mbPausePanelPosGet(s16 panelId, HuVecF *posOut)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    posOut->x = work->pos.x;
    posOut->y = work->pos.y;
    posOut->z = lbl_802C4CB4;
}

/* Applies a rotation to a pause panel model when another board UI caller changes its
 * orientation. */
void mbPausePanelRotSet(s16 panelId, float rotX, float rotY, float rotZ)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    mbObjRotSet(work->modelId, rotX, rotY, rotZ);
}

/* Sets the current scale and both animation endpoints when a caller resets a panel's size. */
void mbPausePanelScaleSet(s16 panelId, float scale)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->scale = work->scaleStart = work->scaleTarget = scale;
}

/* ConfigPadTimeSet uses the current panel scale to keep its controller icons aligned. */
float mbPausePanelScaleGet(s16 panelId)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    return work->scale;
}

/* ConfigOpen and rebuilds select a texture-animation frame on both model tracks and the same
 * numbered bank on the optional label sprite. */
void mbPausePanelBankSet(s16 panelId, int animBank)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->bank = animBank;
    Hu3DAnmNoSet(work->animId[0], animBank);
    Hu3DAnmNoSet(work->animId[1], animBank);
    if (work->sprId >= 0) {
        espBankSet(work->sprId, animBank);
    }
}

/* ConfigOpen and ConfigExec mark choices that are unavailable or disabled for the current mode. */
void mbPausePanelBatsuSet(s16 panelId, BOOL batsuF)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->batsuF = batsuF;
}

/* ConfigGrowWait and the close/rebuild loops poll this until panel movement and flips are idle. */
BOOL mbPausePanelFreezeGet(s16 panelId)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];
    BOOL freezeF = FALSE;

    if (work->motion == 0 && work->animMaxTime == 0) {
        freezeF = TRUE;
    }
    return freezeF;
}

/* Resets an interrupted panel motion to its current position and scale for the next transition. */
void mbPausePanelUnlockSet(s16 panelId)
{
    PAUSE_PANEL_WORK *panelWork;

    panelWork = &pausePanelWork[panelId];
    panelWork->motion = 0;
    ConfigVecCopy(&panelWork->pos, &panelWork->posStart);
    ConfigVecCopy(&panelWork->pos, &panelWork->posTarget);
    panelWork->scaleTarget = panelWork->scaleStart = panelWork->scale;
    panelWork->time = panelWork->maxTime = 0;
}

/* ConfigExec slides the first panel while opening controller assignment; the target depth is
 * fixed. */
void mbPausePanelSlideSet(s16 panelId, int durationFrames, HuVecF *targetPos)
{
    extern float lbl_802C4D1C;
    PAUSE_PANEL_WORK *workP;

    workP = &pausePanelWork[panelId];
    workP->motion = 1;
    workP->maxTime = durationFrames;
    workP->time = 0;
    ConfigVecCopy(targetPos, &workP->posTarget);
    workP->posTarget.z = lbl_802C4D1C;
    ConfigVecCopy(&workP->pos, &workP->posStart);
}

/* ConfigExec animates the highlighted and unhighlighted menu panels to their requested scales. */
void mbPausePanelSizeSet(s16 panelId, int durationFrames, float scale)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->motion = 2;
    work->maxTime = durationFrames;
    work->time = 0;
    work->scaleTarget = scale;
    work->scaleStart = work->scale;
}

/* ConfigExec and ConfigPadMain begin a panel flip when its option or controller setting changes. */
void mbPausePanelAnmNoSet(s16 panelId, s32 animMaxTime, s32 bank)
{
    PAUSE_PANEL_WORK *workP;

    workP = &pausePanelWork[panelId];
    workP->animMaxTime = (s16)animMaxTime;
    workP->animTime = 0;
    Hu3DAnmNoSet(workP->animId[0], (u16)workP->bank);
    workP->bank = (s16)bank;
    Hu3DAnmNoSet(workP->animId[1], (u16)bank);
    /* The label changes halfway through the flip in PauseGuideMain; this test has no effect. */
    workP->sprId >= 0;
}

/* Queues the panel entrance with a delay + 1 counter; movement starts on the update after
 * that delay is cleared. */
void mbPausePanelGrowSet(s16 panelId, int time, int delay, float scale)
{
    extern float lbl_802C4CCC;
    extern float lbl_802C4D34;
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->motion = 4;
    work->maxTime = time;
    work->delay = delay + 1;
    work->time = 0;
    work->scaleTarget = lbl_802C4CCC;
    work->scaleStart = work->scale = lbl_802C4D34;
    work->scaleBase = scale;
    mbObjDispSet(work->modelId, FALSE);
    if (work->batsuModelId != 0) {
        mbObjDispSet(work->batsuModelId, FALSE);
    }
}

/* Queues the panel exit with a delay + 1 counter; movement starts on the update after
 * that delay is cleared. */
void mbPausePanelShrinkSet(s16 panelId, int time, int delay)
{
    PAUSE_PANEL_WORK *work = &pausePanelWork[panelId];

    work->motion = 5;
    work->maxTime = time;
    work->delay = delay + 1;
    work->time = 0;
}

/* ConfigOpen, ConfigExec, and ConfigClose move the guide between board and pause-menu positions.
 * Short horizontal moves ease into place; longer ones play its walking motion and turn. */
void mbPauseGuideMoveSet(MBMODELID modelId, s32 time, HuVecF *posP,
    HuVecF *posNormP)
{
    HuVecF targetPos;
    HuVecF modelPos;
    float weight;
    float rotY;
    s32 i;
    extern float lbl_802C4CCC;
    extern float lbl_802C4CD0;
    extern float lbl_802C4CDC;
    extern float lbl_802C4CE4;
    extern float lbl_802C4D0C;
    extern float lbl_802C4D38;
    extern double lbl_802C4D40;
    extern float lbl_802C4D48;
    extern float lbl_802C4D4C;

    if (posP == NULL) {
        mbNormPosto3D(posNormP, HU3D_CAM2, &targetPos);
    } else {
        ConfigVecCopy(posP, &targetPos);
    }
    mbObjPosGet(modelId, &modelPos);
    /* Only the horizontal distance, truncated to whole units, chooses the movement style. */
    if (abs((int)(targetPos.x - modelPos.x)) < lbl_802C4D38) {
        for (i = 0; i <= time; i++) {
            weight = (float)i / (float)time;
            weight = mbSinDeg(lbl_802C4D0C * weight);
            mbObjPosSet(modelId,
                modelPos.x + weight * (targetPos.x - modelPos.x),
                modelPos.y + weight * (targetPos.y - modelPos.y),
                modelPos.z + weight * (targetPos.z - modelPos.z));
            HuPrcVSleep();
        }
        return;
    }
    /* Walking moves use 1.5 times the requested duration, truncated to whole frames. */
    time = (s32)(time * lbl_802C4D40);
    rotY = lbl_802C4D0C;
    if (targetPos.x < modelPos.x) {
        rotY = lbl_802C4D48;
    }
    mbObjMotionShiftSet(modelId, 3, lbl_802C4CB4, lbl_802C4CDC,
        HU3D_MOTATTR_LOOP);
    mbObjMotionSpeedSet(modelId, lbl_802C4CD0);
    for (i = 0; i <= time; i++) {
        weight = (float)i / (float)time;
        mbObjPosSet(modelId,
            modelPos.x + weight * (targetPos.x - modelPos.x),
            modelPos.y + weight * (targetPos.y - modelPos.y),
            modelPos.z + weight * (targetPos.z - modelPos.z));
        weight = lbl_802C4D4C * (float)i / (float)time;
        if (weight > lbl_802C4CCC) {
            weight = lbl_802C4CCC;
        }
        weight = mbSinDeg(lbl_802C4D0C * weight);
        mbObjRotSet(modelId, lbl_802C4CB4, rotY * weight, lbl_802C4CB4);
        HuPrcVSleep();
    }
    mbObjMotionShiftSet(modelId, 1, lbl_802C4CB4, lbl_802C4CE4,
        HU3D_MOTATTR_LOOP);
    for (i = 0; i <= 12; i++) {
        weight = (float)i / lbl_802C4CE4;
        if (weight > lbl_802C4CCC) {
            weight = lbl_802C4CCC;
        }
        weight = lbl_802C4CCC - mbSinDeg(lbl_802C4D0C * weight);
        mbObjRotSet(modelId, lbl_802C4CB4, rotY * weight, lbl_802C4CB4);
        HuPrcVSleep();
    }
    mbObjRotSet(modelId, lbl_802C4CB4, lbl_802C4CB4, lbl_802C4CB4);
}

/* mbPauseGuideTalkSet starts or stops the guide's talking motion around menu dialogue. */
void mbPauseGuideTalkSet(MBMODELID modelId)
{
    extern float lbl_802C4CDC;
    extern float lbl_802C4D50;
    extern float lbl_802C4D54;

    /* Restore one countdown step so a forced talk stop reaches idle even during a motion blend. */
    if (pauseWork.talkTime == 0 && pauseWork.prevTalkTime > 0) {
        pauseWork.talkTime = 1;
    } else if (mbObjMotionShiftIDGet(modelId) != -1) {
        return;
    }
    if (pauseWork.prevTalkTime == 0 && pauseWork.talkTime > 0) {
        mbObjMotionShiftSet(modelId, 12, lbl_802C4CB4, lbl_802C4CDC,
            HU3D_MOTATTR_LOOP);
        mbObjMotionSpeedSet(modelId, lbl_802C4D50);
    }
    if (pauseWork.talkTime != 0) {
        pauseWork.talkTime--;
        if (pauseWork.talkTime == 0) {
            mbObjMotionShiftSet(modelId, 1, lbl_802C4CB4, lbl_802C4D54,
                HU3D_MOTATTR_LOOP);
        }
    }
    pauseWork.prevTalkTime = pauseWork.talkTime;
}

/* Board choices gate the controller-setup menu entry here: zero disables it, nonzero permits it
 * in party mode. */
void mbConfigPadDisableSet(BOOL enabledF)
{
    configPadEnabled = enabledF;
}

/* Returns whether the game is in story single-player mode. */
static BOOL GWStorySingleCheck(void)
{
    return !GWPartyGet();
}

const float lbl_802C4CC0 = 0.31f;
const float lbl_802C4CC4 = 0.225f;
const float lbl_802C4CC8 = 0.15f;
const float lbl_802C4CCC = 1.0f;
const float lbl_802C4CD0 = 1.2f;
const float lbl_802C4CD4 = -0.225f;
const float lbl_802C4CD8 = -0.425f;
const float lbl_802C4CDC = 8.0f;
const float lbl_802C4CE0 = 400.0f;
const float lbl_802C4CE4 = 12.0f;
const float lbl_802C4CE8 = 0.033333335f;
const float lbl_802C4CEC = 100.0f;
const float lbl_802C4CF0 = 180.0f;
const float lbl_802C4CF4 = 0.395f;
const float lbl_802C4CF8 = 0.33333334f;
const float lbl_802C4CFC = 0.95f;
const float lbl_802C4D00 = -22.0f;
const float lbl_802C4D04 = -30.0f;
const float lbl_802C4D08 = 0.083333336f;
const float lbl_802C4D0C = 90.0f;
const float lbl_802C4D10 = 0.1f;
const float lbl_802C4D14 = 480.0f;
const float lbl_802C4D18 = 640.0f;
const float lbl_802C4D1C = -500.0f;
const float lbl_802C4D20 = 500.0f;
const float lbl_802C4D24 = 0.75f;
const float lbl_802C4D28 = 0.8f;
const float lbl_802C4D2C = 0.85f;
const float lbl_802C4D30 = 255.0f;
const float lbl_802C4D34 = 0.00001f;
const float lbl_802C4D38 = 200.0f;
const double lbl_802C4D40 = 1.5;
const float lbl_802C4D48 = -90.0f;
const float lbl_802C4D4C = 3.0f;
const float lbl_802C4D50 = 2.0f;
const float lbl_802C4D54 = 16.0f;
