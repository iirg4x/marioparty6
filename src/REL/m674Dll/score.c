/* Score display creation, animation, and value updates for all four players. */
#include "dolphin/math.h"
#include "game/object.h"
#include "game/main.h"
#include "game/gamework.h"
#include "game/gamemes.h"
#include "game/sprite.h"
#include "game/memory.h"
#include "game/data.h"
#include "REL/m674Dll/score.h"
#include "datadir_enum.h"
#include "string.h"
#include "REL/m674Dll/main.h"

#define M674_SCORE_WORK_SIZE 116
#define M674_SCORE_DATA_GROUP 0x10000000
#define M674_SCORE_BOX_DATA_INDEX 13
#define M674_SCORE_DATA_HEAP 2
#define M674_SCORE_BOX_X 120
#define M674_SCORE_BOX_Y 48
#define M674_SCORE_STYLE 10158129
#define M674_SCORE_COLOR_RED 255
#define M674_SCORE_COLOR_GREEN 216
#define M674_SCORE_COLOR_BLUE 21
#define M674_SCORE_PRIORITY 10
#define M674_SCORE_SPRITE_PRIORITY 50
#define M674_SCORE_DIGIT_BANK 15
#define M674_SCORE_SPRITE_WHITE 255
#define M674_SCORE_SPRITE_ATTR_4 4

OMOBJ *lbl_1_bss_6C8;
Vec lbl_1_data_680[4] = {
    {76.0f, 64.0f, 0.0f},
    {192.4f, 64.0f, 0.0f},
    {383.6f, 64.0f, 0.0f},
    {500.0f, 64.0f, 0.0f}
};
void fn_1_FAD4(OMOBJ *object);
void fn_1_FB10(OMOBJ *object);
void fn_1_FB38(OMOBJ *object);
void fn_1_FDA0(OMOBJ *object);
void fn_1_FF9C(MinigameScoreEntry *entry);

/* Called by ObjectSetup during setup to create the score display object. */
void fn_1_FA48(HUPROCESS *process)
{
    OMOBJ *object;
    void *scoreWork;

    object = fn_8003AB38(process, 20, 0, 0, -1, fn_1_FAD4);
    lbl_1_bss_6C8 = object;
    scoreWork = HuMemDirectMallocNum(HEAP_HEAP, M674_SCORE_WORK_SIZE, 0x10000000);
    object->data = scoreWork;
    memset(scoreWork, 0, M674_SCORE_WORK_SIZE);
}

void fn_1_FAD0(void)
{
}

/* Called by the object manager to initialize the display, then install updates. */
void fn_1_FAD4(OMOBJ *obj) {
    fn_1_FB38(obj);
    obj->objFunc = fn_1_FB10;
}

/* Called by the object manager each frame to update score displays. */
void fn_1_FB10(OMOBJ *object) {
    fn_1_FDA0(object);
}

/* Called by fn_1_FAD4 to create each player's score box and digit sprites. */
void fn_1_FB38(OMOBJ *object)
{
    M674ScoreWork *work = object->data;
    ANIMDATA *scoreBoxAnimation;
    ANIMDATA *scoreAnimation;
    s32 playerIndex;
    s32 spriteIndex;

    scoreBoxAnimation = HuSprAnimRead(fn_80008DD0(fn_1_E18(M674_SCORE_BOX_DATA_INDEX),
        M674_SCORE_DATA_GROUP, M674_SCORE_DATA_HEAP));
    scoreAnimation =
        HuSprAnimRead(fn_80008DD0(DATANUM(0x9b0000, 48), M674_SCORE_DATA_GROUP,
            M674_SCORE_DATA_HEAP));
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        MinigameScoreEntry *entry = &work->entries[playerIndex];
        s16 scoreBox = MgScoreBoxCreateChar(M674_SCORE_BOX_X, M674_SCORE_BOX_Y,
            lbl_80265728[playerIndex].charNo);
        s16 spriteGroup;
        MGSCORE *score;

        entry->box = scoreBox;
        MgScoreBoxDispSet(scoreBox, 0);
        MgScoreBoxTPLvlSet(scoreBox, 0.0f);
        score = MgScoreCreate(M674_SCORE_STYLE, -1, 1);
        entry->score = score;
        MgScoreMaxDigitSet(score, 2);
        MgScoreDispOff(score);
        MgScoreColorSet(score, M674_SCORE_COLOR_RED, M674_SCORE_COLOR_GREEN,
            M674_SCORE_COLOR_BLUE);
        MgScorePriSet(score, M674_SCORE_PRIORITY);
        entry->animationState = 0;
        entry->digitScale = 1.0f;
        spriteGroup = HuSprGrpCreate(2);
        entry->decorationGroup = spriteGroup;
        HuSprGrpMemberSet(spriteGroup, 0,
                          HuSprCreate(scoreBoxAnimation, M674_SCORE_SPRITE_PRIORITY, 0));
        HuSprPosSet(spriteGroup, 0, -9.0f, 0.0f);
        HuSprGrpMemberSet(spriteGroup, 1,
                          HuSprCreate(scoreAnimation, M674_SCORE_SPRITE_PRIORITY, 0));
        HuSprPosSet(spriteGroup, 1, 6.0f, 0.0f);
        HuSprBankSet(spriteGroup, 1, M674_SCORE_DIGIT_BANK);
        HuSprColorSet(spriteGroup, 1, M674_SCORE_SPRITE_WHITE, M674_SCORE_SPRITE_WHITE,
            M674_SCORE_SPRITE_WHITE);
        for (spriteIndex = 0; spriteIndex < 2; spriteIndex++) {
            HuSprAttrSet(spriteGroup, (s16)spriteIndex, HUSPR_ATTR_NOANIM);
            HuSprAttrSet(spriteGroup, (s16)spriteIndex, M674_SCORE_SPRITE_ATTR_4);
        }
        entry->x = lbl_1_data_680[playerIndex].x;
        entry->y = lbl_1_data_680[playerIndex].y;
        fn_1_FF9C(entry);
    }
}

/* Called each frame to reveal the gameplay score display and animate changed totals. */
void fn_1_FDA0(OMOBJ *object) {
    M674ScoreWork *work = object->data;
    MinigameScoreEntry *entry;
    MGSCORE *score;
    s16 box;
    s16 decorationGroup;
    s32 playerIndex;
    s32 memberIndex;

    switch (work->state) {
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
            decorationGroup = entry->decorationGroup;
            for (memberIndex = 0; memberIndex < 2; memberIndex++) {
                HuSprAttrReset(decorationGroup, (s16) memberIndex, 4);
            }
        }
        work->state++;
        /* Begin the score animation immediately after showing the display. */
    case 2:
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            entry = &work->entries[playerIndex];

            switch (entry->animationState) {
            case 0:
                break;
            case 1:
                entry->animationState++;
                entry->digitScale = 1.0f;
                /* Continue growing the changed score. */
            case 2:
                if ((entry->digitScale += 0.2) < 1.8) {
                    break;
                }
                entry->animationState++;
                /* Shrink the score back to its resting size. */
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

/* Called during setup to place one player's score box and digit sprites. */
void fn_1_FF9C(MinigameScoreEntry *entry) {
    f32 x;
    f32 y;
    x = entry->x;
    y = entry->y;
    MgScoreBoxPosSet(entry->box, x, y);
    MgScorePosSet(entry->score, 22.0f + x, y);
    HuSprGrpPosSet(entry->decorationGroup, x, y);
}

/* Called by gameplay score updates to add coins and start the player's pop animation. */
void fn_1_10028(s32 scoreIndex, s32 coinAmount)
{
    M674ScoreWork *work;
    MinigameScoreEntry *entry;

    if (lbl_1_bss_6C8 != NULL) {
        work = lbl_1_bss_6C8->data;
        if (work->state != 0) {
            entry = &work->entries[scoreIndex];
            coinAmount += entry->value;
            if (coinAmount > 99) {
                coinAmount = 99;
            }
            entry->value = coinAmount;
            entry->animationState = 1;
        }
    }
}

/* Called by the PREMAIN sequence hook to request the gameplay score display's reveal. */
void fn_1_100A4(void) {
    M674ScoreWork *work;
    if (lbl_1_bss_6C8 != NULL) {
        work = lbl_1_bss_6C8->data;
        if (work->state == 0) {
            work->state += 1;
        }
    }
}

/* Called by results logic to read one player's score, or zero before setup. */
s32 fn_1_100F8(s32 scoreIndex)
{
    s32 value;
    M674ScoreWork *table;

    value = 0;
    if (lbl_1_bss_6C8 != NULL) {
        table = lbl_1_bss_6C8->data;
        value = table->entries[scoreIndex].value;
    }
    return value;
}

/* Called by collection-flight setup to read the player's score-box screen position. */
void fn_1_10150(s32 scoreIndex, f32 *x, f32 *y)
{
    M674ScoreWork *table;
    MinigameScoreEntry *record;

    if (lbl_1_bss_6C8 != NULL) {
        table = lbl_1_bss_6C8->data;
        record = &table->entries[scoreIndex];
        *x = record->x;
        *y = record->y;
    }
}
