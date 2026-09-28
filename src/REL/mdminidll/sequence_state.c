#define _MATH_H
#include "game/object.h"

extern OMOBJ *lbl_1_bss_24;

/* This controller also uses its allocated motion-ID slot as a state value. */
void fn_1_B2E0(s16 state)
{
    *lbl_1_bss_24->mtnId = state;
}

s16 fn_1_B2F8(void)
{
    return *lbl_1_bss_24->mtnId;
}
