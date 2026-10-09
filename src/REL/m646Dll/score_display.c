/* Owns the player score boxes and updates totals and rankings for Hyper Sniper results. */
#include "dolphin/math.h"
#include "REL/m646Dll/module_types.h"
#include "game/esprite.h"

#define M646_SCORE_NUMBER_DATA DATANUM(DATA_mgconst, 49)
#define M646_RECORD_ICON_DATA DATANUM(DATA_mgconst, 69)
#define M646_SCORE_WORK_BYTES 144
#define M646_SCORE_OBJECT_PRIORITY 70
#define M646_PLAYER_WINNER_BIT 0x1
#define M646_MAX_SCORE 9999

typedef struct M646ScoreEntry_s {
    s32 score;                    /* Player score, or the record score in the fifth entry. */
    HUSPR_GROUPID box;             /* Sprite group behind this score row. */
    u8 boxPadding[2]; /* Unused bytes between the sprite group and number display. */
    MGSCORE *scoreDisplay;         /* Number display over the score box. */
    s16 character;                 /* Character shown in one of the four player rows. */
    u8 characterPadding[2]; /* Unused bytes following the character index. */
    s32 recordSprite;              /* Record icon sprite; used only by the fifth entry. */
    u8 entryPadding[4]; /* Unused bytes at the end of a score entry. */
} M646ScoreEntry;

typedef struct M646ScoreboardWork_s {
    s32 state;                     /* 0: wait for play; 1: fade rows in; 2: rows visible. */
    f32 alpha;                     /* Score box and number opacity, from 0.0 to 1.0. */
    s32 highestScore;              /* Highest positive score found among the four players. */
    u16 winnerMask;                /* Bit i is set when player i shares the highest score. */
    u8 winnerPadding[2]; /* Unused bytes between the winner mask and record value. */
    s32 displayValue;              /* Value mirrored to the record-score entry. */
    s32 recordFlag;                /* Enables updating the fifth, record-score row. */
    M646ScoreEntry players[5];     /* Four players followed by the record row. */
} M646ScoreboardWork;

typedef struct M646RankingEntry_s {
    s32 player;                     /* Player index represented by this sorted entry. */
    s32 score;                      /* Score used to order the results. */
    s32 rank;                       /* Rank awarded to the player, including ties. */
} M646RankingEntry;

extern OMOBJ *lbl_1_bss_B8;
typedef struct M646ScorePositionTable_s {
    HuVec2f positions[5]; /* Player-row centers followed by the record-row center, in pixels. */
    u32 mgconstDataNumbers[14]; /* Additional resource numbers, not read by this display. */
} M646ScorePositionTable;

extern M646ScorePositionTable lbl_1_data_1D68;
void fn_1_93E0(OMOBJ *obj);
void fn_1_9764(OMOBJ *obj);

void fn_1_8E78(OMOBJMAN *objman);
void fn_1_8F00(void);
void fn_1_8F04(s32 playerIndex, s32 scorePoints);
void fn_1_8F5C(s32 playerIndex);
s32 fn_1_8FA4(s32 playerIndex);
u16 fn_1_8FD4(void);
s32 fn_1_8FFC(void);
s32 fn_1_9024(void);
void fn_1_9118(void);
s32 fn_1_9380(void);
void fn_1_93E0(OMOBJ *obj);
void fn_1_9764(OMOBJ *obj);

OMOBJ *lbl_1_bss_B8;

/* Score row positions and DATA_mgconst resources used by the results display. */
M646ScorePositionTable lbl_1_data_1D68 = {
    {
        {70.0f, 60.0f},
        {186.0f, 60.0f},
        {384.0f, 60.0f},
        {500.0f, 60.0f},
        {290.0f, 422.0f},
    },
    {DATANUM(DATA_mgconst, 32), DATANUM(DATA_mgconst, 33), DATANUM(DATA_mgconst, 34), DATANUM(DATA_mgconst, 35), DATANUM(DATA_mgconst, 36), DATANUM(DATA_mgconst, 37), DATANUM(DATA_mgconst, 38), DATANUM(DATA_mgconst, 40), DATANUM(DATA_mgconst, 39), DATANUM(DATA_mgconst, 41), DATANUM(DATA_mgconst, 42), DATANUM(DATA_mgconst, 43), DATANUM(DATA_mgconst, 44), DATANUM(DATA_mgconst, 45)}
};

/* Creates the score object that owns the four player totals and result display. */
void fn_1_8E78(OMOBJMAN *objman)
{
    OMOBJ *obj = omAddObjEx(objman, M646_SCORE_OBJECT_PRIORITY, 0, 0, OM_GRP_NONE, fn_1_93E0);
    void *data;
    lbl_1_bss_B8 = obj;
    data = HuMemDirectMallocNum(HEAP_HEAP, M646_SCORE_WORK_BYTES, HU_MEMNUM_OVL);
    obj->data = data;
    memset(data, 0, M646_SCORE_WORK_BYTES);
}

void fn_1_8F00(void)
{
}

/* Adds points awarded by a target hit to a player's score, capped at four digits. */
void fn_1_8F04(s32 playerIndex, s32 scorePoints)
{
    M646ScoreboardWork *work;
    s32 *score;
    if (playerIndex < 4) {
        work = lbl_1_bss_B8->data;
        score = &work->players[playerIndex].score;
        scorePoints += *score;
        if (scorePoints > M646_MAX_SCORE) {
            scorePoints = M646_MAX_SCORE;
        }
        *score = scorePoints;
    }
}

/* The projectile contact handler clears a player's total when it hits a score-reset target. */
void fn_1_8F5C(s32 playerIndex)
{
    M646ScoreboardWork *work;
    s32 *score;
    if (playerIndex >= 4) {
        return;
    }
    work = lbl_1_bss_B8->data;
    score = &work->players[playerIndex].score;
    *score = 0;
}

s32 fn_1_8FA4(s32 playerIndex)
{
    M646ScoreboardWork *work = lbl_1_bss_B8->data;
    return work->players[playerIndex].score;
}

u16 fn_1_8FD4(void)
{
    M646ScoreboardWork *work = lbl_1_bss_B8->data;
    return work->winnerMask;
}

s32 fn_1_8FFC(void)
{
    M646ScoreboardWork *work = lbl_1_bss_B8->data;
    return work->displayValue;
}

/* The pre-winner sequence finds the highest positive score and all players tied at that score. */
s32 fn_1_9024(void)
{
    M646ScoreboardWork *work = lbl_1_bss_B8->data;
    s32 playerIndex;
    s32 score;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        score = fn_1_8FA4(playerIndex);
        if (score >= work->highestScore && score > 0) {
            if (score == work->highestScore) {
                work->winnerMask |= M646_PLAYER_WINNER_BIT << playerIndex;
            } else {
                work->winnerMask = M646_PLAYER_WINNER_BIT << playerIndex;
                work->recordFlag = 0;
            }
            work->highestScore = score;
        }
    }
    /* The record row is refreshed only if this flag is set; these callbacks never enable it. */
    if (work->recordFlag) {
        work->displayValue = work->players[4].score = work->highestScore;
        MgScoreValueSet(work->players[4].scoreDisplay, work->players[4].score);
    }
    return work->recordFlag;
}

/* The pre-winner hook stores player ranks in the bonus field and saves scores for Decathlon. */
void fn_1_9118(void)
{
    M646ScoreboardWork *work = lbl_1_bss_B8->data;
    M646RankingEntry ranking[4];
    s32 rankingIndex;
    s32 playerIndex;
    s32 comparisonIndex;
    s32 score;
    s32 rank;
    s32 displacedPlayer;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        ranking[playerIndex].player = playerIndex;
        ranking[playerIndex].score = fn_1_8FA4(playerIndex);
        ranking[playerIndex].rank = 3;
    }
    for (rankingIndex = 0; rankingIndex < 4; rankingIndex++) {
        for (comparisonIndex = rankingIndex; comparisonIndex < 4; comparisonIndex++) {
            if (rankingIndex != comparisonIndex &&
                ranking[rankingIndex].score < ranking[comparisonIndex].score) {
                displacedPlayer = ranking[rankingIndex].player;
                score = ranking[rankingIndex].score;
                ranking[rankingIndex] = ranking[comparisonIndex];
                ranking[comparisonIndex].player = displacedPlayer;
                ranking[comparisonIndex].score = score;
            }
        }
    }
    if (ranking[0].score > 0) {
        /* Equal scores share a rank; each lower distinct score advances it by one. */
        rank = 0;
        score = -1;
        for (rankingIndex = 0; rankingIndex < 4; rankingIndex++) {
            if (score > ranking[rankingIndex].score) {
                rank++;
            }
            ranking[rankingIndex].rank = rank;
            score = ranking[rankingIndex].score;
        }
    }
    /* All-zero totals retain rank 3; a positive leading score enables the ranking pass above. */
    for (rankingIndex = 0; rankingIndex < 4; rankingIndex++) {
        /* This result value is the rank, even though its destination is the coin-bonus field. */
        GWMgCoinBonusSet(ranking[rankingIndex].player, ranking[rankingIndex].rank);
        if (_CheckFlag(FLAG_INST_DECA)) {
            GWMgScoreSet(ranking[rankingIndex].player, ranking[rankingIndex].score);
        }
    }
}

/* Reports whether every player has zero points; the target selector uses this for its result
 * path. */
s32 fn_1_9380(void)
{
    s32 playerIndex;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        M646ScoreboardWork *work = lbl_1_bss_B8->data;
        if (work->players[playerIndex].score != 0) {
            return 0;
        }
    }
    return 1;
}

/* Score object startup callback: creates four player rows and the fifth record row. */
void fn_1_93E0(OMOBJ *obj)
{
    M646ScoreboardWork *work = obj->data;
    M646ScoreEntry *entry;
    s32 entryIndex;
    MGSCORE *scoreDisplay;
    HUSPR_GROUPID box;
    f32 scale = (0.7f);
    f32 digitWidth;
    for (entryIndex = 0; entryIndex < 5; entryIndex++) {
        entry = &work->players[entryIndex];
        if (entryIndex < 4) {
            entry->character = GwPlayerConf[entryIndex].charNo;
            box = MgScoreBoxCreateChar(108, 40, entry->character);
            entry->box = box;
            MgScoreBoxPosSet(box, lbl_1_data_1D68.positions[entryIndex].x,
                             lbl_1_data_1D68.positions[entryIndex].y);
            MgScoreBoxColorSet(box, 64, 64, 64);
            MgScoreBoxDispSet(box, 0);
            scoreDisplay = MgScoreCreate(M646_SCORE_NUMBER_DATA, -1, 1);
            entry->scoreDisplay = scoreDisplay;
            MgScoreMaxDigitSet(scoreDisplay, 4);
            MgScoreDispOff(scoreDisplay);
            MgScorePosSet(scoreDisplay, lbl_1_data_1D68.positions[entryIndex].x - (3.0f),
                (1.0f) + lbl_1_data_1D68.positions[entryIndex].y);
            MgScoreColorSet(scoreDisplay, 255, 216, 21);
            MgScorePriSet(scoreDisplay, 10);
            MgScoreDigitWidthGet(scoreDisplay, &digitWidth);
            MgScoreDigitWidthSet(scoreDisplay, digitWidth * scale);
            MgScoreDigitScaleSet(scoreDisplay, scale, (1.0f));
        } else {
            entry->character = -1;
            box = MgScoreBoxCreate(158, 40);
            entry->box = box;
            MgScoreBoxPosSet(box, lbl_1_data_1D68.positions[entryIndex].x,
                             lbl_1_data_1D68.positions[entryIndex].y);
            MgScoreBoxColorSet(box, 64, 64, 64);
            scoreDisplay = MgScoreCreate(M646_SCORE_NUMBER_DATA, -1, 1);
            entry->scoreDisplay = scoreDisplay;
            MgScoreMaxDigitSet(scoreDisplay, 4);
            work->displayValue = entry->score = 0;
            MgScoreValueSet(scoreDisplay, entry->score);
            MgScorePosSet(scoreDisplay, lbl_1_data_1D68.positions[entryIndex].x - (19.0f),
                          lbl_1_data_1D68.positions[entryIndex].y);
            MgScoreColorSet(scoreDisplay, 66, 255, 122);
            MgScorePriSet(scoreDisplay, 10);
            work->players[entryIndex].recordSprite = espEntry(M646_RECORD_ICON_DATA, 0, 0);
            espAttrSet(entry->recordSprite, HUSPR_ATTR_NOANIM);
            espBankSet(entry->recordSprite, 0);
            espPosSet(entry->recordSprite, lbl_1_data_1D68.positions[entryIndex].x - (41.0f),
                      lbl_1_data_1D68.positions[entryIndex].y);
            MgScoreBoxDispSet(box, 0);
            espDispOff(entry->recordSprite);
            MgScoreDispOff(scoreDisplay);
        }
    }
    obj->objFunc = fn_1_9764;
}

/* Per-frame score object callback: fades the player rows in at main play and refreshes totals. */
void fn_1_9764(OMOBJ *obj)
{
    M646ScoreEntry *entry;
    M646ScoreboardWork *work = obj->data;
    s32 playerIndex;
    switch (work->state) {
        case 0:
            if (MgSeqModeGet() != MGSEQ_MODE_MAIN) {
                break;
            }
            work->state++;
            work->alpha = (0.0f);
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                entry = &work->players[playerIndex];
                MgScoreBoxDispSet(entry->box, 1);
                MgScoreDispOn(entry->scoreDisplay);
            }
        case 1:
            /* Crossing 1.0 advances the phase without clamping the stored alpha. */
            if ((work->alpha += (0.1)) >= (1.0)) {
                work->state++;
            }
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                entry = &work->players[playerIndex];
                MgScoreBoxTPLvlSet(entry->box, work->alpha);
                MgScoreTPLvlSet(entry->scoreDisplay, work->alpha);
            }
            break;
        case 2:
            break;
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        entry = &work->players[playerIndex];
        if (entry->score > M646_MAX_SCORE) {
            entry->score = M646_MAX_SCORE;
        }
        MgScoreValueSet(entry->scoreDisplay, entry->score);
    }
}
