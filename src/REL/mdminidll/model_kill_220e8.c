/* This unit uses no math inlines; avoid emitting their weak constant pools. */
#define _MATH_H
#include "game/hu3d.h"

extern s16 lbl_1_bss_8DA;

void fn_1_220E8(void)
{
    Hu3DModelKill(lbl_1_bss_8DA);
}
