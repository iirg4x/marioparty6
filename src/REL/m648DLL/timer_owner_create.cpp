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

extern "C" { extern void __msl_assertion_failed(const char *expression, const char *file, int line); }

extern void *lbl_1_bss_24;

extern char lbl_1_data_588[];

extern char lbl_1_data_592[];

extern "C" { extern const u8 lbl_1_data_5C[]; }

const HuVec2f lbl_1_rodata_408[4] = { { 61.0f, 63.0f }, { 515.0f, 63.0f }, { 61.0f, 418.0f }, { 515.0f, 418.0f } };

TimerManager::TimerManager(s32 value, u8 enabled) {
    parameter = value;
    display = 0;
    active = enabled;
    if (active == 0) fn_1_D268();
}

TimerManager::~TimerManager() {
    if (display != 0) MgTimerKill(timer);
}

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

void TimerManager::fn_1_D330() {
    if (active != 0) {
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
}

void TimerManager::fn_1_D404()
{
    MgTimerModeOnSet(timer, 0);
}

void TimerManager::fn_1_D438() {
    if (timer->stopF == FALSE) {
        timer->stopF = TRUE;
        timer->mode = MGTIMER_MODE_ON;
    }
}

s32 TimerManager::fn_1_D464()
{
    s32 result = FALSE;
    if (MgTimerDoneCheck(timer) && display) {
        result = TRUE;
    }
    return result;
}

struct TimerModeObject {
    u8 unknown00[0x14]; /* No code names this field. */
    s32 mode;
};

struct TimerWork {
    u8 unknown00[0x0C]; /* No code names this field. */
    u32 flags;
    MGTIMER *timer;
    s32 parameter;
    u8 display;
    u8 type;
};

void TimerManager::fn_1_D4BC()
{
    MgTimerValueGet(timer);
}

void TimerManager::flagEnabled(int mode)
{
    flags = flags & ~(1UL << mode);

    switch (mode) {
    case 0:
        break;
    case 1:
        if (active != 0) {
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
        break;
    case 2:
        MgTimerModeOnSet(timer, 0);
        break;
    case 3:
        if (timer->stopF == 0) {
            timer->stopF = 1;
            timer->mode = 1;
        }
        break;
    case 4:
        break;
    }
}

ScoreManager::ScoreManager() { enabled = 0; }

/* Native C++ callback and singleton-lifetime recovery.
 * Class names are reconstructed. These are consumed interface views, not
 * claims about complete original class layouts; none is allocated here.
 * The callback descriptor at +4 is a real pointer-to-member, not a C callback
 * or a manually encoded compiler helper. Unknown word +0 is not padding.
 * Fragment selectors choose entire functions, not compiler behavior or TUs. */

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

/* Recipient symbol bindings for a retained source (copy of m603DLL fn_1_84E4). */
void ScoreManager::fn_1_D8D8(int index, int value)
{
    slots[index].scoreValue = value;
}

void ScoreManager::fn_1_D8E8(ScoreCounterSlot *slot) {
    MgScoreBoxDispSet(slot->groupId, 1);
    MgScoreBoxTPLvlSet(slot->groupId, 1.0f);
    MgScoreValueSet(slot->score, slot->scoreValue);
    MgScoreDispOn(slot->score);
    MgScoreTPLvlSet(slot->score, 1.0f);
}

struct M648ScoreEntry {
    HUSPR_GROUPID boxId;
    s16 unk02;
    MGSCORE *score;
    s32 unk08;
    s32 unk0C;
    s32 value;
    s32 unk14;
    s32 unk18;
};

struct M648ScoreWork {
    s32 unk00;
    s32 unk04;
    s32 unk08;
    s32 unk0C;
    u8 enabled;
    u8 unk11;
    u8 unk12;
    u8 unk13;
    M648ScoreEntry entries[4];
};

void ScoreManager::update()
{
    if (enabled != 0) {
        for (s32 index = 0; index < 4; index++) {
            ScoreCounterSlot *entry = &slots[index];
            MgScoreBoxDispSet(entry->groupId, 1);
            MgScoreBoxTPLvlSet(entry->groupId, 1.0f);
            MgScoreValueSet(entry->score, entry->scoreValue);
            MgScoreDispOn(entry->score);
            MgScoreTPLvlSet(entry->score, 1.0f);
        }
    }
}

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

ScoreManager::~ScoreManager() {}
