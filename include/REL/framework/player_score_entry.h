#ifndef REL_FRAMEWORK_PLAYER_SCORE_ENTRY_H
#define REL_FRAMEWORK_PLAYER_SCORE_ENTRY_H
extern "C" {
#include "game/mg/score.h"
}

// A character's score and display position.
struct PlayerScoreEntry {
    HUSPR_GROUPID boxId;
    MGSCORE *score;
    f32 unk08; // The game's code never reads this field.
    s32 character;
    s32 value;
    f32 x, y;
};
#endif
