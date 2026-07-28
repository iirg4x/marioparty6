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
} TELOP_TIME_CHANGE_WORK;

static const u32 telopFileTbl[27] = {
    0x0005004D,
    0x0005004E,
    0x0005004F,
    0x00050050,
    0x00050051,
    0x00050052,
    0x00050053,
    0x00050054,
    0x00050055,
    0x00050056,
    0x00050058,
    0x00050057,
    0x00050057,
    0x00050057,
    0x0005005A,
    0x00050059,
    0x00050044,
    0x00050045,
    0x00050046,
    0x00050047,
    0x00050048,
    0x00050049,
    0x0005004C,
    0x0005004B,
    0x0005004A,
    0x00050044,
    0x00050044,
};

static const s32 tauntSeTbl[14] = {
    0x24D,
    0x235,
    0x2C5,
    0x325,
    0x2F5,
    0x1ED,
    0x30D,
    0x205,
    0x2DD,
    0x295,
    0x21D,
    0x295,
    0x295,
    0x295,
};

static u32 telopTurnFileTbl[3] = {
    0x00050073,
    0x00050074,
    0x00050073,
};

static u32 telopTurnLastFileTbl[3] = {
    0x00050072,
    0x00050074,
    0x00050072,
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
    0x0005006D,
    0x0005006E,
};

static u32 telopTimeFileTbl[2] = {
    0x0005006F,
    0x00050070,
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
s32 mbLanguageGet(void);
s16 mbTelopTimeSprCreate(void);
void mbTelopTimeSprKill(s16 grpId);
void mbTelopTimeSprRotSet(s16 grpId, float rot);
void mbTelopTimeStarSet(s16 grpId, s32 starNum);
void mbTelopTimeTPLvlSet(s16 grpId, float tpLvl);
void mbTelopTimeDispSet(s16 grpId, BOOL dispF);
extern float mbSinDeg(float angle);

void mbTelopCreate(int playerNo, int telopNo, BOOL waitF)
{
    TELOP_WORK *work;

    telopOMObj = omAddObj(mbObjMan, 0x106, 1, 0, TelopInitOMExec);
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
        mbAudFXPlay(0x3F3);
    }
    if (waitF == FALSE) {
        return;
    }
    while (telopOMObj) {
        HuPrcVSleep();
    }
}

void mbTelopPlayerCreate(int playerNo)
{
    TELOP_WORK *work;
    int telopNo;

    telopNo = GwPlayer[playerNo].charNo;
    telopOMObj = omAddObj(mbObjMan, 0x106, 1, 0, TelopInitOMExec);
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
        mbAudFXPlay(0x3F3);
    }
    while (telopOMObj) {
        HuPrcVSleep();
    }
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
                    mbAudFXPlay(0x3F4);
                }
            } else {
                int padNo = GwPlayer[work->playerNo].padNo;

                if ((HuPadBtnDown[padNo] & PAD_BUTTON_A) || GwPlayer[work->playerNo].comF) {
                    work->mode = 2;
                    work->time = 0;
                    work->maxTime = work->telopNo >= 16 ? 30 : 15;
                    if (work->telopNo < 16) {
                        mbAudFXPlay(0x3F4);
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
    mbAudFXPlay(0x46C);
    mbPauseHookPush(TelopLastTurnPauseHook);
}

static void TelopLastTurnOMExec(OMOBJ *obj)
{
    TELOP_LAST_TURN_WORK *work;
    float scale;
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
            scale = fabsf(mbSinDeg(720.0f * weight));
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
    s32 timeTurnMax;
    s32 timeTurn;
    s32 starNum;
    s32 turnLeft;
    s32 languageNo;
    s32 timeNo;
    s32 i;
    s16 grpId;
    s16 sprId;

    timeTurnMax = GwSystem.timeTurnMax;
    starNum = timeTurnMax;
    timeTurn = GwSystem.timeTurn;
    starNum -= timeTurn;
    turnLeft = GwSystem.turnMax - GwSystem.turnNo + 1;
    if (turnLeft < starNum) {
        starNum = turnLeft;
    }
    languageNo = mbLanguageGet();
    grpId = HuSprGrpCreate(8);
    timeNo = GwSystem.curTime ? 1 : 0;
    mbSprCreate(mbBoardDataNumGet(telopTimeBackFileTbl[timeNo]), 100, NULL, &sprId);
    HuSprGrpMemberSet(grpId, 0, sprId);
    HuSprTPLvlSet(grpId, 0, 0.6f);
    mbSprCreate(mbBoardDataNumGet(telopTimeFileTbl[timeNo]), 99, NULL, &sprId);
    HuSprGrpMemberSet(grpId, 1, sprId);
    for (i = 0; i < 3; i++) {
        mbSprCreate(mbBoardDataNumGet(0x00050071), 98, NULL, &sprId);
        HuSprGrpMemberSet(grpId, i + 2, sprId);
        mbSprCreate(mbBoardDataNumGet(0x00050071), 97, NULL, &sprId);
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
    mbTelopTimeStarSet(grpId, starNum);
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
    float firstWave;
    float phase;
    float secondScale;
    float scale;
    s32 i;

    group = &HuSprGrpData[grpId];
    for (i = 0; i < 3; i++) {
        spr = &HuSprData[group->sprId[i + 2]];
        if (spr->bank & 1) {
            HuSprTPLvlSet(grpId, i + 2, 0.8f);
            HuSprScaleSet(grpId, i + 2, 1.0f, 1.0f);
            HuSprZRotSet(grpId, i + 2, 0.0f);
            HuSprTPLvlSet(grpId, i + 5, 0.0f);
            HuSprScaleSet(grpId, i + 5, 0.0f, 0.0f);
        } else {
            firstWave = fabsf(mbSinDeg(360.0f * rot));
            scale = 1.0f + (0.2f * firstWave);
            HuSprTPLvlSet(grpId, i + 2, 1.0f);
            HuSprScaleSet(grpId, i + 2, scale, scale);
            HuSprZRotSet(grpId, i + 2, 30.0f * mbSinDeg(360.0f * rot));
            phase = (float)fmod(2.0f * rot, 1.0f);
            secondScale = 1.0f + fabsf(mbSinDeg(90.0f * phase));
            HuSprTPLvlSet(grpId, i + 5, 0.8f * (1.0f - phase));
            HuSprScaleSet(grpId, i + 5, secondScale, secondScale);
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

    tauntOMObj = omAddObj(mbObjMan, 0x7E00, 0, 0, TauntOMExec);
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
        charEnabled = charNo == 10 ? TRUE : GWBankFlagGet(charNo + 0x24);
        if (charEnabled == FALSE) {
            continue;
        }
        if (tauntSeNo[padNo] < 0 && (HuPadBtnDown[padNo] & 0x40)) {
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
        if (HuARDirCheck(BoardDataDirGet(i)) != 0) {
            HuARDirFree(BoardDataDirGet(i));
        }
        if (HuDataReadChk(BoardDataDirGet(i)) >= 0) {
            HuDataDirClose(BoardDataDirGet(i));
        }
    }
    if (HuARDirCheck(BoardDataDirGet(languageNo)) == 0) {
        HuAR_DVDtoARAM(BoardDataDirGet(languageNo));
        while (HuARDMACheck()) {
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
