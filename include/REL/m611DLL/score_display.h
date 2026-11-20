#ifndef REL_M611DLL_SCORE_DISPLAY_H
#define REL_M611DLL_SCORE_DISPLAY_H
extern "C" {
#include "game/mg/score.h"
}
#include "REL/framework/list_object.h"
#include "REL/framework/player_score_entry.h"
#include "REL/framework/singleton.h"

// Owns and updates the four score displays.
struct ScoreDisplay : RequestObject, MinigameSingleton<ScoreDisplay> {
    ScoreDisplay();
    virtual ~ScoreDisplay();
    virtual void flagEnabled(int);
    virtual void update();
    u8 active;
    PlayerScoreEntry entries[4];
    void fn_1_12104(s32 index, s32 character);
    void fn_1_12274(s32 index, s32 value);
    void fn_1_12284(PlayerScoreEntry *entry);
};
#endif
