/* Draws and updates the four player score displays. */
#include "dolphin/math.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/gamemes.h"
#include "game/mg/score.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/sprite.h"
#include "datadir_enum.h"
#include "string.h"

typedef struct ScoreDisplayEntry {
    s32 value; /* Displayed score, capped at 99. */
    f32 x; /* Score-box position in screen pixels. */
    f32 y; /* Score-box position in screen pixels. */
    s16 box; /* Score-panel group containing the player's character tile. */
    s16 decorationGroup; /* Sprite group for the two static score decorations. */
    MGSCORE *score; /* Numeric score renderer. */
    s32 animationState; /* Digit pop-animation phase. */
    f32 digitScale; /* Uniform scale used by the pop animation. */
} ScoreDisplayEntry;

typedef struct ScoreDisplayWork {
    s32 displayState; /* 0 hidden, 1 reveal requested, 2 updating. */
    ScoreDisplayEntry entries[4]; /* One score display for each player. */
} ScoreDisplayWork;

typedef struct ScoreDisplayPosition {
    f32 x; /* Screen-space horizontal position in pixels. */
    f32 y; /* Screen-space vertical position in pixels. */
    f32 unusedCoordinate; /* Third stored coordinate, unread by the score display. */
} ScoreDisplayPosition;

OMOBJ *lbl_1_bss_540;
ScoreDisplayPosition lbl_1_data_860[4] = {
    {76.0f, 416.0f, 0.0f},
    {192.4f, 416.0f, 0.0f},
    {383.6f, 416.0f, 0.0f},
    {5e+02f, 416.0f, 0.0f},
};

s32 fn_1_A38(s32 dataIndex);
void fn_1_EA34(OMOBJ *object);
void fn_1_EA70(OMOBJ *object);
void fn_1_EA98(OMOBJ *object);
void fn_1_ED00(OMOBJ *object);
void fn_1_EEFC(ScoreDisplayEntry *entry);

/* Called by fn_1_A0 during setup to create the score-display object. */
void fn_1_E9A8(HUPROCESS *process)
{
    OMOBJ *object;
    void *displayData;

    object = omAddObjEx(process, 20, 0, 0, -1, fn_1_EA34);
    lbl_1_bss_540 = object;
    displayData = HuMemDirectMallocNum(HEAP_HEAP, 116, HU_MEMNUM_OVL);
    object->data = displayData;
    memset(displayData, 0, 116);
}

void fn_1_EA30(void)
{
}

/* Runs on the score object's first update to build the four player displays. */
void fn_1_EA34(OMOBJ *object) {
    fn_1_EA98(object);
    object->objFunc = fn_1_EA70;
}

/* Runs on every later score-object update to refresh the displays. */
void fn_1_EA70(OMOBJ *object)
{
    fn_1_ED00(object);
}

/* Called by fn_1_EA34 to create each player's score box and digit sprites. */
void fn_1_EA98(OMOBJ *object)

{

    ScoreDisplayWork *work = object->data;

    ANIMDATA *scoreBoxAnimation;

    ANIMDATA *scoreAnimation;

    s32 playerIndex;

    s32 spriteIndex;

    scoreBoxAnimation =
        HuSprAnimRead(HuDataSelHeapReadNum(fn_1_A38(16), HU_MEMNUM_OVL, HEAP_MODEL));

    scoreAnimation = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mgconst, 48), HU_MEMNUM_OVL, HEAP_MODEL));

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {

        ScoreDisplayEntry *entry = &work->entries[playerIndex];

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

        entry->decorationGroup = spriteGroup;

        HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(scoreBoxAnimation, 50, 0));

        HuSprPosSet(spriteGroup, 0, -9.0f, 0.0f);

        HuSprGrpMemberSet(spriteGroup, 1, HuSprCreate(scoreAnimation, 50, 0));

        HuSprPosSet(spriteGroup, 1, 6.0f, 0.0f);

        HuSprBankSet(spriteGroup, 1, 15);

        HuSprColorSet(spriteGroup, 1, 255, 255, 255);

        for (spriteIndex = 0; spriteIndex < 2; spriteIndex++) {

            HuSprAttrSet(spriteGroup, (s16)spriteIndex, HUSPR_ATTR_NOANIM);

            HuSprAttrSet(spriteGroup, (s16)spriteIndex, HUSPR_ATTR_DISPOFF);

        }

        entry->x = lbl_1_data_860[playerIndex].x;

        entry->y = lbl_1_data_860[playerIndex].y;

        fn_1_EEFC(entry);

    }

}

/* The score-object update reveals requested displays and refreshes their digits each frame. */
void fn_1_ED00(OMOBJ *object) {
    ScoreDisplayWork *work = object->data;
    ScoreDisplayEntry *entry;
    MGSCORE *score;
    s16 box;
    s16 decorationGroup;
    s32 playerIndex;
    s32 decorationIndex;

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
            decorationGroup = entry->decorationGroup;
            for (decorationIndex = 0; decorationIndex < 2; decorationIndex++) {
                HuSprAttrReset(decorationGroup, (s16) decorationIndex, HUSPR_ATTR_DISPOFF);
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

/* Called by fn_1_EA98 to place a player's box and digit sprites together. */
void fn_1_EEFC(ScoreDisplayEntry *entry)
{
    f32 x;
    f32 y;
    x = entry->x;
    y = entry->y;
    MgScoreBoxPosSet(entry->box, x, y);
    MgScorePosSet(entry->score, 22.0f + x, y);
    HuSprGrpPosSet(entry->decorationGroup, x, y);
}

/* Gameplay adds collected rewards here while displays are active, starting the digit pop. */
void fn_1_EF88(s32 playerIndex, s32 amount)

{

    ScoreDisplayWork *table;

    ScoreDisplayEntry *record;

    if (lbl_1_bss_540 != NULL) {

        table = lbl_1_bss_540->data;

        if (table->displayState != 0) {

            record = &table->entries[playerIndex];

            amount += record->value;

            if (amount > 99) {

                amount = 99;

            }

            record->value = amount;

            record->animationState = 1;

        }

    }

}

/* The Banana Shake sequence calls this when the four score displays should appear. */
void fn_1_F004(void)
{
    ScoreDisplayWork *work;
    if (lbl_1_bss_540 != NULL) {
        work = lbl_1_bss_540->data;
        if (work->displayState == 0) {
            work->displayState += 1;
        }
    }
}

/* Gameplay and computer-player logic call this to read a player's displayed score. */
s32 fn_1_F058(s32 playerIndex)

{

    s32 value;

    ScoreDisplayWork *table;

    value = 0;

    if (lbl_1_bss_540 != NULL) {

        table = lbl_1_bss_540->data;

        value = table->entries[playerIndex].value;

    }

    return value;

}

/* Gameplay calls this to place objects relative to a player's score display. */
void fn_1_F0B0(s32 playerIndex, f32 *xPosition, f32 *yPosition)

{

    ScoreDisplayWork *table;

    ScoreDisplayEntry *record;

    if (lbl_1_bss_540 != NULL) {

        table = lbl_1_bss_540->data;

        record = &table->entries[playerIndex];

        *xPosition = record->x;

        *yPosition = record->y;

    }

}
