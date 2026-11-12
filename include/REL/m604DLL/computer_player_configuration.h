#ifndef REL_M604DLL_COMPUTER_PLAYER_CONFIGURATION_H
#define REL_M604DLL_COMPUTER_PLAYER_CONFIGURATION_H
#include "REL/m604DLL/player_input.h"

struct ComputerPlayerConfiguration : PlayerInputBase {
    ComputerPlayerConfiguration(Player *player, s32 difficulty);
    virtual ~ComputerPlayerConfiguration();
    virtual void read(StickState *output);
    s32 recordIndex;
    s32 movementState;
    f32 directionX;
    f32 unk28; // Initialized by the constructor; not read by the input routine.
    f32 directionY;
    f32 waitDuration;
    f32 waitElapsed;
    s32 waitInitialized;
    s32 delayInitialized;
    s32 previousActorRecord;
    u32 randomCounter;
};
#endif
