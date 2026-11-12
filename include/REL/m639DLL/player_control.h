#ifndef REL_M639DLL_PLAYER_CONTROL_H
#define REL_M639DLL_PLAYER_CONTROL_H

// How a player moves: by the pad's triggers or, for a computer player, by a motion cycle.
#include "dolphin/types.h"

#ifdef __cplusplus
struct PlayerControl {
    virtual ~PlayerControl() {}
    virtual void clear() {}
    virtual void update() = 0;
    void *owner;
    explicit PlayerControl(void *owner) : owner(owner) {}
};

struct MotionCycleOwner : PlayerControl {
    s32 valueIndex;
    f32 phase;
    explicit MotionCycleOwner(void *owner);
    virtual ~MotionCycleOwner();
    void clear();
    void update();
};
struct ScalarSourceOwner : PlayerControl {
    explicit ScalarSourceOwner(void *owner) : PlayerControl(owner) {}
    virtual ~ScalarSourceOwner();
    void update();
};

#else
typedef struct MotionCycleOwner {
    void *vtable;
    void *owner;
    s32 valueIndex;
    f32 phase;
} MotionCycleOwner;
#endif

#endif
