#define _MATH_H
#include "game/object.h"
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_24;

void fn_1_1240C(OMOBJ *obj)
{
    OMOBJ *first;
    OMOBJ *second;
    if (obj->work[0] == 0) {
        first = lbl_1_bss_C;
        first->work[0] = 2;
        first->work[1] = 0;
        second = lbl_1_bss_C;
        second->work[2] = 2;
        second->work[3] = 0;
    }
    if (obj->work[0]++ > 30U) {
        obj->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}
