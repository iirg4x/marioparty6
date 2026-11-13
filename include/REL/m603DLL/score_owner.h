#ifndef REL_M603DLL_SCORE_OWNER_H
#define REL_M603DLL_SCORE_OWNER_H

#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
extern "C" {
#include "game/sprite.h"
#include "game/mg/score.h"
}

/* Store one character score and its display position. */
struct PlayerScoreEntry {
    HUSPR_GROUPID boxId;
    MGSCORE *score;
    f32 initializedFloat; // The game's code never reads this field.
    s32 character;
    s32 value;
    f32 x, y;
};

/* Own and update the four score displays. */
class ScoreDisplay : public RequestObject, public MinigameSingleton<ScoreDisplay> {
public:
    u8 active;
    u8 unknown11; /* No code names this field. */
    u8 soundPending;
    PlayerScoreEntry entries[4];
    ScoreDisplay();
    virtual void update();
    /* Finish the score display lifetime. */
    virtual ~ScoreDisplay() {}
    virtual void flagEnabled(int);
    void fn_1_8268(s32 index, s32 character);
    void fn_1_83CC(int index, int value);
    unsigned int fn_1_83E4(unsigned int index);
    void fn_1_83F4();
    void fn_1_84E4(PlayerScoreEntry *entry);
};

#endif
