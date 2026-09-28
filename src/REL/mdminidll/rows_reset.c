#define _MATH_H
#include "game/object.h"
#include "REL/mdminidll/row_state.h"

/* Existing target readonly f32 scalar; original declaration/name unknown. */
extern const f32 lbl_1_rodata_74;
extern void fn_1_81FC(OMOBJ *arg0);

void fn_1_8430(OMOBJ *arg0)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_80[var_r31].x = lbl_1_rodata_74;
        lbl_1_bss_80[var_r31].y = lbl_1_rodata_74;
        lbl_1_bss_80[var_r31].z = lbl_1_rodata_74;
        var_r31 += 1;
    }
    arg0->objFunc = fn_1_81FC;
}
