/* Builds and updates the two team score displays used to decide the winner. */
#include "REL/m657Dll.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/gamemes.h"
#include "string.h"

int abs(int value);

#define M657_SCORE_FRAMES_PER_SECOND 60
#define M657_SCORE_INITIAL_FRAMES 600
#define M657_SCORE_OVERRUN_LIMIT_FRAMES 600

OMOBJ *lbl_1_bss_38;

/* Creates the score object and allocates its shared countdown state. */
void fn_1_3E1C(OMOBJMAN *objman)
{
    OMOBJ *obj;
    M657ScoreWork *scoreWork;

    obj = omAddObjEx(objman, 70, 0, 0, -1, fn_1_4004);
    lbl_1_bss_38 = obj;
    scoreWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M657ScoreWork), HU_MEMNUM_OVL);
    obj->data = scoreWork;
    memset(scoreWork, 0, sizeof(M657ScoreWork));
}

void fn_1_3EA4(void)
{
}

/* Stops updating the selected team's score display when its player lands. */
void fn_1_3EA8(s16 team)
{
    M657ScoreTeam *teamScore;
    M657ScoreWork *scoreWork = lbl_1_bss_38->data;

    teamScore = &scoreWork->teams[team];
    teamScore->running = FALSE;
}

/* Returns the selected team's score in 60ths of a second. */
s32 fn_1_3EEC(s32 team)
{
    M657ScoreTeam *teamScore;
    M657ScoreWork *scoreWork = lbl_1_bss_38->data;

    teamScore = &scoreWork->teams[(s16)team];
    return teamScore->frames;
}

/* Returns the shared countdown in 60ths of a second. */
s32 fn_1_3F2C(void)
{
    M657ScoreWork *scoreWork = lbl_1_bss_38->data;
    return scoreWork->frames;
}

/* Hides both countdown panels and their separator sprites during the result transition. */
void fn_1_3F54(void)
{
    M657ScoreTeam *teamScore;
    int teamIndex;
    int memberIndex;
    M657ScoreWork *scoreWork = lbl_1_bss_38->data;

    teamScore = NULL;
    for (teamIndex = 0; teamIndex < 2; teamIndex++) {
        teamScore = &scoreWork->teams[teamIndex];
        MgScoreBoxDispSet(teamScore->box, FALSE);
        MgScoreDispOff(teamScore->seconds);
        MgScoreDispOff(teamScore->hundredths);
        for (memberIndex = 0; memberIndex < 1; memberIndex++) {
            HuSprAttrSet(teamScore->separator, memberIndex, HUSPR_ATTR_DISPOFF);
        }
    }
}

/* Screen origins for the left and right team countdown panels. */
static HuVec2f lbl_1_data_1E8[2] = {
    { 142, 56 }, { 432, 56 }
};

/* Object-create callback that builds each score panel and its display sprites. */
void fn_1_4004(OMOBJ *obj)
{
    M657ScoreTeam *teamScore;
    int team;
    int memberIndex;
    int playerIndex;
    M657ScoreWork *scoreWork = obj->data;
    ANIMDATA *digitAnimation;

    teamScore = NULL;
    digitAnimation = NULL;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        team = GwPlayerConf[playerIndex].grpNo;
        if (team == 0 || team == 1) {
            teamScore = &scoreWork->teams[team];
            teamScore->box = MgScoreBoxCreate(104, 32);
            MgScoreBoxPosSet(teamScore->box, lbl_1_data_1E8[team].x, lbl_1_data_1E8[team].y);
            MgScoreBoxDispSet(teamScore->box, FALSE);
            MgScoreBoxColorSet(teamScore->box, 64, 64, 64);
            teamScore->seconds = MgScoreCreate(DATANUM(DATA_mgconst, 48), -1, TRUE);
            MgScoreMaxDigitSet(teamScore->seconds, 2);
            MgScoreDispOff(teamScore->seconds);
            MgScorePosSet(teamScore->seconds, lbl_1_data_1E8[team].x - 32.0f,
                lbl_1_data_1E8[team].y);
            MgScoreColorSet(teamScore->seconds, 255, 216, 21);
            MgScorePriSet(teamScore->seconds, 10);
            teamScore->hundredths = MgScoreCreate(DATANUM(DATA_mgconst, 48), -1, TRUE);
            MgScoreMaxDigitSet(teamScore->hundredths, 2);
            MgScoreDispOff(teamScore->hundredths);
            MgScorePosSet(teamScore->hundredths, 16.0f + lbl_1_data_1E8[team].x,
                lbl_1_data_1E8[team].y);
            MgScoreColorSet(teamScore->hundredths, 255, 216, 21);
            MgScorePriSet(teamScore->hundredths, 10);
            teamScore->separator = HuSprGrpCreate(1);
            HuSprGrpPosSet(teamScore->separator, lbl_1_data_1E8[team].x, lbl_1_data_1E8[team].y);
            digitAnimation = teamScore->seconds->digitAnim;
            HuSprGrpMemberSet(teamScore->separator, 0, HuSprCreate(digitAnimation, 50, 11));
            HuSprPosSet(teamScore->separator, 0, 0.0f, 0.0f);
            HuSprColorSet(teamScore->separator, 0, 255, 255, 255);
            for (memberIndex = 0; memberIndex < 1; memberIndex++) {
                HuSprAttrSet(teamScore->separator, memberIndex, HUSPR_ATTR_NOANIM);
                HuSprAttrSet(teamScore->separator, memberIndex, HUSPR_ATTR_DISPOFF);
            }
            teamScore->running = TRUE;
            teamScore->frames = M657_SCORE_INITIAL_FRAMES;
        }
    }
    scoreWork->frames = M657_SCORE_INITIAL_FRAMES;
    obj->objFunc = fn_1_42F4;
}

/* Updates the shared countdown and presents each team's time on every object tick. */
void fn_1_42F4(OMOBJ *obj)
{
    M657ScoreWork *scoreWork = obj->data;
    M657ScoreTeam *teamScore;
    int teamIndex;
    int scoreValue;
    int memberIndex;

    teamScore = NULL;
    switch (scoreWork->state) {
        case 0:
            if (MgSeqModeGet() != MGSEQ_MODE_MAIN) {
                break;
            }
            scoreWork->state++;
            for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                teamScore = &scoreWork->teams[teamIndex];
                MgScoreBoxDispSet(teamScore->box, TRUE);
                MgScoreDispOn(teamScore->seconds);
                MgScoreDispOn(teamScore->hundredths);
                for (memberIndex = 0; memberIndex < 1; memberIndex++) {
                    HuSprAttrReset(teamScore->separator, memberIndex, HUSPR_ATTR_DISPOFF);
                }
            }
            /* fall through */
        case 1:
            /* The timer uses 60 frames per second, so 600 frames gives the teams ten seconds. */
            scoreWork->frames = M657_SCORE_INITIAL_FRAMES;
            scoreWork->state++;
            break;
        case 2:
            scoreWork->frames--;
            /* Keep ten seconds of negative time available for players to finish falling. */
            if (scoreWork->frames < -M657_SCORE_OVERRUN_LIMIT_FRAMES) {
                scoreWork->frames = -M657_SCORE_OVERRUN_LIMIT_FRAMES;
            }
            break;
    }
    for (teamIndex = 0; teamIndex < 2; teamIndex++) {
        teamScore = &scoreWork->teams[teamIndex];
        if (teamScore->running) {
            teamScore->frames = scoreWork->frames;
            if (teamScore->frames < -M657_SCORE_OVERRUN_LIMIT_FRAMES) {
                continue;
            }
            if (teamScore->frames < 0) {
                /* A negative team time means its player has outlasted the shared countdown. */
                MgScoreColorSet(teamScore->seconds, 255, 0, 0);
                MgScoreColorSet(teamScore->hundredths, 255, 0, 0);
            }
            scoreValue = teamScore->frames % M657_SCORE_FRAMES_PER_SECOND;
            scoreValue = abs(scoreValue);
            /* Convert the remaining frame fraction to two displayed decimal digits. */
            if (scoreWork->teams[0].running) {
                MgScoreValueSet(scoreWork->teams[0].hundredths,
                    scoreValue * 100 / M657_SCORE_FRAMES_PER_SECOND);
            }
            if (scoreWork->teams[1].running) {
                MgScoreValueSet(scoreWork->teams[1].hundredths,
                    scoreValue * 100 / M657_SCORE_FRAMES_PER_SECOND);
            }
            scoreValue = teamScore->frames / M657_SCORE_FRAMES_PER_SECOND;
            scoreValue = abs(scoreValue);
            if (scoreWork->teams[0].running) {
                MgScoreValueSet(scoreWork->teams[0].seconds, scoreValue);
            }
            if (scoreWork->teams[1].running) {
                MgScoreValueSet(scoreWork->teams[1].seconds, scoreValue);
            }
        }
    }
}
