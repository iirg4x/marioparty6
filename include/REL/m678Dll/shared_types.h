#ifndef M678DLL_SHARED_TYPES_H
#define M678DLL_SHARED_TYPES_H

#include "dolphin/types.h"

typedef struct _struct_lbl_1_bss_2D0_0x8 {
    s32 counts[2];
} _struct_lbl_1_bss_2D0_0x8;

typedef struct _struct_lbl_1_bss_45FC_0x24 {
    s32 state;
    s32 frame;
    s32 duration;
    f32 x;
    f32 y;
    f32 arc;
    f32 startX;
    f32 startY;
    f32 unk20; /* the game's code never reads this field */
} _struct_lbl_1_bss_45FC_0x24;

#endif
