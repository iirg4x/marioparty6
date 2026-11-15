#ifndef REL_M634DLL_STICK_STATE_H
#define REL_M634DLL_STICK_STATE_H
#include "dolphin/types.h"
/* fn_1_6804 writes all five fields; the choice callbacks consume them. */
typedef struct StickState {
    f32 x;
    f32 y;
    f32 magnitude;
    u32 pressedButtons;
    u32 heldButtons;
} StickState;
#endif
