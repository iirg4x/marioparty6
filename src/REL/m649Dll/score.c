/* Creates the four score displays and accumulates Stamp By Me's gameplay scores. */
#include "dolphin/math.h"

#include "game/object.h"

#include "game/memory.h"

#include "string.h"

#include "game/main.h"

#include "game/charman.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

#include "REL/m649Dll/module_types.h"

#include "game/gamemes.h"

#include "game/mg/score.h"

#include "dolphin/gx.h"

#include "game/audio.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/mg/seqman.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "include/game/gamework.h"

typedef struct M649ScoreEntryView_s {
    s32 points; /* Current player score, capped at 99. */
    float x, y; /* Score box position in screen coordinates. */
    s16 box; /* Player's score-box sprite group. */
    MGSCORE *score; /* Number sprite displaying the current score. */
} M649ScoreEntryView;

typedef struct M649ScoreWorkView_s {
    s32 displayState; /* 0 leaves visibility unchanged, 1 shows the displays, and 2 keeps them
                       * visible. */
    M649ScoreEntryView entries[4]; /* Scores and display handles in player order. */
} M649ScoreWorkView;

void fn_1_A2AC(OMOBJ *obj);

extern OMOBJ *lbl_1_bss_5A8;

extern void fn_1_A2E8(OMOBJ *node);

void fn_1_A310(OMOBJ *object);

void fn_1_A434(OMOBJ *object);

static Vec lbl_1_data_560[4] = {
    {151.0f, 214.0f, 0.0f}, {425.0f, 214.0f, 0.0f},
    {151.0f, 416.0f, 0.0f}, {425.0f, 416.0f, 0.0f}
};

void fn_1_A528(void *entry);

/* fn_1_A0 creates this object at startup; its first update builds the player score displays. */
void fn_1_A220(OMOBJMAN *manager)
{
    OMOBJ *object;
    void *scoreWork;

    object = omAddObjEx(manager, 20, 0, 0, -1, fn_1_A2AC);
    lbl_1_bss_5A8 = object;
    scoreWork = HuMemDirectMallocNum(HEAP_HEAP, 84, HU_MEMNUM_OVL);
    object->data = scoreWork;
    memset(scoreWork, 0, 84);
}

/* fn_1_BB0 calls this empty score cleanup hook at CLOSE or EXIT. */
void fn_1_A2A8(void)
{
}

/* Builds the four player displays, then installs their per-frame updater. */
void fn_1_A2AC(OMOBJ *obj) {
    fn_1_A310(obj);
    obj->objFunc = fn_1_A2E8;
}

/* Refreshes player score boxes each time the object manager updates this object. */
extern void fn_1_A2E8(OMOBJ *node)
{
    fn_1_A434(node);
}

/* fn_1_A2AC creates each player's initially hidden score box and two-digit number display. */
void fn_1_A310(OMOBJ *object)
{
    M649ScoreEntryView *entry;
    s32 playerIndex;
    M649ScoreWorkView *work = object->data;
    MGSCORE *score;
    HUSPR_GROUPID box;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        entry = &work->entries[playerIndex];
        box = MgScoreBoxCreateChar(96, 48, GwPlayerConf[playerIndex].charNo);
        entry->box = box;
        MgScoreBoxDispSet(box, 0);
        MgScoreBoxTPLvlSet(box, 0.0f);
        score = MgScoreCreate(DATANUM(DATA_mgconst, 49), -1, 1);
        entry->score = score;
        MgScoreMaxDigitSet(score, 2);
        MgScoreDispOff(score);
        MgScoreColorSet(score, 255, 216, 21);
        MgScorePriSet(score, 10);
        entry->x = lbl_1_data_560[playerIndex].x;
        entry->y = lbl_1_data_560[playerIndex].y;
        fn_1_A528(entry);
    }
}

/* fn_1_A2E8 reveals the displays once enabled, then refreshes all four scores each frame. */
void fn_1_A434(OMOBJ *object)
{
    M649ScoreEntryView *entry;
    s32 playerIndex;
    M649ScoreWorkView *work = object->data;
    MGSCORE *score;
    HUSPR_GROUPID box;
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
        }
        work->displayState++;
        break;
    case 2:
        break;
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        entry = &work->entries[playerIndex];
        if (entry->points > 99) {
            entry->points = 99;
        }
        score = entry->score;
        MgScoreValueSet(score, entry->points);
    }
}

/* Positions one player's score box and number sprite while fn_1_A310 creates the display. */
void fn_1_A528(void *entry) {
    f32 x;
    f32 y;
    x = (*(f32 *)((s8 *)(entry) + (4)));
    y = (*(f32 *)((s8 *)(entry) + (8)));
    MgScoreBoxPosSet((*(s16 *)((s8 *)(entry) + (12))), x, y);
    MgScorePosSet((*(MGSCORE * *)((s8 *)(entry) + (16))), 7.0f + x, y);
}

/* Prop updates award stamp points after scoring is enabled, capping each player's total at 99. */
void fn_1_A5A4(s32 playerIndex, s32 awardedPoints)
{
    M649ScoreEntryView *score;
    M649ScoreWorkView *work = lbl_1_bss_5A8->data;
    if (work->displayState) {
        score = &work->entries[playerIndex];
        awardedPoints += score->points;
        if (awardedPoints > 99) {
            awardedPoints = 99;
        }
        score->points = awardedPoints;
    }
}

/* fn_1_680 enables scoring during PREMAIN; the next score-object update reveals the displays. */
void fn_1_A600(void) {
    OMOBJ *object = lbl_1_bss_5A8;
    s32 *displayEnabled = (s32 *)object->data;
    if (*displayEnabled == 0) {
        ++*displayEnabled;
    }
}

/* Player result updates and the sequence manager read the selected player's accumulated score. */
s32 fn_1_A644(s32 playerIndex)
{
    OMOBJ *object = lbl_1_bss_5A8;
    M649ScoreWorkView *work = object->data;
    return work->entries[playerIndex].points;
}

/* This score helper always returns zero. */
s32 fn_1_A67C(void) { return 0; }
