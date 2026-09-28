#define _MATH_H
#include "game/window.h"

extern s16 lbl_1_bss_858[4];

void fn_1_3948(void)
{
    s16 i;
    i = 0;
    while (i < 4) {
        HuWinExKill(lbl_1_bss_858[i]);
        i += 1;
    }
    HuWinAllKill();
}
