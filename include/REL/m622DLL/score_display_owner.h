// The four players' coin counters on screen.
#ifndef REL_M622DLL_SCORE_DISPLAY_OWNER_H
#define REL_M622DLL_SCORE_DISPLAY_OWNER_H

#include "REL/m622DLL/game_object.h"
#include "REL/framework/minigame_singleton.h"

struct MgScore_s;

struct ScoreDisplay {
    s16 scoreBox;
    s16 spriteGroup;
    struct MgScore_s *score;
    f32 transparency;
    s32 characterId;
    s32 value;
    f32 x;
    f32 y;
};

struct ScoreDisplayOwner : GameObject, MinigameSingleton<ScoreDisplayOwner> {
    u8 active;
    ScoreDisplay displays[4];

    ScoreDisplayOwner();
    virtual ~ScoreDisplayOwner();
    virtual void clearFlag(int bit);
    virtual void update();
};

void fn_1_8D74(ScoreDisplayOwner *owner, s32 index, s32 characterId);
void fn_1_8FC8(ScoreDisplayOwner *owner, s32 index, s32 value);

#endif
