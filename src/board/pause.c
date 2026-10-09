/* Board pause menu, its single-player minigame list, and pause-state controls. */
#define _MATH_H
#define M_PI 3.141592653589793
double sin(double);
double cos(double);

#include "game/board/pause.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/effect.h"
#include "game/board/guide.h"
#include "game/board/player.h"
#include "game/board/window.h"

#include "game/esprite.h"
#include "game/sprite.h"
#include "game/hu3d.h"
#include "game/process.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/mgdata.h"
#include "game/pad.h"
#include "game/flag.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/audio.h"

#include "humath.h"
#include "string.h"
#include "messdir_enum.h"

#define PAUSE_HOOK_MAX 32
#define MES_BPAUSE6_SINGLE_HELP MESSNUM(MESS_BOARD_OPE, 8)
#define MES_BPAUSE6_MULTIPLAYER_HELP MESSNUM(MESS_BOARD_PAUSE, 24)
#define MES_BPAUSE6_SINGLE_MG_HELP MESSNUM(MESS_BOARD_PAUSE, 39)
#define MES_BPAUSE6_MG_LIST_HELP MESSNUM(MESS_BOARD_PAUSE, 26)
#define MES_BPAUSE6_MG_TITLE_0 MESSNUM(MESS_BOARD_PAUSE, 31)
#define MES_BPAUSE6_MG_TITLE_1 MESSNUM(MESS_BOARD_PAUSE, 32)
#define MES_BPAUSE6_MG_TITLE_2 MESSNUM(MESS_BOARD_PAUSE, 33)
#define MES_BPAUSE6_MG_TITLE_3 MESSNUM(MESS_BOARD_PAUSE, 34)
#define MES_BPAUSE6_MG_TITLE_4 MESSNUM(MESS_BOARD_PAUSE, 35)
#define MES_BPAUSE6_MG_TITLE_5 MESSNUM(MESS_BOARD_PAUSE, 38)
#define MES_BPAUSE6_MG_ROW_STATUS_DEFAULT MESSNUM(MESS_BOARD_PAUSE, 29)
#define MES_BPAUSE6_MG_ROW_STATUS_SINGLE_UNLOCK MESSNUM(MESS_BOARD_PAUSE, 30)
#define MES_BPAUSE6_MG_LOCKED_NAME MESSNUM(MESS_BOARD_PAUSE, 28)

/* Keeps the saved minigame pack selection in range before pause settings are stored. */
static inline s32 GWMgPackGet(void)
{
    if(GwSystem.mgPack >= 5) {
        GwSystem.mgPack = 0;
    }
    return GwSystem.mgPack;
}

/* board externs not yet in headers */
extern HUPROCESS *mbMainProc;
extern s32 mbBGRead(int dataNum);
extern void mbBGReadWait(s32 handle);
extern BOOL mbWipeSpecialCheck(void);
extern int mbWipeSpecialStatGet(void);
extern void mbExitReq(void);
extern int mbLanguageGet(void);
extern void mbPauseGuideCreate(void);
extern void mbPauseGuideKill(void);
extern void mbPauseDispCopyCreate(void);
extern void mbPauseDispCopyKill(void);
extern void mbPauseEnableSet(void);
extern void mbPauseEnableReset(void);
extern s16 mbTelopTimeSprCreate(void);
extern void mbTelopTimeSprRotSet(s16 id, float rot);
extern void mbTelopTimeDispSet(s16 id, BOOL disp);
extern void mbTelopTimeSprKill(s16 id);
extern void mbNormPosto3D(HuVecF *src, int camId, HuVecF *dst);
extern BOOL mbConfigExec(int playerNo, int modelId);
extern s16 mbPausePanelCreate(int dataNum, unsigned int espDataNum);
extern void mbPausePanelPosSet(s16 id, float x, float y);
extern void mbPausePanelBankSet(s16 id, int bank);
extern void mbPausePanelGrowSet(s16 id, int a, int b, float scale);
extern void mbPausePanelKill(s16 id);
extern int mbSingleStepGet(void);
extern int mbMasuNumGet(void);
extern BOOL mbSingleMgUnlockGet(int mgNo);

typedef struct MgList_s {
    s16 category;   /* Category index shown by this page. */
    s16 unlockedCount; /* Number of entries unlocked for display. */
    s16 entryCount; /* Number of minigames assigned to this category. */
    struct {
        u8 mgNo;    /* Index into MgDataTbl. */
        u8 flag;    /* Bit 0: unlocked in party play; bit 1: unlocked in single-player. */
    } mg[32];
} MGLIST;

typedef struct pauseWork_s {
    s16 sprId[6];   /* Board logo, turn label, and four turn-number digit sprites. */
    s16 panelId;    /* Pause panel model; zero means no panel is active. */
    s16 telopId;    /* Turn-time sprite; negative means it was not created. */
    s32 winId[2];   /* Help windows used by the pause screen. */
} PAUSEWORK;

typedef struct pauseSingleWork_s {
    s32 bgWin;      /* Background window behind the minigame list. */
    s32 titleWin;   /* Window containing the selected minigame category title. */
    s32 mgWin[2];   /* Two alternating windows containing minigame rows. */
    s32 moveCnt[2]; /* Remaining frames in the corresponding list-edge animation (0 to 8). */
    s32 growF[2];   /* Whether each list-edge arrow remains visible. */
    s16 sprId[14];  /* Category, progress, count, and list-edge arrow sprites. */
} PAUSESINGLEWORK;

static MBPAUSEHOOK pauseHook[PAUSE_HOOK_MAX];
static MGLIST pauseMGList[6];
static PAUSESINGLEWORK pauseSingleWork;

static int pausePlayer = -1;
static s8 guideMotTbl[] = { 3, 12, 11, 23, -1 };
static s8 guideMotSingleTbl[] = { 3, 12, 11, 23, -1 };

static HUPROCESS *pauseProc;
static int pauseHookNum;
static OMOBJ *pauseGuideObj;

static const u32 HelpWinMesTbl[2] = { MES_BPAUSE6_SINGLE_HELP, MES_BPAUSE6_MULTIPLAYER_HELP };

static void PauseMain(void);
static void PauseDestroy(void);
static void PauseScreenCreate(PAUSEWORK *work);
static void PauseScreenSingleCreate(PAUSEWORK *work);
static void PauseScreenKill(PAUSEWORK *work);
static void PauseScreenSingleKill(PAUSEWORK *work);
static int PauseScreenExec(PAUSEWORK *work);
static int PauseSingleMGListGet(MGLIST *list);
static void PauseSingleSprCreate(MGLIST *list, PAUSESINGLEWORK *work);
static void PauseSingleMGTypeSet(MGLIST *list, PAUSESINGLEWORK *work, int page, int top);
static int PauseSingleExec(PAUSEWORK *work);

/* Called during board setup to clear pause hooks and restore the unpaused engine state. */
void mbPauseInit(void)
{
    int i;
    pauseProc = NULL;
    pauseHookNum = 0;
    HuPrcAllPause(FALSE);
    Hu3DPauseSet(FALSE);
    HuSprPauseSet(FALSE);
    for(i=0; i<PAUSE_HOOK_MAX; i++) {
        pauseHook[i] = NULL;
    }
}

/* Starts the pause process for the player whose Start press opened the menu. */
void mbPauseCreate(int playerNo)
{
    pauseProc = HuPrcChildCreate(PauseMain, 0x2012, 0x3800, 0, mbMainProc);
    HuPrcDestructorSet2(pauseProc, PauseDestroy);
    pausePlayer = playerNo;
    HuPrcSetStat(pauseProc, HU_PRC_STAT_PAUSE_ON|HU_PRC_STAT_UPAUSE_ON);
    mbPauseSet(TRUE);
}

/* Polled by board flow to find a connected eligible player pressing Start. */
int mbPauseStartCheck(void)
{
    int i;
    if(omUPauseFlag) {
        return -1;
    }
    if(mbPauseProcCheck()) {
        return -1;
    }
    if(mbPauseDisableGet()) {
        return -1;
    }
    if(_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return -1;
    }
    if(!mbEffFadeCheck()) {
        return -1;
    }
    if(mbWipeSpecialCheck()) {
        return -1;
    }
    if(mbWipeSpecialStatGet()) {
        return -1;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        int padNo = GwPlayer[i].padNo;
        if(HuPadStatGet(padNo) == 0) {
            if ((GWPartyGet() != FALSE || GwPlayer[i].comF == FALSE) &&
                (HuPadBtnDown[padNo] & PAD_BUTTON_START)) {
                return i;
            }
        }
    }
    return -1;
}

/* Lets board flow test whether the pause process currently exists. */
BOOL mbPauseProcCheck(void)
{
    return (pauseProc != NULL) ? TRUE : FALSE;
}

/* Enables or clears the common flag that prevents opening the pause menu. */
void mbPauseDisableSet(BOOL disableF)
{
    if(disableF) {
        _SetFlag(FLAGNUM(FLAG_GROUP_COMMON, 31));
    } else {
        _ClearFlag(FLAGNUM(FLAG_GROUP_COMMON, 31));
    }
}

/* Returns whether board flow currently forbids opening the pause menu. */
BOOL mbPauseDisableGet(void)
{
    return (_CheckFlag(FLAGNUM(FLAG_GROUP_COMMON, 31))) ? TRUE : FALSE;
}

/* Registers a callback to pause or resume a board subsystem with the menu. */
void mbPauseHookPush(MBPAUSEHOOK hook)
{
    pauseHook[pauseHookNum++] = hook;
}

/* Removes a registered pause callback while preserving the order of remaining hooks. */
void mbPauseHookPop(MBPAUSEHOOK hook)
{
    int i;
    for(i=0; i<pauseHookNum; i++) {
        if(pauseHook[i] == hook) {
            break;
        }
    }
    if(i >= pauseHookNum) {
        return;
    }
    for(; i<pauseHookNum-1; i++) {
        pauseHook[i] = pauseHook[i+1];
    }
    pauseHookNum--;
}

/* Pause process body: fades to the menu, runs it, then restores the board scene unless
 * configuration exits. */
static void PauseMain(void)
{
    PAUSEWORK pauseWork;
    PAUSEWORK *work = &pauseWork;
    int i;
    s32 bgHandle;
    int cancelF = FALSE;
    int result = TRUE;
    int boardNo;
    int partyF;
    static const u32 logoFileTbl[] = {
        DATANUM(DATA_board, 68), DATANUM(DATA_board, 69),
        DATANUM(DATA_board, 70), DATANUM(DATA_board, 71),
        DATANUM(DATA_board, 72), DATANUM(DATA_board, 73),
        DATANUM(DATA_board, 76), DATANUM(DATA_board, 75),
        DATANUM(DATA_board, 74), DATANUM(DATA_board, 68),
        DATANUM(DATA_board, 0x44),
    };
    HuMemHeapDump(HuMemHeapPtrGet(HEAP_DVD), -1);
    pauseGuideObj = NULL;
    boardNo = GwSystem.boardNo;
    work->sprId[0] = espEntry(mbBoardDataNumGet(logoFileTbl[boardNo]), 100, 0);
    espDispOff(work->sprId[0]);
    work->telopId = -1;
    partyF = GwSystem.partyF;
    if(partyF) {
        work->telopId = mbTelopTimeSprCreate();
        mbTelopTimeSprRotSet(work->telopId, 0.0f);
        mbTelopTimeDispSet(work->telopId, FALSE);
        HuSprGrpPosSet(work->telopId, 288.0f * 0.35f, 216.0f);
        HuSprGrpScaleSet(work->telopId, 0.65f, 0.65f);
    }
    HuDataDirClose(mbBoardDataNumGet(DATA_board));
    bgHandle = mbBGRead(mbBoardDataNumGet(DATA_bpause6));
    mbEffFadeCreate(30, 160);
    for(i=0; i<pauseHookNum; i++) {
        if(pauseHook[i] != NULL) {
            pauseHook[i](FALSE);
        }
    }
    HuPadRumbleAllStop();
    HuPrcSleep(1);
    mbAudFXPlay(5);
    while(!mbEffFadeDoneCheck()) {
        HuPrcVSleep();
    }
    mbPauseGuideCreate();
    mbPauseDispCopyCreate();
    mbBGReadWait(bgHandle);
    while(cancelF == 0 && result != 0) {
        PauseScreenCreate(work);
        result = PauseScreenExec(work);
        PauseScreenKill(work);
        if(result != 0) {
            cancelF = mbConfigExec(pausePlayer, mbGuideModelGet(pauseGuideObj));
        }
    }
    if(pauseGuideObj) {
        mbGuideKill(pauseGuideObj);
        pauseGuideObj = NULL;
    }
    espKill(work->sprId[0]);
    work->sprId[0] = -1;
    if(work->telopId >= 0) {
        mbTelopTimeSprKill(work->telopId);
        work->telopId = -1;
    }
    mbPauseGuideKill();
    HuDataDirClose(mbBoardDataNumGet(DATA_bpause6));
    mbPauseDispCopyKill();
    if(cancelF) {
        mbExitReq();
        HuPrcSleep(-1);
    }
    bgHandle = mbBGRead(mbBoardDataNumGet(DATA_board));
    mbEffFadeOutSet(30);
    HuPrcSleep(30);
    mbBGReadWait(bgHandle);
    HuPrcEnd();
}

/* Process destructor resumes hooks and clears pause when the board is not exiting, then stores
 * current pause-related settings. */
static void PauseDestroy(void)
{
    int i;
    if(mbExitCheck() == FALSE) {
        for(i=0; i<pauseHookNum; i++) {
            if(pauseHook[i] != NULL) {
                pauseHook[i](TRUE);
            }
        }
        mbPauseSet(FALSE);
    }
    if(GWMgInstDispGet()) {
        _SetFlag(FLAGNUM(FLAG_GROUP_SAVE, 5));
    } else {
        _ClearFlag(FLAGNUM(FLAG_GROUP_SAVE, 5));
    }
    if(GWPartyGet() == FALSE) {
        GwCommon.storyMgInstDispF = GWMgInstDispGet();
        GwCommon.storyMgComDispF = GWMgComDispGet();
        GwCommon.storyMgPack = GWMgPackGet();
        GwCommon.storyMessSpeed = GWMessSpeedGet();
        GwCommon.storySaveMode = GWSaveModeGet();
    } else {
        GwCommon.partyMgInstDispF = GWMgInstDispGet();
        GwCommon.partyMgComDispF = GWMgComDispGet();
        GwCommon.partyMgPack = GWMgPackGet();
        GwCommon.partyMessSpeed = GWMessSpeedGet();
        GwCommon.partySaveMode = GWSaveModeGet();
    }
    pausePlayer = -1;
    pauseProc = NULL;
}

/* Builds the multiplayer pause screen, or delegates to the single-player list screen. */
static void PauseScreenCreate(PAUSEWORK *work)
{
    int i;
    int lang;
    int bank;
    int turn;
    int partyF;
    int tag;
    HuVec2f center;
    HuVecF pos3d;
    static const HuVec2f turnNoPos[] = {
        { -74, -25 },
        { -30, -25 },
        { 30, 25 },
        { 74, 25 },
    };
    static const float turnOfsTbl[] = { 0.0f, 64.0f, 100.0f, 100.0f, 100.0f, 100.0f };
    static const int anmNoTbl[] = { 2, 0, 1 };
    static HuVecF guidePos = { -0.65f, -0.75f, -750.0f };

    partyF = GwSystem.partyF;
    if(!partyF) {
        PauseScreenSingleCreate(work);
        return;
    }
    lang = mbLanguageGet();
    espDispOn(work->sprId[0]);
    espAttrSet(work->sprId[0], HUSPR_ATTR_LINEAR);
    espPosSet(work->sprId[0], 288.0f, 128.0f);
    work->panelId = mbPausePanelCreate(mbBoardDataNumGet(DATA_bpause6), 0);
    mbPausePanelPosSet((s16)work->panelId, 0.65f, 0.0f);
    tag = GwSystem.tagF;
    if(tag) {
        bank = 2;
    } else {
        bank = GWPartyGet();
    }
    mbPausePanelBankSet((s16)work->panelId, anmNoTbl[bank]);
    mbPausePanelGrowSet((s16)work->panelId, 16, 0, 1.5f);
    work->sprId[1] = espEntry(mbBoardDataNumGet(DATANUM(DATA_bpause6, 1)), 100, 0);
    espAttrSet(work->sprId[1], HUSPR_ATTR_LINEAR);
    pos3d.x = 273.6f + turnOfsTbl[lang];
    pos3d.y = 304.0f;
    espPosSet(work->sprId[1], pos3d.x, pos3d.y);
    for(i=0; i<4; i++) {
        work->sprId[i+2] = espEntry(mbBoardDataNumGet(DATANUM(DATA_bpause6, 2)), 95, 0);
        espPosSet(work->sprId[i+2], pos3d.x + turnNoPos[i].x, pos3d.y + turnNoPos[i].y);
        espAttrSet(work->sprId[i+2], HUSPR_ATTR_LINEAR);
    }
    turn = GwSystem.turnNo;
    if(turn > 99) {
        /* The two digit sprites display values only through 99. */
        turn = 99;
    }
    if(turn/10 != 0) {
        espBankSet(work->sprId[2], turn/10);
    } else {
        espAttrSet(work->sprId[2], HUSPR_ATTR_DISPOFF);
    }
    espBankSet(work->sprId[3], turn%10);
    turn = GwSystem.turnMax;
    if(turn > 99) {
        turn = 99;
    }
    if(turn/10 != 0) {
        espBankSet(work->sprId[4], turn/10);
    } else {
        espAttrSet(work->sprId[4], HUSPR_ATTR_DISPOFF);
    }
    espBankSet(work->sprId[5], turn%10);
    if(pauseGuideObj == NULL) {
        mbNormPosto3D(&guidePos, 4, &pos3d);
        pauseGuideObj = mbGuideCreateFlag(&pos3d, guideMotTbl, TRUE, FALSE, TRUE);
        mbObjDispSet(mbGuideModelGet(pauseGuideObj), FALSE);
    }
    if(work->telopId >= 0) {
        mbTelopTimeDispSet(work->telopId, TRUE);
    }
    work->winId[0] = work->winId[1] = -1;
    work->winId[0] = mbWinCreateHelp(HelpWinMesTbl[0]);
    mbWinCenterGet(work->winId[0], &center);
    mbWinPosSet(work->winId[0], center.x, 376);
    HuPrcVSleep();
    mbObjDispSet(mbGuideModelGet(pauseGuideObj), TRUE);
}

/* Sets up the single-player pause screen logo and creates its two help windows. */
static void PauseScreenSingleCreate(PAUSEWORK *work)
{
    HuVec2f center;
    HuVecF pos3d;
    static HuVecF guidePos = { -0.7f, -0.8f, -750.0f };

    espDispOn(work->sprId[0]);
    espAttrSet(work->sprId[0], HUSPR_ATTR_LINEAR);
    espPosSet(work->sprId[0], 288.0f * 0.32f, 120.0f);
    espScaleSet(work->sprId[0], 0.5f, 0.5f);
    work->panelId = 0;
    if(pauseGuideObj == NULL) {
        mbNormPosto3D(&guidePos, 4, &pos3d);
        pauseGuideObj = mbGuideCreateFlag(&pos3d, guideMotSingleTbl, TRUE, FALSE, TRUE);
    }
    work->winId[0] = work->winId[1] = -1;
    work->winId[0] = mbWinCreateHelp(MES_BPAUSE6_SINGLE_HELP);
    mbWinCenterGet(work->winId[0], &center);
    mbWinPosSet(work->winId[0], center.x, 376);
    work->winId[1] = mbWinCreateHelp(MES_BPAUSE6_SINGLE_MG_HELP);
    mbWinCenterGet(work->winId[1], &center);
    mbWinPosSet(work->winId[1], center.x, 332);
}

/* Releases sprites, panel, and windows created for the multiplayer pause screen. */
static void PauseScreenKill(PAUSEWORK *work)
{
    int i;
    int partyF = GwSystem.partyF;
    if(!partyF) {
        PauseScreenSingleKill(work);
        return;
    }
    if(work->winId[0] != -1) {
        mbWinKill(work->winId[0]);
        work->winId[0] = -1;
    }
    espDispOff(work->sprId[0]);
    if(work->telopId >= 0) {
        mbTelopTimeDispSet(work->telopId, FALSE);
    }
    for(i=1; i<6; i++) {
        if(work->sprId[i] >= 0) {
            espKill(work->sprId[i]);
        }
        work->sprId[i] = -1;
    }
    if(work->panelId != 0) {
        mbPausePanelKill((s16)work->panelId);
        work->panelId = 0;
    }
}

/* Hides the single-player logo and closes its active help windows. */
static void PauseScreenSingleKill(PAUSEWORK *work)
{
    espDispOff(work->sprId[0]);
    if(work->winId[0] != -1) {
        mbWinKill(work->winId[0]);
        work->winId[0] = -1;
    }
    if(work->winId[1] != -1) {
        mbWinKill(work->winId[1]);
        work->winId[1] = -1;
    }
}

/* Waits for A to open configuration or Start to close the pause screen. */
static int PauseScreenExec(PAUSEWORK *work)
{
    int i = 0;
    int result;
    int padNo;
    int partyF = GwSystem.partyF;
    if(!partyF) {
        return PauseSingleExec(work);
    }
    while(1) {
        HuPrcVSleep();
        padNo = GwPlayer[pausePlayer].padNo;
        if(HuPadStatGet(padNo)) {
            continue;
        }
        if(HuPadBtnDown[padNo] & PAD_BUTTON_A) {
            result = 1;
            mbAudFXPlay(2);
            break;
        }
        i++;
        if(i >= 60) {
            i = 0;
        }
        if(work->telopId >= 0) {
            mbTelopTimeSprRotSet(work->telopId, i * (1.0f/60.0f));
        }
        if(HuPadBtnDown[padNo] & PAD_BUTTON_START) {
            result = 0;
            break;
        }
    }
    return result;
}

/* Pauses board objects and engine subsystems, or resumes them when pauseF is false. */
void mbPauseSet(BOOL pauseF)
{
    OMOBJWORK *objWork = mbObjMan->property;
    int i;
    if(pauseF) {
        for(i=0; i<objWork->objMax; i++) {
            if((objWork->objData[i].stat & (OM_STAT_DELETED|OM_STAT_NOPAUSE)) == 0) {
                omSetStatBit(&objWork->objData[i], OM_STAT_PAUSED);
            }
        }
        mbPauseEnableSet();
    } else {
        for(i=0; i<objWork->objMax; i++) {
            if((objWork->objData[i].stat & (OM_STAT_DELETED|OM_STAT_NOPAUSE)) == 0) {
                omResetStatBit(&objWork->objData[i], OM_STAT_PAUSED);
            }
        }
        mbPauseEnableReset();
    }
    HuPrcAllPause(pauseF);
    Hu3DPauseSet(pauseF);
    HuSprPauseSet(pauseF);
    HuAudFXPauseAll(pauseF);
    HuAudSeqPauseAll(pauseF);
    mbMusPauseSet(pauseF);
}

static u32 pauseDataDirTbl[] = {
    DATA_bpause6,
    DATA_bpause6_us,
    DATA_bpause6,
    DATA_bpause6,
    DATA_bpause6,
    DATA_bpause6,
};

/* Selects the board pause data directory for a language index. */
int mbPauseDataDirGet(int type)
{
    if(type < 0) {
        type = 0;
    }
    return pauseDataDirTbl[type];
}

/* Replaces the default pause data directory with the current language variant. */
int mbPauseDataNumGet(int dataNum)
{
    int lang = mbLanguageGet();
    if(DIRNUM(dataNum) != DATA_bpause6) {
        return dataNum;
    }
    if(lang < 0) {
        lang = 0;
    }
    return (dataNum & 0xFFFF) | mbPauseDataDirGet(lang);
}

static s16 mgTypeTbl[6] = { 0, 1, 2, 3, 6, -1 };

/* Assigns minigames to single-player menu categories and counts unlocked entries. */
static int PauseSingleMGListGet(MGLIST *list)
{
    int i;
    int j;
    int categoryIndex;
    int unlockedTotal;
    MGLIST *category;

    memset(list, 0, sizeof(pauseMGList));
    for(i=0; i<6; i++) {
        list[i].category = i;
    }
    for(i=0; MgDataTbl[i].ovl != 0xFFFF; i++) {
        categoryIndex = 6;
        if(MgDataTbl[i].flag & 0x80) {
            categoryIndex = 5;
            if(MgDataTbl[i].ovl == 0x52) {
                /* This special overlay is omitted from the single-player category list. */
                categoryIndex = 6;
            }
        } else {
            for(j=0; j<5; j++) {
                if(mgTypeTbl[j] == MgDataTbl[i].type) {
                    categoryIndex = j;
                    break;
                }
            }
        }
        if(categoryIndex >= 6) {
            continue;
        }
        category = &list[categoryIndex];
        category->mg[category->entryCount].mgNo = i;
        if(GWMgUnlockGet(MgNoGet(MgDataTbl[i].ovl) + 0x259)) {
            category->mg[category->entryCount].flag = 1;
        }
        if(mbSingleMgUnlockGet(MgNoGet(MgDataTbl[i].ovl) + 0x259)) {
            category->mg[category->entryCount].flag |= 2;
        }
        if(category->mg[category->entryCount].flag != 0) {
            category->unlockedCount++;
        }
        category->entryCount++;
    }
    unlockedTotal = 0;
    for(i=0; i<6; i++) {
        unlockedTotal += list[i].unlockedCount;
    }
    return unlockedTotal;
}

/* Creates the single-player minigame list graphics and its three help windows. */
static void PauseSingleSprCreate(MGLIST *list, PAUSESINGLEWORK *work)
{
    int i;
    int remainingCount;
    int page;
    int total;
    int lang;
    HuVec2f winSize;
    HuVecF winPos;
    static int fileTbl[] = {
        DATANUM(DATA_bpause6, 44), DATANUM(DATA_bpause6, 40),
        DATANUM(DATA_bpause6, 41), DATANUM(DATA_bpause6, 41),
        DATANUM(DATA_bpause6, 43), DATANUM(DATA_bpause6, 43),
        DATANUM(DATA_bpause6, 42), DATANUM(DATA_bpause6, 42),
        DATANUM(DATA_bpause6, 39), DATANUM(DATA_bpause6, 38),
        DATANUM(DATA_bpause6, 38), DATANUM(DATA_bpause6, 38),
        DATANUM(DATA_bpause6, 32), DATANUM(DATA_bpause6, 32),
    };
    static HuVec2f posTbl[] = {
        { 288, 136 }, { 472, 72 }, { 424, 72 }, { 520, 72 },
        { 424, 72 }, { 520, 72 }, { 460, 72 }, { 484, 72 },
        { 480, 152 }, { 504, 152 }, { 524, 152 }, { 542, 152 },
        { 288, 172 }, { 288, 312 },
    };
    static s16 langOfsTbl[6][6] = {
        { -48, 48, -48, 48, -12, 12 },
        { -28, -28, -28, -28, -64, -40 },
        { -28, -28, -28, -28, -64, -40 },
        { -28, -28, -28, -28, -64, -40 },
        { -28, -28, -28, -28, -64, -40 },
        { -28, -28, -28, -28, -64, -40 },
    };

    total = PauseSingleMGListGet(list);
    memset(work, 0, sizeof(PAUSESINGLEWORK));
    for(i=0; i<14; i++) {
        work->sprId[i] = espEntry(mbBoardDataNumGet(fileTbl[i]), 100, 0);
        espAttrSet(work->sprId[i], HUSPR_ATTR_LINEAR);
        espPosSet(work->sprId[i], posTbl[i].x, posTbl[i].y);
    }
    espBankSet(work->sprId[12], 1);
    espBankSet(work->sprId[13], 3);
    espDispOff(work->sprId[12]);
    espDispOff(work->sprId[13]);
    espPriSet(work->sprId[0], 2000);
    work->moveCnt[0] = work->moveCnt[1] = 0;
    work->growF[0] = work->growF[1] = 0;
    lang = mbLanguageGet();
    for(i=0; i<6; i++) {
        espPosSet(work->sprId[i+2], langOfsTbl[lang][i] + 472, 72.0f);
    }
    espBankSet(work->sprId[3], 1);
    espBankSet(work->sprId[5], 1);
    espDispOff(work->sprId[6]);
    i = mbSingleStepGet();
    if(i <= 1) {
        espDispOff(work->sprId[2]);
        espDispOff(work->sprId[3]);
    } else {
        espDispOff(work->sprId[4]);
        espDispOff(work->sprId[5]);
    }
    if(i < 0) {
        i = 0;
    }
    if(i >= mbMasuNumGet()) {
        i = 0;
    }
    remainingCount = i;
    page = remainingCount/10;
    if(page > 0) {
        espDispOn(work->sprId[6]);
        espBankSet(work->sprId[6], page);
    }
    remainingCount = remainingCount - (page*10);
    if(remainingCount > 0) {
        if(i <= 5) {
            remainingCount += 9;
        }
        espBankSet(work->sprId[7], remainingCount);
    }
    for(i=0; i<4; i++) {
        espScaleSet(work->sprId[i+8], 1.2f, 1.2f);
    }
    espBankSet(work->sprId[9], 10);
    espDispOff(work->sprId[11]);
    i = 0;
    page = total/10;
    if(page > 0) {
        espDispOn(work->sprId[11]);
        espBankSet(work->sprId[10], page);
        i++;
    }
    remainingCount = total - (page*10);
    if(remainingCount > 0) {
        espBankSet(work->sprId[i+10], remainingCount);
    }
    winPos.x = 288.0f;
    winPos.y = 240.0f;
    winPos.z = 0.0f;
    work->bgWin = mbWinCreateBlank();
    winSize.x = 540.0f;
    winSize.y = 120.0f;
    mbWinSizeSet(work->bgWin, winSize.x, winSize.y);
    winPos.x = winPos.x + (0.5f * -winSize.x);
    winPos.y = winPos.y + (0.5f * -winSize.y);
    mbWinPosSet(work->bgWin, winPos.x, winPos.y);
    winPos.x += 4.0f;
    winPos.y += 4.0f;
    for(i=0; i<2; i++) {
        work->mgWin[i] = mbWinCreateHelp(MES_BPAUSE6_MG_LIST_HELP);
        mbWinMesMaxSizeGet(work->mgWin[i], &winSize);
        mbWinSizeSet(work->mgWin[i], 268, winSize.y);
        mbWinPosSet(work->mgWin[i], winPos.x + ((i & 1) ? 268 : 0), winPos.y);
    }
    work->titleWin = mbWinCreateHelp(MES_BPAUSE6_MG_TITLE_0);
    mbWinMesMaxSizeGet(work->titleWin, &winSize);
    winPos.x = 288.0f;
    winPos.y = 240.0f;
    mbWinPosSet(work->titleWin, winPos.x - (0.5f * winSize.x),
                (winPos.y - 108.0f) - (0.5f * winSize.y));
}

/* Fills the title and visible rows for one category and list offset. */
static void PauseSingleMGTypeSet(MGLIST *list, PAUSESINGLEWORK *work, int page, int top)
{
    int i;
    MGLIST *entry;
    u32 mesA;
    u32 mesB;
    int lang;
    HuVec2f winSize;
    static u32 insertMesTbl[] = {
        MES_BPAUSE6_MG_TITLE_0, MES_BPAUSE6_MG_TITLE_1, MES_BPAUSE6_MG_TITLE_2,
        MES_BPAUSE6_MG_TITLE_3, MES_BPAUSE6_MG_TITLE_4, MES_BPAUSE6_MG_TITLE_5,
    };
    static int insertMesTbl2[] = { 0, 0, 1, 1, 2, 2, 3, 3 };
    static int insertMesTbl3[] = { 4, 4, 5, 5, 6, 6, 7, 7 };
    static const float nameScaleTbl[] = { 1.0f, 0.85f, 0.85f, 0.85f, 0.85f, 0.85f };

    entry = &list[page];
    lang = mbLanguageGet();
    mbWinCenterInsertGet(work->titleWin, insertMesTbl[page]);
    mbWinScaleSet(work->titleWin, nameScaleTbl[lang], 1.0f);
    mbWinMesMaxSizeGet(work->titleWin, &winSize);
    mbWinPosSet(work->titleWin, 288.0f - (0.5f * winSize.x * nameScaleTbl[lang]),
                136.0f - (0.5f * winSize.y));
    for(i=0; i<8; i++) {
        int slot = i + top;
        mesA = (u32)" ";
        mesB = MES_BPAUSE6_MG_ROW_STATUS_DEFAULT;
        if(slot < entry->entryCount) {
            if(entry->mg[slot].flag != 0) {
                mesA = MgDataTbl[entry->mg[slot].mgNo].nameMes;
                if(entry->mg[slot].flag >= 2) {
                    mesB = MES_BPAUSE6_MG_ROW_STATUS_SINGLE_UNLOCK;
                }
            } else {
                mesA = MES_BPAUSE6_MG_LOCKED_NAME;
            }
        }
        mbWinInsertMesSet(work->mgWin[i & 1], mesA, insertMesTbl2[i]);
        mbWinInsertMesSet(work->mgWin[i & 1], mesB, insertMesTbl3[i]);
    }
    for(i=0; i<2; i++) {
        mbWinCenterInsertGet(work->mgWin[i], MES_BPAUSE6_MG_LIST_HELP);
    }
    work->growF[0] = work->growF[1] = 1;
    if(top <= 0) {
        work->growF[0] = 0;
    }
    if(top + 8 >= entry->entryCount) {
        work->growF[1] = 0;
    }
}

/* Runs single-player pause input, category paging, scrolling, and arrow animation. */
static int PauseSingleExec(PAUSEWORK *work)
{
    int i;
    int n;
    int result;
    int padNo;
    int inputDelay = 0;
    int page = 0;
    int top = 0;
    int newPage;
    int newTop;
    float scale;
    float target;
    PauseSingleSprCreate(pauseMGList, &pauseSingleWork);
    HuPrcVSleep();
    PauseSingleMGTypeSet(pauseMGList, &pauseSingleWork, 0, 0);
    while(1) {
        HuPrcVSleep();
        padNo = GwPlayer[pausePlayer].padNo;
        if(HuPadStatGet(padNo)) {
            continue;
        }
        if(HuPadBtnDown[padNo] & PAD_BUTTON_A) {
            result = 1;
            mbAudFXPlay(2);
            break;
        }
        if(HuPadBtnDown[padNo] & PAD_BUTTON_START) {
            result = 0;
            break;
        }
        if(inputDelay != 0) {
            inputDelay--;
        } else {
            newPage = page;
            newTop = top;
            if(HuPadBtnRep[padNo] & PAD_TRIGGER_L) {
                newPage--;
                if(newPage < 0) {
                    newPage += 6;
                }
                newTop = 0;
                inputDelay = 8;
            } else if(HuPadBtnRep[padNo] & PAD_TRIGGER_R) {
                newPage++;
                if(newPage >= 6) {
                    newPage -= 6;
                }
                newTop = 0;
                inputDelay = 8;
            } else if(HuPadDStkRep[padNo] & PAD_BUTTON_DOWN) {
                if(newTop + 8 < pauseMGList[page].entryCount) {
                    newTop += 2;
                    pauseSingleWork.moveCnt[1] = 8;
                }
            } else if(HuPadDStkRep[padNo] & PAD_BUTTON_UP) {
                if(newTop > 0) {
                    newTop -= 2;
                    if(newTop < 0) {
                        newTop = 0;
                    }
                    pauseSingleWork.moveCnt[0] = 8;
                }
            }
            if(inputDelay != 0) {
                pauseSingleWork.moveCnt[0] = pauseSingleWork.moveCnt[1] = 0;
            }
            if(page != newPage || top != newTop) {
                mbAudFXPlay(0);
                PauseSingleMGTypeSet(pauseMGList, &pauseSingleWork, newPage, newTop);
                page = newPage;
                top = newTop;
            }
        }
        for(i=0; i<2; i++) {
            scale = 1.0f;
            if(pauseSingleWork.growF[i]) {
                espDispOn(pauseSingleWork.sprId[i+12]);
                espTPLvlSet(pauseSingleWork.sprId[i+12], 1.0f);
            }
            if(pauseSingleWork.moveCnt[i]) {
                pauseSingleWork.moveCnt[i]--;
                target = pauseSingleWork.moveCnt[i] / 8.0f;
                scale = target;
                scale = 1.5f - (0.5f * mbCosDeg(90.0f * scale));
                if(!pauseSingleWork.growF[i]) {
                    espTPLvlSet(pauseSingleWork.sprId[i+12], mbSinDeg(90.0f * target));
                }
            } else if(!pauseSingleWork.growF[i]) {
                espDispOff(pauseSingleWork.sprId[i+12]);
            }
            espScaleSet(pauseSingleWork.sprId[i+12], scale, scale);
        }
    }
    for(n=0; n<14; n++) {
        espKill(pauseSingleWork.sprId[n]);
        pauseSingleWork.sprId[n] = -1;
    }
    for(n=0; n<2; n++) {
        mbWinKill(pauseSingleWork.mgWin[n]);
        pauseSingleWork.mgWin[n] = -1;
    }
    mbWinKill(pauseSingleWork.titleWin);
    pauseSingleWork.titleWin = -1;
    mbWinKill(pauseSingleWork.bgWin);
    pauseSingleWork.bgWin = -1;
    return result;
}
