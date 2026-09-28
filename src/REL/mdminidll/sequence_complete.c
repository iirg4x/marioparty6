#define _MATH_H
#include "game/object.h"

extern OMOBJ *lbl_1_bss_24;
void fn_1_B6EC(OMOBJ *obj);

void fn_1_C6A8(OMOBJ *obj)
{
    obj->work[0] = 10;
    obj->objFunc = fn_1_B6EC;
    *lbl_1_bss_24->mtnId = 1;
}
