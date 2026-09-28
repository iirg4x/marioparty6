#define _MATH_H
#include "game/object.h"
#include "game/sprite.h"
extern OMOBJ *lbl_1_bss_24;
extern s16 lbl_1_bss_752[9];
extern s16 lbl_1_data_86E[6];

void fn_1_C6D8(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_752[2], 4, 4);
    lbl_1_data_86E[4] = 0;
    HuSprAttrSet(lbl_1_bss_752[2], 5, 4);
    lbl_1_data_86E[5] = 0;
    obj->objFunc = NULL;
    *lbl_1_bss_24->mtnId = 1;
}
