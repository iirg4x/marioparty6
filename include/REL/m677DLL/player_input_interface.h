// Defines the input interface shared by human and computer players.

#ifndef REL_SEER_TERROR_PLAYER_INPUT_INTERFACE_H
#define REL_SEER_TERROR_PLAYER_INPUT_INTERFACE_H

#include "dolphin/types.h"

struct PlayerWork;

// Starts player input and processes it each frame.
struct PlayerInputInterface {
    virtual ~PlayerInputInterface();
    virtual void beginInput();
    virtual void updateInput() = 0;
    PlayerWork *owner;
    // Stores the character receiving the input.
    PlayerInputInterface(PlayerWork *player) : owner(player)
    {
    }
};

// Reads input from the player's controller.
struct HumanPlayerInput : PlayerInputInterface {
    // Binds the controller input to the character.
    HumanPlayerInput(PlayerWork *player) : PlayerInputInterface(player)
    {
    }
    virtual ~HumanPlayerInput();
    virtual void updateInput();
};

// Chooses computer inputs using the player's difficulty.
struct ComputerPlayerInput : PlayerInputInterface {
    s32 difficulty;
    ComputerPlayerInput(PlayerWork *player);
    virtual ~ComputerPlayerInput();
    virtual void beginInput();
    virtual void updateInput();
};

#endif
