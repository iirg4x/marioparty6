/* This unit uses no math inlines; avoid emitting their weak constant pools. */
#define _MATH_H
#include "game/hu3d.h"

extern s16 lbl_1_bss_8E4;

void fn_1_20F48(void)
{
    Hu3DModelKill(lbl_1_bss_8E4);
}
