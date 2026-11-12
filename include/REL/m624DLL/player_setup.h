#ifndef REL_M624DLL_PLAYER_SETUP_H
#define REL_M624DLL_PLAYER_SETUP_H
extern "C" {
#include "dolphin/mtx.h"
#include "game/mg/actman.h"
}
#include "REL/m624DLL/player_task.h"
#include "REL/m624DLL/tail_types.h"

typedef struct StartPositions {
    Position first;
    Position second;
} StartPositions;

struct ManualControl : PlayerControl {
    ManualControl(PlayerRunner *owner) : PlayerControl(owner) {}
    virtual ~ManualControl();
    virtual void update();
};

struct ComControl : PlayerControl {
    s32 *routeSelection;
    s32 currentRoute;
    u8 active : 1;
    M624PadControl control;
    s32 countdown;
    ComControl(PlayerRunner *owner);
    virtual ~ComControl();
    virtual void reset();
    virtual void update();
};

inline PlayerControl::~PlayerControl() {}

#endif
