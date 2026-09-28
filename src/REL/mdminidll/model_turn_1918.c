#define _MATH_H
#include "game/hu3d.h"
#include "REL/mdminidll/readonly_scalars.h"

/* Consumed views of existing target readonly scalars. These declarations
 * allocate no storage; the original declarations and names are unknown. */
extern const f32 lbl_1_rodata_98;
extern const f32 lbl_1_rodata_9C;

void fn_1_1918(s16 arg0, f32 farg0)
{
    Point3d sp18;
    Point3d spC;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;

    Hu3DModelRotGet(arg0, &sp18);
    spC.y = farg0;
    if ((sp18.y - spC.y) > lbl_1_rodata_90) {
        sp18.y -= lbl_1_rodata_94;
    } else if ((sp18.y - spC.y) < lbl_1_rodata_98) {
        sp18.y += lbl_1_rodata_94;
    }
    spC.x = sp18.x;
    var_f31 = spC.y;
    var_f29 = sp18.y;
    if (var_f29 == var_f31) {
        var_f30 = var_f31;
    } else {
        var_f30 = (var_f31 + (var_f29 * lbl_1_rodata_9C)) / lbl_1_rodata_A0;
    }
    spC.y = var_f30;
    spC.z = sp18.z;
    Hu3DModelRotSetV(arg0, &spC);
}
