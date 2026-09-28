#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"

extern HUPROCESS *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;

void fn_1_157B4(void)
{
    OMOBJ *third;
    OMOBJ *second;
    s16 j;
    OMOBJ *first;
    s16 i;

    first = lbl_1_bss_8;
    if (first) {
        first->objFunc = NULL;
        i = 0;
        while (i < 3) {
            Hu3DMotionKill(first->mtnId[i]);
            Hu3DModelKill(first->mdlId[i]);
            i += 1;
        }
        omDelObjEx(lbl_1_bss_4, first);
    }
    /* These local clears are present in the retail cleanup sequence. */
    first = NULL;
    second = lbl_1_bss_C;
    if (second) {
        j = 0;
        while (j < 2) {
            Hu3DAnimKill(second->mtnId[j * 2]);
            Hu3DAnimKill(second->mtnId[j * 2 + 1]);
            second->mtnId[j * 2] = -1;
            second->mtnId[j * 2 + 1] = -1;
            j += 1;
        }
        j = 1;
        while (j >= 0) {
            Hu3DModelKill(second->mdlId[j]);
            j -= 1;
        }
        omDelObjEx(lbl_1_bss_4, second);
    }
    second = NULL;
    third = lbl_1_bss_10;
    if (third) {
        Hu3DModelHookReset(third->mdlId[1]);
        Hu3DMotionKill(third->mtnId[0]);
        Hu3DMotionKill(third->mtnId[1]);
        Hu3DMotionKill(third->mtnId[2]);
        Hu3DMotionKill(third->mtnId[3]);
        Hu3DMotionKill(third->mtnId[4]);
        Hu3DModelKill(third->mdlId[0]);
        Hu3DModelKill(third->mdlId[1]);
        omDelObjEx(lbl_1_bss_4, third);
    }
    /* Retail performs this store unconditionally after deletion. */
    third->objFunc = NULL;
}
