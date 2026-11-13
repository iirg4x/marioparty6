#ifndef REL_M639DLL_PLAYER_CONTROL_H
#define REL_M639DLL_PLAYER_CONTROL_H

// How a player moves: by the pad's triggers or, for a computer player, by a motion cycle.
#include "dolphin/types.h"

class Player;

struct PlayerControl {
    virtual ~PlayerControl() {}
    virtual void clear() {}
    virtual void update() = 0;
    Player *owner;
    explicit PlayerControl(Player *owner) : owner(owner) {}
};

struct MotionCycleOwner : PlayerControl {
    s32 valueIndex;
    f32 phase;
    explicit MotionCycleOwner(Player *owner);
    virtual ~MotionCycleOwner();
    void clear();
    void update();
};
struct ScalarSourceOwner : PlayerControl {
    explicit ScalarSourceOwner(Player *owner) : PlayerControl(owner) {}
    virtual ~ScalarSourceOwner();
    void update();
};

#endif
