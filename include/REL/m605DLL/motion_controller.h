#ifndef REL_M605DLL_MOTION_CONTROLLER_H
#define REL_M605DLL_MOTION_CONTROLLER_H

#include "dolphin/types.h"

struct PlayerCharacter;

// Stores the timing and direction of a character motion request.
struct MotionController {
    s32 duration;
    s32 sampledDuration;
    s32 secondaryDuration;
    s32 wait;
    u8 waitExpired;
    u8 unk11[3]; /* No code names this field. */
    f32 direction;
    f32 randomScale;

    MotionController(PlayerCharacter *owner);
};

void fn_1_AA70(MotionController *controller, PlayerCharacter *owner);

void fn_1_A90C(MotionController *controller, PlayerCharacter *player);

void fn_1_AD80(MotionController *controller, PlayerCharacter *player);
u8 fn_1_AF80(MotionController *controller, PlayerCharacter *owner);

void fn_1_B2AC(PlayerCharacter *owner);

#endif
