#include "REL/m648DLL/managers.h"
extern "C" {
#include "dolphin/types.h"
}

extern "C" {
#include "dolphin/gx.h"
}

extern "C" {
#include "game/data.h"
}

extern "C" {
#include "game/memory.h"
}

extern "C" {
#include "game/sprite.h"
}

extern "C" {
#include "game/main.h"
}

extern "C" {
#include "game/object.h"
}

extern "C" {
#include "game/audio.h"
}

extern "C" {
#include "game/charman.h"
}

extern "C" {
#include "game/gamemes.h"
}

extern "C" {
#include "game/hsfex.h"
}

extern "C" {
#include "game/gamework.h"
}

extern "C" {
#include "game/mg/seqman.h"
}

extern "C" {
#include "game/mg/timer.h"
}

extern "C" {
#include "game/mg/score.h"
}

extern "C" {
#include "game/pad.h"
}

extern "C" {
#include "game/frand.h"
}

extern "C" {
#include "game/wipe.h"
}

extern "C" {
#include "game/mg/actman.h"
}

extern "C" {
#include "game/board/object.h"
}

extern "C" {
#include "game/board/tutorial.h"
}

extern "C" {
#include "datadir_enum.h"
}

#include "REL/framework/list_object.h"

extern "C" {
#include <math.h>
}

const HuVec2f lbl_1_rodata_408[4] = { { 61.0f, 63.0f }, { 515.0f, 63.0f }, { 61.0f, 418.0f }, { 515.0f, 418.0f } };

/* Stores the timer settings and creates the timer immediately when enabled is zero. */
TimerManager::TimerManager(s32 value, u8 enabled) {
    parameter = value;
    display = 0;
    active = enabled;
    if (active == 0) fn_1_D268();
}

/* Kills the timer when its display has been created. */
TimerManager::~TimerManager() {
    if (display != 0) MgTimerKill(timer);
}

/* Creates the selected timer mode, positions it and enables its record display. */
void TimerManager::fn_1_D268() {
    if (active != 0) {
        timer = MgTimerCreate(2);
        MgTimerParamSet(timer, 0, 0x1518, parameter);
        MgTimerPosSet(timer, 292.0f, 58.0f);
    } else {
        timer = MgTimerCreate(1);
        MgTimerParamSet(timer, 0, 0x4650, parameter);
        MgTimerPosSet(timer, 292.0f, 70.0f);
    }
    MgTimerRecordDispOn(timer);
    display = 1;
}

/* Creates the timer display when its active flag is set. */
void TimerManager::fn_1_D330()
{
    if (active != 0) {
        fn_1_D268();
    }
}

/* Turns on timer mode zero. */
void TimerManager::fn_1_D404()
{
    MgTimerModeOnSet(timer, 0);
}

/* Stops the timer and sets its mode to on when it has not already stopped. */
void TimerManager::fn_1_D438() {
    if (timer->stopF == FALSE) {
        timer->stopF = TRUE;
        timer->mode = MGTIMER_MODE_ON;
    }
}

/* Reports completion only when the timer display exists and is done. */
s32 TimerManager::fn_1_D464()
{
    s32 result = FALSE;
    if (MgTimerDoneCheck(timer) && display) {
        result = TRUE;
    }
    return result;
}

/* Reads and discards the timer value. */
void TimerManager::fn_1_D4BC()
{
    MgTimerValueGet(timer);
}

/* Clears the requested flag and creates, starts or stops the timer for its mode. */
void TimerManager::flagEnabled(int mode)
{
    flags = flags & ~(1UL << mode);

    switch (mode) {
    case 0:
        break;
    case 1:
        fn_1_D330();
        break;
    case 2:
        fn_1_D404();
        break;
    case 3:
        fn_1_D438();
        break;
    case 4:
        break;
    }
}

/* Starts with score display updates disabled. */
ScoreManager::ScoreManager() { enabled = 0; }

/* Creates and positions the selected player score box and digits. */
void ScoreManager::fn_1_D77C(s32 playerNo, s32 characterNo)
{
    ScoreCounterSlot *slot = &slots[playerNo];
    MGSCORE *score;

    slot->groupId = MgScoreBoxCreateChar(0x5C, 0x30, (s16)characterNo);
    MgScoreBoxTPLvlSet(slot->groupId, 0.0f);
    MgScoreBoxDispSet(slot->groupId, 0);
    score = slot->score = MgScoreCreate(DATANUM(0x9b0000, 49), -1, 1);
    MgScoreMaxDigitSet(score, 2);
    MgScoreDispOff(score);
    MgScorePosSet(score, 6.0f + lbl_1_rodata_408[playerNo].x,
                  lbl_1_rodata_408[playerNo].y);
    MgScoreColorSet(score, 0xFF, 0xD8, 0x15);
    slot->characterNo = characterNo;
    slot->scoreValue = 0;
    slot->posX = lbl_1_rodata_408[playerNo].x;
    slot->posY = lbl_1_rodata_408[playerNo].y;
    MgScoreBoxPosSet(slot->groupId, slot->posX, slot->posY);
}

/* Stores the score value for the selected player slot. */
void ScoreManager::fn_1_D8D8(int index, int value)
{
    slots[index].scoreValue = value;
}

/* Shows one score box and its current score at full opacity. */
void ScoreManager::fn_1_D8E8(ScoreCounterSlot *slot) {
    MgScoreBoxDispSet(slot->groupId, 1);
    MgScoreBoxTPLvlSet(slot->groupId, 1.0f);
    MgScoreValueSet(slot->score, slot->scoreValue);
    MgScoreDispOn(slot->score);
    MgScoreTPLvlSet(slot->score, 1.0f);
}

/* Shows all four score entries when score display updates are enabled. */
void ScoreManager::update()
{
    if (enabled != 0) {
        for (s32 index = 0; index < 4; index++) {
            ScoreCounterSlot *entry = &slots[index];
            fn_1_D8E8(entry);
        }
    }
}

/* Clears the requested flag and enables score display updates for request 2. */
void ScoreManager::flagEnabled(int request)
{
    flags &= ~(1UL << request);
    switch (request) {
    case 0:
    case 1:
    case 3:
    case 4:
        break;
    case 2:
        enabled = 1;
        break;
    }
}

/* Releases the object through its member and base destructors. */
ScoreManager::~ScoreManager() {}
