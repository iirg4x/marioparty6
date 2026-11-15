#ifndef REL_M634DLL_STICK_STATE_H
#define REL_M634DLL_STICK_STATE_H
#include "dolphin/types.h"
/* Stick direction, button state, and magnitude. */
typedef struct StickState {
    f32 x;
    f32 y;
    f32 magnitude;
    u32 pressedButtons;
    u32 heldButtons;
} StickState;
#endif
