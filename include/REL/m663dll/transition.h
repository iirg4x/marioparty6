#ifndef REL_M663DLL_TRANSITION_H
#define REL_M663DLL_TRANSITION_H

#include "dolphin/mtx.h"

typedef struct M663TransitionControl {
    s16 playerIndex;
    s16 mode;
    u8 unk_04[2]; /* No code names this field. */
    s16 state;
    s16 presetIndex;
    f32 positionX;
    f32 positionY;
    f32 positionZ;
    f32 progress;
    f32 targetY;
    f32 targetZ;
    Vec position;
} M663TransitionControl;

extern M663TransitionControl lbl_1_bss_0[6];

#endif
