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

typedef struct TauntWork_s {
    unsigned killF : 1;
} TAUNT_WORK;

typedef struct TelopWork_s {
    unsigned killF : 1;
    unsigned mode : 3;
    s8 playerNo;
    s8 telopNo;
    s8 unk3;
    s8 comDelay;
    s16 time;
    s16 maxTime;
} TELOP_WORK;

typedef struct TelopLastTurnWork_s {
    unsigned killF : 1;
    unsigned mode : 3;
    unsigned lastTurn : 1;
    u8 delay;
    s16 angle;
    s16 grpId;
    s16 sprId[3];
} TELOP_LAST_TURN_WORK;

typedef struct TelopTimeWork_s {
    unsigned killF : 1;
    unsigned mode : 3;
    s16 time;
    s16 maxTime;
} TELOP_TIME_WORK;

typedef struct TelopTimeChangeWork_s {
    unsigned killF : 1;
    unsigned completeF : 1;
    unsigned mode : 4;
    s16 time;
    s16 maxTime;
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

void mbTelopCreate(int playerNo, int telopNo, BOOL waitF)
{
    TELOP_WORK *work;

    telopOMObj = omAddObj(mbObjMan, 262, 1, 0, TelopInitOMExec);
    omSetStatBit(telopOMObj, OM_STAT_MODELPAUSE);
    work = omObjGetWork(telopOMObj, TELOP_WORK);
    work->killF = FALSE;
    work->playerNo = playerNo;
    work->mode = 0;
    work->telopNo = telopNo;
    if ((playerNo >= 0 && GwPlayer[playerNo].comF) || playerNo < 0) {
        work->comDelay = 30;
    } else {
        work->comDelay = 0;
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

void mbTelopPlayerCreate(int playerNo)
{
    mbTelopCreate(playerNo, GwPlayer[playerNo].charNo, TRUE);
}

void mbTelopPlayerSkipCreate(int playerNo)
{
    mbTelopCreate(-1, playerNo + 16, FALSE);
}

static void TelopInitOMExec(OMOBJ *obj)
{
    TELOP_WORK *work;

    work = omObjGetWork(obj, TELOP_WORK);
    work->mode = 0;
    work->time = 0;
    work->maxTime = work->telopNo >= 16 ? 60 : 15;
    espPosSet(obj->mdlId[0], 288.0f, 240.0f);
    espTPLvlSet(obj->mdlId[0], 0.0f);
    espScaleSet(obj->mdlId[0], 0.0f, 0.0f);
    espDispOn(obj->mdlId[0]);
    obj->objFunc = TelopOMExec;
}

static void TelopOMExec(OMOBJ *obj)
{
    TELOP_WORK *work;
    float weight;

    work = omObjGetWork(obj, TELOP_WORK);
    if (work->killF || mbExitCheck()) {
        espKill(obj->mdlId[0]);
        telopOMObj = NULL;
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    switch (work->mode) {
        case 0:
            if (++work->time >= work->maxTime) {
                work->mode = 1;
            }
            weight = (float)work->time / (float)work->maxTime;
            espTPLvlSet(obj->mdlId[0], weight);
            espScaleSet(obj->mdlId[0], weight, weight);
            break;
        case 1:
            if (work->comDelay != 0) {
                work->comDelay--;
                break;
            }
            if (work->playerNo < 0) {
                work->mode = 2;
                work->time = 0;
                work->maxTime = work->telopNo >= 16 ? 60 : 30;
                if (work->telopNo < 16) {
                    mbAudFXPlay(TELOP_SE_CHARACTER_DISMISS);
                }
            } else {
                int padNo = GwPlayer[work->playerNo].padNo;

                if ((HuPadBtnDown[padNo] & PAD_BUTTON_A) || GwPlayer[work->playerNo].comF) {
                    work->mode = 2;
                    work->time = 0;
                    work->maxTime = work->telopNo >= 16 ? 30 : 15;
                    if (work->telopNo < 16) {
                        mbAudFXPlay(TELOP_SE_CHARACTER_DISMISS);
                    }
                }
            }
            break;
        case 2:
            if (++work->time >= work->maxTime) {
                work->killF = TRUE;
            }
            weight = (float)work->time / (float)work->maxTime;
            espTPLvlSet(obj->mdlId[0], 1.0f - weight);
            espScaleSet(obj->mdlId[0], 1.0f + weight, 1.0f + weight);
            break;
    }
}

BOOL mbTelopCheck(void)
{
    return telopOMObj == NULL;
}

void mbTelopLastTurnCreate(void)
{
    TELOP_LAST_TURN_WORK *work;
    s32 i;
    s32 turnLeft;
    OMOBJ *obj;
    s32 languageNo;
    s32 type;

    turnLeft = GwSystem.turnMax - GwSystem.turnNo;
    telopLastTurnOMObj = obj = omAddObj(mbObjMan, 0, 0, 0, TelopLastTurnOMExec);
    work = omObjGetWork(obj, TELOP_LAST_TURN_WORK);
    work->killF = FALSE;
    work->delay = 0;
    work->angle = 0;
    work->grpId = HuSprGrpCreate(3);
    if (turnLeft == 0) {
        work->lastTurn = TRUE;
        type = 1;
    } else {
        work->lastTurn = FALSE;
        type = 0;
    }
    languageNo = mbLanguageGet();
    for (i = 0; i < 3; i++) {
        if (type) {
            mbSprCreate(mbBoardDataNumGet(telopTurnLastFileTbl[i]), telopTurnSprPrioTbl[i], NULL,
                &work->sprId[i]);
        } else {
            mbSprCreate(mbBoardDataNumGet(telopTurnFileTbl[i]), telopTurnSprPrioTbl[i], NULL,
                &work->sprId[i]);
        }
        HuSprGrpMemberSet(work->grpId, i, work->sprId[i]);
        HuSprAttrSet(work->grpId, i, HUSPR_ATTR_LINEAR);
        HuSprBankSet(work->grpId, i, telopTurnSprBankTbl[i]);
        if (type) {
            HuSprPosSet(work->grpId, i, telopTurnLastSprOfsTbl[type][i].x,
                telopTurnLastSprOfsTbl[type][i].y);
        } else {
            HuSprPosSet(work->grpId, i, telopTurnSprOfsTbl[languageNo][i].x,
                telopTurnSprOfsTbl[languageNo][i].y);
        }
    }
    if (work->lastTurn == FALSE) {
        HuSprBankSet(work->grpId, 1, turnLeft - 1);
    } else {
        HuSprAttrSet(work->grpId, 1, HUSPR_ATTR_DISPOFF);
    }
    obj->trans.x = 0.0f;
    HuSprGrpTPLvlSet(work->grpId, obj->trans.x);
    HuSprGrpPosSet(work->grpId, 288.0f, 96.0f);
    mbAudFXPlay(TELOP_SE_TIME_NOTICE);
    mbPauseHookPush(TelopLastTurnPauseHook);
}

static void TelopLastTurnOMExec(OMOBJ *obj)
{
    TELOP_LAST_TURN_WORK *work;
    float scale;
    float sinValue;
    float weight;

    work = omObjGetWork(obj, TELOP_LAST_TURN_WORK);
    if (work->killF || mbExitCheck()) {
        HuSprGrpKill(work->grpId);
        mbPauseHookPop(TelopLastTurnPauseHook);
        telopLastTurnOMObj = NULL;
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    if (work->delay != 0) {
        work->delay--;
        return;
    }
    switch (work->mode) {
        case 0:
            obj->trans.x += 1.0f / 30.0f;
            if (obj->trans.x > 1.0f) {
                obj->trans.x = 1.0f;
                work->mode = 1;
            }
            HuSprGrpTPLvlSet(work->grpId, obj->trans.x);
            break;
        case 1:
            weight = work->angle * (1.0f / 80.0f);
            sinValue = mbSinDeg(720.0f * weight);
            sinValue = __fabsf(sinValue);
            scale = sinValue;
            obj->trans.y = 1.0f + (0.5f * scale);
            if (work->lastTurn) {
                HuSprGrpScaleSet(work->grpId, obj->trans.y, obj->trans.y);
            } else {
                HuSprScaleSet(work->grpId, 1, obj->trans.y, obj->trans.y);
            }
            if (weight >= 1.0f) {
                work->mode = 2;
                work->delay = 90;
            } else {
                work->angle++;
            }
            break;
        case 2:
            obj->trans.x -= 1.0f / 30.0f;
            if (obj->trans.x < 0.0f) {
                obj->trans.x = 0.0f;
                work->killF = TRUE;
            }
            HuSprGrpTPLvlSet(work->grpId, obj->trans.x);
            break;
    }
}

static void TelopLastTurnPauseHook(BOOL dispF)
{
    TELOP_LAST_TURN_WORK *work;
    s32 i;

    work = omObjGetWork(telopLastTurnOMObj, TELOP_LAST_TURN_WORK);
    for (i = 0; i < 3; i++) {
        if (dispF) {
            HuSprAttrReset(work->grpId, i, HUSPR_ATTR_DISPOFF);
        } else {
            HuSprAttrSet(work->grpId, i, HUSPR_ATTR_DISPOFF);
        }
        if (work->lastTurn) {
            HuSprAttrSet(work->grpId, 1, HUSPR_ATTR_DISPOFF);
        }
    }
}

void mbTelopTimeCreate(void)
{
    OMOBJ *obj;
    TELOP_TIME_WORK *work;

    telopTimeOMObj = obj = omAddObj(mbObjMan, 0, 1, 0, TelopTimeOMExec);
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    work = omObjGetWork(obj, TELOP_TIME_WORK);
    work->killF = FALSE;
    work->time = work->maxTime = 0;
    work->mode = 0;
    work->maxTime = 120;
    obj->mdlId[0] = mbTelopTimeSprCreate();
    HuSprGrpPosSet(obj->mdlId[0], 288.0f, 224.0f);
    mbTelopTimeTPLvlSet(obj->mdlId[0], 1.0f);
    mbPauseHookPush(TelopTimePauseHook);
}

static void TelopTimeOMExec(OMOBJ *obj)
{
    TELOP_TIME_WORK *work;
    float rot;
    float weight;

    work = omObjGetWork(obj, TELOP_TIME_WORK);
    if (work->killF || mbExitCheck()) {
        mbTelopTimeSprKill(obj->mdlId[0]);
        obj->mdlId[0] = -1;
        telopTimeOMObj = NULL;
        omDelObjEx(HuPrcCurrentGet(), obj);
        mbPauseHookPop(TelopTimePauseHook);
        return;
    }
    switch (work->mode) {
        case 0:
            work->time++;
            weight = (float)work->time / (float)work->maxTime;
            rot = 2.0f * weight;
            mbTelopTimeSprRotSet(obj->mdlId[0], rot);
            if (work->time >= work->maxTime) {
                work->time = 0;
                work->maxTime = 30;
                work->mode++;
            }
            break;
        case 1:
            work->time++;
            weight = (float)work->time / (float)work->maxTime;
            mbTelopTimeTPLvlSet(obj->mdlId[0], 1.0f - weight);
            if (work->time >= work->maxTime) {
                work->mode++;
                work->killF = TRUE;
            }
            break;
    }
}

static void TelopTimePauseHook(BOOL dispF)
{
    if (telopTimeOMObj) {
        mbTelopTimeDispSet(telopTimeOMObj->mdlId[0], dispF);
    }
}

s16 mbTelopTimeSprCreate(void)
{
    s32 starNum;
    s32 timeTurnMax;
    s32 timeTurn;
    s32 starLeft;
    s32 turnLeft;
    s32 languageNo;
    s32 i;
    s16 grpId;
    s16 sprId;

    timeTurnMax = GwSystem.timeTurnMax;
    starNum = timeTurnMax;
    timeTurn = GwSystem.timeTurn;
    starLeft = starNum - timeTurn;
    turnLeft = GwSystem.turnMax - GwSystem.turnNo + 1;
    if (turnLeft < starLeft) {
        starLeft = turnLeft;
    }
    languageNo = mbLanguageGet();
    grpId = HuSprGrpCreate(8);
    i = 0;
    if (GwSystem.curTime) {
        i++;
    }
    mbSprCreate(mbBoardDataNumGet(telopTimeBackFileTbl[i]), 100, NULL, &sprId);
    HuSprGrpMemberSet(grpId, 0, sprId);
    HuSprTPLvlSet(grpId, 0, 0.6f);
    mbSprCreate(mbBoardDataNumGet(telopTimeFileTbl[i]), 99, NULL, &sprId);
    HuSprGrpMemberSet(grpId, 1, sprId);
    for (i = 0; i < 3; i++) {
        mbSprCreate(
            mbBoardDataNumGet(BOARD_ANM_telopTimeStar), 98, NULL, &sprId);
        HuSprGrpMemberSet(grpId, i + 2, sprId);
        mbSprCreate(
            mbBoardDataNumGet(BOARD_ANM_telopTimeStar), 97, NULL, &sprId);
        HuSprGrpMemberSet(grpId, i + 5, sprId);
        HuSprAttrSet(grpId, i + 5, HUSPR_ATTR_LINEAR | HUSPR_ATTR_ADDCOL);
    }
    for (i = 0; i < 8; i++) {
        HuSprAttrSet(grpId, i, HUSPR_ATTR_LINEAR);
        HuSprPosSet(grpId, i, telopTimeSprOfsTbl[i].x, telopTimeSprOfsTbl[i].y);
        HuSprScaleSet(grpId, i, telopTimeSprScaleTbl[i], telopTimeSprScaleTbl[i]);
    }
    HuSprPosSet(grpId, 1, telopTimeNewSprOfsTbl[languageNo].x,
        telopTimeNewSprOfsTbl[languageNo].y);
    for (i = 0; i < 3; i++) {
        HuSprPosSet(grpId, i + 2, telopTimeStarSprOfsTbl[languageNo][i].x,
            telopTimeStarSprOfsTbl[languageNo][i].y);
        HuSprPosSet(grpId, i + 5, telopTimeStarSprOfsTbl[languageNo][i].x,
            telopTimeStarSprOfsTbl[languageNo][i].y);
    }
    mbTelopTimeStarSet(grpId, starLeft);
    mbTelopTimeTPLvlSet(grpId, 1.0f);
    return grpId;
}

void mbTelopTimeSprKill(s16 grpId)
{
    HuSprGrpKill(grpId);
}

void mbTelopTimeSprRotSet(s16 grpId, float rot)
{
    HUSPR_GROUP *group;
    HUSPRITE *spr;
    float phase;
    float scale;
    BOOL bankF;
    s32 i;

    group = &HuSprGrpData[grpId];
    for (i = 0; i < 3; i++) {
        bankF = FALSE;
        spr = &HuSprData[group->sprId[i + 2]];
        if (spr->bank & 1) {
            bankF = TRUE;
        }
        if (bankF) {
            HuSprTPLvlSet(grpId, i + 2, 0.8f);
            HuSprScaleSet(grpId, i + 2, 1.0f, 1.0f);
            HuSprZRotSet(grpId, i + 2, 0.0f);
            HuSprTPLvlSet(grpId, i + 5, 0.0f);
            HuSprScaleSet(grpId, i + 5, 0.0f, 0.0f);
        } else {
            scale = 1.0f + (0.2f * fabs(mbSinDeg(360.0f * rot)));
            HuSprTPLvlSet(grpId, i + 2, 1.0f);
            HuSprScaleSet(grpId, i + 2, scale, scale);
            HuSprZRotSet(grpId, i + 2, 30.0f * mbSinDeg(360.0f * rot));
            phase = (float)fmod(2.0f * rot, 1.0f);
            scale = 1.0f + fabs(mbSinDeg(90.0f * phase));
            HuSprTPLvlSet(grpId, i + 5, 0.8f * (1.0f - phase));
            HuSprScaleSet(grpId, i + 5, scale, scale);
            if (phase < 0.001) {
                HuSprScaleSet(grpId, i + 5, 0.0f, 0.0f);
            }
        }
    }
}

void mbTelopTimeStarSet(s16 grpId, s32 starNum)
{
    s32 emptyNum = 3 - starNum;
    s32 i;

    for (i = 0; i < 3; i++) {
        s32 bank = 0;

        if (GwSystem.curTime) {
            bank += 2;
        }
        if (i < emptyNum) {
            bank++;
        }
        HuSprBankSet(grpId, i + 2, bank);
        HuSprBankSet(grpId, i + 5, bank);
        HuSprTPLvlSet(grpId, i + 5, 0.0f);
    }
}

void mbTelopTimeTPLvlSet(s16 grpId, float tpLvl)
{
    s32 i;

    for (i = 0; i < 8; i++) {
        HuSprTPLvlSet(grpId, i, tpLvl * telopTimeBaseTPLvlTbl[i]);
    }
}

void mbTelopTimeDispSet(s16 grpId, BOOL dispF)
{
    s32 i;

    for (i = 0; i < 8; i++) {
        if (dispF) {
            HuSprDispOn(grpId, i);
        } else {
            HuSprDispOff(grpId, i);
        }
    }
}

s8 mbPadStkXGet(s32 playerNo)
{
    s8 stkX = HuPadStkX[playerNo];
    s8 subStkX = HuPadSubStkX[playerNo];

    if (abs(stkX) > abs(subStkX)) {
        if (abs(stkX) < 8) {
            return 0;
        } else {
            return stkX;
        }
    } else {
        if (abs(subStkX) < 8) {
            return 0;
        } else {
            return subStkX;
        }
    }
}

s8 mbPadStkYGet(s32 playerNo)
{
    s8 stkY = HuPadStkY[playerNo];
    s8 subStkY = HuPadSubStkY[playerNo];

    if (abs(stkY) > abs(subStkY)) {
        if (abs(stkY) < 8) {
            return 0;
        } else {
            return stkY;
        }
    } else {
        if (abs(subStkY) < 8) {
            return 0;
        } else {
            return subStkY;
        }
    }
}

void mbTauntInit(void)
{
    s32 i;

    tauntOMObj = omAddObj(mbObjMan, 32256, 0, 0, TauntOMExec);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        tauntSeNo[i] = MSM_SENO_NONE;
    }
    _SetFlag(FLAG_BOARD_WALKDONE);
}

void mbTauntClose(void)
{
    if (tauntOMObj) {
        TAUNT_WORK *work = omObjGetWork(tauntOMObj, TAUNT_WORK);
        work->killF = TRUE;
        _SetFlag(FLAG_BOARD_WALKDONE);
    }
}

static void TauntOMExec(OMOBJ *obj)
{
    int padNo;
    int charNo;
    TAUNT_WORK *work;
    BOOL charEnabled;
    int i;

    work = omObjGetWork(obj, TAUNT_WORK);
    if (work->killF || mbExitCheck()) {
        for (i = 0; i < GW_PLAYER_MAX; i++) {
            if (tauntSeNo[i] >= 0) {
                mbAudFXStop(tauntSeNo[i]);
                tauntSeNo[i] = MSM_SENO_NONE;
            }
        }
        tauntOMObj = NULL;
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (tauntSeNo[i] >= 0 && HuAudFXStatusGet(tauntSeNo[i]) == MSM_SE_DONE) {
            tauntSeNo[i] = MSM_SENO_NONE;
        }
    }
    if (mbPauseProcCheck() || _CheckFlag(FLAG_BOARD_WALKDONE) || WipeCheck()
        || GwSystem.turnPlayerNo == -1 || GWPartyGet() == FALSE
        || _CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return;
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (i == GwSystem.turnPlayerNo) {
            continue;
        }
        if (GwPlayer[i].comF) {
            continue;
        }
        padNo = GwPlayer[i].padNo;
        charNo = GwPlayer[i].charNo;
        charEnabled = charNo == 10
            ? TRUE
            : GWBankFlagGet(charNo + GW_BANK_FLAG_CHARACTER_BASE);
        if (charEnabled == FALSE) {
            continue;
        }
        if (tauntSeNo[padNo] < 0
            && (HuPadBtnDown[padNo] & PAD_TRIGGER_L)) {
            tauntSeNo[padNo] = mbAudFXPlay((s16)tauntSeTbl[charNo]);
        }
    }
}

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
    s32 languageNo = GWLanguageGet();
    s32 i;

    for (i = 0; i < 6; i++) {
        if (languageNo == languageTbl[i][0]) {
            break;
        }
    }
    return languageTbl[i][1];
}

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
    s32 i;

    for (i = 0; i < 6; i++) {
        if (languageNo == languageTbl[i][0]) {
            break;
        }
    }
    GWLanguageSet(languageTbl[i][1]);
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

static inline u32 BoardDataDirGet(s32 boardNo)
{
    if (boardNo < 0) {
        boardNo = 0;
    }
    return boardDataDirTbl[boardNo];
}

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

void mbBoardDataDirRead(void)
{
    s32 languageNo = mbLanguageGet();
    s32 i;

    if (languageNo < 0) {
        languageNo = 0;
    }
    for (i = 0; i < 6; i++) {
        if (BoardDataDirGet(i) == BoardDataDirGet(languageNo)) {
            continue;
        }
        if ((void *)HuARDirCheck(BoardDataDirGet(i)) != NULL) {
            HuARDirFree(BoardDataDirGet(i));
        }
        if (HuDataReadChk(BoardDataDirGet(i)) >= 0) {
            HuDataDirClose(BoardDataDirGet(i));
        }
    }
    if ((void *)HuARDirCheck(BoardDataDirGet(languageNo)) == NULL) {
        HuAR_DVDtoARAM(BoardDataDirGet(languageNo));
        while (HuARDMACheck()) {
        }
    }
}

static inline s32 TelopTimeTurnMaxGet(void)
{
    return GwSystem.timeTurnMax;
}

static inline s32 TelopTimeTurnGet(void)
{
    return GwSystem.timeTurn;
}

void mbTelopTimeChangeCreate(void)
{
    s32 starNum;
    s32 starLeft;
    s32 languageNo;
    s32 i;
    s32 j;
    OMOBJ *obj;
    TELOP_TIME_CHANGE_WORK *work;

    telopTimeChangeOMObj = obj = omAddObj(
        mbObjMan, 0, 8, 0, TelopTimeChangeOMExec);
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    work = omObjGetWork(obj, TELOP_TIME_CHANGE_WORK);
    work->killF = FALSE;
    work->mode = 0;
    work->time = 0;
    work->completeF = FALSE;
    work->maxTime = 16;

    languageNo = mbLanguageGet();
    starNum = TelopTimeTurnMaxGet();
    starLeft = starNum - TelopTimeTurnGet();

    i = GwSystem.turnMax - GwSystem.turnNo + 1;
    if (i < starLeft) {
        starLeft = i;
    }
    obj->trans.x = 0.0f;

    obj->mdlId[0] = espEntry(
        mbBoardDataNumGet(telopTimeChangeBackFileTbl[GwSystem.nextTime]), 101, 0);
    obj->mdlId[1] = espEntry(
        mbBoardDataNumGet(telopTimeChangeBackFileTbl[GwSystem.curTime]), 99, 0);
    obj->mdlId[2] = espEntry(
        mbBoardDataNumGet(telopTimeChangeBackFileTbl[GwSystem.curTime]), 97, 0);
    obj->mdlId[3] = espEntry(
        mbBoardDataNumGet(telopTimeChangeFileTbl[GwSystem.nextTime]), 100, 0);
    obj->mdlId[4] = espEntry(
        mbBoardDataNumGet(telopTimeChangeFileTbl[GwSystem.curTime]), 96, 0);
    for (i = 0; i < 5; i++) {
        espAttrSet(obj->mdlId[i], HUSPR_ATTR_LINEAR);
        espPosSet(obj->mdlId[i], 288.0f + telopTimeChangeSprOfsTbl[i].x,
            240.0f + telopTimeChangeSprOfsTbl[i].y);
        espTPLvlSet(obj->mdlId[i], 0.0f);
    }
    espPosSet(obj->mdlId[3], 288.0f + telopTimeNewSprOfsTbl[languageNo].x,
        240.0f + telopTimeNewSprOfsTbl[languageNo].y);
    espPosSet(obj->mdlId[4], 288.0f + telopTimeNewSprOfsTbl[languageNo].x,
        240.0f + telopTimeNewSprOfsTbl[languageNo].y);

    starLeft = 3 - starLeft;
    for (j = 0; j < 3; j++) {
        i = 0;
        if (GwSystem.curTime) {
            i += 2;
        }
        if (j < starLeft) {
            i++;
        }
        obj->mdlId[j + 5] = espEntry(
            mbBoardDataNumGet(BOARD_ANM_telopTimeStar), 98, i);
        espAttrSet(obj->mdlId[j + 5], HUSPR_ATTR_LINEAR);
        espDispOff(obj->mdlId[j + 5]);
        espPosSet(obj->mdlId[j + 5],
            288.0f + telopTimeStarSprOfsTbl[languageNo][j].x,
            240.0f + telopTimeStarSprOfsTbl[languageNo][j].y);
        espScaleSet(obj->mdlId[j + 5], 0.75f, 0.75f);
    }
    mbAudFXPlay(TELOP_SE_TIME_NOTICE);
}

static void TelopTimeChangeOMExec(OMOBJ *obj)
{
    TELOP_TIME_CHANGE_WORK *work;
    float weight;
    float angle;
    float scale;
    float posY;
    s32 languageNo;

    work = omObjGetWork(obj, TELOP_TIME_CHANGE_WORK);
    if (work->killF || mbExitCheck()) {
        s32 i;

        for (i = 0; i < 8; i++) {
            espKill(obj->mdlId[i]);
            obj->mdlId[i] = 0;
        }
        telopTimeChangeOMObj = NULL;
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    languageNo = mbLanguageGet();
    work->time++;
    if (work->time > work->maxTime) {
        work->time = work->maxTime;
    }
    weight = (float)work->time / (float)work->maxTime;
    {
        s32 i;

        switch (work->mode) {
        case 0:
            espTPLvlSet(obj->mdlId[0], weight);
            espTPLvlSet(obj->mdlId[3], weight);
            obj->trans.x = 288.0f;
            obj->trans.y = 256.0f;
            espPosSet(obj->mdlId[3], obj->trans.x, obj->trans.y);
            espPosSet(obj->mdlId[4], obj->trans.x, obj->trans.y);
            if (work->time >= work->maxTime) {
                obj->trans.z = 2.0f;
                work->time = 0;
                work->maxTime = 40;
                work->mode++;
            }
            break;
        case 1:
            angle = 90.0f * weight;
            scale = 1.0f - (0.75f * weight);
            espTPLvlSet(obj->mdlId[0], 1.0f - weight);
            espTPLvlSet(obj->mdlId[3], 1.0f - weight);
            posY = 720.0f - (480.0f * mbSinDeg(90.0f - angle));
            espPosSet(obj->mdlId[0],
                288.0f + (480.0f * mbCosDeg(90.0f - angle)), posY);
            espZRotSet(obj->mdlId[0], 4.0f * angle);
            espScaleSet(obj->mdlId[0], scale, scale);
            scale = 1.0f - (0.75f * (1.0f - weight));
            espTPLvlSet(obj->mdlId[1], weight);
            posY = 720.0f - (480.0f * mbSinDeg(180.0f - angle));
            espPosSet(obj->mdlId[1],
                288.0f + (480.0f * mbCosDeg(180.0f - angle)), posY);
            espZRotSet(obj->mdlId[1], 4.0f * (angle - 90.0f));
            espScaleSet(obj->mdlId[1], scale, scale);
            scale = 1.0f - (0.75f * weight);
            espZRotSet(obj->mdlId[3], -angle);
            obj->trans.z += 0.5f;
            obj->trans.y += obj->trans.z;
            espPosSet(obj->mdlId[3], obj->trans.x, obj->trans.y);
            espScaleSet(obj->mdlId[3], scale, scale);
            if (work->time + 12 > work->maxTime) {
                angle = (float)(work->maxTime - work->time);
                angle *= 0.083333336f;
                scale = 1.0f + (2.0f * angle);
                espScaleSet(obj->mdlId[4], scale, scale);
                espTPLvlSet(obj->mdlId[4], 1.0f - angle);
            }
            if (work->time >= work->maxTime) {
                espAttrSet(obj->mdlId[2], HUSPR_ATTR_ADDCOL);
                work->time = 0;
                work->maxTime = 30;
                work->mode++;
            }
            break;
        case 2:
            scale = 1.0f + (0.5f * weight);
            espTPLvlSet(obj->mdlId[2], 1.0f - weight);
            espScaleSet(obj->mdlId[2], scale, scale);
            for (i = 0; i < 3; i++) {
                espDispOn(obj->mdlId[i + 5]);
            }
            if (work->time < work->maxTime) {
                break;
            }
            work->time = 0;
            work->maxTime = 30;
            work->mode++;
        case 3:
            if (work->time >= work->maxTime) {
                work->mode = 4;
            }
            break;
        case 4:
            work->completeF = TRUE;
            if (WipeCheck()) {
                work->time = 0;
                work->maxTime = 30;
                work->mode = 5;
            }
            break;
        case 5:
            if (WipeCheck() == 0) {
                obj->trans.x = 504.0f;
                obj->trans.y = 84.0f;
                espPosSet(obj->mdlId[1], obj->trans.x, obj->trans.y);
                espPosSet(obj->mdlId[4],
                    obj->trans.x + (0.5f * telopTimeNewSprOfsTbl[languageNo].x),
                    obj->trans.y + (0.5f * telopTimeNewSprOfsTbl[languageNo].y));
                espScaleSet(obj->mdlId[1], 0.5f, 0.5f);
                espScaleSet(obj->mdlId[4], 0.5f, 0.5f);
                espTPLvlSet(obj->mdlId[1], 0.65f);
                for (i = 0; i < 3; i++) {
                    espPosSet(obj->mdlId[i + 5],
                        obj->trans.x + (0.5f * telopTimeStarSprOfsTbl[languageNo][i].x),
                        obj->trans.y + (0.5f * telopTimeStarSprOfsTbl[languageNo][i].y));
                    espScaleSet(obj->mdlId[i + 5], 0.375f, 0.375f);
                    espTPLvlSet(obj->mdlId[i + 5], 0.85f);
                }
            }
            break;
        case 10:
            espTPLvlSet(obj->mdlId[1], 1.0f - weight);
            espTPLvlSet(obj->mdlId[4], 1.0f - weight);
            if (work->time >= work->maxTime) {
                work->mode++;
                work->killF = TRUE;
            }
            break;
        }
    }
}

void mbTelopTimeChangeKill(void)
{
    if (telopTimeChangeOMObj) {
        TELOP_TIME_CHANGE_WORK *work = omObjGetWork(telopTimeChangeOMObj, TELOP_TIME_CHANGE_WORK);
        work->killF = TRUE;
    }
}

BOOL mbTelopTimeChangeCheck(void)
{
    TELOP_TIME_CHANGE_WORK *work;

    if (telopTimeChangeOMObj == NULL) {
        return FALSE;
    }
    work = omObjGetWork(telopTimeChangeOMObj, TELOP_TIME_CHANGE_WORK);
    return work->completeF == FALSE;
}
