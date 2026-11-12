#ifndef REL_M655DLL_PLAYER_MOTION_INTERFACE_H
#define REL_M655DLL_PLAYER_MOTION_INTERFACE_H
#include "dolphin/types.h"
struct PlayerWork;
struct PlayerInputInterface {
    virtual ~PlayerInputInterface();
    virtual void setPosition();
    virtual void invoke() = 0;
    PlayerWork *owner;
    PlayerInputInterface(PlayerWork *object);
};

struct HumanPlayerInput : PlayerInputInterface {
    virtual ~HumanPlayerInput();
    virtual void invoke();
    HumanPlayerInput(PlayerWork *object);
};

struct ComputerPlayerInput : PlayerInputInterface {
    virtual ~ComputerPlayerInput();
    virtual void setPosition();
    virtual void invoke();
    s32 selection;
    ComputerPlayerInput(PlayerWork *object);
};
#endif
