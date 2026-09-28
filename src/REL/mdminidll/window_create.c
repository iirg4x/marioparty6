#define _MATH_H
#include "game/window.h"
#include "REL/mdminidll/window_callback.h"

extern const f32 lbl_1_rodata_F4;
extern const f32 lbl_1_rodata_F8;
extern const f32 lbl_1_rodata_FC;
extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_100;
extern HUWINID lbl_1_bss_858[4];

void fn_1_3720(void)
{
    s16 i;

    HuWinInit(1);
    lbl_1_bss_858[0] = HuWinExCreateFrame(lbl_1_rodata_F4, lbl_1_rodata_F8, 544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_858[0]);
    HuWinBGTPLvlSet(lbl_1_bss_858[0], lbl_1_rodata_74);
    lbl_1_bss_858[1] = HuWinExCreateFrame(lbl_1_rodata_F4, lbl_1_rodata_FC, 544, 68, -1, 0);
    HuWinDispOff(lbl_1_bss_858[1]);
    HuWinBGTPLvlSet(lbl_1_bss_858[1], lbl_1_rodata_100);
    lbl_1_bss_858[2] = HuWinExCreateFrame(lbl_1_rodata_F4, lbl_1_rodata_FC, 544, 68, -1, 3);
    HuWinDispOff(lbl_1_bss_858[2]);
    HuWinBGTPLvlSet(lbl_1_bss_858[2], lbl_1_rodata_100);
    lbl_1_bss_858[3] = HuWinExCreateFrame(lbl_1_rodata_F4, lbl_1_rodata_FC, 544, 68, -1, 4);
    HuWinDispOff(lbl_1_bss_858[3]);
    HuWinBGTPLvlSet(lbl_1_bss_858[3], lbl_1_rodata_100);
    for (i = 0; i < 4; i++) {
        winData[lbl_1_bss_858[i]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_858[i], fn_1_0);
    }
}
