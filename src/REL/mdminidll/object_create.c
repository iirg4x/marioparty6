/* This unit uses no math inlines; avoid emitting their weak constant pools. */
#define _MATH_H
#include "game/object.h"

extern HUPROCESS *lbl_1_bss_878;
extern OMOBJ *lbl_1_bss_87C;
void fn_1_1FBE8(OMOBJ *obj);

void fn_1_1FE7C(void)
{
    lbl_1_bss_87C = omAddObjEx(lbl_1_bss_878, 4096, 1, 0, -1, fn_1_1FBE8);
}
