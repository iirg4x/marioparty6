#define _MATH_H
#include "game/hu3d.h"
extern s16 lbl_1_bss_8CE[];

void fn_1_22CB4(s16 index, s16 visible)
{
    if (visible != 0) {
        Hu3DModelAttrReset(lbl_1_bss_8CE[index], 1U);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_8CE[index], 1U);
    }
}
