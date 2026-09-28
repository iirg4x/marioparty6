#define _MATH_H
#include "game/object.h"

extern OMOBJ *lbl_1_bss_C;

void fn_1_7700(void)
{
    OMOBJ *first;
    OMOBJ *second;
    first = lbl_1_bss_C;
    first->work[0] = 2;
    first->work[1] = 0;
    second = lbl_1_bss_C;
    second->work[2] = 2;
    second->work[3] = 0;
}
