#define _MATH_H
#include "game/object.h"

extern OMOBJ *lbl_1_bss_C;

void fn_1_7384(void)
{
    OMOBJ *obj;
    obj = lbl_1_bss_C;
    obj->work[2] = 2;
    obj->work[3] = 0;
}
