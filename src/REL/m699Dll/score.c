/* Dunk Bros. score displays, score-change animation and game timer. */
#include "string.h"

#include "math.h"

#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/gx.h"

#include "dolphin/mtx.h"

#include "REL/m699Dll/m699_natural_views.h"

#include "game/object.h"

#include "game/memory.h"

#include "game/gamemes.h"

#include "game/mg/score.h"

#include "game/mg/seqman.h"

#include "game/data.h"

#include "game/esprite.h"

#include "dolphin.h"

#include "humath.h"

#include "datadir_enum.h"

#include "messdir_enum.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/audio.h"

#include "game/sprite.h"

#include "game/main.h"

#include "game/pad.h"

#include "game/window.h"

#include "game/wipe.h"

#include "game/frand.h"

#include "game/charman.h"

#include "game/process.h"

#include "game/hsfex.h"

#include "game/flag.h"

#include "game/armem.h"

#include "game/mgdata.h"

#include "game/printfunc.h"

#include "game/saveload.h"

#include "game/thpmain.h"

#include "game/mg/actman.h"

#include "game/mg/timer.h"

#include "game/board/object.h"

#include "game/board/main.h"

#include "game/board/audio.h"

#include "game/board/tutorial.h"

#include "game/board/masu.h"

#include "game/board/player.h"

#include "include/game/gamework.h"


extern void fn_1_1923C(OMOBJ *obj);

OMOBJ *lbl_1_bss_A60;

/* Create the score controller and clear its work allocation. */
void fn_1_191B0(HUPROCESS *process) {
    OMOBJ *object;
    void *data;

    object = fn_8003AB38(process, 0x14, 0, 0, -1, fn_1_1923C);
    lbl_1_bss_A60 = object;
    data = HuMemDirectMallocNum(HEAP_HEAP, 0x6DC, 0x10000000);
    object->data = data;
    memset(data, 0, 0x6DC);
}

void fn_1_19238(void) {
}

extern void fn_1_19288(OMOBJ *obj);

extern void fn_1_192C8(OMOBJ *controller);

extern void fn_1_195BC(OMOBJ *obj);

extern void fn_1_1AF38(OMOBJ *obj);

/* Initialize the score displays and timer, then install their update callback. */
void fn_1_1923C(OMOBJ *obj) {
    fn_1_192C8(obj);
    fn_1_195BC(obj);
    fn_1_1AF38(obj);
    obj->objFunc = fn_1_19288;
}

extern void fn_1_19434(OMOBJ *controller);

extern void fn_1_19B94(OMOBJ *obj);

extern void fn_1_1B2A0(OMOBJ *obj);

void fn_1_19288(OMOBJ *obj) {
    fn_1_19434(obj);
    fn_1_19B94(obj);
    fn_1_1B2A0(obj);
}


typedef struct M699ScoreItem {
    s32 value;
    s16 boxId;
    u8 unk_6[2]; /* No code names this field. */
    MGSCORE *score;
} M699ScoreItem;

typedef struct M699ScoreTransition {
    s32 active;
    u8 unk_4[8]; /* No code names this field. */
    s32 previousScoreA;
    s32 previousScoreB;
    s32 scoreA;
    s32 scoreB;
    s32 changedPlayer;
} M699ScoreTransition;

typedef struct M699ScoreWork {
    u8 unk_0[0x24]; /* No code names this field. */
    M699ScoreItem items[2];
    M699ScoreTransition transition;
} M699ScoreWork;

Vec lbl_1_data_EC8[2] = {{60.0f, 60.0f, 0.0f}, {516.0f, 60.0f, 0.0f}};

Vec lbl_1_data_EE0 = {288.0f, 56.0f, 0.0f};

void fn_1_192C8(OMOBJ *controller)
{
    M699ScoreWork *work;
    s16 boxId;
    s32 index;
    MGSCORE *score;

    work = controller->data;
    index = 0;
    while (index < 2) {
        work->items[index].boxId = boxId = MgScoreBoxCreate(0x58, 0x28);
        MgScoreBoxPosSet(boxId, lbl_1_data_EC8[index].x, lbl_1_data_EC8[index].y);
        MgScoreBoxDispSet(boxId, 0);
        if (index == 0) {
            MgScoreBoxColorSet(boxId, 0xA0, 0x40, 0x40);
        } else {
            MgScoreBoxColorSet(boxId, 0x40, 0x40, 0xA0);
        }
        score = MgScoreCreate(DATANUM(0x9b0000, 0x31), -1, 1);
        work->items[index].score = score;
        MgScoreMaxDigitSet(score, 3);
        MgScoreDispOff(score);
        MgScorePosSet(score, lbl_1_data_EC8[index].x - 20.0f,
                      lbl_1_data_EC8[index].y);
        MgScoreColorSet(score, 0xFF, 0xD8, 0x15);
        MgScorePriSet(score, 0xA);
        index++;
    }
}

typedef struct M699ScoreDisplayItem {
    s32 value;
    s16 boxId;
    u8 unk_6[2]; /* No code names this field. */
    MGSCORE *score;
} M699ScoreDisplayItem;

typedef struct M699DualScoreDisplayWork {
    s32 phase;
    f32 transitionAlpha;
    u8 unk_8[0x1C]; /* No code names this field. */
    M699ScoreDisplayItem items[2];
} M699DualScoreDisplayWork;

void fn_1_19434(OMOBJ *controller)
{
    M699DualScoreDisplayWork *work;
    s32 value;
    s16 boxId;
    MGSCORE *score;
    s32 index;

    work = controller->data;
    switch (work->phase) {
    case 0:
        if (fn_80071A8C() != 5) {
            break;
        }
        work->phase++;
        work->transitionAlpha = 0.0f;
        for (index = 0; index < 2; index++) {
            boxId = work->items[index].boxId;
            MgScoreBoxDispSet(boxId, 1);
            score = work->items[index].score;
            MgScoreDispOn(score);
        }
        /* fall through */
    case 1:
        if ((work->transitionAlpha += 0.1) >=
            1.0) {
            work->phase++;
        }
        for (index = 0; index < 2; index++) {
            boxId = work->items[index].boxId;
            MgScoreBoxTPLvlSet(boxId, work->transitionAlpha);
            score = work->items[index].score;
            MgScoreTPLvlSet(score, work->transitionAlpha);
        }
        break;
    case 2:
        break;
    }

    for (index = 0; index < 2; index++) {
        value = work->items[index].value;
        if (value > 0x3E7) {
            value = 0x3E7;
        }
        score = work->items[index].score;
        MgScoreValueSet(score, value);
    }
}

typedef struct M699SpriteAssetRecord {
    s32 imageIds[4];
} M699SpriteAssetRecord;

typedef struct M699SpriteDisplaySlot {
    s32 displayIds[2];
    u8 unk_8[8]; /* No code names this field. */
} M699SpriteDisplaySlot;

typedef struct M699SpriteClearGroup {
    s16 values[3];
    u8 unk_6[0x2E]; /* No code names this field. */
} M699SpriteClearGroup;

typedef struct M699SpriteLayoutWork {
    u8 unk_0[0x20]; /* No code names this field. */
    M699SpriteClearGroup clearGroups[2];
    s16 thirdDisplayEffectIds[3];
    u8 unk_8E[0x2E]; /* No code names this field. */
    s16 fourthDisplayEffectIds[3];
    u8 unk_C2[0x2E]; /* No code names this field. */
    M699SpriteDisplaySlot slots[3][30];
    s32 extraDisplayIds[2];
} M699SpriteLayoutWork;

typedef struct M699SpriteWork {
    u8 unk_0[0x3C]; /* No code names this field. */
    M699SpriteLayoutWork layout;
} M699SpriteWork;

M699SpriteAssetRecord lbl_1_data_EEC[11] = {
    {{DATANUM(0x240000, 0x12), 0, 0, 0}},
    {{DATANUM(0x240000, 0x13), 0, 0, 0}},
    {{DATANUM(0x240000, 0x14), 0, 0, 0}},
    {{DATANUM(0x240000, 0x15), 0, 0, 0}},
    {{DATANUM(0x240000, 0x16), 0, 0, 0}},
    {{DATANUM(0x240000, 0x17), 0, 0, 0}},
    {{DATANUM(0x240000, 0x18), 0, 0, 0}},
    {{DATANUM(0x240000, 0x19), 0, 0, 0}},
    {{DATANUM(0x240000, 0x1a), 0, 0, 0}},
    {{DATANUM(0x240000, 0x1b), 0, 0, 0}},
    {{DATANUM(0x240000, 0x50), 0, 0, 0}}
};

void fn_1_195BC(OMOBJ *object)
{
    M699SpriteWork *work;
    M699SpriteLayoutWork *layout;
    s32 group;
    s32 entry;
    s32 slot;

    work = object->data;
    layout = &work->layout;
    for (group = 0; group < 3; group++) {
        for (entry = 0; entry < 30; entry++) {
            for (slot = 0; slot < 2; slot++) {
                if (lbl_1_data_EEC[entry % 10].imageIds[slot] != 0) {
                    layout->slots[group][entry].displayIds[slot] =
                        espEntry(lbl_1_data_EEC[entry % 10].imageIds[slot], 0x31, 0);
                    espAttrSet((s16)layout->slots[group][entry].displayIds[slot], 8);
                    espDispOff((s16)layout->slots[group][entry].displayIds[slot]);
                } else {
                    layout->slots[group][entry].displayIds[slot] = -1;
                }
            }
        }
    }
    for (slot = 0; slot < 2; slot++) {
        if (lbl_1_data_EEC[10].imageIds[slot] != 0) {
            layout->extraDisplayIds[slot] = espEntry(lbl_1_data_EEC[10].imageIds[slot], 0x31, 0);
            espAttrSet((s16)layout->extraDisplayIds[slot], 8);
            espDispOff((s16)layout->extraDisplayIds[slot]);
        } else {
            layout->extraDisplayIds[slot] = -1;
        }
    }
    for (group = 0; group < 2; group++) {
        for (slot = 0; slot < 3; slot++) {
            layout->clearGroups[group].values[slot] = -1;
        }
    }
    for (slot = 0; slot < 3; slot++) {
        layout->thirdDisplayEffectIds[slot] = -1;
        layout->fourthDisplayEffectIds[slot] = -1;
    }
    fn_800097F4(lbl_1_data_EEC[0].imageIds[0]);
}

typedef struct M699EffectLayoutWork {
    s16 effectIds[3];
    u8 unk_6[2]; /* No code names this field. */
    f32 heights[3];
    f32 centerX;
    f32 centerY;
    f32 velocityX;
    f32 velocityY;
    f32 horizontalScale;
    f32 verticalScale;
    f32 alpha;
    f32 angle;
} M699EffectLayoutWork;

void fn_1_19870(M699EffectLayoutWork *work)
{
    s32 index;
    f32 horizontalOffset;
    f32 verticalOffset;

    index = 0;
    while (index < 3) {
        if (work->effectIds[index] >= 0) {
            espDispOn(work->effectIds[index]);
            horizontalOffset = (f32)((f64)(work->heights[index] * work->horizontalScale) *
                                     cos(M_PI*((f64)work->angle)/180.0));
            verticalOffset = (f32)((f64)(work->heights[index] * work->horizontalScale) *
                                   sin(M_PI*((f64)work->angle)/180.0));
            espPosSet(work->effectIds[index], work->centerX + horizontalOffset,
                      work->centerY + verticalOffset);
            espZRotSet(work->effectIds[index], work->angle);
            espScaleSet(work->effectIds[index], work->horizontalScale, work->verticalScale);
            espTPLvlSet(work->effectIds[index], work->alpha);
        }
        index++;
    }
}

typedef struct M699RankEntry {
    s32 value;
    s32 secondValue;
    u8 unk_8[8]; /* No code names this field. */
} M699RankEntry;

typedef M699EffectLayoutWork M699RankDisplay;

void fn_1_199C8(M699RankEntry *entries, M699RankDisplay *display, u32 value)
{
    u32 ones;
    u32 tens;
    u32 hundreds;

    hundreds = value / 100;
    tens = (value % 100) / 10;
    ones = value % 10;

    if (value < 10) {
        display->effectIds[0] = (s16)entries[ones].value;
        display->effectIds[1] = -1;
        display->effectIds[2] = -1;
        display->heights[0] = 0.0f;
    } else if (value < 100) {
        display->effectIds[0] = (s16)entries[10 + tens].value;
        display->effectIds[1] = (s16)entries[ones].value;
        display->effectIds[2] = -1;
        display->heights[0] = -23.039999f;
        display->heights[1] = 23.039999f;
    } else {
        display->effectIds[0] = (s16)entries[20 + hundreds].value;
        display->effectIds[1] = (s16)entries[10 + tens].value;
        display->effectIds[2] = (s16)entries[ones].value;
        display->heights[0] = -46.079998f;
        display->heights[1] = 0.0f;
        display->heights[2] = 46.079998f;
    }
    display->centerX = display->centerY = 0.0f;
    display->velocityX = display->velocityY = 0.0f;
    display->horizontalScale = display->verticalScale = 1.4f;
    display->alpha = 1.0f;
    display->angle = 0.0f;
}

typedef struct M699RankLayout {
    s32 phase;
    u32 frame;
    u32 pulseFrame;
    u32 previousScores[2]; /* Bit-preserving view; exact setter takes s32. */
    u32 scores[2];
    u32 changedPlayer;
    M699EffectLayoutWork displays[4];
    M699RankEntry digitEntries[3][30];
    M699RankEntry extraEntry;
} M699RankLayout;

typedef struct M699RankOwner {
    u8 precedingWork[0x3C]; /* Other score/time UI; not reconstructed by this packet. */
    M699RankLayout rank;
} M699RankOwner;


/* Advance the score-change animation and update its digit effects. */
void fn_1_19B94(OMOBJ *object) {
    M699RankOwner *owner;
    f32 offsetY;
    f32 animationValue;
    f32 scaleOffset;
    f32 velocityX;

    s32 entryIndex;
    s32 groupIndex;
    s32 slotIndex;
    u32 firstPreviousScore;
    u32 secondPreviousScore;
    u32 changedScore;
    M699RankEntry *entry;
    M699RankLayout *rank;
    M699EffectLayoutWork *changedDisplay;
    owner = object->data;
    rank = &owner->rank;
    velocityX = 2.88f;
    if ((u32) rank->changedPlayer == 0)
    {
        changedDisplay = &rank->displays[0];
    }
    else
    {
        changedDisplay = &rank->displays[1];
        velocityX *= (-1.0f);
    }
    switch (rank->phase)
    {
    case 1 :
        firstPreviousScore = rank->previousScores[0];
        fn_1_199C8(rank->digitEntries[0], &rank->displays[0], firstPreviousScore);
        secondPreviousScore = rank->previousScores[1];
        fn_1_199C8(rank->digitEntries[1], &rank->displays[1], secondPreviousScore);
        fn_1_199C8(&rank->extraEntry, &rank->displays[2], 0);
        changedScore = rank->scores[rank->changedPlayer];
        fn_1_199C8(rank->digitEntries[2], &rank->displays[3], changedScore);
        offsetY = -48.0f;
        rank->displays[0].centerX = 144.0f;
        rank->displays[0].centerY = 360.0f - offsetY;
        rank->displays[1].centerX = 432.0f;
        rank->displays[1].centerY = 360.0f - offsetY;
        rank->displays[2].centerX = 288.0f;
        rank->displays[2].centerY = 360.0f - offsetY;
        rank->displays[0].alpha = rank->displays[1].alpha =
            rank->displays[2].alpha = 0.0f;
        offsetY /= 19.199999f;
        rank->displays[0].velocityY = offsetY;
        rank->displays[1].velocityY = offsetY;
        rank->displays[2].velocityY = offsetY;
        rank->displays[3].centerX = 806.39996f;
        rank->displays[3].centerY = 144.0f;
        rank->displays[3].horizontalScale += 0.5f;
        rank->displays[3].verticalScale = (f32)(rank->displays[3].verticalScale - 0.5f);
        rank->displays[3].velocityX = (f32)((432.0f - rank->displays[3].centerX) / 24.0f);
        rank->displays[3].velocityY = (f32)((360.0f - rank->displays[3].centerY) / 24.0f);
        if ((u32) rank->changedPlayer == 0)
        {
            rank->displays[3].centerX = - 230.40001f;
            rank->displays[3].velocityX *= -1.0f;
        }
        rank->phase = (s32)(rank->phase + 1);
        rank->frame = 0U;
        break;
    case 2 :
        animationValue = rank->displays[0].alpha;
        animationValue += 0.055555556f;
        if (animationValue > 0.95f)
        {
            animationValue = 1.0f;
        }
        rank->displays[2].alpha = animationValue;
        rank->displays[1].alpha = animationValue;
        rank->displays[0].alpha = animationValue;
        rank->displays[0].centerY = (f32)(rank->displays[0].centerY + rank->displays[0].velocityY);
        rank->displays[1].centerY = (f32)(rank->displays[1].centerY + rank->displays[1].velocityY);
        rank->displays[2].centerY = (f32)(rank->displays[2].centerY + rank->displays[2].velocityY);
        rank->displays[3].centerX = (f32)(rank->displays[3].centerX + rank->displays[3].velocityX);
        rank->displays[3].centerY = (f32)(rank->displays[3].centerY + rank->displays[3].velocityY);
        if ((f32) rank->frame >= 19.199999f)
        {
            changedDisplay->velocityX = velocityX;
            changedDisplay->velocityY = - 16.0f;
            rank->phase = (s32)(rank->phase + 1);
            rank->pulseFrame = 0U;
        }
        break;
    case 3 :
        if ((f32) rank->frame <= 24.0f)
        {
            rank->displays[3].centerX = (f32)(rank->displays[3].centerX + rank->displays[3].velocityX);
            rank->displays[3].centerY = (f32)(rank->displays[3].centerY + rank->displays[3].velocityY);
        }
        if ((f32) rank->pulseFrame < 24.0f)
        {
            rank->pulseFrame = (u32)(rank->pulseFrame + 1);
            animationValue = 33.75f * (f32) rank->pulseFrame;
            scaleOffset = 0.041666668f * (f32) rank->pulseFrame;
            scaleOffset = 0.5f * (1.0f - (scaleOffset * scaleOffset));
            if ((f32) rank->pulseFrame == 24.0f)
            {
                scaleOffset = 0.0f;
            }
            rank->displays[3].horizontalScale = (f32)(1.4f + ((f64) scaleOffset * cos(M_PI*((f64) animationValue)/180.0)));
            rank->displays[3].verticalScale = (f32)(1.4f + ((f64) scaleOffset * sin(M_PI*((f64)(animationValue - 90.0f))/180.0)));
        }
        changedDisplay->horizontalScale = (f32) rank->displays[3].verticalScale;
        changedDisplay->verticalScale = (f32) rank->displays[3].horizontalScale;
        changedDisplay->centerX = (f32)(changedDisplay->centerX + changedDisplay->velocityX);
        changedDisplay->centerY = (f32)(changedDisplay->centerY + changedDisplay->velocityY);
        changedDisplay->velocityY += 0.8;
        changedDisplay->angle += 8.333333333333334;
        changedDisplay->alpha = (f32)((f64) changedDisplay->alpha - 0.01783333420753479);
        if (changedDisplay->alpha < 0.0f)
        {
            changedDisplay->alpha = 0.0f;
        }
        if ((f32) rank->frame >= 102.0f)
        {
            rank->phase = (s32)(rank->phase + 1);
            rank->frame = 0U;
        }
        break;
    case 4 :
        animationValue = rank->displays[3].alpha;
        animationValue -= 0.055555556f;
        if (animationValue < 0.05f)
        {
            animationValue = 0.0f;
            rank->phase = (s32)(rank->phase + 1);
            rank->frame = 0U;
        }
        rank->displays[3].alpha = animationValue;
        rank->displays[2].alpha = animationValue;
        rank->displays[1].alpha = animationValue;
        rank->displays[0].alpha = animationValue;
        break;
    case 0 :
        break;
    default :
        for (groupIndex = 0; groupIndex < 2; groupIndex++)
        {
            for (slotIndex = 0; slotIndex < 3; slotIndex++)
            {
                rank->displays[groupIndex].effectIds[slotIndex] = - 1;
            }
        }
        for (slotIndex = 0; slotIndex < 3; slotIndex++)
        {
            rank->displays[2].effectIds[slotIndex] = - 1;
            rank->displays[3].effectIds[slotIndex] = - 1;
        }
        rank->phase = 0;
        break;
    }
    for (groupIndex = 0; groupIndex < 3; groupIndex++)
    {
        entry = rank->digitEntries[groupIndex];
        for (entryIndex = 0; entryIndex < 0x14; entryIndex++, entry++)
        {
            if ((s32) entry->value >= 0)
            {
                espDispOff((s16) entry->value);
            }
            if ((s32) entry->secondValue >= 0)
            {
                espDispOff((s16) entry->secondValue);
            }
        }
    }
    entry = &rank->extraEntry;
    if ((s32) entry->value >= 0)
    {
        espDispOff((s16) entry->value);
    }
    if ((s32) entry->secondValue >= 0)
    {
        espDispOff((s16) entry->secondValue);
    }
    fn_1_19870(&rank->displays[0]);
    fn_1_19870(&rank->displays[1]);
    fn_1_19870(&rank->displays[2]);
    fn_1_19870(&rank->displays[3]);
    rank->frame = (u32)(rank->frame + 1);
}

void fn_1_1AEDC(s32 firstScore, s32 secondScore, s32 changedPlayer)
{
    OMOBJ *object = lbl_1_bss_A60;
    M699RankOwner *owner = object->data;
    M699RankLayout *rank = &owner->rank;

    rank->phase = 1;
    rank->previousScores[0] = rank->scores[0];
    rank->previousScores[1] = rank->scores[1];
    rank->scores[0] = firstScore;
    rank->scores[1] = secondScore;
    rank->changedPlayer = changedPlayer;
}

typedef struct M699ScoreDisplayWork {
    u8 unk_0[8]; /* No code names this field. */
    s32 state;
    f32 transparency;
    s32 time;
    s16 scoreBox;
    s16 spriteGroup;
    MGSCORE *hours;
    MGSCORE *minutes;
    MGSCORE *seconds;
} M699ScoreDisplayWork;

typedef struct M699ScoreSetupWork {
    u8 unk_0[0x10]; /* No code names this field. */
    s32 gameDuration;
    s16 scoreBox;
    s16 spriteGroup;
    MGSCORE *hours;
    MGSCORE *minutes;
    MGSCORE *seconds;
} M699ScoreSetupWork;

void fn_1_1AF38(OMOBJ *object)
{
    M699ScoreSetupWork *work;
    MGSCORE *score;
    ANIMDATA *digitAnim;
    s16 scoreBox;
    s16 spriteGroup;
    s32 gameDuration;
    s32 spriteIndex;

    work = object->data;
    switch (MgGameNo) {
    case 1:
        gameDuration = 0xE10;
        break;
    case 2:
        gameDuration = 0x2A30;
        break;
    case 3:
        gameDuration = 0x4650;
        break;
    default:
        gameDuration = 0x2A30;
        break;
    }
    work->gameDuration = gameDuration;
    work->scoreBox = scoreBox = MgScoreBoxCreate(0x88, 0x20);
    MgScoreBoxPosSet(scoreBox, lbl_1_data_EE0.x, lbl_1_data_EE0.y);
    MgScoreBoxDispSet(scoreBox, 0);
    score = MgScoreCreate(DATANUM(0x9b0000, 0x30), -1, 1);
    work->hours = score;
    MgScoreMaxDigitSet(score, 1);
    MgScoreDispOff(score);
    MgScorePosSet(score, lbl_1_data_EE0.x - 48.0f, lbl_1_data_EE0.y);
    MgScoreColorSet(score, 0xFF, 0xFF, 0xFF);
    MgScorePriSet(score, 0xA);
    score = MgScoreCreate(DATANUM(0x9b0000, 0x30), -1, 1);
    work->minutes = score;
    MgScoreMaxDigitSet(score, 2);
    MgScoreDispOff(score);
    MgScorePosSet(score, lbl_1_data_EE0.x - 16.0f, lbl_1_data_EE0.y);
    MgScoreColorSet(score, 0xFF, 0xFF, 0xFF);
    MgScorePriSet(score, 0xA);
    score = MgScoreCreate(DATANUM(0x9b0000, 0x30), -1, 1);
    work->seconds = score;
    MgScoreMaxDigitSet(score, 2);
    MgScoreDispOff(score);
    MgScorePosSet(score, 32.0f + lbl_1_data_EE0.x, lbl_1_data_EE0.y);
    MgScoreColorSet(score, 0xFF, 0xFF, 0xFF);
    MgScorePriSet(score, 0xA);
    spriteGroup = HuSprGrpCreate(2);
    work->spriteGroup = spriteGroup;
    HuSprGrpPosSet(spriteGroup, lbl_1_data_EE0.x, lbl_1_data_EE0.y);
    digitAnim = work->hours->digitAnim;
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(digitAnim, 0x32, 0xA));
    HuSprPosSet(spriteGroup, 0, -32.0f, 0.0f);
    HuSprColorSet(spriteGroup, 0, 0xFF, 0xFF, 0xFF);
    HuSprGrpMemberSet(spriteGroup, 1, HuSprCreate(digitAnim, 0x32, 0xB));
    HuSprPosSet(spriteGroup, 1, 16.0f, 0.0f);
    HuSprColorSet(spriteGroup, 1, 0xFF, 0xFF, 0xFF);
    for (spriteIndex = 0; spriteIndex < 2; spriteIndex++) {
        HuSprAttrSet(spriteGroup, (s16)spriteIndex, 1);
        HuSprAttrSet(spriteGroup, (s16)spriteIndex, 4);
    }
}

void fn_1_1B2A0(OMOBJ *object)
{
    M699ScoreDisplayWork *work;
    s32 remainingTime;
    s16 spriteGroup;
    s32 spriteIndex;
    s32 displayedValue;

    work = object->data;
    switch (work->state) {
    case 0:
        if (fn_80071A8C() != 3) {
            break;
        }
        work->state++;
        work->transparency = 0.0f;
        MgScoreBoxDispSet(work->scoreBox, 1);
        MgScoreDispOn(work->hours);
        MgScoreDispOn(work->minutes);
        MgScoreDispOn(work->seconds);
        spriteGroup = work->spriteGroup;
        for (spriteIndex = 0; spriteIndex < 2; spriteIndex++) {
            HuSprAttrReset(spriteGroup, (s16)spriteIndex, 4);
        }
        /* fall through */
    case 1:
        if ((work->transparency += 0.1) < 1.0) {
            break;
        }
        work->state++;
        work->transparency = 1.0f;
        /* fall through */
    case 2:
        if (fn_80071A8C() != 5) {
            break;
        }
        work->state++;
        /* fall through */
    case 3:
        if (--work->time > 0) {
            break;
        }
        work->state++;
        /* fall through */
    case 4:
        if ((work->transparency -= 0.1) <= 0.0) {
            work->state++;
            work->transparency = 0.0f;
            MgScoreBoxDispSet(work->scoreBox, 0);
            MgScoreDispOff(work->hours);
            MgScoreDispOff(work->minutes);
            MgScoreDispOff(work->seconds);
            spriteGroup = work->spriteGroup;
            for (spriteIndex = 0; spriteIndex < 2; spriteIndex++) {
                HuSprAttrSet(spriteGroup, (s16)spriteIndex, 4);
            }
        }
        break;
    case 5:
        break;
    }
    MgScoreBoxTPLvlSet(work->scoreBox, work->transparency);
    MgScoreTPLvlSet(work->hours, work->transparency);
    MgScoreTPLvlSet(work->minutes, work->transparency);
    MgScoreTPLvlSet(work->seconds, work->transparency);
    spriteGroup = work->spriteGroup;
    for (spriteIndex = 0; spriteIndex < 2; spriteIndex++) {
        HuSprTPLvlSet(spriteGroup, (s16)spriteIndex, work->transparency);
    }
    remainingTime = work->time;
    displayedValue = remainingTime / 3600;
    remainingTime -= displayedValue * 3600;
    MgScoreValueSet(work->hours, displayedValue);
    displayedValue = remainingTime / 60;
    MgScoreValueSet(work->minutes, displayedValue);
    displayedValue = remainingTime % 60;
    MgScoreValueSet(work->seconds, (displayedValue * 0x64) / 60);
}


void fn_1_1B5A0(s32 player, s32 points)
{
    M699ScoreWork *displayWork;
    M699ScoreItem *score;
    M699ScoreTransition *transition;
    s32 scoreA;
    s32 scoreB;
    OMOBJ *displayObject;
    M699ScoreWork *scoreWork;

    scoreWork = lbl_1_bss_A60->data;
    score = &scoreWork->items[player];
    points += score->value;
    if (points > 999) {
        points = 999;
    }
    score->value = points;
    scoreB = scoreWork->items[1].value;
    scoreA = scoreWork->items[0].value;
    displayObject = lbl_1_bss_A60;
    displayWork = displayObject->data;
    transition = &displayWork->transition;
    transition->active = 1;
    transition->previousScoreA = transition->scoreA;
    transition->previousScoreB = transition->scoreB;
    transition->scoreA = scoreA;
    transition->scoreB = scoreB;
    transition->changedPlayer = player;
    HuAudFXPlay(0x927);
}

s32 fn_1_1B650(s32 index)
{
    OMOBJ *object;
    M699ScoreWork *data;
    object = lbl_1_bss_A60;
    data = object->data;
    return data->items[index].value;
}



void *fn_1_8F58(s32 index);

s32 fn_1_1B688(void) { return 0; }


/* Return the remaining game time, ending the timer during sequence state 6. */
s32 fn_1_1B690(void) {
    OMOBJ *object;
    M699SequenceWork *work;

    object = lbl_1_bss_A60;
    work = (M699SequenceWork *)object->data;
    if (fn_80071A8C() == 6U) {
        work->gameTimeRemaining = 0;
    }
    return work->gameTimeRemaining;
}
