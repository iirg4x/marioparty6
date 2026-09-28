#define _MATH_H
#include "game/hu3d.h"
#include "game/object.h"
extern OMOBJMAN *lbl_1_bss_4;

void fn_1_92C0(OMOBJ *obj)
{
    s16 modelIndex;
    if (obj) {
        Hu3DMotionKill(*obj->mtnId);
        modelIndex = 3;
        while (modelIndex >= 0) {
            Hu3DModelKill(obj->mdlId[modelIndex]);
            modelIndex -= 1;
        }
        omDelObjEx(lbl_1_bss_4, obj);
    }
    /* Models the observed terminal register clear; original source intent is unknown. */
    obj = NULL;
}
