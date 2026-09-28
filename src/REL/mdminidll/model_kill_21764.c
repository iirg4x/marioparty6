/* This unit uses no math inlines; avoid emitting their weak constant pools. */
#define _MATH_H
#include "game/hu3d.h"

extern s16 lbl_1_bss_8DC[4];

void fn_1_21764(void)
{
    s16 i;

    i = 0;
    while (i < 4) {
        Hu3DModelKill(lbl_1_bss_8DC[i]);
        i += 1;
    }
}
