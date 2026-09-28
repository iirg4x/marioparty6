/* This unit uses no math inlines; avoid emitting their weak constant pools. */
#define _MATH_H
#include "game/hu3d.h"

extern s16 lbl_1_bss_8CE[2];

void fn_1_234A0(void)
{
    s16 i;

    i = 0;
    while (i < 2) {
        Hu3DModelKill(lbl_1_bss_8CE[i]);
        i += 1;
    }
}
