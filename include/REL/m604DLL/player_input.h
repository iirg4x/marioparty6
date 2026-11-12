#ifndef M604_PLAYER_INPUT_H
#define M604_PLAYER_INPUT_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"

#include "REL/m604DLL/player.h"

struct StickState {
    f32 x;
    f32 y;
    f32 magnitude;
    u32 pressedButtons;
    s32 mode;
};
struct PlayerInputBase {
    explicit PlayerInputBase(Player *owner) : configuration(owner) {}
    virtual ~PlayerInputBase() {}
    virtual void read(StickState *) = 0;
    // The stick as the last read left it: its x and y, their magnitude and the buttons pressed.
    // Its mode is the computer player's difficulty, which the computer's read takes from it.
    StickState stick;
    Player *configuration;
};
struct HumanPlayerConfiguration : PlayerInputBase {
    explicit HumanPlayerConfiguration(Player *owner);
    virtual ~HumanPlayerConfiguration() {}
    virtual void read(StickState *state);
    s32 padNumber;
};
void fn_1_6DC(PlayerInputBase *input);

#endif
