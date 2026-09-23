#define _MATH_H
#include "game/object.h"
#include "game/memory.h"
#include "string.h"
extern OMOBJMAN *lbl_1_bss_4;
void *fn_1_178(s32 priority, u32 dataSize, OMOBJ_FUNC callback)
{
    OMOBJ *obj;
    obj = omAddObjEx(lbl_1_bss_4, (s16)priority, 0, 0, 0, callback);
    if (dataSize != 0) {
        obj->data = HuMemDirectMallocNum(HEAP_HEAP, dataSize, 268435456U);
        memset(obj->data, 0, dataSize);
    } else {
        obj->data = NULL;
    }
    return obj->data;
}

void fn_1_218(OMOBJ *obj, OMOBJ_FUNC callback)
{
    obj->objFunc = callback;
}
