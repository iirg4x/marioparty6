#define _MATH_H
#include "game/hu3d.h"
extern s16 lbl_1_bss_8CA[];

void fn_1_234F8(s16 index, s16 visible)
{
    if (visible != 0) {
        Hu3DModelAttrReset(lbl_1_bss_8CA[index], 1U);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_8CA[index], 1U);
    }
}
