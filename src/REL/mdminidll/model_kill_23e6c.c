/* This unit uses no math inlines; avoid emitting their weak constant pools. */
#define _MATH_H
#include "game/hu3d.h"

extern s16 lbl_1_bss_8CA[2];

void fn_1_23E6C(void)
{
    s16 i;

    i = 0;
    while (i < 2) {
        Hu3DModelKill(lbl_1_bss_8CA[i]);
        i += 1;
    }
}
