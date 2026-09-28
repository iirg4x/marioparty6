#define _MATH_H
#include "game/object.h"
#include "game/wipe.h"

void fn_1_15B3C(OMOBJ *obj);

void fn_1_15CD8(OMOBJ *obj)
{
    if (omSysExitReq != 0) {
        WipeCreate(2, 0, 60);
        obj->objFunc = fn_1_15B3C;
    }
}
