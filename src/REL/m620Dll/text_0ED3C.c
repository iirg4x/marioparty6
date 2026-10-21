/* Trap Ease Artist score board and collision-manager setup. */
#include "dolphin/math.h"
#include "game/main.h"
#include "game/object.h"
#include "game/charman.h"
#include "game/hu3d.h"
#include "game/gamework.h"
#include "game/memory.h"
#include <string.h>
#include "game/wipe.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "REL/m620Dll/module_types.h"
#include "dolphin/types.h"
#include "game/gamemes.h"
#include "game/audio.h"

#define M620_SCORE_UPDATE_SOUND_ID 1774

typedef struct Position {
    HuVecF position;
    /* This twelve-byte interval is not read by the routines in this file. */
    unsigned char unread_0C[12];
    HuVecF offset;
} Position;

extern s32 lbl_1_data_F18;
extern float lbl_1_bss_1BC;
extern s8 lbl_1_bss_218, lbl_1_bss_219, lbl_1_data_2758;
s32 fn_1_367CC(s32 *player);

typedef struct M620ScoreBoardStorage {
    /* Score-board state cleared before its displays are initialized. */
    void *entries[13];
} M620ScoreBoardStorage;

extern OMOBJ *lbl_1_bss_658[2];
void fn_1_ED40(OMOBJ *obj);

/* Create the score-board object and clear its state during setup. */
void fn_1_ECB4(OMOBJMAN *manager)
{
    OMOBJ *obj;
    M620ScoreBoardStorage *scoreBoardStorage;

    obj = omAddObjEx(manager, 20, 0, 0, -1, fn_1_ED40);
    lbl_1_bss_658[0] = obj;
    scoreBoardStorage =
        HuMemDirectMallocNum(HEAP_HEAP, sizeof(M620ScoreBoardStorage), HU_MEMNUM_OVL);
    obj->data = scoreBoardStorage;
    memset(scoreBoardStorage, 0, sizeof(M620ScoreBoardStorage));
}

void fn_1_ED3C(void)
{
}

extern void fn_1_ED7C(OMOBJ *obj);
extern void fn_1_EDA4(OMOBJ *obj);

void fn_1_ED40(OMOBJ *obj) {
    fn_1_EDA4(obj);
    obj->objFunc = fn_1_ED7C;
}

void fn_1_EF00(OMOBJ *object);
void fn_1_ED7C(OMOBJ *object) { fn_1_EF00(object); }

typedef struct M620ScoreEntryView {
    s32 value;
    s16 box;
    MGSCORE *score;
} M620ScoreEntryView;

typedef struct M620ScoreWorkView {
    s32 state;
    M620ScoreEntryView entries[4];
} M620ScoreWorkView;

typedef struct M620ScoreBoxLayout {
    f32 positionX;
    f32 positionY;
    /* The score-box setup and update callbacks do not read this word. */
    u32 unread_08;
} M620ScoreBoxLayout;

M620ScoreBoxLayout lbl_1_data_6E0[4] = {
    {64.0f, 64.0f, 0},
    {512.0f, 64.0f, 0},
    {64.0f, 416.0f, 0},
    {512.0f, 416.0f, 0}
};

typedef struct M620ScoreEntry {
    s32 value;
    s16 characterBoxId;
    /* This halfword is not read by the score-board callbacks. */
    u16 unread_06;
    MGSCORE *score;
} M620ScoreEntry;

typedef struct M620ScoreBoardWork {
    u32 state;
    M620ScoreEntry entries[4];
} M620ScoreBoardWork;

/* Create and position each player's character box and hidden score display. */
void fn_1_EDA4(OMOBJ *object)
{
    M620ScoreBoardWork *work = object->data;
    s32 index;

    for (index = 0; index < 4; index++) {
        M620ScoreEntry *entry = &work->entries[index];
        HUSPR_GROUPID characterBoxId = MgScoreBoxCreateChar(
            96, 48, GwPlayerConf[index].charNo);
        MGSCORE *score;

        entry->characterBoxId = characterBoxId;
        MgScoreBoxPosSet(characterBoxId, lbl_1_data_6E0[index].positionX,
                         lbl_1_data_6E0[index].positionY);
        MgScoreBoxDispSet(characterBoxId, 0);
        MgScoreBoxTPLvlSet(characterBoxId, 0.0f);

        score = MgScoreCreate(DATANUM(DATA_mgconst, 49), -1, 1);
        entry->score = score;
        MgScoreMaxDigitSet(score, 2);
        MgScoreDispOff(score);
        MgScorePosSet(score,
                      7.0f + lbl_1_data_6E0[index].positionX,
                      lbl_1_data_6E0[index].positionY);
        MgScoreColorSet(score, 255, 216, 21);
        MgScorePriSet(score, 10);
    }
}

/* Show score displays when the board enters its active state and refresh values each tick. */
void fn_1_EF00(OMOBJ *object)
{
    M620ScoreEntryView *entry;
    s32 i;
    M620ScoreWorkView *work = object->data;
    MGSCORE *score;
    HUSPR_GROUPID box;
    switch (work->state) {
    case 0:
        break;
    case 1:
        for (i = 0; i < 4; i++) {
            entry = &work->entries[i];
            box = entry->box;
            MgScoreBoxDispSet(box, 1);
            MgScoreBoxTPLvlSet(box, 1.0f);
            score = entry->score;
            MgScoreDispOn(score);
        }
        work->state++;
        break;
    case 2:
        break;
    }
    for (i = 0; i < 4; i++) {
        entry = &work->entries[i];
        if (entry->value > 99) {
            entry->value = 99;
        }
        score = entry->score;
        MgScoreValueSet(score, entry->value);
    }
}

/* Add points to one player's score, cap it at 99, and play the score update effect. */
void fn_1_EFF4(s32 playerIndex, s32 points)
{
    M620ScoreBoardWork *scoreRecords;
    s32 *scoreValue;

    scoreRecords = lbl_1_bss_658[0]->data;
    scoreValue = &scoreRecords->entries[playerIndex].value;
    points += *scoreValue;
    if (points > 99) {
        points = 99;
    }
    *scoreValue = points;
    HuAudFXPlay(M620_SCORE_UPDATE_SOUND_ID);
}

/* Mark the score-board state active on its first sequence update. */
void fn_1_F070(void) {
    OMOBJ *object = lbl_1_bss_658[0];
    s32 *state = (s32 *)object->data;
    if (*state == 0) {
        ++*state;
    }
}

/* Return a player's score while fn_1_708 selects the result winners. */
s32 fn_1_F0B4(s32 playerIndex) {
    OMOBJ *object = lbl_1_bss_658[0];
    s32 (*scoreRecords)[3] = (s32 (*)[3])object->data;
    return scoreRecords[playerIndex][1];
}

s32 fn_1_F0EC(void)
{
    return 0;
}

extern OMOBJ *lbl_1_bss_660;
extern void fn_1_F180(OMOBJ *obj);

/* Create and clear the collision-registration object's pointer tables during setup. */
void fn_1_F0F4(OMOBJMAN *objectManager)
{
    OMOBJ *object = omAddObjEx(objectManager, 18, 0, 0, OM_GRP_NONE, fn_1_F180);
    void *data;
    lbl_1_bss_660 = object;
    data = HuMemDirectMallocNum(HEAP_HEAP, 196, HU_MEMNUM_OVL);
    object->data = data;
    memset(data, 0, 196);
}

void fn_1_F17C(void)
{
}
