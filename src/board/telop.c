// Board speech bubbles, turn notices, timer displays, and taunt input effects.

#include "dolphin/math.h"

#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/pause.h"

#include "game/armem.h"
#include "game/data.h"
#include "game/esprite.h"
#include "game/sprite.h"
#include "game/flag.h"
#include "game/pad.h"
#include "game/wipe.h"

#include "msm.h"

#define FLAG_BOARD_WALKDONE FLAGNUM(FLAG_GROUP_COMMON, 16)

enum {
    BOARD_ANM_telopMario = DATANUM(DATA_board, 77),
    BOARD_ANM_telopLuigi = DATANUM(DATA_board, 78),
    BOARD_ANM_telopPeach = DATANUM(DATA_board, 79),
    BOARD_ANM_telopYoshi = DATANUM(DATA_board, 80),
    BOARD_ANM_telopWario = DATANUM(DATA_board, 81),
    BOARD_ANM_telopDaisy = DATANUM(DATA_board, 82),
    BOARD_ANM_telopWaluigi = DATANUM(DATA_board, 83),
    BOARD_ANM_telopKinopio = DATANUM(DATA_board, 84),
    BOARD_ANM_telopTeresa = DATANUM(DATA_board, 85),
    BOARD_ANM_telopMinikoopa = DATANUM(DATA_board, 86),
    BOARD_ANM_telopMinikoopaR = DATANUM(DATA_board, 87),
    BOARD_ANM_telopMinikoopaG = DATANUM(DATA_board, 87),
    BOARD_ANM_telopMinikoopaB = DATANUM(DATA_board, 87),
    BOARD_ANM_telopKinopiko = DATANUM(DATA_board, 88),
    BOARD_ANM_telopStarHost1 = DATANUM(DATA_board, 89),
    BOARD_ANM_telopStarHost0 = DATANUM(DATA_board, 90),
    BOARD_ANM_telopBoardW01 = DATANUM(DATA_board, 68),
    BOARD_ANM_telopBoardW02 = DATANUM(DATA_board, 69),
    BOARD_ANM_telopBoardW03 = DATANUM(DATA_board, 70),
    BOARD_ANM_telopBoardW04 = DATANUM(DATA_board, 71),
    BOARD_ANM_telopBoardW05 = DATANUM(DATA_board, 72),
    BOARD_ANM_telopBoardW06 = DATANUM(DATA_board, 73),
    BOARD_ANM_telopBoardS03 = DATANUM(DATA_board, 74),
    BOARD_ANM_telopBoardS02 = DATANUM(DATA_board, 75),
    BOARD_ANM_telopBoardS01 = DATANUM(DATA_board, 76),
    BOARD_ANM_telopBoardW10 = BOARD_ANM_telopBoardW01,
    BOARD_ANM_telopBoardW11 = BOARD_ANM_telopBoardW01,
    BOARD_ANM_telopTimeBackDay = DATANUM(DATA_board, 109),
    BOARD_ANM_telopTimeBackNight = DATANUM(DATA_board, 110),
    BOARD_ANM_telopTimeDay = DATANUM(DATA_board, 111),
    BOARD_ANM_telopTimeNight = DATANUM(DATA_board, 112),
    BOARD_ANM_telopTimeStar = DATANUM(DATA_board, 113),
    BOARD_ANM_telopLast = DATANUM(DATA_board, 114),
    BOARD_ANM_telopTurn = DATANUM(DATA_board, 115),
    BOARD_ANM_telopTurnMulti = DATANUM(DATA_board, 116),
};

enum {
    TELOP_SE_CHARACTER_APPEAR = 1011,
    TELOP_SE_CHARACTER_DISMISS = 1012,
    TELOP_SE_TIME_NOTICE = 1132,
};

enum {
    GW_BANK_FLAG_CHARACTER_BASE = 36,
};

/* Kinopiko's slot has no unlock flag in the board character bank. */
#define BOARD_TAUNT_ALWAYS_ENABLED_CHAR 10

typedef struct TauntWork_s {
    unsigned killF : 1; /* Set when the board is closing the taunt sound task. */
} TAUNT_WORK;

typedef struct TelopWork_s {
    unsigned killF : 1; /* Set after the speech bubble finishes its exit animation. */
    unsigned mode : 3; /* 0 enters, 1 waits for input, and 2 exits the bubble. */
    s8 playerNo; /* Player whose bubble is shown, or -1 for an automatic notice. */
    s8 telopNo; /* Index into the board bubble animation table. */
    s8 unused; /* Work storage that this code does not read. */
    s8 dismissDelay; /* Frames a bubble waits before automatic dismissal. */
    s16 time; /* Frames elapsed in the current bubble animation mode. */
    s16 maxTime; /* Number of frames in the current bubble animation mode. */
} TELOP_WORK;

typedef struct TelopLastTurnWork_s {
    unsigned killF : 1; /* Set when the last-turn notice should be removed. */
    unsigned mode : 3; /* 0 fades in, 1 animates, and 2 fades out the notice. */
    unsigned lastTurn : 1; /* True when the notice is for the final turn. */
    u8 delay; /* Frames held after the turn number finishes its spin. */
    s16 angle; /* Progress angle used by the turn-number flip animation. */
    s16 grpId; /* Sprite group containing the three turn notice sprites. */
    s16 sprId[3]; /* Sprite IDs for the notice label and turn number. */
} TELOP_LAST_TURN_WORK;

typedef struct TelopTimeWork_s {
    unsigned killF : 1; /* Set after the timer notice has faded out. */
    unsigned mode : 3; /* 0 rotates the timer notice in, and 1 fades it out. */
    s16 time; /* Frames elapsed in the current timer animation mode. */
    s16 maxTime; /* Number of frames in the current timer animation mode. */
} TELOP_TIME_WORK;

typedef struct TelopTimeChangeWork_s {
    unsigned killF : 1; /* Set when the time-of-day notice should be removed. */
    unsigned completeF : 1; /* Set when the notice reaches its stable display. */
    unsigned mode : 4; /* Animation phase for the time-of-day transition. */
    s16 time; /* Frames elapsed in the current transition phase. */
    s16 maxTime; /* Number of frames in the current transition phase. */
} TELOP_TIME_CHANGE_WORK;

static const u32 telopFileTbl[27] = {
    BOARD_ANM_telopMario,
    BOARD_ANM_telopLuigi,
    BOARD_ANM_telopPeach,
    BOARD_ANM_telopYoshi,
    BOARD_ANM_telopWario,
    BOARD_ANM_telopDaisy,
    BOARD_ANM_telopWaluigi,
    BOARD_ANM_telopKinopio,
    BOARD_ANM_telopTeresa,
    BOARD_ANM_telopMinikoopa,
    BOARD_ANM_telopKinopiko,
    BOARD_ANM_telopMinikoopaR,
    BOARD_ANM_telopMinikoopaG,
    BOARD_ANM_telopMinikoopaB,
    BOARD_ANM_telopStarHost0,
    BOARD_ANM_telopStarHost1,
    BOARD_ANM_telopBoardW01,
    BOARD_ANM_telopBoardW02,
    BOARD_ANM_telopBoardW03,
    BOARD_ANM_telopBoardW04,
    BOARD_ANM_telopBoardW05,
    BOARD_ANM_telopBoardW06,
    BOARD_ANM_telopBoardS01,
    BOARD_ANM_telopBoardS02,
    BOARD_ANM_telopBoardS03,
    BOARD_ANM_telopBoardW10,
    BOARD_ANM_telopBoardW11,
};

static const s32 tauntSeTbl[14] = {
    MSM_SE_CHARVOICE_MARIO + 16,
    MSM_SE_CHARVOICE_LUIGI + 16,
    MSM_SE_CHARVOICE_PEACH + 16,
    MSM_SE_CHARVOICE_YOSHI + 16,
    MSM_SE_CHARVOICE_WARIO + 16,
    MSM_SE_CHARVOICE_DAISY + 16,
    MSM_SE_CHARVOICE_WALUIGI + 16,
    MSM_SE_CHARVOICE_KINOPIO + 16,
    MSM_SE_CHARVOICE_TERESA + 16,
    MSM_SE_CHARVOICE_MINIKOOPA + 16,
    MSM_SE_CHARVOICE_KINOPIKO + 16,
    MSM_SE_CHARVOICE_MINIKOOPA + 16,
    MSM_SE_CHARVOICE_MINIKOOPA + 16,
    MSM_SE_CHARVOICE_MINIKOOPA + 16,
};

static u32 telopTurnFileTbl[3] = {
    BOARD_ANM_telopTurn,
    BOARD_ANM_telopTurnMulti,
    BOARD_ANM_telopTurn,
};

static u32 telopTurnLastFileTbl[3] = {
    BOARD_ANM_telopLast,
    BOARD_ANM_telopTurnMulti,
    BOARD_ANM_telopLast,
};

static HuVec2f telopTurnLastSprOfsTbl[2][3] = {
    { { -24.0f, 0.0f }, { 0.0f, 0.0f }, { 24.0f, 0.0f } },
    { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } },
};

static HuVec2f telopTurnSprOfsTbl[6][3] = {
    { { -24.0f, 0.0f }, { 0.0f, 0.0f }, { 24.0f, 0.0f } },
    { { 32.0f, 0.0f }, { -104.0f, 0.0f }, { 32.0f, 0.0f } },
    { { -24.0f, 0.0f }, { 0.0f, 0.0f }, { 24.0f, 0.0f } },
    { { -24.0f, 0.0f }, { 0.0f, 0.0f }, { 24.0f, 0.0f } },
    { { -24.0f, 0.0f }, { 0.0f, 0.0f }, { 24.0f, 0.0f } },
    { { -24.0f, 0.0f }, { 0.0f, 0.0f }, { 24.0f, 0.0f } },
};

static HuVec2f telopTimeSprOfsTbl[8] = {
    { 0.0f, 0.0f },
    { 0.0f, 0.0f },
    { -88.0f, 8.0f },
    { 0.0f, 80.0f },
    { 88.0f, 8.0f },
    { -88.0f, 8.0f },
    { 0.0f, 80.0f },
    { 88.0f, 8.0f },
};

static HuVec2f telopTimeNewSprOfsTbl[6] = {
    { 0.0f, 0.0f },
    { 0.0f, -8.0f },
    { 0.0f, -8.0f },
    { 0.0f, -8.0f },
    { 0.0f, -8.0f },
    { 0.0f, -8.0f },
};

static HuVec2f telopTimeStarSprOfsTbl[6][3] = {
    { { -88.0f, 8.0f }, { 0.0f, 80.0f }, { 88.0f, 8.0f } },
    { { -80.0f, 40.0f }, { 0.0f, 80.0f }, { 80.0f, 40.0f } },
    { { -80.0f, 40.0f }, { 0.0f, 80.0f }, { 80.0f, 40.0f } },
    { { -80.0f, 40.0f }, { 0.0f, 80.0f }, { 80.0f, 40.0f } },
    { { -80.0f, 40.0f }, { 0.0f, 80.0f }, { 80.0f, 40.0f } },
    { { -80.0f, 40.0f }, { 0.0f, 80.0f }, { 80.0f, 40.0f } },
};

static float telopTimeSprScaleTbl[8] = {
    1.0f,
    0.75f,
    1.0f,
    1.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
};

static float telopTimeBaseTPLvlTbl[8] = {
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
};

static s32 tauntSeNo[GW_PLAYER_MAX] = {
    MSM_SENO_NONE,
    MSM_SENO_NONE,
    MSM_SENO_NONE,
    MSM_SENO_NONE,
};

static s16 telopTurnSprPrioTbl[3] = {
    1400,
    1000,
    1400,
};

static s8 telopTurnSprBankTbl[6] = {
    0,
    0,
    1,
    0,
    0,
    0,
};

static u32 telopTimeBackFileTbl[2] = {
    BOARD_ANM_telopTimeBackDay,
    BOARD_ANM_telopTimeBackNight,
};

static u32 telopTimeFileTbl[2] = {
    BOARD_ANM_telopTimeDay,
    BOARD_ANM_telopTimeNight,
};

static u32 telopTimeChangeBackFileTbl[2] = {
    BOARD_ANM_telopTimeBackDay,
    BOARD_ANM_telopTimeBackNight,
};

static u32 telopTimeChangeFileTbl[2] = {
    BOARD_ANM_telopTimeDay,
    BOARD_ANM_telopTimeNight,
};

static OMOBJ *telopTimeChangeOMObj;
static OMOBJ *tauntOMObj;
static OMOBJ *telopTimeOMObj;
static OMOBJ *telopLastTurnOMObj;
static OMOBJ *telopOMObj;

static void TelopInitOMExec(OMOBJ *obj);
static void TelopOMExec(OMOBJ *obj);
static void TelopLastTurnOMExec(OMOBJ *obj);
static void TelopLastTurnPauseHook(BOOL dispF);
static void TelopTimeOMExec(OMOBJ *obj);
static void TelopTimePauseHook(BOOL dispF);
static void TauntOMExec(OMOBJ *obj);
static void TelopTimeChangeOMExec(OMOBJ *obj);
s32 mbLanguageGet(void);
s16 mbTelopTimeSprCreate(void);
void mbTelopTimeSprKill(s16 grpId);
void mbTelopTimeSprRotSet(s16 grpId, float rot);
void mbTelopTimeStarSet(s16 grpId, s32 starNum);
void mbTelopTimeTPLvlSet(s16 grpId, float tpLvl);
void mbTelopTimeDispSet(s16 grpId, BOOL dispF);
extern float mbSinDeg(float angle);
extern float mbCosDeg(float angle);

/* Creates a character or board speech bubble; callers wait for its object to be deleted when waitF
 * is true. */
void mbTelopCreate(int playerNo, int telopNo, BOOL waitF)
{
    TELOP_WORK *telopWork;

    telopOMObj = omAddObj(mbObjMan, 262, 1, 0, TelopInitOMExec);
    omSetStatBit(telopOMObj, OM_STAT_MODELPAUSE);
    telopWork = omObjGetWork(telopOMObj, TELOP_WORK);
    telopWork->killF = FALSE;
    telopWork->playerNo = playerNo;
    telopWork->mode = 0;
    telopWork->telopNo = telopNo;
    /* Computer and automatic notices get a short fixed delay before dismissal. */
    if ((playerNo >= 0 && GwPlayer[playerNo].comF) || playerNo < 0) {
        telopWork->dismissDelay = 30;
    } else {
        telopWork->dismissDelay = 0;
    }
    telopOMObj->mdlId[0] = espEntry(mbBoardDataNumGet(telopFileTbl[telopNo]), 100, 0);
    espDrawNoSet(telopOMObj->mdlId[0], 32);
    if (telopNo < 16) {
        mbAudFXPlay(TELOP_SE_CHARACTER_APPEAR);
    }
    if (waitF) {
        while (telopOMObj) {
            HuPrcVSleep();
        }
    }
}

/* Shows the character bubble for a player during the board turn sequence. */
void mbTelopPlayerCreate(int playerNo)
{
    mbTelopCreate(playerNo, GwPlayer[playerNo].charNo, TRUE);
}

/* Creates a nonblocking automatic notice using the board notice index derived from its argument. */
void mbTelopPlayerSkipCreate(int boardNo)
{
    mbTelopCreate(-1, boardNo + 16, FALSE);
}

/* The object manager calls this once before handing the bubble to TelopOMExec. */
static void TelopInitOMExec(OMOBJ *obj)
{
    TELOP_WORK *telopWork;

    telopWork = omObjGetWork(obj, TELOP_WORK);
    telopWork->mode = 0;
    telopWork->time = 0;
    telopWork->maxTime = telopWork->telopNo >= 16 ? 60 : 15;
    espPosSet(obj->mdlId[0], 288.0f, 240.0f);
    espTPLvlSet(obj->mdlId[0], 0.0f);
    espScaleSet(obj->mdlId[0], 0.0f, 0.0f);
    espDispOn(obj->mdlId[0]);
    obj->objFunc = TelopOMExec;
}

/* The object manager calls this each frame for bubble entry, input wait, and exit. */
static void TelopOMExec(OMOBJ *obj)
{
    TELOP_WORK *telopWork;
    float animationProgress;

    telopWork = omObjGetWork(obj, TELOP_WORK);
    if (telopWork->killF || mbExitCheck()) {
        espKill(obj->mdlId[0]);
        telopOMObj = NULL;
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    switch (telopWork->mode) {
        case 0:
            if (++telopWork->time >= telopWork->maxTime) {
                telopWork->mode = 1;
            }
            animationProgress = (float)telopWork->time / (float)telopWork->maxTime;
            espTPLvlSet(obj->mdlId[0], animationProgress);
            espScaleSet(obj->mdlId[0], animationProgress, animationProgress);
            break;
        case 1:
            if (telopWork->dismissDelay != 0) {
                telopWork->dismissDelay--;
                break;
            }
            if (telopWork->playerNo < 0) {
                /* Negative player numbers identify notices that dismiss automatically. */
                telopWork->mode = 2;
                telopWork->time = 0;
                telopWork->maxTime = telopWork->telopNo >= 16 ? 60 : 30;
                if (telopWork->telopNo < 16) {
                    mbAudFXPlay(TELOP_SE_CHARACTER_DISMISS);
                }
            } else {
                int playerPadNo = GwPlayer[telopWork->playerNo].padNo;

                if ((HuPadBtnDown[playerPadNo] & PAD_BUTTON_A)
                    || GwPlayer[telopWork->playerNo].comF) {
                    telopWork->mode = 2;
                    telopWork->time = 0;
                    telopWork->maxTime = telopWork->telopNo >= 16 ? 30 : 15;
                    if (telopWork->telopNo < 16) {
                        mbAudFXPlay(TELOP_SE_CHARACTER_DISMISS);
                    }
                }
            }
            break;
        case 2:
            if (++telopWork->time >= telopWork->maxTime) {
                telopWork->killF = TRUE;
            }
            animationProgress = (float)telopWork->time / (float)telopWork->maxTime;
            espTPLvlSet(obj->mdlId[0], 1.0f - animationProgress);
            espScaleSet(obj->mdlId[0], 1.0f + animationProgress, 1.0f + animationProgress);
            break;
    }
}

/* opening.c polls this while waiting for the current bubble object to be deleted. */
BOOL mbTelopCheck(void)
{
    return telopOMObj == NULL;
}

/* board.c calls this during the final five turns to show the remaining-turn notice. */
void mbTelopLastTurnCreate(void)
{
    TELOP_LAST_TURN_WORK *turnNoticeWork;
    s32 spriteIndex;
    s32 turnsRemaining;
    OMOBJ *turnNoticeObj;
    s32 languageIndex;
    s32 useLastTurnAssets;

    turnsRemaining = GwSystem.turnMax - GwSystem.turnNo;
    telopLastTurnOMObj = turnNoticeObj = omAddObj(
        mbObjMan, 0, 0, 0, TelopLastTurnOMExec);
    turnNoticeWork = omObjGetWork(turnNoticeObj, TELOP_LAST_TURN_WORK);
    turnNoticeWork->killF = FALSE;
    turnNoticeWork->delay = 0;
    turnNoticeWork->angle = 0;
    turnNoticeWork->grpId = HuSprGrpCreate(3);
    if (turnsRemaining == 0) {
        turnNoticeWork->lastTurn = TRUE;
        useLastTurnAssets = 1;
    } else {
        turnNoticeWork->lastTurn = FALSE;
        useLastTurnAssets = 0;
    }
    languageIndex = mbLanguageGet();
    for (spriteIndex = 0; spriteIndex < 3; spriteIndex++) {
        if (useLastTurnAssets) {
            mbSprCreate(mbBoardDataNumGet(telopTurnLastFileTbl[spriteIndex]),
                telopTurnSprPrioTbl[spriteIndex], NULL,
                &turnNoticeWork->sprId[spriteIndex]);
        } else {
            mbSprCreate(mbBoardDataNumGet(telopTurnFileTbl[spriteIndex]),
                telopTurnSprPrioTbl[spriteIndex], NULL,
                &turnNoticeWork->sprId[spriteIndex]);
        }
        HuSprGrpMemberSet(turnNoticeWork->grpId, spriteIndex,
            turnNoticeWork->sprId[spriteIndex]);
        HuSprAttrSet(turnNoticeWork->grpId, spriteIndex, HUSPR_ATTR_LINEAR);
        HuSprBankSet(turnNoticeWork->grpId, spriteIndex, telopTurnSprBankTbl[spriteIndex]);
        if (useLastTurnAssets) {
            HuSprPosSet(turnNoticeWork->grpId, spriteIndex,
                telopTurnLastSprOfsTbl[useLastTurnAssets][spriteIndex].x,
                telopTurnLastSprOfsTbl[useLastTurnAssets][spriteIndex].y);
        } else {
            HuSprPosSet(turnNoticeWork->grpId, spriteIndex,
                telopTurnSprOfsTbl[languageIndex][spriteIndex].x,
                telopTurnSprOfsTbl[languageIndex][spriteIndex].y);
        }
    }
    if (turnNoticeWork->lastTurn == FALSE) {
        /* The notice stores the zero-based turn number in the sprite bank. */
        HuSprBankSet(turnNoticeWork->grpId, 1, turnsRemaining - 1);
    } else {
        HuSprAttrSet(turnNoticeWork->grpId, 1, HUSPR_ATTR_DISPOFF);
    }
    turnNoticeObj->trans.x = 0.0f;
    HuSprGrpTPLvlSet(turnNoticeWork->grpId, turnNoticeObj->trans.x);
    HuSprGrpPosSet(turnNoticeWork->grpId, 288.0f, 96.0f);
    mbAudFXPlay(TELOP_SE_TIME_NOTICE);
    mbPauseHookPush(TelopLastTurnPauseHook);
}

/* The object manager calls this each frame until the turn notice and pause hook are removed. */
static void TelopLastTurnOMExec(OMOBJ *obj)
{
    TELOP_LAST_TURN_WORK *turnNoticeWork;
    float spriteScale;
    float sineValue;
    float animationProgress;

    turnNoticeWork = omObjGetWork(obj, TELOP_LAST_TURN_WORK);
    if (turnNoticeWork->killF || mbExitCheck()) {
        HuSprGrpKill(turnNoticeWork->grpId);
        mbPauseHookPop(TelopLastTurnPauseHook);
        telopLastTurnOMObj = NULL;
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    if (turnNoticeWork->delay != 0) {
        turnNoticeWork->delay--;
        return;
    }
    switch (turnNoticeWork->mode) {
        case 0:
            obj->trans.x += 1.0f / 30.0f;
            if (obj->trans.x > 1.0f) {
                obj->trans.x = 1.0f;
                turnNoticeWork->mode = 1;
            }
            HuSprGrpTPLvlSet(turnNoticeWork->grpId, obj->trans.x);
            break;
        case 1:
            animationProgress = turnNoticeWork->angle * (1.0f / 80.0f);
            sineValue = mbSinDeg(720.0f * animationProgress);
            sineValue = __fabsf(sineValue);
            spriteScale = sineValue;
            obj->trans.y = 1.0f + (0.5f * spriteScale);
            if (turnNoticeWork->lastTurn) {
                HuSprGrpScaleSet(turnNoticeWork->grpId, obj->trans.y, obj->trans.y);
            } else {
                HuSprScaleSet(turnNoticeWork->grpId, 1, obj->trans.y, obj->trans.y);
            }
            if (animationProgress >= 1.0f) {
                turnNoticeWork->mode = 2;
                turnNoticeWork->delay = 90;
            } else {
                turnNoticeWork->angle++;
            }
            break;
        case 2:
            obj->trans.x -= 1.0f / 30.0f;
            if (obj->trans.x < 0.0f) {
                obj->trans.x = 0.0f;
                turnNoticeWork->killF = TRUE;
            }
            HuSprGrpTPLvlSet(turnNoticeWork->grpId, obj->trans.x);
            break;
    }
}

/* The board pause hook calls this when the last-turn notice display state changes. */
static void TelopLastTurnPauseHook(BOOL dispF)
{
    TELOP_LAST_TURN_WORK *turnNoticeWork;
    s32 spriteIndex;

    turnNoticeWork = omObjGetWork(telopLastTurnOMObj, TELOP_LAST_TURN_WORK);
    for (spriteIndex = 0; spriteIndex < 3; spriteIndex++) {
        if (dispF) {
            HuSprAttrReset(turnNoticeWork->grpId, spriteIndex, HUSPR_ATTR_DISPOFF);
        } else {
            HuSprAttrSet(turnNoticeWork->grpId, spriteIndex, HUSPR_ATTR_DISPOFF);
        }
        if (turnNoticeWork->lastTurn) {
            HuSprAttrSet(turnNoticeWork->grpId, 1, HUSPR_ATTR_DISPOFF);
        }
    }
}

/* player.c calls this when a player's board turn begins. */
void mbTelopTimeCreate(void)
{
    OMOBJ *timerObj;
    TELOP_TIME_WORK *timerWork;

    telopTimeOMObj = timerObj = omAddObj(mbObjMan, 0, 1, 0, TelopTimeOMExec);
    omSetStatBit(timerObj, OM_STAT_MODELPAUSE);
    timerWork = omObjGetWork(timerObj, TELOP_TIME_WORK);
    timerWork->killF = FALSE;
    timerWork->time = timerWork->maxTime = 0;
    timerWork->mode = 0;
    timerWork->maxTime = 120;
    timerObj->mdlId[0] = mbTelopTimeSprCreate();
    HuSprGrpPosSet(timerObj->mdlId[0], 288.0f, 224.0f);
    mbTelopTimeTPLvlSet(timerObj->mdlId[0], 1.0f);
    mbPauseHookPush(TelopTimePauseHook);
}

/* The object manager calls this to rotate the timer in and then fade it out. */
static void TelopTimeOMExec(OMOBJ *obj)
{
    TELOP_TIME_WORK *timerWork;
    float rotation;
    float fadeProgress;

    timerWork = omObjGetWork(obj, TELOP_TIME_WORK);
    if (timerWork->killF || mbExitCheck()) {
        mbTelopTimeSprKill(obj->mdlId[0]);
        obj->mdlId[0] = -1;
        telopTimeOMObj = NULL;
        omDelObjEx(HuPrcCurrentGet(), obj);
        mbPauseHookPop(TelopTimePauseHook);
        return;
    }
    switch (timerWork->mode) {
        case 0:
            timerWork->time++;
            fadeProgress = (float)timerWork->time / (float)timerWork->maxTime;
            rotation = 2.0f * fadeProgress;
            mbTelopTimeSprRotSet(obj->mdlId[0], rotation);
            if (timerWork->time >= timerWork->maxTime) {
                timerWork->time = 0;
                timerWork->maxTime = 30;
                timerWork->mode++;
            }
            break;
        case 1:
            timerWork->time++;
            fadeProgress = (float)timerWork->time / (float)timerWork->maxTime;
            mbTelopTimeTPLvlSet(obj->mdlId[0], 1.0f - fadeProgress);
            if (timerWork->time >= timerWork->maxTime) {
                timerWork->mode++;
                timerWork->killF = TRUE;
            }
            break;
    }
}

/* Shows or hides the timer sprites while the board pause system is active. */
static void TelopTimePauseHook(BOOL dispF)
{
    if (telopTimeOMObj) {
        mbTelopTimeDispSet(telopTimeOMObj->mdlId[0], dispF);
    }
}

/* pause.c and mbTelopTimeCreate use this to build the day/night timer and star rows. */
s16 mbTelopTimeSprCreate(void)
{
    s32 starCount;
    s32 maxTimedTurns;
    s32 completedTimedTurns;
    s32 starsRemaining;
    s32 turnsRemaining;
    s32 languageIndex;
    s32 spriteIndex;
    s16 spriteGroupId;
    s16 spriteId;

    maxTimedTurns = GwSystem.timeTurnMax;
    /* The star row is limited by both the configured timer and turns still available. */
    starCount = maxTimedTurns;
    completedTimedTurns = GwSystem.timeTurn;
    starsRemaining = starCount - completedTimedTurns;
    turnsRemaining = GwSystem.turnMax - GwSystem.turnNo + 1;
    if (turnsRemaining < starsRemaining) {
        starsRemaining = turnsRemaining;
    }
    languageIndex = mbLanguageGet();
    spriteGroupId = HuSprGrpCreate(8);
    spriteIndex = 0;
    if (GwSystem.curTime) {
        spriteIndex++;
    }
    mbSprCreate(mbBoardDataNumGet(telopTimeBackFileTbl[spriteIndex]), 100, NULL, &spriteId);
    HuSprGrpMemberSet(spriteGroupId, 0, spriteId);
    HuSprTPLvlSet(spriteGroupId, 0, 0.6f);
    mbSprCreate(mbBoardDataNumGet(telopTimeFileTbl[spriteIndex]), 99, NULL, &spriteId);
    HuSprGrpMemberSet(spriteGroupId, 1, spriteId);
    for (spriteIndex = 0; spriteIndex < 3; spriteIndex++) {
        mbSprCreate(
            mbBoardDataNumGet(BOARD_ANM_telopTimeStar), 98, NULL, &spriteId);
        HuSprGrpMemberSet(spriteGroupId, spriteIndex + 2, spriteId);
        mbSprCreate(
            mbBoardDataNumGet(BOARD_ANM_telopTimeStar), 97, NULL, &spriteId);
        HuSprGrpMemberSet(spriteGroupId, spriteIndex + 5, spriteId);
        HuSprAttrSet(spriteGroupId, spriteIndex + 5, HUSPR_ATTR_LINEAR | HUSPR_ATTR_ADDCOL);
    }
    for (spriteIndex = 0; spriteIndex < 8; spriteIndex++) {
        HuSprAttrSet(spriteGroupId, spriteIndex, HUSPR_ATTR_LINEAR);
        HuSprPosSet(spriteGroupId, spriteIndex, telopTimeSprOfsTbl[spriteIndex].x,
            telopTimeSprOfsTbl[spriteIndex].y);
        HuSprScaleSet(spriteGroupId, spriteIndex, telopTimeSprScaleTbl[spriteIndex],
            telopTimeSprScaleTbl[spriteIndex]);
    }
    HuSprPosSet(spriteGroupId, 1, telopTimeNewSprOfsTbl[languageIndex].x,
        telopTimeNewSprOfsTbl[languageIndex].y);
    for (spriteIndex = 0; spriteIndex < 3; spriteIndex++) {
        HuSprPosSet(spriteGroupId, spriteIndex + 2,
            telopTimeStarSprOfsTbl[languageIndex][spriteIndex].x,
            telopTimeStarSprOfsTbl[languageIndex][spriteIndex].y);
        HuSprPosSet(spriteGroupId, spriteIndex + 5,
            telopTimeStarSprOfsTbl[languageIndex][spriteIndex].x,
            telopTimeStarSprOfsTbl[languageIndex][spriteIndex].y);
    }
    mbTelopTimeStarSet(spriteGroupId, starsRemaining);
    mbTelopTimeTPLvlSet(spriteGroupId, 1.0f);
    return spriteGroupId;
}

/* TelopTimeOMExec and pause.c call this when the timer sprite group is no longer needed. */
void mbTelopTimeSprKill(s16 grpId)
{
    HuSprGrpKill(grpId);
}

/* TelopTimeOMExec and pause.c call this for each timer rotation step. */
void mbTelopTimeSprRotSet(s16 grpId, float rot)
{
    s32 starIndex;
    HUSPRITE *starSprite;
    HUSPR_GROUP *group = &HuSprGrpData[grpId];
    BOOL starIsBanked;
    float secondSine;
    float firstSine;
    float firstSineAbs;
    float secondSineAbs;
    float secondScale;
    float wrappedRotation;

    for (starIndex = 0; starIndex < 3; starIndex++) {
        starIsBanked = FALSE;
        starSprite = &HuSprData[group->sprId[starIndex + 2]];
        if (starSprite->bank & 1) {
            starIsBanked = TRUE;
        }
        if (starIsBanked) {
            HuSprTPLvlSet(grpId, starIndex + 2, 0.8f);
            HuSprScaleSet(grpId, starIndex + 2, 1.0f, 1.0f);
            HuSprZRotSet(grpId, starIndex + 2, 0.0f);
            HuSprTPLvlSet(grpId, starIndex + 5, 0.0f);
            HuSprScaleSet(grpId, starIndex + 5, 0.0f, 0.0f);
        } else {
            firstSine = mbSinDeg(360.0f * rot);
            firstSine = __fabsf(firstSine);
            firstSineAbs = firstSine;
            {
                float frontScale = 1.0f + (0.2f * firstSineAbs);

                HuSprTPLvlSet(grpId, starIndex + 2, 1.0f);
                HuSprScaleSet(grpId, starIndex + 2, frontScale, frontScale);
                frontScale = 30.0f * mbSinDeg(360.0f * rot);
                HuSprZRotSet(grpId, starIndex + 2, frontScale);
                wrappedRotation = fmod(2.0f * rot, 1.0f);
                frontScale = wrappedRotation;
                secondSine = mbSinDeg(90.0f * frontScale);
                secondSine = __fabsf(secondSine);
                secondSineAbs = secondSine;
                secondScale = 1.0f + secondSineAbs;
                HuSprTPLvlSet(grpId, starIndex + 5, 0.8f * (1.0f - frontScale));
                HuSprScaleSet(grpId, starIndex + 5, secondScale, secondScale);
                if (frontScale < 0.001) {
                    HuSprScaleSet(grpId, starIndex + 5, 0.0f, 0.0f);
                }

                /* These repeated reads have no visible effect after both star faces are updated. */
                (void)firstSine;
                (void)firstSine;
                (void)firstSine;
                (void)firstSine;
                (void)firstSine;
                (void)firstSine;
                (void)secondSine;
                (void)secondSine;
                (void)secondSine;
                (void)secondSine;
                (void)secondSine;
            }
        }
    }
}

/* mbTelopTimeSprCreate calls this to select banks for each remaining-star position. */
void mbTelopTimeStarSet(s16 grpId, s32 starNum)
{
    s32 emptyStarCount = 3 - starNum;
    s32 starIndex;

    for (starIndex = 0; starIndex < 3; starIndex++) {
        s32 spriteBank = 0;

        if (GwSystem.curTime) {
            spriteBank += 2;
        }
        if (starIndex < emptyStarCount) {
            spriteBank++;
        }
        HuSprBankSet(grpId, starIndex + 2, spriteBank);
        HuSprBankSet(grpId, starIndex + 5, spriteBank);
        HuSprTPLvlSet(grpId, starIndex + 5, 0.0f);
    }
}

/* Timer creation and animation callbacks call this to apply sprite transparency. */
void mbTelopTimeTPLvlSet(s16 grpId, float tpLvl)
{
    s32 spriteIndex;

    for (spriteIndex = 0; spriteIndex < 8; spriteIndex++) {
        HuSprTPLvlSet(grpId, spriteIndex, tpLvl * telopTimeBaseTPLvlTbl[spriteIndex]);
    }
}

/* pause.c and TelopTimePauseHook call this during pause display transitions. */
void mbTelopTimeDispSet(s16 grpId, BOOL dispF)
{
    s32 spriteIndex;

    for (spriteIndex = 0; spriteIndex < 8; spriteIndex++) {
        if (dispF) {
            HuSprDispOn(grpId, spriteIndex);
        } else {
            HuSprDispOff(grpId, spriteIndex);
        }
    }
}

/* Branch and board event callers pass a pad number; this returns the stronger horizontal main-stick
 * or C-stick value, or 0 when its magnitude is below 8. */
s8 mbPadStkXGet(s32 padNo)
{
    s8 mainStickX = HuPadStkX[padNo];
    s8 cStickX = HuPadSubStkX[padNo];

    if (abs(mainStickX) > abs(cStickX)) {
        if (abs(mainStickX) < 8) {
            return 0;
        } else {
            return mainStickX;
        }
    } else {
        if (abs(cStickX) < 8) {
            return 0;
        } else {
            return cStickX;
        }
    }
}

/* Branch and board event callers pass a pad number; this returns the stronger vertical main-stick
 * or C-stick value, or 0 when its magnitude is below 8. */
s8 mbPadStkYGet(s32 padNo)
{
    s8 mainStickY = HuPadStkY[padNo];
    s8 cStickY = HuPadSubStkY[padNo];

    if (abs(mainStickY) > abs(cStickY)) {
        if (abs(mainStickY) < 8) {
            return 0;
        } else {
            return mainStickY;
        }
    } else {
        if (abs(cStickY) < 8) {
            return 0;
        } else {
            return cStickY;
        }
    }
}

/* Board setup starts the taunt watcher; it accepts input only after player turn code clears
 * FLAG_BOARD_WALKDONE. */
void mbTauntInit(void)
{
    s32 playerIndex;

    tauntOMObj = omAddObj(mbObjMan, 32256, 0, 0, TauntOMExec);
    for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
        tauntSeNo[playerIndex] = MSM_SENO_NONE;
    }
    _SetFlag(FLAG_BOARD_WALKDONE);
}

/* Board teardown requests watcher shutdown; its next callback stops active taunt sounds and deletes
 * the watcher. */
void mbTauntClose(void)
{
    if (tauntOMObj) {
        TAUNT_WORK *tauntWork = omObjGetWork(tauntOMObj, TAUNT_WORK);
        tauntWork->killF = TRUE;
        _SetFlag(FLAG_BOARD_WALKDONE);
    }
}

/* During eligible board play, starts an off-turn human player's taunt voice when L is pressed,
 * after the pause, wipe, tutorial, and character-unlock checks. */
static void TauntOMExec(OMOBJ *obj)
{
    int playerPadNo;
    int characterNo;
    TAUNT_WORK *tauntWork;
    BOOL characterEnabled;
    int playerIndex;

    tauntWork = omObjGetWork(obj, TAUNT_WORK);
    if (tauntWork->killF || mbExitCheck()) {
        for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
            if (tauntSeNo[playerIndex] >= 0) {
                mbAudFXStop(tauntSeNo[playerIndex]);
                tauntSeNo[playerIndex] = MSM_SENO_NONE;
            }
        }
        tauntOMObj = NULL;
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
        if (tauntSeNo[playerIndex] >= 0
            && HuAudFXStatusGet(tauntSeNo[playerIndex]) == MSM_SE_DONE) {
            tauntSeNo[playerIndex] = MSM_SENO_NONE;
        }
    }
    if (mbPauseProcCheck() || _CheckFlag(FLAG_BOARD_WALKDONE) || WipeCheck()
        || GwSystem.turnPlayerNo == -1 || GWPartyGet() == FALSE
        || _CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return;
    }
    for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
        if (playerIndex == GwSystem.turnPlayerNo) {
            continue;
        }
        if (GwPlayer[playerIndex].comF) {
            continue;
        }
        playerPadNo = GwPlayer[playerIndex].padNo;
        characterNo = GwPlayer[playerIndex].charNo;
        /* Character slot 10 is always available and has no unlock flag. */
        characterEnabled = characterNo == BOARD_TAUNT_ALWAYS_ENABLED_CHAR
            ? TRUE
            : GWBankFlagGet(characterNo + GW_BANK_FLAG_CHARACTER_BASE);
        if (characterEnabled == FALSE) {
            continue;
        }
        if (tauntSeNo[playerPadNo] < 0
            && (HuPadBtnDown[playerPadNo] & PAD_TRIGGER_L)) {
            tauntSeNo[playerPadNo] = mbAudFXPlay((s16)tauntSeTbl[characterNo]);
        }
    }
}

/* Board asset callers use this to convert the system language to an asset table index. */
s32 mbLanguageGet(void)
{
    static s32 languageTbl[6][2] = {
        { 0, 0 },
        { 1, 1 },
        { 2, 2 },
        { 3, 3 },
        { 4, 4 },
        { 5, 5 },
    };
    s32 systemLanguage = GWLanguageGet();
    s32 languageIndex;

    for (languageIndex = 0; languageIndex < 6; languageIndex++) {
        if (systemLanguage == languageTbl[languageIndex][0]) {
            break;
        }
    }
    return languageTbl[languageIndex][1];
}

/* Board setup calls this to store a selected language through the game language table. */
void mbLanguageSet(s32 languageNo)
{
    static s32 languageTbl[6][2] = {
        { 0, 0 },
        { 1, 1 },
        { 2, 2 },
        { 3, 3 },
        { 4, 4 },
        { 5, 5 },
    };
    s32 languageIndex;

    for (languageIndex = 0; languageIndex < 6; languageIndex++) {
        if (languageNo == languageTbl[languageIndex][0]) {
            break;
        }
    }
    GWLanguageSet(languageTbl[languageIndex][1]);
}

static u32 boardDataDirTbl[6] = {
    DATA_board,
    DATA_board_us,
    DATA_board,
    DATA_board,
    DATA_board,
    DATA_board,
};

static HuVec2f telopTimeChangeSprOfsTbl[8] = {
    { 0.0f, 0.0f },
    { 0.0f, 0.0f },
    { 0.0f, 0.0f },
    { 0.0f, 16.0f },
    { 0.0f, 16.0f },
    { 0.0f, 0.0f },
    { 0.0f, 0.0f },
    { 0.0f, 0.0f },
};

/* Board asset loading calls this for a language archive, clamping negative indices to English. */
static inline u32 BoardDataDirGet(s32 boardNo)
{
    if (boardNo < 0) {
        boardNo = 0;
    }
    return boardDataDirTbl[boardNo];
}

/* Board sprite and effect callers use this to rebuild a language-specific asset number. */
int mbBoardDataNumGet(int dataNum)
{
    s32 languageNo = mbLanguageGet();

    if (DIRNUM(dataNum) != DATA_board) {
        return mbPauseDataNumGet(dataNum);
    }
    if (languageNo < 0) {
        languageNo = 0;
    }
    return FILENUM(dataNum) | BoardDataDirGet(languageNo);
}

/* board.c calls this during board setup to load the current archive and release other languages. */
void mbBoardDataDirRead(void)
{
    s32 languageIndex = mbLanguageGet();
    s32 archiveIndex;

    if (languageIndex < 0) {
        languageIndex = 0;
    }
    for (archiveIndex = 0; archiveIndex < 6; archiveIndex++) {
        if (BoardDataDirGet(archiveIndex) == BoardDataDirGet(languageIndex)) {
            continue;
        }
        if ((void *)HuARDirCheck(BoardDataDirGet(archiveIndex)) != NULL) {
            HuARDirFree(BoardDataDirGet(archiveIndex));
        }
        if (HuDataReadChk(BoardDataDirGet(archiveIndex)) >= 0) {
            HuDataDirClose(BoardDataDirGet(archiveIndex));
        }
    }
    if ((void *)HuARDirCheck(BoardDataDirGet(languageIndex)) == NULL) {
        HuAR_DVDtoARAM(BoardDataDirGet(languageIndex));
        while (HuARDMACheck()) {
        }
    }
}

/* The time-change notice uses this to read the number of timed turns in the board game. */
static inline s32 TelopTimeTurnMaxGet(void)
{
    return GwSystem.timeTurnMax;
}

/* The time-change notice uses this to read the number of timed turns already completed. */
static inline s32 TelopTimeTurnGet(void)
{
    return GwSystem.timeTurn;
}

/* board.c calls this when the board transitions between day and night. */
void mbTelopTimeChangeCreate(void)
{
    s32 starCount;
    s32 starsRemaining;
    s32 languageIndex;
    s32 spriteIndex;
    s32 starIndex;
    OMOBJ *timeChangeObj;
    TELOP_TIME_CHANGE_WORK *timeChangeWork;

    telopTimeChangeOMObj = timeChangeObj = omAddObj(
        mbObjMan, 0, 8, 0, TelopTimeChangeOMExec);
    omSetStatBit(timeChangeObj, OM_STAT_MODELPAUSE);
    timeChangeWork = omObjGetWork(timeChangeObj, TELOP_TIME_CHANGE_WORK);
    timeChangeWork->killF = FALSE;
    timeChangeWork->mode = 0;
    timeChangeWork->time = 0;
    timeChangeWork->completeF = FALSE;
    timeChangeWork->maxTime = 16;

    languageIndex = mbLanguageGet();
    starCount = TelopTimeTurnMaxGet();
    starsRemaining = starCount - TelopTimeTurnGet();

    spriteIndex = GwSystem.turnMax - GwSystem.turnNo + 1;
    if (spriteIndex < starsRemaining) {
        starsRemaining = spriteIndex;
    }
    timeChangeObj->trans.x = 0.0f;

    timeChangeObj->mdlId[0] = espEntry(
        mbBoardDataNumGet(telopTimeChangeBackFileTbl[GwSystem.nextTime]), 101, 0);
    timeChangeObj->mdlId[1] = espEntry(
        mbBoardDataNumGet(telopTimeChangeBackFileTbl[GwSystem.curTime]), 99, 0);
    timeChangeObj->mdlId[2] = espEntry(
        mbBoardDataNumGet(telopTimeChangeBackFileTbl[GwSystem.curTime]), 97, 0);
    timeChangeObj->mdlId[3] = espEntry(
        mbBoardDataNumGet(telopTimeChangeFileTbl[GwSystem.nextTime]), 100, 0);
    timeChangeObj->mdlId[4] = espEntry(
        mbBoardDataNumGet(telopTimeChangeFileTbl[GwSystem.curTime]), 96, 0);
    for (spriteIndex = 0; spriteIndex < 5; spriteIndex++) {
        espAttrSet(timeChangeObj->mdlId[spriteIndex], HUSPR_ATTR_LINEAR);
        espPosSet(timeChangeObj->mdlId[spriteIndex],
            288.0f + telopTimeChangeSprOfsTbl[spriteIndex].x,
            240.0f + telopTimeChangeSprOfsTbl[spriteIndex].y);
        espTPLvlSet(timeChangeObj->mdlId[spriteIndex], 0.0f);
    }
    espPosSet(timeChangeObj->mdlId[3], 288.0f + telopTimeNewSprOfsTbl[languageIndex].x,
        240.0f + telopTimeNewSprOfsTbl[languageIndex].y);
    espPosSet(timeChangeObj->mdlId[4], 288.0f + telopTimeNewSprOfsTbl[languageIndex].x,
        240.0f + telopTimeNewSprOfsTbl[languageIndex].y);

    starsRemaining = 3 - starsRemaining;
    for (starIndex = 0; starIndex < 3; starIndex++) {
        spriteIndex = 0;
        if (GwSystem.curTime) {
            spriteIndex += 2;
        }
        if (starIndex < starsRemaining) {
            spriteIndex++;
        }
        timeChangeObj->mdlId[starIndex + 5] = espEntry(
            mbBoardDataNumGet(BOARD_ANM_telopTimeStar), 98, spriteIndex);
        espAttrSet(timeChangeObj->mdlId[starIndex + 5], HUSPR_ATTR_LINEAR);
        espDispOff(timeChangeObj->mdlId[starIndex + 5]);
        espPosSet(timeChangeObj->mdlId[starIndex + 5],
            288.0f + telopTimeStarSprOfsTbl[languageIndex][starIndex].x,
            240.0f + telopTimeStarSprOfsTbl[languageIndex][starIndex].y);
        espScaleSet(timeChangeObj->mdlId[starIndex + 5], 0.75f, 0.75f);
    }
    mbAudFXPlay(TELOP_SE_TIME_NOTICE);
}

/* The object manager calls this each frame through the day/night notice phases. */
static void TelopTimeChangeOMExec(OMOBJ *obj)
{
    TELOP_TIME_CHANGE_WORK *timeChangeWork;
    float animationProgress;
    float rotationAngle;
    float spriteScale;
    float spritePosY;
    s32 languageIndex;

    timeChangeWork = omObjGetWork(obj, TELOP_TIME_CHANGE_WORK);
    if (timeChangeWork->killF || mbExitCheck()) {
        s32 spriteIndex;

        for (spriteIndex = 0; spriteIndex < 8; spriteIndex++) {
            espKill(obj->mdlId[spriteIndex]);
            obj->mdlId[spriteIndex] = 0;
        }
        telopTimeChangeOMObj = NULL;
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    languageIndex = mbLanguageGet();
    timeChangeWork->time++;
    if (timeChangeWork->time > timeChangeWork->maxTime) {
        timeChangeWork->time = timeChangeWork->maxTime;
    }
    animationProgress = (float)timeChangeWork->time / (float)timeChangeWork->maxTime;
    {
        s32 starIndex;

        switch (timeChangeWork->mode) {
        case 0:
            espTPLvlSet(obj->mdlId[0], animationProgress);
            espTPLvlSet(obj->mdlId[3], animationProgress);
            obj->trans.x = 288.0f;
            obj->trans.y = 256.0f;
            espPosSet(obj->mdlId[3], obj->trans.x, obj->trans.y);
            espPosSet(obj->mdlId[4], obj->trans.x, obj->trans.y);
            if (timeChangeWork->time >= timeChangeWork->maxTime) {
                obj->trans.z = 2.0f;
                timeChangeWork->time = 0;
                timeChangeWork->maxTime = 40;
                timeChangeWork->mode++;
            }
            break;
        case 1:
            rotationAngle = 90.0f * animationProgress;
            spriteScale = 1.0f - (0.75f * animationProgress);
            espTPLvlSet(obj->mdlId[0], 1.0f - animationProgress);
            espTPLvlSet(obj->mdlId[3], 1.0f - animationProgress);
            spritePosY = 720.0f - (480.0f * mbSinDeg(90.0f - rotationAngle));
            espPosSet(obj->mdlId[0],
                288.0f + (480.0f * mbCosDeg(90.0f - rotationAngle)), spritePosY);
            espZRotSet(obj->mdlId[0], 4.0f * rotationAngle);
            espScaleSet(obj->mdlId[0], spriteScale, spriteScale);
            spriteScale = 1.0f - (0.75f * (1.0f - animationProgress));
            espTPLvlSet(obj->mdlId[1], animationProgress);
            spritePosY = 720.0f - (480.0f * mbSinDeg(180.0f - rotationAngle));
            espPosSet(obj->mdlId[1],
                288.0f + (480.0f * mbCosDeg(180.0f - rotationAngle)), spritePosY);
            espZRotSet(obj->mdlId[1], 4.0f * (rotationAngle - 90.0f));
            espScaleSet(obj->mdlId[1], spriteScale, spriteScale);
            spriteScale = 1.0f - (0.75f * animationProgress);
            espZRotSet(obj->mdlId[3], -rotationAngle);
            obj->trans.z += 0.5f;
            obj->trans.y += obj->trans.z;
            espPosSet(obj->mdlId[3], obj->trans.x, obj->trans.y);
            espScaleSet(obj->mdlId[3], spriteScale, spriteScale);
            if (timeChangeWork->time + 12 > timeChangeWork->maxTime) {
                rotationAngle = (float)(timeChangeWork->maxTime - timeChangeWork->time);
                rotationAngle *= 0.083333336f;
                spriteScale = 1.0f + (2.0f * rotationAngle);
                espScaleSet(obj->mdlId[4], spriteScale, spriteScale);
                espTPLvlSet(obj->mdlId[4], 1.0f - rotationAngle);
            }
            if (timeChangeWork->time >= timeChangeWork->maxTime) {
                espAttrSet(obj->mdlId[2], HUSPR_ATTR_ADDCOL);
                timeChangeWork->time = 0;
                timeChangeWork->maxTime = 30;
                timeChangeWork->mode++;
            }
            break;
        case 2:
            spriteScale = 1.0f + (0.5f * animationProgress);
            espTPLvlSet(obj->mdlId[2], 1.0f - animationProgress);
            espScaleSet(obj->mdlId[2], spriteScale, spriteScale);
            for (starIndex = 0; starIndex < 3; starIndex++) {
                espDispOn(obj->mdlId[starIndex + 5]);
            }
            if (timeChangeWork->time < timeChangeWork->maxTime) {
                break;
            }
            timeChangeWork->time = 0;
            timeChangeWork->maxTime = 30;
            timeChangeWork->mode++;
        case 3:
            if (timeChangeWork->time >= timeChangeWork->maxTime) {
                timeChangeWork->mode = 4;
            }
            break;
        case 4:
            timeChangeWork->completeF = TRUE;
            if (WipeCheck()) {
                timeChangeWork->time = 0;
                timeChangeWork->maxTime = 30;
                timeChangeWork->mode = 5;
            }
            break;
        case 5:
            if (WipeCheck() == 0) {
                obj->trans.x = 504.0f;
                obj->trans.y = 84.0f;
                espPosSet(obj->mdlId[1], obj->trans.x, obj->trans.y);
                espPosSet(obj->mdlId[4],
                    obj->trans.x + (0.5f * telopTimeNewSprOfsTbl[languageIndex].x),
                    obj->trans.y + (0.5f * telopTimeNewSprOfsTbl[languageIndex].y));
                espScaleSet(obj->mdlId[1], 0.5f, 0.5f);
                espScaleSet(obj->mdlId[4], 0.5f, 0.5f);
                espTPLvlSet(obj->mdlId[1], 0.65f);
                for (starIndex = 0; starIndex < 3; starIndex++) {
                    espPosSet(obj->mdlId[starIndex + 5],
                        obj->trans.x + (0.5f * telopTimeStarSprOfsTbl[languageIndex][starIndex].x),
                        obj->trans.y + (0.5f * telopTimeStarSprOfsTbl[languageIndex][starIndex].y));
                    espScaleSet(obj->mdlId[starIndex + 5], 0.375f, 0.375f);
                    espTPLvlSet(obj->mdlId[starIndex + 5], 0.85f);
                }
            }
            break;
        case 10:
            espTPLvlSet(obj->mdlId[1], 1.0f - animationProgress);
            espTPLvlSet(obj->mdlId[4], 1.0f - animationProgress);
            if (timeChangeWork->time >= timeChangeWork->maxTime) {
                timeChangeWork->mode++;
                timeChangeWork->killF = TRUE;
            }
            break;
        }
    }
}

/* board.c calls this to request cleanup of the time-of-day notice on its next callback. */
void mbTelopTimeChangeKill(void)
{
    if (telopTimeChangeOMObj) {
        TELOP_TIME_CHANGE_WORK *timeChangeWork = omObjGetWork(
            telopTimeChangeOMObj, TELOP_TIME_CHANGE_WORK);
        timeChangeWork->killF = TRUE;
    }
}

/* board.c and world01.c poll this until the time-of-day notice reaches its completion state. */
BOOL mbTelopTimeChangeCheck(void)
{
    TELOP_TIME_CHANGE_WORK *timeChangeWork;

    if (telopTimeChangeOMObj == NULL) {
        return FALSE;
    }
    timeChangeWork = omObjGetWork(telopTimeChangeOMObj, TELOP_TIME_CHANGE_WORK);
    return timeChangeWork->completeF == FALSE;
}
