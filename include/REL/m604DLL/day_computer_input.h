#ifndef REL_M604DLL_DAY_COMPUTER_INPUT_H
#define REL_M604DLL_DAY_COMPUTER_INPUT_H
#include "REL/m604DLL/player_input.h"

struct DayComputerInput : PlayerInputBase {
    DayComputerInput(Player *owner, s32 difficulty);
    virtual ~DayComputerInput();
    virtual void read(StickState *output);
    s32 recordIndex;
    s32 state;
    f32 directionX;
    f32 directionY;
    f32 waitDuration;
    f32 waitElapsed;
    s32 initialDelaySet;
    u8 initialized;
};
#endif
