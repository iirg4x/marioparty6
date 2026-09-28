#define _MATH_H
#include "game/sprite.h"

extern s16 lbl_1_bss_752[9];
extern s16 lbl_1_data_86E[6];

void fn_1_8888(void)
{
    s16 i;
    i = 0;
    while (i < 6) {
        HuSprAttrSet(lbl_1_bss_752[2], i, 4);
        lbl_1_data_86E[i] = 0;
        /* Retail repeats this store; its original reason is unknown. */
        lbl_1_data_86E[i] = 0;
        i += 1;
    }
}
