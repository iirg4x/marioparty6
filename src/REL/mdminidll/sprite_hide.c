#define _MATH_H
#include "game/sprite.h"
extern s16 lbl_1_bss_752[9];
extern s16 lbl_1_data_86E[6];

void fn_1_8694(s16 index)
{
    HuSprAttrSet(lbl_1_bss_752[2], index, 4);
    lbl_1_data_86E[index] = 0;
}
