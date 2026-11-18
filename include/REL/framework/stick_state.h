#ifndef REL_FRAMEWORK_STICK_STATE_H
#define REL_FRAMEWORK_STICK_STATE_H
#include "dolphin/types.h"

// Carries stick axes, magnitude and the pressed and held button states.
typedef struct StickState {
    f32 x;
    f32 y;
    f32 magnitude;
    u32 pressedButtons;
    u32 heldButtons;
} StickState;

#endif
