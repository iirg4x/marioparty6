#include "datadir_enum.h"
#include "REL/m611DLL/score_display.h"
extern "C" {
#include "dolphin/types.h"
#include "game/gamemes.h"
}

// Screen positions of the four score displays.
const HuVec2f lbl_1_rodata_9F0[4] = {
    { 61.0f, 63.0f },
    { 514.0f, 63.0f },
    { 61.0f, 418.0f },
    { 514.0f, 418.0f }
};

// Initializes the inactive score display.
ScoreDisplay::ScoreDisplay()
{
    s32 index;
    PlayerScoreEntry *entry;
    this->active = 0;
    for (index = 0; index < 4; index++) {
        entry = &this->entries[index];
        entry->unk08 = 0.0f;
    }
}

// Creates and positions one score display.
void ScoreDisplay::fn_1_12104(s32 index, s32 character)
{
    PlayerScoreEntry *slot = &entries[index];
    MGSCORE *score;

    slot->boxId = MgScoreBoxCreateChar(92, 48, character);
    MgScoreBoxColorSet(slot->boxId, 64, 64, 64);
    MgScoreBoxTPLvlSet(slot->boxId, 0.0f);
    MgScoreBoxDispSet(slot->boxId, FALSE);
    score = slot->score = MgScoreCreate(DATANUM(0x9B0000, 49), -1, TRUE);
    MgScoreMaxDigitSet(score, 2);
    MgScoreDispOff(score);
    MgScorePosSet(score, 7.0f + lbl_1_rodata_9F0[index].x,
                  lbl_1_rodata_9F0[index].y);
    MgScoreColorSet(score, 255, 216, 21);
    slot->character = character;
    slot->value = 0;
    slot->x = lbl_1_rodata_9F0[index].x;
    slot->y = lbl_1_rodata_9F0[index].y;
    MgScoreBoxPosSet(slot->boxId, slot->x, slot->y);
}

// Stores a value in the indexed score record.
void ScoreDisplay::fn_1_12274(s32 index, s32 value)
{
    entries[index].value = value;
}

// Displays one score entry.
void ScoreDisplay::fn_1_12284(PlayerScoreEntry *entry)
{
    MgScoreBoxDispSet(entry->boxId, TRUE);
    MgScoreBoxTPLvlSet(entry->boxId, 1.0f);
    MgScoreValueSet(entry->score, entry->value);
    MgScoreDispOn(entry->score);
    MgScoreTPLvlSet(entry->score, 1.0f);
}

// Displays all four score entries while the display is active.
void ScoreDisplay::update()
{
    if (active != 0) {
        for (s32 index = 0; index < 4; index++) {
            PlayerScoreEntry *entry = &entries[index];
            fn_1_12284(entry);
        }
    }
}

// Acknowledges requests and activates the score display on request two.
void ScoreDisplay::flagEnabled(int request)
{
    flags &= ~(1UL << request);
    switch (request) {
    case 0:
    case 1:
    case 3:
    case 4:
        break;
    case 2:
        active = 1;
        break;
    default:
        break;
    }
}

// Destroys the score display.
ScoreDisplay::~ScoreDisplay()
{
}
