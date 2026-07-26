#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/pause.h"

#include "game/armem.h"
#include "game/data.h"
#include "game/sprite.h"
#include "game/flag.h"
#include "game/pad.h"
#include "game/wipe.h"

#include "msm.h"

#define FLAG_BOARD_WALKDONE FLAGNUM(FLAG_GROUP_COMMON, 16)

typedef struct TauntWork_s {
    unsigned killF : 1;
} TAUNT_WORK;

typedef struct TelopTimeChangeWork_s {
    unsigned killF : 1;
    unsigned completeF : 1;
} TELOP_TIME_CHANGE_WORK;

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

static OMOBJ *telopOMObj;
static OMOBJ *telopTimeOMObj;
static OMOBJ *tauntOMObj;
static OMOBJ *telopTimeChangeOMObj;

static void TauntOMExec(OMOBJ *obj);
void mbTelopTimeDispSet(s16 grpId, BOOL dispF);

BOOL mbTelopCheck(void)
{
    return telopOMObj == NULL;
}

static void TelopTimePauseHook(BOOL dispF)
{
    if (telopTimeOMObj) {
        mbTelopTimeDispSet(telopTimeOMObj->mdlId[0], dispF);
    }
}

void mbTelopTimeSprKill(s16 grpId)
{
    HuSprGrpKill(grpId);
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
