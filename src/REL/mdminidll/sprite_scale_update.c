#define _MATH_H
#include "game/sprite.h"
#include "REL/mdminidll/readonly_scalars.h"

extern f32 lbl_1_bss_38[6];
extern s16 lbl_1_bss_752[9];

void fn_1_8764(void)
{
    s16 var_r31;
    f32 var_f31;
    f32 var_f30;

    var_r31 = 0;
    while (var_r31 < 6) {
        var_f30 = lbl_1_bss_38[var_r31];
        if (var_f30 == lbl_1_rodata_6C) {
            var_f31 = lbl_1_rodata_6C;
        } else {
            var_f31 = (lbl_1_rodata_6C + (var_f30 * lbl_1_rodata_1BC)) / lbl_1_rodata_B4;
        }
        lbl_1_bss_38[var_r31] = var_f31;
        HuSprScaleSet(lbl_1_bss_752[2], var_r31, lbl_1_bss_38[var_r31], lbl_1_bss_38[var_r31]);
        var_r31 += 1;
    }
}
