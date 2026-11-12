#ifndef REL_M604DLL_SCORE_OWNER_H
#define REL_M604DLL_SCORE_OWNER_H

#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
extern "C" {
#include "game/mg/score.h"
}

// Holds the four score-box handles and score displays.
class M604ScoreOwner : public RequestObject, public MinigameSingleton<M604ScoreOwner> {
public:
    s16 spriteIds[4];
private:
    s16 scoreBoxIds[4];
    MGSCORE *scoreDisplays[4];

public:
    M604ScoreOwner();
    virtual ~M604ScoreOwner();
    void fn_1_14E14(s32 index, s32 x, s32 y);
    virtual void flagEnabled(int bit);
    void fn_1_14FF8();
    void fn_1_15060();
    void fn_1_150C8(s32 index, s32 value);
};

#endif
