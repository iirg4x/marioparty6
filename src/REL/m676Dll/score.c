/* Four player score boxes and their animated score digits. */
#include "dolphin/math.h"
#include "REL/m676Dll/score_types.h"
#include "datadir_enum.h"
#include "game/gamework.h"
#include "game/sprite.h"
#include "game/gamemes.h"
#include "game/data.h"
#include "game/memory.h"
#include <string.h>

OMOBJ *lbl_1_bss_10B0;
s32 fn_1_1480(s32 index);
M676ScorePosition lbl_1_data_7B0[4] = {{76.0f,64.0f},{192.4f,64.0f},{383.6f,64.0f},{500.0f,64.0f}};

void fn_1_11F34(HUPROCESS *process);
void fn_1_11FBC(void);
void fn_1_11FC0(OMOBJ *obj);
void fn_1_11FFC(OMOBJ *node);
void fn_1_12024(OMOBJ *object);
void fn_1_1228C(OMOBJ *object);
void fn_1_12488(M676ScoreEntry *entry);
void fn_1_12514(s32 index, s32 amount);
void fn_1_12590(void);
s32 fn_1_125E4(s32 index);
void fn_1_1263C(s32 playerIndex, f32 *x, f32 *y);

/* Sequence setup: creates the score-box object and clears its work area. */
void fn_1_11F34(HUPROCESS *process)
{
    OMOBJ *object;
    void *data;

    object = omAddObjEx(process, 20, 0, 0, -1, fn_1_11FC0);
    lbl_1_bss_10B0 = object;
    data = HuMemDirectMallocNum(HEAP_HEAP, 116, HU_MEMNUM_OVL);
    object->data = data;
    memset(data, 0, 116);
}

void fn_1_11FBC(void)
{
}

void fn_1_11FC0(OMOBJ *obj) {
    fn_1_12024(obj);
    obj->objFunc = fn_1_11FFC;
}

void fn_1_11FFC(OMOBJ *node)
{
    fn_1_1228C(node);
}

/* Score-object setup: creates a score box, numeric score display, and an additional two-sprite
 * group for each player. */
void fn_1_12024(OMOBJ *object)
{
    M676ScoreWork *work = object->data;
    ANIMDATA *scoreBoxAnimation;
    ANIMDATA *scoreAnimation;
    s32 playerIndex;
    s32 spriteIndex;

    scoreBoxAnimation =
        HuSprAnimRead(HuDataSelHeapReadNum(fn_1_1480(12), HU_MEMNUM_OVL, HEAP_MODEL));
    scoreAnimation =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_mgconst, 48), HU_MEMNUM_OVL, HEAP_MODEL));
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        M676ScoreEntry *entry = &work->entries[playerIndex];
        s16 scoreBox = MgScoreBoxCreateChar(120, 48, GwPlayerConf[playerIndex].charNo);
        s16 spriteGroup;
        MGSCORE *score;

        entry->box = scoreBox;
        MgScoreBoxDispSet(scoreBox, 0);
        MgScoreBoxTPLvlSet(scoreBox, 0.0f);
        score = MgScoreCreate(DATANUM(DATA_mgconst, 49), -1, 1);
        entry->score = score;
        MgScoreMaxDigitSet(score, 2);
        MgScoreDispOff(score);
        MgScoreColorSet(score, 255, 216, 21);
        MgScorePriSet(score, 10);
        entry->animationState = 0;
        entry->digitScale = 1.0f;
        spriteGroup = HuSprGrpCreate(2);
        entry->extraSprites = spriteGroup;
        HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(scoreBoxAnimation, 50, 0));
        HuSprPosSet(spriteGroup, 0, -9.0f, 0.0f);
        HuSprGrpMemberSet(spriteGroup, 1, HuSprCreate(scoreAnimation, 50, 0));
        HuSprPosSet(spriteGroup, 1, 6.0f, 0.0f);
        HuSprBankSet(spriteGroup, 1, 15);
        HuSprColorSet(spriteGroup, 1, 255, 255, 255);
        for (spriteIndex = 0; spriteIndex < 2; spriteIndex++) {
            HuSprAttrSet(spriteGroup, (s16)spriteIndex, 1);
            HuSprAttrSet(spriteGroup, (s16)spriteIndex, 4);
        }
        entry->x = lbl_1_data_7B0[playerIndex].x;
        entry->y = lbl_1_data_7B0[playerIndex].y;
        fn_1_12488(entry);
    }
}

/* Score-object callback: displays the boxes and updates each player's score animation. */
void fn_1_1228C(OMOBJ *object) {
    M676ScoreWork *work = object->data;
    M676ScoreEntry *entry;
    MGSCORE *score;
    s16 box;
    s16 extraSprites;
    s32 playerIndex;
    s32 memberIndex;

    switch (work->displayState) {
    case 0:
        break;
    case 1:
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            entry = &work->entries[playerIndex];

            box = entry->box;
            MgScoreBoxDispSet(box, 1);
            MgScoreBoxTPLvlSet(box, 1.0f);
            score = entry->score;
            MgScoreDispOn(score);
            extraSprites = entry->extraSprites;
            for (memberIndex = 0; memberIndex < 2; memberIndex++) {
                HuSprAttrReset(extraSprites, (s16) memberIndex, 4);
            }
        }
        work->displayState++;
        /* fall through */
    case 2:
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            entry = &work->entries[playerIndex];

            switch (entry->animationState) {
            case 0:
                break;
            case 1:
                entry->animationState++;
                entry->digitScale = 1.0f;
                /* fall through */
            case 2:
                if ((entry->digitScale += 0.2) < 1.8) {
                    break;
                }
                entry->animationState++;
                /* fall through */
            case 3:
                if ((entry->digitScale -= 0.1) > 1.0) {
                    break;
                }
                entry->animationState = 0;
                entry->digitScale = 1.0f;
                break;
            }

            score = entry->score;
            if (entry->value > 99) {
                entry->value = 99;
            }
            MgScoreValueSet(score, entry->value);
            MgScoreDigitScaleSet(score, entry->digitScale, entry->digitScale);
        }
    }
}

/* Applies one player's configured screen position to the box, numeric score, and additional
 * sprites. */
void fn_1_12488(M676ScoreEntry *entry) {
    f32 scoreBoxX;
    f32 scoreBoxY;
    scoreBoxX = entry->x;
    scoreBoxY = entry->y;
    MgScoreBoxPosSet(entry->box, scoreBoxX, scoreBoxY);
    MgScorePosSet(entry->score, 22.0f + scoreBoxX, scoreBoxY);
    HuSprGrpPosSet(entry->extraSprites, scoreBoxX, scoreBoxY);
}

/* Adds a player's score change while the score-box display state is enabled. */
void fn_1_12514(s32 playerIndex, s32 scoreChange)
{
    M676ScoreWork *table;
    M676ScoreEntry *record;

    if (lbl_1_bss_10B0 != NULL) {
        table = lbl_1_bss_10B0->data;
        if (table->displayState != 0) {
            record = &table->entries[playerIndex];
            scoreChange += record->value;
            if (scoreChange > 99) {
                scoreChange = 99;
            }
            record->value = scoreChange;
            record->animationState = 1;
        }
    }
}

/* Called when scoring begins: enables the score-box display state. */
void fn_1_12590(void)
{
    M676ScoreWork *work;
    if (lbl_1_bss_10B0 != NULL) {
        work = lbl_1_bss_10B0->data;
        if (work->displayState == 0) { work->displayState++; }
    }
}

/* Returns a player's accumulated score, or zero before the score object exists. */
s32 fn_1_125E4(s32 playerIndex)
{
    s32 value;
    M676ScoreWork *table;

    value = 0;
    if (lbl_1_bss_10B0 != NULL) {
        table = lbl_1_bss_10B0->data;
        value = table->entries[playerIndex].value;
    }
    return value;
}

/* Returns a player's configured score-box screen coordinates when state exists. */
void fn_1_1263C(s32 playerIndex, f32 *x, f32 *y)
{
    M676ScoreWork *table;
    M676ScoreEntry *record;

    if (lbl_1_bss_10B0 != NULL) {
        table = lbl_1_bss_10B0->data;
        record = &table->entries[playerIndex];
        *x = record->x;
        *y = record->y;
    }
}
