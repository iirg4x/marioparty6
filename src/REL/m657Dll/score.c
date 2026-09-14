#include "REL/m657Dll.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/gamemes.h"
#include "string.h"

int abs(int value);

OMOBJ *lbl_1_bss_38;

void fn_1_3E1C(OMOBJMAN *objman)
{
    OMOBJ *obj;
    M657ScoreWork *work;

    obj = omAddObjEx(objman, 70, 0, 0, -1, fn_1_4004);
    lbl_1_bss_38 = obj;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M657ScoreWork), HU_MEMNUM_OVL);
    obj->data = work;
    memset(work, 0, sizeof(M657ScoreWork));
}

void fn_1_3EA4(void)
{
}

void fn_1_3EA8(s16 team)
{
    M657ScoreTeam *score;
    M657ScoreWork *work = lbl_1_bss_38->data;

    score = &work->teams[team];
    score->running = FALSE;
}

s32 fn_1_3EEC(s32 team)
{
    M657ScoreTeam *score;
    M657ScoreWork *work = lbl_1_bss_38->data;

    score = &work->teams[(s16)team];
    return score->frames;
}

s32 fn_1_3F2C(void)
{
    M657ScoreWork *work = lbl_1_bss_38->data;
    return work->frames;
}

void fn_1_3F54(void)
{
    M657ScoreTeam *score;
    int i;
    int j;
    M657ScoreWork *work = lbl_1_bss_38->data;

    score = NULL;
    for (i = 0; i < 2; i++) {
        score = &work->teams[i];
        MgScoreBoxDispSet(score->box, FALSE);
        MgScoreDispOff(score->seconds);
        MgScoreDispOff(score->hundredths);
        for (j = 0; j < 1; j++) {
            HuSprAttrSet(score->separator, j, HUSPR_ATTR_DISPOFF);
        }
    }
}

static HuVec2f lbl_1_data_1E8[2] = {
    { 142, 56 }, { 432, 56 }
};

void fn_1_4004(OMOBJ *obj)
{
    M657ScoreTeam *score;
    int team;
    int j;
    int i;
    M657ScoreWork *work = obj->data;
    ANIMDATA *anim;

    score = NULL;
    anim = NULL;
    for (i = 0; i < 4; i++) {
        team = GwPlayerConf[i].grpNo;
        if (team == 0 || team == 1) {
            score = &work->teams[team];
            score->box = MgScoreBoxCreate(104, 32);
            MgScoreBoxPosSet(score->box, lbl_1_data_1E8[team].x, lbl_1_data_1E8[team].y);
            MgScoreBoxDispSet(score->box, FALSE);
            MgScoreBoxColorSet(score->box, 64, 64, 64);
            score->seconds = MgScoreCreate(DATANUM(DATA_mgconst, 48), -1, TRUE);
            MgScoreMaxDigitSet(score->seconds, 2);
            MgScoreDispOff(score->seconds);
            MgScorePosSet(score->seconds, lbl_1_data_1E8[team].x - 32.0f,
                lbl_1_data_1E8[team].y);
            MgScoreColorSet(score->seconds, 255, 216, 21);
            MgScorePriSet(score->seconds, 10);
            score->hundredths = MgScoreCreate(DATANUM(DATA_mgconst, 48), -1, TRUE);
            MgScoreMaxDigitSet(score->hundredths, 2);
            MgScoreDispOff(score->hundredths);
            MgScorePosSet(score->hundredths, 16.0f + lbl_1_data_1E8[team].x,
                lbl_1_data_1E8[team].y);
            MgScoreColorSet(score->hundredths, 255, 216, 21);
            MgScorePriSet(score->hundredths, 10);
            score->separator = HuSprGrpCreate(1);
            HuSprGrpPosSet(score->separator, lbl_1_data_1E8[team].x, lbl_1_data_1E8[team].y);
            anim = score->seconds->digitAnim;
            HuSprGrpMemberSet(score->separator, 0, HuSprCreate(anim, 50, 11));
            HuSprPosSet(score->separator, 0, 0.0f, 0.0f);
            HuSprColorSet(score->separator, 0, 255, 255, 255);
            for (j = 0; j < 1; j++) {
                HuSprAttrSet(score->separator, j, HUSPR_ATTR_NOANIM);
                HuSprAttrSet(score->separator, j, HUSPR_ATTR_DISPOFF);
            }
            score->running = TRUE;
            score->frames = 600;
        }
    }
    work->frames = 600;
    obj->objFunc = fn_1_42F4;
}

void fn_1_42F4(OMOBJ *obj)
{
    M657ScoreWork *work = obj->data;
    M657ScoreTeam *score;
    int i;
    int value;
    int j;

    score = NULL;
    switch (work->state) {
        case 0:
            if (MgSeqModeGet() != MGSEQ_MODE_MAIN) {
                break;
            }
            work->state++;
            for (i = 0; i < 2; i++) {
                score = &work->teams[i];
                MgScoreBoxDispSet(score->box, TRUE);
                MgScoreDispOn(score->seconds);
                MgScoreDispOn(score->hundredths);
                for (j = 0; j < 1; j++) {
                    HuSprAttrReset(score->separator, j, HUSPR_ATTR_DISPOFF);
                }
            }
            /* fall through */
        case 1:
            work->frames = 600;
            work->state++;
            break;
        case 2:
            work->frames--;
            if (work->frames < -600) {
                work->frames = -600;
            }
            break;
    }
    for (i = 0; i < 2; i++) {
        score = &work->teams[i];
        if (score->running) {
            score->frames = work->frames;
            if (score->frames < -600) {
                continue;
            }
            if (score->frames < 0) {
                MgScoreColorSet(score->seconds, 255, 0, 0);
                MgScoreColorSet(score->hundredths, 255, 0, 0);
            }
            value = score->frames % 60;
            value = abs(value);
            if (work->teams[0].running) {
                MgScoreValueSet(work->teams[0].hundredths, value * 100 / 60);
            }
            if (work->teams[1].running) {
                MgScoreValueSet(work->teams[1].hundredths, value * 100 / 60);
            }
            value = score->frames / 60;
            value = abs(value);
            if (work->teams[0].running) {
                MgScoreValueSet(work->teams[0].seconds, value);
            }
            if (work->teams[1].running) {
                MgScoreValueSet(work->teams[1].seconds, value);
            }
        }
    }
}
