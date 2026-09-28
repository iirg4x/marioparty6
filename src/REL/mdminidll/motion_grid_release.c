#define _MATH_H
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/memory.h"

extern HUPROCESS *lbl_1_bss_4;
extern void *lbl_1_bss_888;

void fn_1_6D14(OMOBJ *arg0)
{
    s16 i;

    if (arg0) {
        i = 0;
        while (i < 11) {
            Hu3DMotionKill(arg0->mtnId[i * 2]);
            Hu3DMotionKill(arg0->mtnId[(i * 2) + 1]);
            Hu3DModelKill(arg0->mdlId[i]);
            i += 1;
        }
        omDelObjEx(lbl_1_bss_4, arg0);
    }
    arg0 = NULL;
}
