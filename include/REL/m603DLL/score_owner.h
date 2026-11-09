#ifndef REL_M603DLL_SCORE_OWNER_H
#define REL_M603DLL_SCORE_OWNER_H

#include "REL/m603DLL/callback_owner.h"
#include "REL/framework/minigame_singleton.h"
extern "C" {
#include "game/sprite.h"
#include "game/mg/score.h"
}

struct PlayerScoreEntry {
    HUSPR_GROUPID boxId;
    MGSCORE *score;
    f32 initializedFloat; // The game's code never reads this field.
    s32 character;
    s32 value;
    f32 x, y;
};

class ScoreDisplay : public CallbackOwner, public MinigameSingleton<ScoreDisplay> {
public:
    u8 active;
    u8 unknown11;
    u8 soundPending;
    PlayerScoreEntry entries[4];
    ScoreDisplay();
    virtual void extra();
    virtual ~ScoreDisplay();
    virtual void update(unsigned long);
    void fn_1_8268(s32 index, s32 character);
    void fn_1_83CC(int index, int value);
    unsigned int fn_1_83E4(unsigned int index);
    void fn_1_83F4();
};

#endif
