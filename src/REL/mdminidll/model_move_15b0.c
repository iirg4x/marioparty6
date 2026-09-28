#define _MATH_H
#include "game/hu3d.h"
#include "REL/mdminidll/readonly_scalars.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"

/* Consumed views of existing target readonly scalars; the original names and
 * declarations are unknown. These declarations allocate no storage. */
extern const f64 lbl_1_rodata_78;
extern const f64 lbl_1_rodata_88;
extern const f32 lbl_1_rodata_98;
extern const f32 lbl_1_rodata_9C;

void fn_1_15B0(s16 arg0, Point3d *arg1, Point3d *arg2, f32 farg0, f32 farg1)
{
    Point3d sp2C;
    Point3d sp20;
    Point3d sp14;
    Point3d sp8;
    f32 var_f29;
    f32 var_f28;
    f32 var_f27;
    f32 var_f26;
    f32 var_f25;
    f32 var_f24;
    f32 var_f23;
    f32 var_f22;
    f32 var_f21;
    f32 var_f20;
    f32 var_f19;
    f32 var_f18;

    Hu3DModelPosGet(arg0, &sp2C);
    Hu3DModelRotGet(arg0, &sp20);
    var_f22 = arg2->x;
    var_f28 = arg1->x;
    if (farg0 <= lbl_1_rodata_74) {
        var_f29 = var_f28;
    } else if (farg0 >= farg1) {
        var_f29 = var_f22;
    } else {
        var_f29 = var_f28 + ((farg0 / farg1) * (var_f22 - var_f28));
    }
    sp14.x = var_f29;
    var_f21 = arg2->y;
    var_f26 = arg1->y;
    if (farg0 <= lbl_1_rodata_74) {
        var_f27 = var_f26;
    } else if (farg0 >= farg1) {
        var_f27 = var_f21;
    } else {
        var_f27 = var_f26 + ((farg0 / farg1) * (var_f21 - var_f26));
    }
    sp14.y = var_f27;
    var_f20 = arg2->z;
    var_f24 = arg1->z;
    if (farg0 <= lbl_1_rodata_74) {
        var_f25 = var_f24;
    } else if (farg0 >= farg1) {
        var_f25 = var_f20;
    } else {
        var_f25 = var_f24 + ((farg0 / farg1) * (var_f20 - var_f24));
    }
    sp14.z = var_f25;
    sp2C.x -= sp14.x;
    sp2C.z -= sp14.z;
    sp8.y = (f32) -(lbl_1_rodata_88 *
                    (atan2((f64) sp2C.x, -sp2C.z) / lbl_1_rodata_78));
    if ((sp20.y - sp8.y) > lbl_1_rodata_90) {
        sp20.y -= lbl_1_rodata_94;
    } else if ((sp20.y - sp8.y) < lbl_1_rodata_98) {
        sp20.y += lbl_1_rodata_94;
    }
    sp8.x = sp20.x;
    var_f23 = sp8.y;
    var_f18 = sp20.y;
    if (var_f18 == var_f23) {
        var_f19 = var_f23;
    } else {
        var_f19 = (var_f23 + (var_f18 * lbl_1_rodata_9C)) / lbl_1_rodata_A0;
    }
    sp8.y = var_f19;
    sp8.z = sp20.z;
    Hu3DModelPosSetV(arg0, &sp14);
    Hu3DModelRotSetV(arg0, &sp8);
}
